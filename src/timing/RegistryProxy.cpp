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

#include <LTE/timing/RegistryProxy.hpp>

#include <LTE/macg/MACg.hpp>
#include <LTE/macr/PhyUser.hpp>
#include <LTE/timing/TimingScheduler.hpp>
#include <LTE/controlplane/RRHandler.hpp>
#include <LTE/controlplane/associationHandler/AssociationHandler.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>
#include <LTE/macg/MACgCommand.hpp>
#include <LTE/rlc/RLCCommand.hpp>

#include <DLL/StationManager.hpp>
#include <DLL/services/control/Association.hpp>
#include <DLL/services/management/InterferenceCache.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/node/Node.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/service/qos/QoSClasses.hpp>
#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/ldk/sar/SAR.hpp>
#include <WNS/Ttos.hpp>
#include <WNS/scheduler/harq/HARQInterface.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace lte::timing;
using namespace wns::scheduler;

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

STATIC_FACTORY_REGISTER_WITH_CREATOR(RegistryProxy, wns::scheduler::RegistryProxyInterface, "lte.RegistryProxy", wns::ldk::FUNConfigCreator);

RegistryProxy::RegistryProxy(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config)
    : HasModeName(config),
      fun(_fun),
      layer2(NULL),
      logger(config.get("logger")),
      associationService(NULL),
      friends(),
      //Colleagues(),
      phyModeMapper(wns::service::phy::phymode::PhyModeMapperInterface::getPhyModeMapper(config.getView("phyModeMapper"))),
      fakeChannelQualities(), // who sets that SmartPtr content?
      queueSize(config.get<uint32_t>("queueSize")),
      relaysOnly(false), // TODO: make this obsolete
      powerUT(config.get("powerCapabilitiesUT")),
      powerBS(config.get("powerCapabilitiesBS")),
      eirpLimited(false),
      isForDL(true),
      myTask(wns::scheduler::TaskBSorUT::TaskBaseStation()),
      numberOfPriorities(config.get<int>("numberOfPriorities")),
      numberOfQosClasses(config.get<int>("numberOfQosClasses")),
      qosToPriorityMapping(numberOfQosClasses),
      cidToUserId(), // RN|UT
      cidToPrio(),
      userIdToConnections(),
      connectionsForPriority(numberOfPriorities+1) // vector size from Python
{
    MESSAGE_SINGLE(NORMAL, logger, "RegistryProxy() constructor. myTask="<<wns::scheduler::TaskBSorUT::toString(myTask));
    assure(powerBS.nominalPerSubband!=wns::Power(),"undefined power="<<powerBS.nominalPerSubband);
    assure(powerUT.nominalPerSubband!=wns::Power(),"undefined power="<<powerUT.nominalPerSubband);
    wns::pyconfig::View qosClasses = config.getView("qosClassMapping");
    // see qos.py
    assure(numberOfQosClasses == qosClasses.len("mapEntries"),"invalid qos to priority mapping");
    for (int i=0; i<numberOfQosClasses; ++i) {
        wns::pyconfig::View mapEntry = qosClasses.getView("mapEntries",i);
        int number = mapEntry.get<int>("number");
        assure(number == i,"QoS index mismatch: "<<number<<"!="<<i);
        int priority = mapEntry.get<int>("priority");
        // priority is out of [0..MaxPriority-1]:
        assure(priority>=0 && priority<numberOfPriorities,"invalid priority="<<priority<<" (numberOfPriorities="<<numberOfPriorities<<")");
        MESSAGE_SINGLE(NORMAL, logger, "RegistryProxy(): QoS="<<i<<"=\""<< mapEntry.get<std::string>("name")<<"\" => priority="<<priority);
        qosToPriorityMapping[i] = priority;
    }
} // constructor

RegistryProxy::~RegistryProxy()
{
    connectionsForPriority.clear();
    userIdToConnections.clear();
    cidToUserId.clear();
    cidToPrio.clear();
}

void
RegistryProxy::setFriends(const wns::ldk::CommandTypeSpecifierInterface* _classifier)
{
    assure(false,"RegistryProxy::setFriends(): obsolete");
}

void
RegistryProxy::setHARQ(wns::scheduler::harq::HARQInterface* harq)
{
    friends.harq = harq;
}

void
RegistryProxy::setFUN(const wns::ldk::fun::FUN* _fun)
{
    MESSAGE_SINGLE(NORMAL, logger, "RegistryProxy()::setFUN()");
    // fun already known from constructor
    assure (_fun == fun, "invalid fun");
    // rs: why again?
    fun = const_cast<wns::ldk::fun::FUN*>(_fun);
    assure(fun, "RegistryProxy needs a FUN");
    layer2 = fun->getLayer<dll::Layer2*>();
    assure(layer2, "could not get Layer2 from FUN");
    //dll::StationManager* stationManager = layer2->getStationManager()

    associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);
    iCache = layer2->getManagementService<dll::services::management::InterferenceCache>("INTERFERENCECACHE"+modeBase);
    friends.timer = layer2->getManagementService<lte::timing::TimingScheduler>(mode+separator+"Timer");
    friends.rlcCommandReader  = fun->getCommandReader("rlc");
    assure(friends.rlcCommandReader, "Need rlcCommandReader as friend");
    friends.macgCommandReader = fun->getCommandReader("macg");
    assure(friends.macgCommandReader, "Need macgCommandReader as friend");

    // Register as Observer at the association Info providers
    dll::Layer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(mode);
    dll::Layer2::AssociationInfoContainer::const_iterator iter = ais.begin();
    for (; iter != ais.end(); ++iter)
        this->startObserving(*iter);

    // to know in which task the relay node is
    if (layer2->getStationType() == wns::service::dll::StationTypes::eNB())
    {
        myTask = wns::scheduler::TaskBSorUT::TaskBaseStation();
        friends.flowManager = layer2->getControlService<lte::controlplane::flowmanagement::FlowManager>("FlowManagerBS");
    } else {
	myTask = wns::scheduler::TaskBSorUT::TaskUserTerminal();
	friends.flowManager = layer2->getControlService<lte::controlplane::flowmanagement::FlowManager>("FlowManagerUT");
	isForDL = false;
    }

    assure(friends.flowManager, "FlowManager not set!");

    wns::scheduler::UserID bcUserID;
    bcUserID.setBroadcast();
    registerCID(friends.flowManager->getBCHFlowID(), bcUserID);

    if ( layer2->getStationType() == wns::service::dll::StationTypes::eNB() ||
         (layer2->getStationType() == wns::service::dll::StationTypes::FRS() && this->getTaskSuffix()=="BS"))
    {
        friends.rrHandlerUT = NULL;
        friends.rrHandler = fun->findFriend<lte::controlplane::RRHandlerBS*>(mode+separator+"RRHandler");
    }
    else
    {
        friends.rrHandlerUT = fun->findFriend<lte::controlplane::RRHandlerUT*>(mode+separator+"RRHandler");
        friends.rrHandler = NULL;
    }
} // setFUN

bool
RegistryProxy::getCQIAvailable() const
{
    return false;
}

wns::scheduler::UserID
RegistryProxy::getUserForCID(wns::scheduler::ConnectionID cid)
{
    assure(cidToUserId.knows(cid),"unknown cid="<<cid);
    wns::scheduler::UserID userID = cidToUserId.find(cid);
    MESSAGE_SINGLE(VERBOSE, logger, "getUserForCID(cid="<<cid<<") = "<<getNameForUser(userID));
    // search for cid in list; if not there, add it.
    if( find(userIdToConnections[userID].begin(),userIdToConnections[userID].end(), cid ) == userIdToConnections[userID].end())
    { // not found
        userIdToConnections[userID].push_back(cid);
        // TODO: should better be done only in registerCID
        MESSAGE_SINGLE(NORMAL, logger, "getUserForCID(cid="<< cid << "): new cid added into userIdToConnections["<<getNameForUser(userID)<<"] = "<<wns::scheduler::printConnectionVector(userIdToConnections[userID]));
    }
    return userID;
}

wns::service::dll::UnicastAddress
RegistryProxy::getPeerAddressForCID(wns::scheduler::ConnectionID cid)
{
    wns::scheduler::UserID user = getUserForCID(cid);
    wns::service::dll::UnicastAddress peerAddress
        = layer2->getStationManager()->getStationByNode(user.getNode())->getDLLAddress();
    return peerAddress;
}

ConnectionVector
RegistryProxy::getConnectionsForUser(const wns::scheduler::UserID userID/*nextHop!*/)
{
    assure(userIdToConnections.find(userID)!=userIdToConnections.end(),"userIdToConnections["<<getNameForUser(userID)<<"] doesn't exist");
    MESSAGE_SINGLE(NORMAL, logger, "getConnectionsForUser("<<getNameForUser(userID)<<") = "<<wns::scheduler::printConnectionVector(userIdToConnections[userID]));
    return userIdToConnections[userID];
}

// does nothing if already registered
void
RegistryProxy::registerCID(wns::scheduler::ConnectionID cid, wns::scheduler::UserID user /*nextHop!*/)
{
    if(!cidToPrio.knows(cid)) {
        wns::service::dll::FlowID flowID = cid;
        wns::service::qos::QoSClass qosClass;
        if (myTask == wns::scheduler::TaskBSorUT::TaskBaseStation()) {
            qosClass = friends.flowManager->getQoSClassForBSFlowID(flowID);
        } else {
            qosClass = friends.flowManager->getQoSClassForUTFlowID(flowID);
        }
        assure(qosClass>=0 && qosClass<getNumberOfQoSClasses(),"invalid QoS class "<<qosClass);
        int priority = mapQoSClassToPriority(qosClass);
        // priority is out of [0..MaxPriority-1]:
        assure(priority>=0 && priority<numberOfPriorities,"invalid priority="<<priority<<" (numberOfPriorities="<<numberOfPriorities<<")");
        //registerCIDForPriority(cid,priority);
        cidToPrio.insert(cid, priority);
        connectionsForPriority[priority].push_back(cid);
        MESSAGE_SINGLE(NORMAL, logger, "registerCID(cid="<<cid<<",user="<<getNameForUser(user)<<"): QoS="<<lte::helper::QoSClasses::toString(qosClass)<<", prio="<<priority);
    }
    if(!cidToUserId.knows(cid))
    {
        cidToUserId.insert(cid, user);
        MESSAGE_SINGLE(NORMAL, logger, "registerCID(cid="<<cid<<",user="<<getNameForUser(user)<<"): cidToUserId["<<cid<<"]="<<getNameForUser(user));
    }
    if (userIdToConnections.find(user)==userIdToConnections.end()) // not known
    {
        userIdToConnections[user] = wns::scheduler::ConnectionVector(); // empty
    }
    if( find(userIdToConnections[user].begin(),userIdToConnections[user].end(), cid ) == userIdToConnections[user].end())
    { // not found
        userIdToConnections[user].push_back(cid);
        MESSAGE_SINGLE(NORMAL, logger, "registerCID(cid="<<cid<<",user="<<getNameForUser(user)<<"): userIdToConnections["<<getNameForUser(user)<<"] = "<<wns::scheduler::printConnectionVector(userIdToConnections[user]));
    }
    // if (!userIdToConnections[user].knows(cid)) gibt's nicht
}

void
RegistryProxy::deregisterCID(wns::scheduler::ConnectionID cid, wns::scheduler::UserID user /*nextHop!*/)
{
    MESSAGE_SINGLE(NORMAL, logger, "deregisterCID(cid="<<cid<<",user="<<getNameForUser(user)<<")");
    if (cidToPrio.knows(cid))
    {
        int priority = cidToPrio.find(cid);
        cidToPrio.erase(cid);
        for ( wns::scheduler::ConnectionList::iterator iter = connectionsForPriority[priority].begin();
              iter != connectionsForPriority[priority].end(); ++iter)
        {
            if (cid == *iter) {
                connectionsForPriority[priority].remove(cid);
                break;
            }
        }
    }
    if(cidToUserId.knows(cid))
    {
        assure(cidToUserId.find(cid) == user,"wrong user");
        cidToUserId.erase(cid);
    }
    // delete cid in userIdToConnections[user]
    for(wns::scheduler::ConnectionVector::iterator iter = userIdToConnections[user].begin();
        iter != userIdToConnections[user].end(); ++iter )
    { // forall connections/flows of one nextHop user
        wns::scheduler::ConnectionID cidOfUser = *iter;
        if (cid == cidOfUser)
        { // delete cid in list
            userIdToConnections[user].erase(iter); break; // ready
        }
    }
}

/** @brief deregisterUser (important e.g. for Handover) */
void
RegistryProxy::deregisterUser(const wns::scheduler::UserID user/*nextHop!*/)
{
    assure(userIdToConnections.find(user)!=userIdToConnections.end(),"userIdToConnection["<<getNameForUser(user)<<"] unknown");
    MESSAGE_SINGLE(NORMAL, logger, "deregisterUser("<<user.getName()<<")");
    ConnectionVector connectionsForUser = getConnectionsForUser(user);
    for(wns::scheduler::ConnectionVector::iterator iter = connectionsForUser.begin();
        iter != connectionsForUser.end(); ++iter )
    { // forall connections/flows of one nextHop user
        wns::scheduler::ConnectionID cid = *iter;
        deregisterCID(cid, user);
    }
    userIdToConnections.erase(user);
}

bool
RegistryProxy::hasResourcesGranted() const
{
    assure(friends.rrHandlerUT, "No rrHandlerUT in hasGrantedResources(), are you sure you are in a UT?");
    return friends.rrHandlerUT->hasResourcesGranted();
}

void
RegistryProxy::onAssociated(wns::service::dll::UnicastAddress /*userAdr*/, wns::service::dll::UnicastAddress /*dstAdr*/)
{
    //MESSAGE_SINGLE(NORMAL, logger, "onAssociated("<<A2N(userAdr)<<","<<A2N(dstAdr)<<")");
}

void
RegistryProxy::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    // userAdr is always UT; dstAdr is always RAP (UT's next hop)
    if (layer2->getDLLAddress() == userAdr) // I am UT
    {
        MESSAGE_SINGLE(NORMAL, logger, "onDisassociated("<<A2N(userAdr)<<","<<A2N(dstAdr)<<"): deleting my old rap="<<A2N(dstAdr));
        wns::node::Interface* rap = layer2->getStationManager()->getStationByMAC(dstAdr)->getNode();
        deregisterUser(wns::scheduler::UserID(rap)/*nextHop!*/);
    }
    else if (layer2->getDLLAddress() == dstAdr) // user is directly connected to me
    {
        if (myTask == wns::scheduler::TaskBSorUT::TaskBaseStation()) // I am (BS || RN-BS)
        {
            MESSAGE_SINGLE(NORMAL, logger, "onDisassociated("<<A2N(userAdr)<<","<<A2N(dstAdr)<<"): deleting my user="<<A2N(userAdr));
            wns::node::Interface* user = layer2->getStationManager()->getStationByMAC(userAdr)->getNode();
            deregisterUser(wns::scheduler::UserID(user)/*nextHop!*/); // user is end-user here (UT), there is no more RN in between
        }
    }
} // onDisassociated

wns::scheduler::ConnectionID
RegistryProxy::getCIDforPDU(const wns::ldk::CompoundPtr& compound)
{
    // cid is flowID here.
    wns::ldk::CommandPool* commandPool = compound->getCommandPool();
    assure(friends.rlcCommandReader, "Need rlcCommandReader as friend, please set first");
    lte::rlc::RLCCommand* rlcCommand = friends.rlcCommandReader->readCommand<lte::rlc::RLCCommand>(commandPool);
    assure(rlcCommand!=NULL, "couldn't get rlcCommand");
    wns::scheduler::ConnectionID cid = rlcCommand->peer.flowID; // wns::service::dll::FlowID
    //wns::scheduler::UserID userID = rlcCommand->peer.destination; // this is end-userID (UT, never RN)!
    assure(friends.macgCommandReader, "Need macgCommandReader as friend, please set first");
    lte::macg::MACgCommand* macgCommand = friends.macgCommandReader->readCommand<lte::macg::MACgCommand>(commandPool);
    //int destIdInt = macgCommand->peer.dest.getInteger(); // MACg has next hop peer address
    //assure(destIdInt > 0, "destIdInt="<<destIdInt);
    //UserID destUserID = layer2->getStationManager()->getNodeByID(destIdInt);
    wns::service::dll::UnicastAddress nextHop = macgCommand->peer.dest; /*nextHop!*/
    if (nextHop != wns::service::dll::UnicastAddress() )
    {
        // Unicast
        wns::scheduler::UserID userID = wns::scheduler::UserID(layer2->getStationManager()->getStationByMAC(nextHop)->getNode());
        registerCID(cid,userID); // does nothing if already registered
    }
    else
    {
		wns::scheduler::UserID bcUserID;
		bcUserID.setBroadcast();
        // Broadcast
        registerCID(cid, bcUserID);
    }
    // ^ TODO: In the long term someone should register cid from outside; not implicitly like it is done here
    return cid;
}

std::string
RegistryProxy::getNameForUser(const UserID user) {
    return user.getName();
}

wns::service::phy::phymode::PhyModeMapperInterface*
RegistryProxy::getPhyModeMapper() const {
    assure (phyModeMapper != NULL, "phyModeMapper==NULL");
    return phyModeMapper;
}

wns::service::phy::phymode::PhyModeInterfacePtr
RegistryProxy::getBestPhyMode(const wns::Ratio& sinr)
{
    return getPhyModeMapper()->getBestPhyMode(sinr);
}

UserID
RegistryProxy::getMyUserID() {
    return wns::scheduler::UserID(layer2->getNode());
}

// rather old method. CQI should be preferred.
wns::scheduler::ChannelQualityOnOneSubChannel
RegistryProxy::estimateTxSINRAt(const UserID user, int subchannel, int timeSlot)
{
    wns::Power rxPower;
    wns::Power interference;
    wns::Ratio pathloss;

    if (user.isBroadcast())
    	return wns::scheduler::ChannelQualityOnOneSubChannel();

    if(subchannel == WIDEBAND)
    {
        rxPower = iCache->getPerSCAveragedCarrier(user.getNode());
        interference = iCache->getPerSCAveragedInterference(user.getNode());
        pathloss = iCache->getPerSCAveragedPathloss(user.getNode());
    }
    else
    {
        rxPower = iCache->getAveragedCarrier(user.getNode(), subchannel);
        interference = iCache->getAveragedInterference(user.getNode(), subchannel, timeSlot);
        pathloss = iCache->getAveragedPathloss(user.getNode(), subchannel);
    }

    return wns::scheduler::ChannelQualityOnOneSubChannel(pathloss, interference, rxPower);
}

wns::scheduler::ChannelQualityOnOneSubChannel
RegistryProxy::estimateRxSINROf(const UserID user, int subchannel, int timeSlot) 
{

    // lookup the results previously reported by us to the remote side
    dll::services::management::InterferenceCache* remoteCache =
        layer2->
        getStationManager()->
        getStationByNode(user.getNode())->
        getManagementService<dll::services::management::InterferenceCache>("INTERFERENCECACHE"+modeBase);

    wns::Power rxPower;
    wns::Power interference;
    wns::Ratio pathloss;

    if(subchannel == WIDEBAND)
    {
        rxPower = remoteCache->getPerSCAveragedCarrier(getMyUserID().getNode());
        interference = remoteCache->getPerSCAveragedInterference(getMyUserID().getNode());
        pathloss = remoteCache->getPerSCAveragedPathloss(getMyUserID().getNode());
    }
    else
    {
        rxPower = remoteCache->getAveragedCarrier(getMyUserID().getNode(), subchannel);
        interference = remoteCache->getAveragedInterference(getMyUserID().getNode(), subchannel, timeSlot);
        pathloss = remoteCache->getAveragedPathloss(getMyUserID().getNode(), subchannel);
    }
    

    /* Ask our cache about the UL interference caused by this node */
    wns::Ratio interferencePathloss = iCache->getAverageEmittedInterferencePathloss(user.getNode());

    return wns::scheduler::ChannelQualityOnOneSubChannel(pathloss, interference, rxPower);
}

wns::Ratio
RegistryProxy::getEffectiveUplinkSINR(const wns::scheduler::UserID user, 
    const std::set<unsigned int>& scs,
    const int timeSlot, 
    const wns::Power& txPower)
{
    std::set<unsigned int>::iterator iter;

    std::map<unsigned int, wns::Power> interferences;

    std::set<unsigned int>::const_iterator it;
    for(it = scs.begin(); it != scs.end(); it++)
    {
        interferences[*it] = iCache->getAveragedInterference(getMyUserID().getNode(), *it, timeSlot);
    }
    dll::services::management::InterferenceCache* remoteCache =
        layer2->
        getStationManager()->
        getStationByNode(user.getNode())->
        getManagementService<dll::services::management::InterferenceCache>(
            "INTERFERENCECACHE"+modeBase);

    return remoteCache->getEffectiveSINR(getMyUserID().getNode(), scs, txPower, interferences);
}

wns::Ratio
RegistryProxy::getEffectiveDownlinkSINR(const wns::scheduler::UserID user, 
    const std::set<unsigned int>& scs,
    const int timeSlot, 
    const wns::Power& txPower,
    const bool worstCase)
{
    dll::services::management::InterferenceCache* remoteCache =
        layer2->
        getStationManager()->
        getStationByNode(user.getNode())->
        getManagementService<dll::services::management::InterferenceCache>(
            "INTERFERENCECACHE"+modeBase);

    if(!worstCase)
    {
        std::set<unsigned int>::iterator iter;

        std::map<unsigned int, wns::Power> interferences;

        std::set<unsigned int>::const_iterator it;
        for(it = scs.begin(); it != scs.end(); it++)
        {
            interferences[*it] = remoteCache->getAveragedInterference(user.getNode(), *it, timeSlot);
        }

        return iCache->getEffectiveSINR(user.getNode(), scs, txPower, interferences);
    }
    else
    {
        wns::Power carrier = remoteCache->getAveragedCarrier(user.getNode(), WIDEBAND, ANYTIME);
        wns::Power interf = remoteCache->getAveragedInterference(user.getNode(), WIDEBAND, ANYTIME);

        return carrier / interf;
    }
}

void 
RegistryProxy::updateUserSubchannels (const wns::scheduler::UserID user, std::set<int>& channels)
{  
  dll::services::management::InterferenceCache* remoteCache =
    layer2->
    getStationManager()->
    getStationByNode(user.getNode())->
    getManagementService<dll::services::management::InterferenceCache>("INTERFERENCECACHE"+modeBase);
    
    remoteCache->updateUserSubchannels (getMyUserID().getNode(), channels);
}


Bits
RegistryProxy::getQueueSizeLimitPerConnection() {
    return queueSize;
}

wns::service::dll::StationType
RegistryProxy::getStationType(const UserID user) {
    wns::service::dll::StationType stationType = layer2->getStationManager()->getStationByNode(user.getNode())->getStationType();
    return stationType;
}

// This is obsolete, but RegistryProxyInterface member and required be the old strategies
// accept frameNumber argument instead of using "static" frameNumberToBeScheduled
wns::scheduler::UserSet
RegistryProxy::filterReachable(wns::scheduler::UserSet users)
{
    // workaround when frameNr is not given
    int frameNr=0; //= frameNumberToBeScheduled
    return filterReachable(users, frameNr);
}

// filter those user from a set of users that can be reached at the given frameNr
wns::scheduler::UserSet
RegistryProxy::filterReachable(wns::scheduler::UserSet users, const int frameNr)
{
    wns::scheduler::UserSet result;
    for (wns::scheduler::UserSet::iterator iter = users.begin();
         iter != users.end(); ++iter) {

        if (iter->isBroadcast())
        {
            MESSAGE_SINGLE(NORMAL, logger, "filterReachable: broadcast (true)");
            result.insert(*iter);
            continue;
        }

        wns::service::dll::StationType stationType = getStationType(*iter);
        // TODO: get rid of "relaysOnly" question.
        if (( relaysOnly == true && stationType==wns::service::dll::StationTypes::FRS() )
            || relaysOnly ==false)
        {
            if (isReachableAt(*iter,frameNr, false))
            {
                MESSAGE_SINGLE(NORMAL, logger, "filterReachable: trying user=" << iter->getName() << " type=" << wns::service::dll::StationTypes::toString(stationType));
                if ( (stationType!=wns::service::dll::StationTypes::FRS()) && (stationType!=wns::service::dll::StationTypes::eNB()) )
                {
                    if (duplexGroupIsReachableAt(*iter, frameNr) == true)
                    {
                        result.insert(*iter);
                        MESSAGE_SINGLE(NORMAL, logger, "filterReachable: "<<iter->getName()<<" is reachable in frame=" << frameNr);
                    } else {
                        MESSAGE_SINGLE(NORMAL, logger, "filterReachable: "<<iter->getName()<<" is NOT reachable in frame=" << frameNr);
                    }
                } else { // BS | RN
                    result.insert(*iter);
                    MESSAGE_SINGLE(NORMAL, logger, "filterReachable: " << iter->getName() << " is reachable in frame=" << frameNr);
                }
            }
        }
    }
    MESSAGE_SINGLE(VERBOSE, logger, "filterReachable: " << result.size() << " users reachable in frame=" << frameNr);
    return result;
}

wns::scheduler::ConnectionSet
RegistryProxy::filterReachable(wns::scheduler::ConnectionSet connections, const int frameNr, bool useHARQ)
{
    wns::scheduler::ConnectionSet result;

    for ( wns::scheduler::ConnectionSet::iterator iterConnection = connections.begin();
          iterConnection != connections.end(); ++iterConnection)
    {
        UserID user = getUserForCID(*iterConnection);
        if (user.isBroadcast())
        {
            MESSAGE_SINGLE(NORMAL, logger, "filterReachable: broadcast (true)");
            result.insert(*iterConnection);
            continue;
        }

        // user from the connection is reachable
        if ( isReachableAt(user,frameNr, useHARQ) )
        {
            result.insert(*iterConnection);
        }
    }

    return result;
}

// only true for RS-Tx in BS or RN-BS:
// isForDL should be constant within an object. So why...?
void RegistryProxy::setDL(bool _isForDL)
{
    isForDL = _isForDL;
}

// only true for RS-Tx in BS or RN-BS:
bool RegistryProxy::getDL() const
{
    return isForDL;
}

void
RegistryProxy::setAssociationHandler(lte::controlplane::associationHandler::AssociationHandler* ah)
{
    friends.associationHandler = ah;
}

bool
RegistryProxy::duplexGroupIsReachableAt(const wns::scheduler::UserID user, const int frameNr) const
{
    // retrieve id of the target user
    wns::service::dll::UnicastAddress targetAddress = layer2->getStationManager()->getStationByNode(user.getNode())->getDLLAddress();

    int peerDuplexGroup = friends.associationHandler->getPeerDuplexGroup(targetAddress);
    int myDuplexGroup   = friends.associationHandler->getMyDuplexGroup(frameNr, isForDL/*member*/);
    // isForDL is set via setDL(downLink) by Lte scheduler.
    // at the same time and for FDD: myDuplexGroup is different for DL and UL
    // therefore myDuplexGroup differs (1->2, 2->1) for UL compared to DL

    // 0=TDD, 3=fullduplexFDD
    if ( (peerDuplexGroup == myDuplexGroup) || (peerDuplexGroup == 0) || (peerDuplexGroup == 3) )
    {
        MESSAGE_SINGLE(NORMAL, logger, "duplexGroupIsReachableAt(User=" <<A2N(targetAddress)<< ",frameNr="<<frameNr<<"): is reachable, peerDuplexGroup=" << peerDuplexGroup << ", myDuplexGroup=" << myDuplexGroup);
        return true;
    } else {
        MESSAGE_SINGLE(NORMAL, logger, "duplexGroupIsReachableAt(User=" <<A2N(targetAddress)<< ",frameNr="<<frameNr<<"): is NOT reachable, peerDuplexGroup=" << peerDuplexGroup << ", myDuplexGroup=" << myDuplexGroup);
        return false;
    }
}

// checks if the peer is reachable now and able to receive the map
// needed for RNs
bool
RegistryProxy::isReachableAt(const UserID user, const int frameNr, bool useHARQ) const
{
    if (user.isBroadcast())
    {
        // Broadcast messages are always being sent, regardless of the target user
        return true;
    }

    // retrieve id of the target user
    wns::service::dll::UnicastAddress targetAddress = layer2->getStationManager()->getStationByNode(user.getNode())->getDLLAddress();
    // only perform the subsequent check if we have an association in this mode,
    // i.e. we are RN or UT
    if (associationService->hasAssociation()){
        // check whether the target station is the one we are associated to,
        // because we should always be able to reach our association (i.e. our RAP) once we
        // were triggered to do so as "slave"
        if (targetAddress == associationService->getAssociation())
        {
            // This is an UL packet. Of course, we can reach our RAP
            // We have been granted UL resources in the UL map
            MESSAGE_SINGLE(NORMAL, logger, "isReachableAt(User=" << A2N(targetAddress) << ", frameNr=" << frameNr << "): UT UL: Yes");

            bool freeHARQProcesses = true;
            if (useHARQ)
            {
                freeHARQProcesses = friends.harq->hasFreeSenderProcess(user);
            }

            if(!freeHARQProcesses)
            {
                MESSAGE_SINGLE(NORMAL, logger, "isReachableAt(User=" << A2N(targetAddress) << ", frameNr=" << frameNr << "): max HARQ processes reached.");
            }

            return freeHARQProcesses;
        }
        else if (friends.timer->stationTaskAtFrame(frameNr) == lte::timing::StationTasks::UT())
        {
            // This was a packet of a UT/RN to its old RAP, but the old one is not
            // reachable any more due to a reassociation
            MESSAGE_SINGLE(NORMAL, logger, "isReachableAt(User=" << A2N(targetAddress) << ", frameNr=" << frameNr << "): UT assoc lost");
            return false;
        }
    }
    assure(friends.timer->stationTaskAtFrame(frameNr)
           == lte::timing::StationTasks::RAP(), "We must be a RAP in this frame!");
    bool ir = (friends.timer->isPeerListeningAt(targetAddress, frameNr) /* for data transmission in future frame */
               && friends.timer->canReceiveMapNow(targetAddress));

    bool freeHARQProcesses = true;

    if (useHARQ)
    {
        if (getDL())
        {
            freeHARQProcesses = friends.harq->hasFreeSenderProcess(user);
            if(!freeHARQProcesses)
            {
                MESSAGE_SINGLE(NORMAL, logger, "isReachableAt(User=" << A2N(targetAddress) << ", frameNr=" << frameNr << "): max HARQ processes reached.");
            }
        }
        else
        {
            freeHARQProcesses = friends.harq->hasFreeReceiverProcess(user);
            if(!freeHARQProcesses)
            {
                MESSAGE_SINGLE(NORMAL, logger, "isReachableAt(User=" << A2N(targetAddress) << ", frameNr=" << frameNr << "): max HARQ processes reached.");
            }
        }
    }

    MESSAGE_SINGLE(NORMAL, logger, "isReachableAt(User=" << A2N(targetAddress) << ", frameNr=" << frameNr << "): reachable=" << ir);
    return ir && (freeHARQProcesses);
}

/**
 *@todo dbn: DEPRECATED: calcULResources is only used in old PCRR scheduling strategy. To be removed
 */
wns::scheduler::PowerMap
RegistryProxy::calcULResources(const wns::scheduler::UserSet& users, unsigned long int rapResources) const
{
    // generate empty container for result
    wns::scheduler::PowerMap result;
    return result;
}

wns::scheduler::UserSet
RegistryProxy::getActiveULUsers() const
{
    assure(friends.rrHandler, "UL Resource Allocation may only be done in uplink");
    return friends.rrHandler->getActiveUsers();
}

int
RegistryProxy::getTotalNumberOfUsers(wns::scheduler::UserID user)
{
   // The purpose of this method is to provide fairness for RNs by the BS
    // This way the BS knows how many RUTs are connected to the RN and accordingly
    // can consider this when scheduling
    if (layer2->getStationType() == wns::service::dll::StationTypes::eNB() &&
        layer2->getStationManager()->getStationByNode(user.getNode())->getStationType()
        == wns::service::dll::StationTypes::FRS())
    {
        // We determine the number of user plane flows
        // This is an indicator for how many UTs there are,
        // if and only if there is exactly one user plane flow per UT
        int numUpConnections = 0;
        ConnectionVector connections = getConnectionsForUser(user);
        wns::service::dll::UnicastAddress userAddress
            = layer2->getStationManager()->getStationByNode(user.getNode())->getDLLAddress();

        for(wns::scheduler::ConnectionVector::iterator iter = connections.begin();
            iter != connections.end(); ++iter )
        { // for all connections/flows of the RN
            wns::service::dll::FlowID flowID = *iter;
            if (!friends.flowManager->isControlPlaneFlowID(userAddress, flowID))
            {
                // count user plane flows/connections
                ++numUpConnections;
            }
        }
        if (numUpConnections != 0)
        {
            assure(numUpConnections>0,"numUpConnections(" << user.getName() <<")="<<numUpConnections);
            return numUpConnections;
        }
    }
    return 1;
}

int
RegistryProxy::getTotalNumberOfFlows() const
{
    return friends.flowManager->countFlows();
}

int
RegistryProxy::getNumberOfFlowsForUser(const wns::scheduler::UserID user) const
{
    wns::service::dll::UnicastAddress peerAdress = layer2->getStationManager()->getStationByNode(user.getNode())->getDLLAddress();
    return friends.flowManager->countFlows(peerAdress);
}

// what is this? Please explain. [rs]
void
RegistryProxy::setRelaysOnly()
{
    relaysOnly = true;
}

// what is this? Please explain. [rs]
void
RegistryProxy::setAllStations()
{
    relaysOnly = false;
}

// This is CQI related
wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
RegistryProxy::getChannelQualities4UserOnUplink(UserID user, int frameNr)
{
    MESSAGE_SINGLE(NORMAL, logger, "getChannelQualities4UserOnUplink("<<user.getName()<<","<<frameNr<<")");
    assure(false, "This should not be called. We disable the use of CQI");
    return fakeChannelQualities;
}

// This is CQI related
wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
RegistryProxy::getChannelQualities4UserOnDownlink(UserID user, int frameNr)
{
    MESSAGE_SINGLE(NORMAL, logger, "getChannelQualities4UserOnDownlink("<<user.getName()<<","<<frameNr<<")");
    assure(false, "This should not be called. We disable the use of CQI");
    return fakeChannelQualities;
} // getChannelQualities4UserOnDownlink

// This is CQI related
wns::scheduler::PowerCapabilities
RegistryProxy::getPowerCapabilities(const UserID user) const
{
    wns::scheduler::PowerCapabilities result;
    // get other station type
    wns::service::dll::StationType stationType;
    if (user.isValid()) { // peer known
        stationType = layer2->getStationManager()->getStationByNode(user.getNode())->getStationType();
    } else { // peer unknown. assume peer=UT
        stationType = wns::service::dll::StationTypes::UE();
    }
    if ( stationType == wns::service::dll::StationTypes::UE() )
        result=powerUT; // this may be variable in the future (different userTerminal classes)
    else if ( stationType == wns::service::dll::StationTypes::eNB() )
        result=powerBS;
    else
        assure(false, "oops, don't know other station ("<<user.getName()<<") stationType="<<stationType);

    assure(result.nominalPerSubband!=wns::Power(),"undefined power nominalPerSubband="<<result.nominalPerSubband);
    if (user.isValid()) {
        MESSAGE_SINGLE(NORMAL, logger, "getPowerCapabilities("<<user.getName()<<"): nominal="<<result.nominalPerSubband<<" ("<< wns::service::dll::StationTypes::toString(stationType)<<")");
    } else {
        MESSAGE_SINGLE(NORMAL, logger, "getPowerCapabilities(NULL): nominal="<<result.nominalPerSubband<<" ("<< wns::service::dll::StationTypes::toString(stationType)<<")");
    }
    return result;
}

wns::scheduler::PowerCapabilities
RegistryProxy::getPowerCapabilities() const
{
    wns::scheduler::PowerCapabilities result;
    // get my own station type
    wns::service::dll::StationType stationType = layer2->getStationType();
    if ( stationType == wns::service::dll::StationTypes::UE() )
        result=powerUT;
    else if ( stationType == wns::service::dll::StationTypes::eNB() )
        result=powerBS;
    else
        assure(false, "oops, don't know my own station stationType="<<stationType);
    assure(result.nominalPerSubband!=wns::Power(),"undefined power nominalPerSubband="<<result.nominalPerSubband);
    MESSAGE_SINGLE(NORMAL, logger, "getPowerCapabilities(): nominal="<<result.nominalPerSubband);
    return result;
}

// This is QoS/priority related
int
RegistryProxy::getNumberOfQoSClasses()
{
    return numberOfQosClasses;
}

// This is QoS/priority related
int
RegistryProxy::getNumberOfPriorities()
{
    return numberOfPriorities;
}

// This is QoS/priority related
wns::scheduler::ConnectionList&
RegistryProxy::getCIDListForPriority(int priority)
{
    MESSAGE_SINGLE(NORMAL, logger, "getCIDListforPriority("<<priority<<")");
    // priority is out of [0..MaxPriority-1]:
    assure(priority>=0 && priority<numberOfPriorities,"invalid priority="<<priority<<" (numberOfPriorities="<<numberOfPriorities<<")");
    return connectionsForPriority[priority];
}


// gets the cids in a set, because the strategy can better handle sorted list of
// cids (a set implicit sorts the cids)
wns::scheduler::ConnectionSet
RegistryProxy::getConnectionsForPriority(int priority)
{
    wns::scheduler::ConnectionSet result;
    //MESSAGE_SINGLE(NORMAL, logger, "getConnectionsforPriority("<<priority<<")");
    // priority is out of [0..MaxPriority-1]:
    assure(priority>=0 && priority<numberOfPriorities,"invalid priority="<<priority<<" (numberOfPriorities="<<numberOfPriorities<<")");
    //MESSAGE_SINGLE(NORMAL, logger, "getConnectionsforPriority("<<priority<<"): count="<<connectionsForPriority[priority].size());
    for ( wns::scheduler::ConnectionList::iterator iter = connectionsForPriority[priority].begin();
          iter != connectionsForPriority[priority].end(); ++iter)
    {
        ConnectionID cid = *iter;
        result.insert(cid);
        //MESSAGE_SINGLE(NORMAL, logger, "getConnectionsforPriority("<<priority<<"): added cid=" << cid);
    }
    MESSAGE_SINGLE(NORMAL, logger, "getConnectionsforPriority("<<priority<<"): "<<wns::scheduler::printConnectionSet(result));
    return result;
}


int
RegistryProxy::mapQoSClassToPriority(wns::service::qos::QoSClass qosClass)
{
    assure(qosClass>=0 && qosClass<getNumberOfQoSClasses(),"invalid QoS class "<<qosClass);
    int priority=qosToPriorityMapping[qosClass];
    return priority;
}


int
RegistryProxy::getPriorityForConnection(wns::scheduler::ConnectionID cid)
{
    assure(cidToPrio.knows(cid),"cid "<<cid<<" is not registered");
    return cidToPrio.find(cid);
}

// this is only complete after OnWorldCreated!
dll::NodeList
RegistryProxy::getNodeList()
{
    assure(layer2!=NULL,"layer2==NULL");
    dll::StationManager* stationManager = layer2->getStationManager();
    assure(stationManager!=NULL,"stationManager==NULL");
    return stationManager->getNodeList();
}
