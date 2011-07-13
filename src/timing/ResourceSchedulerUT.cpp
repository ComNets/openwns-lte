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

#include <LTE/timing/ResourceSchedulerUT.hpp>

#include <LTE/timing/RegistryProxy.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>

#include <WNS/StaticFactory.hpp>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::timing;

STATIC_FACTORY_REGISTER_WITH_CREATOR(ResourceSchedulerUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.timing.ResourceScheduler.UT",
                                     wns::ldk::FUNConfigCreator);

ResourceSchedulerUT::ResourceSchedulerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    ResourceScheduler(fun,config),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasDeliverer<>(),
    wns::Cloneable<ResourceSchedulerUT>(),
    symbolDuration(config.get<simTimeType>("symbolDuration")),
    myMasterUserID(NULL)
{
    MESSAGE_SINGLE(NORMAL, logger, "RSUT: symbolDuration = " << symbolDuration);
}

void
ResourceSchedulerUT::onFUNCreated()
{
    ResourceScheduler::onFUNCreated();
    MESSAGE_SINGLE(NORMAL, logger, "ResourceSchedulerUT::onFUNCreated()");

    // Registry must know it. Caution: this is set quite late, after all onFUNCreated() actions
    colleagues.registry->setDL(false);
    
}

void
ResourceSchedulerUT::startCollection(int frameNr) 
{
    using namespace wns::scheduler;
    MESSAGE_BEGIN(NORMAL, logger, m, "Slave(");
        m << (myMasterUserID?myMasterUserID->getName():""); 
        m << ") Scheduling for frame=" << frameNr << ": ";
        m << "Queue(cid:bits,pdus) = " << colleagues.queue->printAllQueues(); 
    MESSAGE_END();

    // If we cannot send, then we do nothing.
    if(colleagues.registry->hasResourcesGranted())
    {
        MESSAGE_SINGLE(NORMAL, logger, "Slave(hasResourcesGranted==true)");
        ResourceScheduler::startCollection(frameNr, slotDuration);
        ResourceScheduler::finishCollection(frameNr, wns::simulator::getEventScheduler()->getTime());
    }
    else
    {
        MESSAGE_SINGLE(NORMAL, logger, "Slave(hasResourcesGranted==false)");
    }
} // startCollection

void
ResourceSchedulerUT::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    // userAdr is always UT; dstAdr is always RAP (UT's next hop)
    if (layer2->getDLLAddress() == userAdr) // I am UT
    {
        assure(associationService->getAssociation() == dstAdr, "address mismatch when releasing association");

        // Notification about Disassociation of UT from RAP (in the UT)
        wns::node::Interface* rap = layer2->getStationManager()->getStationByMAC(dstAdr)->getNode();
        MESSAGE_SINGLE(NORMAL, logger, "onDisassociated("<<A2N(userAdr)
            << "," << A2N(dstAdr) << "): Removing Packets from " 
            << A2N(userAdr) << " to " << A2N(dstAdr));
        colleagues.queue->resetQueues(wns::scheduler::UserID(rap));
    }
    // I am RN (UT task)
    else 
    {
        // Notification about Disassociation of UT from RAP (in the RN)
        if (layer2->getDLLAddress() == dstAdr)
        {
            // If we have no association, we are not forwarding in this mode
            if (!associationService->hasAssociation())
                return;

            wns::service::dll::UnicastAddress myAssociation = associationService->getAssociation();
            wns::node::Interface* hopDestination = layer2->getStationManager()->getStationByMAC(myAssociation)->getNode();
            MESSAGE_SINGLE(NORMAL, logger, "onDisassociated("
                << A2N(userAdr) << ","<<A2N(dstAdr) 
                << "): Removing Packets from " << A2N(userAdr) 
                << " via " << A2N(dstAdr) <<  " to " << A2N(myAssociation));
            this->deletePacketsToFrom(hopDestination, userAdr);
        }
    }
}

void
ResourceSchedulerUT::onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    MESSAGE_SINGLE(NORMAL, logger, "onAssociated("<<A2N(userAdr)<<","<<A2N(dstAdr)<<")");
    wns::service::dll::UnicastAddress myAssociation = associationService->getAssociation();
    myMasterUserID = layer2->getStationManager()->getStationByMAC(myAssociation)->getNode();
}

wns::scheduler::QueueStatusContainer
ResourceSchedulerUT::getQueueStatus() const
{
    wns::scheduler::QueueStatusContainer result = colleagues.queue->getQueueStatus();
    return result;
}

void
ResourceSchedulerUT::deletePacketsToFrom(wns::node::Interface* destination,
                                         wns::service::dll::UnicastAddress source)
{
    wns::ldk::CommandReaderInterface* rlcReader = getFUN()->getCommandReader("rlc");
    wns::scheduler::ConnectionVector conns = colleagues.registry->getConnectionsForUser(wns::scheduler::UserID(destination));

    MESSAGE_SINGLE(NORMAL, logger, "deletePacketsToFrom("
        << destination->getName() << ","
        << A2N(source) << "): " << conns.size() << " conns");

    if (conns.size() == 0)
    {
        MESSAGE_SINGLE(NORMAL, logger, "No active queue for node " << destination->getName() << ", nothing had to be removed.");
        return;
    }

    /* iterate over all cids */
    for (wns::scheduler::ConnectionVector::const_iterator iter = conns.begin(); iter != conns.end(); ++iter)
    {
        wns::scheduler::ConnectionID cid = *iter;
        
        // don't delete. ControlPlane is always single/next-hop
        if (friends.flowManager->isControlPlaneFlowID(source, cid)) 
        {
            continue; 
        } 
        // data plane flowID. Delete queue
        else 
        { 
            colleagues.queue->resetQueue(cid);
        }
    } // for all cids in connections
} // deletePacketsToFrom()


