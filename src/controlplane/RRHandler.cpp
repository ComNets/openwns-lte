/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
 * email: info@openwns.org
 * www: http://www.openwns.org
 * _____________________________________________________________________________
 *
 * openWNS is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 2 as published by the
 * Free Software Foundation;
 *
 * openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <LTE/controlplane/RRHandler.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>
#include <LTE/timing/ResourceSchedulerUT.hpp>
#include <LTE/timing/ResourceSchedulerBS.hpp>
#include <LTE/macg/MACg.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/helper/QoSClasses.hpp>

#include <DLL/StationManager.hpp>
#include <DLL/Layer2.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/ldk/helper/FakePDU.hpp>
#include <WNS/service/dll/StationTypes.hpp>

#include <vector>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(RRHandlerBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.RRHandler.BS",
                                     wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(RRHandlerUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.RRHandler.UT",
                                     wns::ldk::FUNConfigCreator);

RRHandler::RRHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::CommandTypeSpecifier<RRCommand>(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<lte::helper::SwitchConnector>(),
    wns::ldk::HasDeliverer<>(),
    helper::HasModeName(config),
    connector(dynamic_cast<lte::helper::SwitchConnector*>(getConnector())),
    commandSize(config.get<Bit>("commandSize")),
    pcchSize(config.get<Bit>("pcchSize")),
    logger(config.get("logger")),
    layer2(NULL),
    pyco(config),
    usesShortcut(config.get<bool>("usesShortcut"))
{
    MESSAGE_SINGLE(VERBOSE, logger, "RRHandler::RRHandler()");
    rlcReader = NULL;
}

RRHandler::~RRHandler()
{
    MESSAGE_SINGLE(NORMAL, logger, "RRHandler::~RRHandler()");
    // cleanup: see derived classes
}

void
RRHandler::onFUNCreated()
{
    wns::ldk::fun::FUN* fun = getFUN();

    layer2 = fun->getLayer<dll::Layer2*>();
    associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);

    // get handles to outgoing FUs (for sending the ResourceRequests)
    typedef std::list<wns::ldk::IConnectorReceptacle*> FUList;
    FUList connectorSet = connector->getFUs();
    for (FUList::const_iterator iter = connectorSet.begin();
         iter != connectorSet.end();
         ++iter)
    {
        size_t found = (*iter)->getFU()->getName().find("rachDispatcher");
        if (found != std::string::npos)
            friends.rachDispatcher = (*iter);
        found = (*iter)->getFU()->getName().find(pyco.get<std::string>("cpDispatcherName"));
        if (found != std::string::npos)
            friends.cpDispatcher = (*iter);
    }
    assure(friends.rachDispatcher, "RRHandler requires a Dispatcher/OpcodeSetter friend with name: "+mode+separator+"rachDispatcher");
    assure(friends.cpDispatcher, "RRHandler requires a Dispatcher/OpcodeSetter friend with name: "+mode+separator+pyco.get<std::string>("cpDispatcherName"));

    std::string flowmanagername="FlowManager";

    if (layer2->getStationType() == wns::service::dll::StationTypes::UE()) {
        // For UTs, initially activate the RACH as outgoing Data Connection
        if (usesShortcut)
        {
            connector->activate(friends.cpDispatcher);
        }
        else
        {
            connector->activate(friends.rachDispatcher);
        }
        flowmanagername += "UT";
    } else { // BS
        flowmanagername += "BS";
    }
    // in the BS we don't activate the connector since we don't send anything

    friends.flowManager = layer2->getControlService<lte::controlplane::flowmanagement::FlowManager>(flowmanagername);
    assure(friends.flowManager, "FlowManager not set.");
    rlcReader  = fun->getCommandReader("rlc");
    // The macg command needs to be manipulated
    if (mode == modeBase) {
        friends.macg = fun->findFriend<lte::macg::MACg*>("macg");
    } else {
        // taskID is inherited from HasModeName class
        friends.macg = fun->findFriend<lte::macg::MACg*>("macg"+separator+taskID);
    }
}

void
RRHandler::setColleagues(wns::scheduler::RegistryProxyInterface* registry)
{
    // only required in BS: RegistryProxy of RS-RX (uplink master). Never for RS-TX.
    assure((friends.registry == NULL) || (registry == friends.registry),"failed attempt to overload friends.registry");
    friends.registry = registry;
    assure(friends.registry != NULL,"friends.registry==NULL");
}

void
RRHandler::doSendData(const wns::ldk::CompoundPtr&)
{
    assure(false, "Should never be called!");
}

bool
RRHandler::doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const
{
    assure(false,"nobody may downconnect on the MapHandler");
    // will never be called since nobody is likely to downconnect on the MapHandler
    return false;
} // doIsAccepting

RRHandlerUT::RRHandlerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    RRHandler(fun, config),
    wns::Cloneable<RRHandlerUT>(),
    resourceGrantState(false),
    rstx(NULL),
    allRelayedRequestedResources(),
    pcchFlowID(0),
    updatesOnly_(config.get<bool>("updatesOnly"))
{
}

RRHandlerUT::~RRHandlerUT()
{
    // cleanup:
    allRelayedRequestedResources.clear();
}

void
RRHandlerUT::doOnData(const wns::ldk::CompoundPtr&
#ifndef NDEBUG
                      compound
#endif
    )
{
    assure(false, "RRHandlerUT does not expect incoming compounds. This compound's journey:\n"<<compound->dumpJourney());
}


void
RRHandlerUT::onFUNCreated()
{
    RRHandler::onFUNCreated();

    // The RRHandler needs to listen to the Notification about granted resources
    this->startObserving(getFUN()->findFriend<wns::Subject<ResourceGrantNotificationInterface>*>(mode+separator+"mapHandler"));

    // a tx scheduler friend is needed to request queue occupancy levels
    rstx = getFUN()->findFriend<lte::timing::ResourceSchedulerUT*>(mode+separator+rsNameSuffix+"TX");
}

// notified by MapHandler if we got resources granted (indicated in the map)
void
RRHandlerUT::resourcesGranted(bool state)
{
    resourceGrantState = state;
    if (state == true)
    {
        MESSAGE_SINGLE(NORMAL, logger, "resourcesGranted(true): choosing cpDispatcher connector");
        connector->activate(friends.cpDispatcher);
        // activate another signaling of requested resources
        this->createRRCompound();
    }
    else
    {
        if (usesShortcut)
        {
            MESSAGE_SINGLE(NORMAL, logger, "resourcesGranted(false), useShortcut(true): choosing cpDispatcher connector");
            connector->activate(friends.cpDispatcher);
        }
        else
        {
            MESSAGE_SINGLE(NORMAL, logger, "resourcesGranted(false): choosing rachDispatcher connector");
            connector->activate(friends.rachDispatcher);
        }
        // if we are a relay then...
        // assure(false,"resourcesGranted(false) must not happen anymore");
        // take care that at least one RRCompound is still in the Queue
        // and that it will be delivered soon in the future
        /**
         * @todo dbn: With new ShortcutRACH there is no longer the createRRCompound
         * trigger from the RACH unit if no resources are available. We need to
         * trigger it here. Unfortunately, this breaks the RelayNode. Fix it!
         */
        this->createRRCompound();
    }

    // Reset last state since it will change after we got resources
    // Do this after createRRCompound() was calles because we
    // cannot use the resources until the next frame
    if(state == true && updatesOnly_)
    {
        MESSAGE_SINGLE(NORMAL, logger, "resourcesGranted(true), clearing previous state.");
        lastState_.clear();
    }
        
}

void
RRHandlerUT::createRRCompound()
{
    // We can not send a Request if we don't know where to send it.
    if (! associationService->hasAssociation())
        return;

    // reserve at least resources for the next UL resource request:
    wns::scheduler::QueueStatusContainer totalQueueStatus;
    totalQueueStatus = rstx->getQueueStatus();

    // We only report changes but nothing has changed since last call
    if(updatesOnly_ && !isUpdated(totalQueueStatus))
    {
        MESSAGE_SINGLE(NORMAL, logger, "createRRCompound(), state has not changed. No request created.");
        return;
    }
    lastState_ = totalQueueStatus;

    // Nothing to do if nothing queued
    Bit sumBit = 0;
    int sumCompounds = 0;

    for(wns::scheduler::QueueStatusContainer::const_iterator iter = 
            totalQueueStatus.begin();
        iter != totalQueueStatus.end(); 
        ++iter)
    {
        sumBit = sumBit + iter->second.numOfBits;
        sumCompounds = sumCompounds + iter->second.numOfCompounds;
    }
    if(sumBit == 0 && sumCompounds == 0)
    {
        MESSAGE_SINGLE(NORMAL, logger, "createRRCompound(), no queued compounds. No request created.");
        return;
    }

    /** method to generate an UL RR compound in the UT (or RN behaving as UT)*/

    /** For a start, create one RR compound regardless of what we really need */

    /** generate an empty PDU */
    wns::ldk::CompoundPtr requestCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool(),
                                                     wns::ldk::helper::FakePDUPtr(new wns::ldk::helper::FakePDU())));

    /** activate and set MACg command */
    lte::macg::MACgCommand* macgCommand = friends.macg->activateCommand(requestCompound->getCommandPool());
    macgCommand->peer.source = layer2->getDLLAddress();
    wns::service::dll::UnicastAddress peerAddress =  associationService->getAssociation();
    macgCommand->peer.dest = peerAddress;

    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(requestCompound->getCommandPool()));
    rlcCommand->peer.source = layer2->getDLLAddress();
    //rlcCommand->peer.destination = ?
    if (layer2->getStationType() == wns::service::dll::StationTypes::UE())
    {
        lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs =
            friends.flowManager->getControlPlaneFlowIDs(peerAddress);
        pcchFlowID = controlPlaneFlowIDs[lte::helper::QoSClasses::PCCH()];
    }
    rlcCommand->peer.flowID = pcchFlowID;
    assure(pcchFlowID>0, "PCCH FlowID must be > 0");

    MESSAGE_SINGLE(NORMAL, logger, "createRRCompound() to "<<A2N(peerAddress)<<", PCCH FlowID="<<pcchFlowID);

    // Node pointer of the peer node is needed
    wns::scheduler::UserID peer = wns::scheduler::UserID(layer2->getStationManager()->getStationByMAC(peerAddress)->getNode());

    /** activate my command and set the number of compounds waiting and their total size */
    RRCommand* outgoingCommand = this->activateCommand(requestCompound->getCommandPool());

    outgoingCommand->peer.request.user = wns::scheduler::UserID(layer2->getNode());
    outgoingCommand->magic.source = macgCommand->peer.source;

    // reserve UL resources for the PCCH which is generated at the end of this method, but only if we do not use the magic shortcut below:
    if (!usesShortcut)
    {
        wns::scheduler::QueueStatus pcchRequestedResources;
        pcchRequestedResources.numOfCompounds = 1;
        pcchRequestedResources.numOfBits = pcchSize;
        if (totalQueueStatus.knows(pcchFlowID))
        {
            totalQueueStatus.update(pcchFlowID, pcchRequestedResources);
        }
        else
        {
            totalQueueStatus.insert(pcchFlowID, pcchRequestedResources);
        }

        assure(totalQueueStatus.size()>0,"totalQueueStatus must contain at least one element for following resource requests");
    }
    else
    { // UT
        for(wns::scheduler::QueueStatusContainer::const_iterator iter = totalQueueStatus.begin();
            iter != totalQueueStatus.end(); ++iter)
        {
            wns::service::dll::FlowID flowId = iter->first;

            MESSAGE_SINGLE(NORMAL, logger, "requestedResources[FlowID="<<flowId<<"]="<<iter->second.numOfCompounds<<"pdus, "<<iter->second.numOfBits<<"bits");
        }
    }

    outgoingCommand->peer.request.allRequestedResources = totalQueueStatus;
    outgoingCommand->magic.source = layer2->getDLLAddress();

    /** and send the compound */
    if (getConnector()->hasAcceptor(requestCompound))
    {
        MESSAGE_BEGIN(NORMAL, logger, m, "createRRCompound(): prepared ResourceRequest (");
        int bitsWaiting=0, compoundsWaiting=0, numFlows=0;
        for(wns::scheduler::QueueStatusContainer::const_iterator iter = totalQueueStatus.begin();
            iter != totalQueueStatus.end(); ++iter)
        {
            bitsWaiting += iter->second.numOfBits;
            compoundsWaiting += iter->second.numOfCompounds;
            ++numFlows;
        }
        m << bitsWaiting << " bits, ";
        m << compoundsWaiting << " compounds, ";
        m << numFlows << " flows)";
        assure(compoundsWaiting>0 || usesShortcut,"compounds==0 in forwarded resource request");
        MESSAGE_END();
        getConnector()->getAcceptor(requestCompound)->sendData(requestCompound);
    }
    else
    {
        MESSAGE_SINGLE(NORMAL, logger, "createRRCompound(): Failed to send ResourceRequest Compound to "<< A2N(outgoingCommand->magic.source));
        assure(false, "Lower FU is not accepting scheduled ResourceRequest but is supposed to do so");
    }
}

bool
RRHandlerUT::isUpdated(const wns::scheduler::QueueStatusContainer& currentState)
{
    std::list<wns::scheduler::ConnectionID> cids = currentState.keys();
    
    std::list<wns::scheduler::ConnectionID>::iterator it;

    for(it = cids.begin(); it != cids.end(); it++)
    {
        /* State has changed if new CIDs apear or number of */
        /* bits / compounds for CID changed */
        if(!lastState_.knows(*it) || lastState_.find(*it) != currentState.find(*it))
            return true;
    }
    return false;
}

int
RRHandler::getTotalNumberOfUsers(const wns::scheduler::UserID /*user*/) const
{
    return 1;
}

// this method is called for each createFakePDU.
void
RRHandler::calculateSizes(const wns::ldk::CommandPool* commandPool, Bit& commandPoolSize, Bit& dataSize) const
{
    getFUN()->calculateSizes(commandPool, commandPoolSize, dataSize, this);
    // now we have the size of all previous Commands and of the original Compound

    // add my commandSize. No Data to add.
    commandPoolSize += this->commandSize;
}
/***************************************************************************/
RRHandlerBS::RRHandlerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    RRHandler(fun, config),
    wns::Cloneable<RRHandlerBS>(),
    firstWakeup(true),
    rrStorage(new RequestStorage(logger)),
    rnUplinkRRHandler(NULL),
    rsrx(NULL),
    ulSegmentSize(config.get<int>("ulSegmentSize"))
{
}

RRHandlerBS::~RRHandlerBS()
{
    delete rrStorage; rrStorage=NULL;
}

void
RRHandlerBS::onFUNCreated()
{
    // Do the general stuff
    RRHandler::onFUNCreated();

    // The rx scheduler is needed to queue the resource requests
    rsrx = getFUN()->findFriend<lte::timing::ResourceSchedulerBS*>(mode+separator+rsNameSuffix+"RX");

    // Register as Observer at the association Info providers
    dll::Layer2* layer2 = getFUN()->getLayer<dll::Layer2*>();
    dll::Layer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(mode);
    dll::Layer2::AssociationInfoContainer::const_iterator iter = ais.begin();
    //dll::ILayer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(mode);
    //dll::ILayer2::AssociationInfoContainer::const_iterator iter = ais.begin();
    for (; iter != ais.end(); ++iter)
        this->dll::services::control::AssociationObserver::startObserving(*iter);

    lte::timing::TimingScheduler* timing = layer2->getManagementService<lte::timing::TimingScheduler>(mode+separator+"Timer");
    this->wns::Observer<SuperFrameStartNotificationInterface>::startObserving(timing);
}

int
RRHandlerBS::getDefaultBitsPerPDU()
{
    return ulSegmentSize;
}

wns::ldk::CompoundPtr
RRHandlerBS::createFakePDU(wns::service::dll::UnicastAddress destinationAddress, wns::scheduler::Bits bits, wns::service::dll::FlowID flowID)
{
    MESSAGE_SINGLE(NORMAL, logger, "createFakePDU(flowID="<<flowID<<","<<bits<<"bits)");
    wns::ldk::CompoundPtr pdu = wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool(),wns::ldk::helper::FakePDUPtr(new wns::ldk::helper::FakePDU( bits ))));
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(pdu->getCommandPool()));
    rlcCommand->peer.flowID = flowID;
    lte::macg::MACgCommand* macgCommand = friends.macg->activateCommand(pdu->getCommandPool());
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest   = destinationAddress;
    return pdu;
}

// only called once at simulation start
void
RRHandlerBS::fakeRelayRRs() const
{
    typedef std::vector<wns::service::dll::UnicastAddress> IDList;
    // all associated users (RN, UT):
    IDList allUsers = associationService->getAssociated();
    MESSAGE_SINGLE(NORMAL, logger, "fakeRelayRRs(): "<<allUsers.size()<<" associated users");

    // fake requests for every user and hand them down to the lower layer
    for (IDList::iterator iter = allUsers.begin(); iter != allUsers.end(); ++iter)
    {
        ResourceRequest request;
        wns::service::dll::UnicastAddress peerAddress = *iter;
        wns::node::Interface* peerUserID = layer2->getStationManager()->getStationByMAC(peerAddress)->getNode();
        request.user = wns::scheduler::UserID(layer2->getStationManager()->getStationByMAC(peerAddress)->getNode());
        lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs =
            friends.flowManager->getControlPlaneFlowIDs(peerAddress);
        wns::service::dll::FlowID pcchFlowID = controlPlaneFlowIDs[lte::helper::QoSClasses::PCCH()];

        wns::scheduler::QueueStatus initialPcchResourceRequest;
        initialPcchResourceRequest.numOfBits = pcchSize;
        initialPcchResourceRequest.numOfCompounds = 1;
        wns::scheduler::QueueStatusContainer rnResourceRequests;
        rnResourceRequests.insert(pcchFlowID, initialPcchResourceRequest);
        rrStorage->storeRequest(wns::scheduler::UserID(peerUserID),rnResourceRequests);
        assure(friends.registry!=NULL,"registry==NULL");
        friends.registry->registerCID(pcchFlowID,wns::scheduler::UserID(peerUserID));
    } // for all users
    MESSAGE_SINGLE(NORMAL, logger, "ResourceRequests="<<rrStorage->printQueueStatus());
} // fakeRelayRRs()

void
RRHandler::doWakeup()
{} // doWakeup

void
RRHandlerBS::doWakeup()
{
    if (firstWakeup == true)
    {
        MESSAGE_SINGLE(NORMAL, logger, "doWakeup(): firstWakeup");
        fakeRelayRRs();
        firstWakeup = false;
    }
} // doWakeup

void
RRHandlerBS::onSuperFrameStart()
{
    if (firstWakeup == true)
    {
        MESSAGE_SINGLE(NORMAL, logger, "onSuperFrameStart(): firstWakeup");
        fakeRelayRRs();
        firstWakeup = false;
    }
}

// incoming ResourceRequests (from UT/RN to BS task)
void
RRHandlerBS::doOnData(const wns::ldk::CompoundPtr& compound)
{
    /** the RAP Received incoming Resource Requests. We have to put appropriately
     * Numbered and sized Dummy PDUs into the RX scheduler's queue at the RAP
     */
    RRCommand* incomingCommand = getCommand(compound->getCommandPool());
    wns::service::dll::UnicastAddress rapAddress  = layer2->getDLLAddress(); // invariant -> member
    wns::service::dll::UnicastAddress userAddress = incomingCommand->magic.source; // the requester

    // If the originator of the request is not associated with us, we ignore the request
    if (!associationService->hasAssociated(userAddress))
        return;

    wns::scheduler::UserID peer = incomingCommand->peer.request.user;

    wns::scheduler::QueueStatusContainer requestedResources = incomingCommand->peer.request.allRequestedResources;
    lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs =
        friends.flowManager->getControlPlaneFlowIDs(userAddress);
    wns::service::dll::FlowID pcchFlowID = controlPlaneFlowIDs[lte::helper::QoSClasses::PCCH()];
    // alternatively we could read the rlc command here: rlcCommand->peer.flowID

    MESSAGE_BEGIN(NORMAL, logger, m, "doOnData(): Received Resource Request from: ");
    m << peer.getName();
    m << " ("<<A2N(userAddress)<<")";
    m << " for "<<requestedResources.size()<<" flows";
    m << " (PCCH FlowID="<<pcchFlowID<<")";
    MESSAGE_END();

    uint32_t assumedBitsPerPDU = ulSegmentSize;
    if (requestedResources.size() > 0)
    {
        rrStorage->storeRequest(peer,requestedResources);

        wns::node::Interface* peerUserID = layer2->getStationManager()->getStationByMAC(userAddress)->getNode();
        for( wns::scheduler::QueueStatusContainer::const_iterator iter = requestedResources.begin(); iter != requestedResources.end(); ++iter)
        {
            wns::scheduler::ConnectionID flowId = iter->first;
            assure(friends.registry!=NULL,"registry==NULL");
            friends.registry->registerCID(flowId, wns::scheduler::UserID(peerUserID));
            // manipulations in the numbers to adjust for inaccuracies:
            int pdus = iter->second.numOfCompounds;
            int bits = iter->second.numOfBits;
            if (bits<pdus*assumedBitsPerPDU) {
                // adjust bits to be at least a multiple of assumedBitsPerPDU
                MESSAGE_SINGLE(NORMAL, logger, "doOnData(): adjusting flowId="<<flowId<<": "<<iter->second.numOfCompounds<<" pdus, "<<iter->second.numOfBits<<" bits -> "<<pdus*assumedBitsPerPDU<<" bits");
                //iter->second.numOfBits = pdus*assumedBitsPerPDU; // add bits
                wns::scheduler::QueueStatus modifiedQueueStatus = iter->second;
                modifiedQueueStatus.numOfBits = pdus*assumedBitsPerPDU; // add bits
                requestedResources.update(flowId,modifiedQueueStatus);
            }
        }
        MESSAGE_SINGLE(NORMAL, logger, "ResourceRequests("<<peer.getName()<<")="<<rrStorage->printQueueStatus());
    }
}

ResourceShares
RRHandlerBS::getResourceShares(const wns::scheduler::UserSet& users) const
{
    return rrStorage->getResourceShares(users);
}

wns::scheduler::UserSet
RRHandlerBS::getActiveUsers() const
{
    wns::scheduler::UserSet result = rrStorage->getActiveUsers();

    MESSAGE_BEGIN(VERBOSE, logger, m, "Active Users are: ");
    for (wns::scheduler::UserSet::const_iterator iter = result.begin(); iter != result.end(); ++iter)
        m << iter->getName() << ", ";
    MESSAGE_END();

    return result;
}

RequestStorageInterface*
RRHandlerBS::getRRStorage()
{
    return rrStorage;
}

void
RRHandlerBS::reset(const wns::scheduler::UserSet& users)
{
    rrStorage->reset(users);
}

void
RRHandlerBS::onAssociated(wns::service::dll::UnicastAddress userAdr,
                          wns::service::dll::UnicastAddress dstAdr)
{
}

void
RRHandlerBS::onDisassociated(wns::service::dll::UnicastAddress userAdr,
                             wns::service::dll::UnicastAddress dstAdr)
{
    // If we received forwarded association Info, there's nothing to do
    if (layer2->getDLLAddress() != dstAdr)
        return;

    wns::scheduler::UserID peer = wns::scheduler::UserID(layer2->getStationManager()->getStationByMAC(userAdr)->getNode());

    // Else remove the disassociating user from the rrStorage
    MESSAGE_BEGIN(NORMAL, logger, m, "onDisassociated("<<A2N(userAdr)<<","<<A2N(dstAdr)<<"): removing user ");
    m << A2N(userAdr) << " from Resource Request rrStorage (peer="<<peer.getName()<<")";
    MESSAGE_END();

    rrStorage->resetUser(peer);
}

/***************************************************************************/

RequestStorage::RequestStorage(wns::logger::Logger& _logger)
    : logger(_logger)
{
    MESSAGE_SINGLE(VERBOSE, logger, "RequestStorage::RequestStorage()");
}

RequestStorage::~RequestStorage()
{
}

void
RequestStorage::storeRequest(const wns::scheduler::UserID user, wns::scheduler::QueueStatusContainer& partialQueueStatusContainer)
{
    //resetUser(user);
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = partialQueueStatusContainer.begin(); iter != partialQueueStatusContainer.end(); ++iter)
    {
        wns::scheduler::ConnectionID flowId = iter->first;
        if (queueStatusContainer.knows(flowId)) {
            queueStatusContainer.update(flowId,iter->second);
        } else {
            queueStatusContainer.insert(flowId,iter->second);
        }
        if (!connectionUserMapping.knows(flowId)) {
            connectionUserMapping.insert(flowId,user);
        }
    }
    // insert user that has requested resources into the set of active users
    activeULUsers.insert(user);
}

void
RequestStorage::resetFlow(const wns::scheduler::ConnectionID flowId)
{
    if (queueStatusContainer.knows(flowId)) {
        wns::scheduler::QueueStatus emptyQueueStatus; // =0
        queueStatusContainer.update(flowId,emptyQueueStatus);
    }
}

void
RequestStorage::resetUser(const wns::scheduler::UserID user)
{
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = queueStatusContainer.begin(); iter != queueStatusContainer.end(); ++iter)
    {
        wns::scheduler::ConnectionID     flowId = iter->first;
        wns::scheduler::QueueStatus queueStatus = iter->second;
        if (connectionUserMapping.find(flowId) == user) {
            queueStatus.numOfCompounds = 0;
            queueStatus.numOfBits = 0;
            queueStatusContainer.update(flowId,queueStatus);
        }
    }
    // remove user from set of active users
    activeULUsers.erase(user);
}

void
RequestStorage::deleteUser(const wns::scheduler::UserID user)
{
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = queueStatusContainer.begin(); iter != queueStatusContainer.end(); )
    {
        wns::scheduler::QueueStatusContainer::const_iterator nextIter = iter;
        ++nextIter;
        wns::scheduler::ConnectionID flowId = iter->first;
        if (connectionUserMapping.find(flowId) == user) {
            queueStatusContainer.erase(flowId);
        }
        iter=nextIter;
    }
    // remove user from set of active users
    activeULUsers.erase(user);
}

void
RequestStorage::reset(const wns::scheduler::UserSet& users)
{
    wns::scheduler::QueueStatusContainer tmpQueueStatusContainer = queueStatusContainer;
    queueStatusContainer.clear();
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = tmpQueueStatusContainer.begin(); iter != tmpQueueStatusContainer.end(); ++iter)
    {
        wns::scheduler::ConnectionID flowId = iter->first;
        wns::scheduler::UserID userID = connectionUserMapping.find(flowId);
        if (users.find(userID)==users.end()) // not in list of users to delete
            queueStatusContainer.insert(flowId,iter->second);
    }
    // remove users from set of active users
    for (wns::scheduler::UserSet::const_iterator iter = users.begin();
         iter != users.end();
         ++iter)
    {
        activeULUsers.erase(*iter);
    }
}

// used only in unitTest:
ResourceShares
RequestStorage::getResourceShares(const wns::scheduler::UserSet& users) const
{
    typedef std::map<wns::scheduler::UserID, uint32_t> RequestCounter;
    RequestCounter compoundsPerUser;
    uint32_t totalCompounds = 0;
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = queueStatusContainer.begin(); iter != queueStatusContainer.end(); ++iter)
    {
        wns::scheduler::ConnectionID flowId = iter->first;
        wns::scheduler::UserID userID = connectionUserMapping.find(flowId);
        if (users.find(userID) != users.end())
        {
            wns::scheduler::QueueStatus queueStatus = iter->second;
            int compoundsWaiting = queueStatus.numOfCompounds;
            totalCompounds += compoundsWaiting;
            if ( compoundsPerUser.find(userID) != compoundsPerUser.end() ) // found
            {
                compoundsPerUser[userID] += compoundsWaiting;
            } else { // new entry
                compoundsPerUser[userID] = compoundsWaiting;
            }
        } // else: Ignore requests from all users not in the given userset
    }

    ResourceShares shares;

    for (RequestCounter::const_iterator iter = compoundsPerUser.begin();
         iter != compoundsPerUser.end();
         ++iter)
    {
        wns::scheduler::UserID user = iter->first;
        uint32_t compoundsThisUser = iter->second;
        shares[user] = double(compoundsThisUser) / double(totalCompounds);
    }

    return shares;
}

std::string
RequestStorage::printQueueStatus() const
{
    std::stringstream s;
    s << "QueueStatus(";
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = queueStatusContainer.begin(); iter != queueStatusContainer.end(); ++iter)
    {
        wns::scheduler::ConnectionID flowId = iter->first;
        s << "flowID="<<flowId<<": p="<<iter->second.numOfCompounds<<", b="<<iter->second.numOfBits<<"; ";
    }
    s << ")";
    return s.str();
}

wns::scheduler::UserSet
RequestStorage::getActiveUsers() const
{
    return activeULUsers;
}

wns::scheduler::ConnectionSet
RequestStorage::getActiveConnections() const
{
    wns::scheduler::ConnectionSet result;
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = queueStatusContainer.begin(); iter != queueStatusContainer.end(); ++iter)
    {
        if (iter->second.numOfCompounds > 0)
            result.insert(iter->first);
    }
    return result;
}

wns::scheduler::ConnectionSet
RequestStorage::filterActiveConnections(wns::scheduler::ConnectionSet& inputConnectionSet) const
{
    wns::scheduler::ConnectionSet result;
    for (wns::scheduler::ConnectionSet::const_iterator iter = inputConnectionSet.begin(); iter != inputConnectionSet.end(); ++iter)
    {
        wns::scheduler::ConnectionID flowId = (*iter);
        if (queueStatusContainer.knows(flowId)) {
            const wns::scheduler::QueueStatus& queueStatus = queueStatusContainer.find(flowId);
            if (queueStatus.numOfCompounds > 0)
                result.insert(flowId);
        }
    }
    return result;
}

bool
RequestStorage::knowsFlow(wns::scheduler::ConnectionID flowId) const
{
    return queueStatusContainer.knows(flowId);
}

bool
RequestStorage::isEmpty() const
{
    for( wns::scheduler::QueueStatusContainer::const_iterator iter = queueStatusContainer.begin(); iter != queueStatusContainer.end(); ++iter)
        if (iter->second.numOfCompounds > 0)
            return false;
    return true;
}

uint32_t
RequestStorage::numBitsForCid(wns::scheduler::ConnectionID cid) const
{
    if (queueStatusContainer.knows(cid))
    {
        const wns::scheduler::QueueStatus& queueStatus = queueStatusContainer.find(cid);
        return queueStatus.numOfBits;
    } else {
        return 0;
    }
}

uint32_t
RequestStorage::numCompoundsForCid(wns::scheduler::ConnectionID cid) const
{
    if (queueStatusContainer.knows(cid))
    {
        const wns::scheduler::QueueStatus& queueStatus = queueStatusContainer.find(cid);
        return queueStatus.numOfCompounds;
    } else {
        return 0;
    }
}

wns::scheduler::Bits
RequestStorage::decrementRequest(wns::scheduler::ConnectionID cid, wns::scheduler::Bits bits)
{
    if (queueStatusContainer.knows(cid))
    {
        assure(bits>0,"decrementRequest(cid="<<cid<<",bits="<<bits<<") must request bits>0");
        wns::scheduler::QueueStatus queueStatus = queueStatusContainer.find(cid);
        int queuedPdus = queueStatus.numOfCompounds;
        int queuedBits = queueStatus.numOfBits;
        MESSAGE_SINGLE(NORMAL, logger, "RequestStorage::decrementRequest(cid="<<cid<<",bits="<<bits<<"): queued: "<<queuedPdus<<" pdus, "<<queuedBits<<" bits");
        if ((queuedPdus>0) && (queuedBits>0)) {
            queueStatus.numOfCompounds --;
            if (queuedBits>=bits) {
                queueStatus.numOfBits -= bits;
            } else { // not enough bits in queue
                bits = queuedBits;
                queueStatus.numOfBits = 0;
            }
            // any remaining bits or compounds?
            queuedPdus = queueStatus.numOfCompounds;
            queuedBits = queueStatus.numOfBits;
            if (((queuedPdus>0) && (queuedBits>0))
                || (queuedPdus==0) && (queuedBits==0)) { // ok
            } else if (queuedPdus>0) { // but queuedBits=0
                MESSAGE_SINGLE(NORMAL, logger, "RequestStorage::decrementRequest(cid="<<cid<<"): last "<<bits<<" bits served. Leftover "<<queuedPdus<<" pdus set to zero");
                queueStatus.numOfCompounds = 0;
            } else if (queuedBits>0) { // but queuedPdus=0
                MESSAGE_SINGLE(NORMAL, logger, "RequestStorage::decrementRequest(cid="<<cid<<"): "<<bits<<" bits served but zero="<<queuedPdus<<" pdus. Assuming one pdu.");
                queueStatus.numOfCompounds += 1;
            }
            queueStatusContainer.update(cid,queueStatus);
        } else {
            assure(false,"decrementRequest(cid="<<cid<<",bits="<<bits<<") for empty RR queue");
            return 0;
        }
    } else {
        assure(false,"decrementRequest(cid="<<cid<<",bits="<<bits<<") for unknown cid");
        throw wns::Exception("decrementRequest for unknown cid");
        return 0;
    }
    return bits;
}

wns::scheduler::QueueStatusContainer
RequestStorage::getQueueStatus() const
{
    return queueStatusContainer;
}
