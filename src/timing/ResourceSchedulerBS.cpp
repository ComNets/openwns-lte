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

#include <LTE/timing/ResourceSchedulerBS.hpp>

#include <LTE/timing/RegistryProxy.hpp>
#include <LTE/helper/QueueProxy.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>

#include <DLL/StationManager.hpp>

#include <WNS/StaticFactory.hpp>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::timing;

STATIC_FACTORY_REGISTER_WITH_CREATOR(ResourceSchedulerBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.timing.ResourceScheduler.BS",
                                     wns::ldk::FUNConfigCreator);

ResourceSchedulerBS::ResourceSchedulerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    ResourceScheduler(fun,config),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasDeliverer<>(),
    wns::Cloneable<ResourceSchedulerBS>()
{
}

ResourceSchedulerBS::~ResourceSchedulerBS()
{
    MESSAGE_SINGLE(VERBOSE, logger, "ResourceSchedulerBS::~ResourceSchedulerBS()");
}

void
ResourceSchedulerBS::onFUNCreated()
{
    ResourceScheduler::onFUNCreated();
    MESSAGE_SINGLE(NORMAL, logger, "ResourceSchedulerBS::onFUNCreated()");

    lte::timing::RegistryProxy* registryInLte = dynamic_cast<lte::timing::RegistryProxy*>(colleagues.registry);

    // Registry must know it. Caution: this is set quite late, after all onFUNCreated() actions
    registryInLte->setDL(!IamUplinkMaster);
}

void
ResourceSchedulerBS::startCollection(int frameNr) 
{
    MESSAGE_BEGIN(NORMAL, logger, m, "Starting Master Scheduling("
        << (IamUplinkMaster?"UL":"DL")
        << ") for frame=" << frameNr);
        if (!IamUplinkMaster) 
        { // RS-TX
            assure(colleagues.queue!=NULL,"colleagues.queue==NULL");
            m << ": Queue(cid:bits,pdus) = " << colleagues.queue->printAllQueues(); 
        } 
        else 
        { // RS-RX
            assure(colleagues.queueProxy!=NULL,"colleagues.queueProxy==NULL");
            m << ": Queue(cid:bits,pdus) = " << colleagues.queueProxy->printAllQueues(); // show queue sizes:
        }
    MESSAGE_END();

    ResourceScheduler::startCollection(frameNr, slotDuration);

    lte::timing::RegistryProxy* registryInLte = dynamic_cast<lte::timing::RegistryProxy*>(colleagues.registry);

    // TODO: still needed?
    registryInLte->setAllStations(); 
}

void
ResourceSchedulerBS::resetQueues(wns::scheduler::UserID user)
{
    // Delete all PDUs from the queue
    assure(!colleagues.strategy->isTx(), 
        "The scheduler queues should only be resetted for RX schedulers, exactly the fake PDUs.");
    colleagues.queue->resetQueues(user);
}

void
ResourceSchedulerBS::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    // userAdr is always UT; dstAdr is always RAP (UT's next hop)
    // user is directly connected to me (BS || RN-BS)
    if (layer2->getDLLAddress() == dstAdr) 
    {
        wns::node::Interface* user = layer2->getStationManager()->getStationByMAC(userAdr)->getNode();
        colleagues.queue->resetQueues(wns::scheduler::UserID(user));
        MESSAGE_SINGLE(NORMAL, logger, "onDisassociated("
            << A2N(userAdr) << "," << A2N(dstAdr)
            << "): Removed Packets from " << A2N(dstAdr) << " to " << A2N(userAdr));
    }
    else // multihop case: I am BS; user is connected via RN to me
    {
        // disassociation of userAdr from dstAdr
        // (in RAP that has dstAdr in its set of associated stations)
        assure(associationService->hasAssociated(dstAdr), "Disassociation notification on unknown route.");
        wns::node::Interface* viaRelay = layer2->getStationManager()->getStationByMAC(dstAdr)->getNode();

        MESSAGE_SINGLE(NORMAL, logger, "onDisassociated(" 
            << A2N(userAdr) << ","<<A2N(dstAdr)<<"): Removing Packets from " 
            << A2N(layer2->getDLLAddress()) << " via " << A2N(dstAdr) <<  " to " << A2N(userAdr));

        this->deletePacketsToVia(userAdr, viaRelay);
    }

    // disassociation of userAdr from dstAdr (in RAP that has dstAdr)
    if (scorer.hasRoute(userAdr))
        scorer.deleteRoute(userAdr);
}

void
ResourceSchedulerBS::onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    if (layer2->getDLLAddress() == dstAdr)
    {
        scorer.setRoute(userAdr, userAdr);
        MESSAGE_SINGLE(NORMAL, logger, "onAssociated: setting route to address " << A2N(userAdr));
    }

    else if (!(layer2->getDLLAddress() == userAdr))
    {
        assure(associationService->hasAssociated(dstAdr), 
            "onAssociated: trying to set route to a RN which not associated!");
        scorer.setRoute(userAdr, dstAdr);
        MESSAGE_SINGLE(NORMAL, logger, "onAssociated: setting route to address " 
            << A2N(userAdr) << " via address " << A2N(dstAdr));
    }
}

// called during disassociation:
void
ResourceSchedulerBS::deletePacketsToVia(wns::service::dll::UnicastAddress destination,
                                        wns::node::Interface* via)
{
    wns::ldk::CommandReaderInterface* rlcReader = getFUN()->getCommandReader("rlc");
    wns::scheduler::ConnectionVector conns = colleagues.registry->getConnectionsForUser(wns::scheduler::UserID(via));
    wns::service::dll::UnicastAddress RNAddress = layer2->getStationManager()->getStationByNode(via)->getDLLAddress();

    MESSAGE_SINGLE(NORMAL, logger, "deletePacketsToVia("<<A2N(destination)<<","<<via->getName()<<"): "<<conns.size()<<" connections");

    if (conns.size() == 0)
    {
        MESSAGE_SINGLE(NORMAL, logger, "No active queue for node " << via->getName() << ", nothing had to be removed.");
        return;
    }

    /* iterate over all cids */
    for (wns::scheduler::ConnectionVector::const_iterator iter = conns.begin(); iter != conns.end(); ++iter)
    {
        wns::scheduler::ConnectionID cid = *iter;
        
        // don't delete. ControlPlane is always single/next-hop
        if(friends.flowManager->isControlPlaneFlowID(RNAddress,cid)) 
        {
            continue; 
        } 
        else 
        { // data plane flowID. Delete queue
            colleagues.queue->resetQueue(cid);
        }
    } // for all cids in connections
} // deletePacketsToVia()
