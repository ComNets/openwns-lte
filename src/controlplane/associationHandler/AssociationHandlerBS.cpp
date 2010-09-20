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

#include <LTE/controlplane/associationHandler/AssociationHandlerBS.hpp>
#include <LTE/controlplane/AssociationsProxy.hpp>
#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>
#include <LTE/macg/MACgCommand.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/helper/Keys.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/ldk/IConnectorReceptacle.hpp>
#include <DLL/services/control/Association.hpp>
#include <DLL/StationManager.hpp>

#include <boost/bind.hpp>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace lte::controlplane::associationHandler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(AssociationHandlerBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.AssociationHandler.BaseStation",
                                     wns::ldk::FUNConfigCreator);

AssociationHandlerBS::AssociationHandlerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    AssociationHandler(fun, config),
    wns::Cloneable<AssociationHandlerBS>(),
    associationsProxy()
{
}

void
AssociationHandlerBS::onFUNCreated()
{
    AssociationHandler::onFUNCreated();

    associationsProxy = dynamic_cast<lte::controlplane::AssociationsProxyBS*>(layer2->getControlService<wns::ldk::ControlServiceInterface>("AssociationsProxy"));

    // get handles to outgoing FUs
    typedef std::list<wns::ldk::IConnectorReceptacle*> FUList;
    FUList connectorSet = connector->getFUs();
    for (FUList::const_iterator iter = connectorSet.begin();
         iter != connectorSet.end();
         ++iter)
    {
        size_t found = (*iter)->getFU()->getName().find("bchBuffer");
        if (found != std::string::npos)
            friends.bchBuffer = (*iter);
        found = (*iter)->getFU()->getName().find("controlPlaneDispatcher");
        if (found != std::string::npos)
            friends.cpDispatcher = (*iter);
    }

    phyUser = getFUN()->findFriend<lte::macr::PhyUser*>(modeBase + separator + "phyUser");
}

void
AssociationHandlerBS::doOnData(const wns::ldk::CompoundPtr& compound)
{
    lte::controlplane::associationHandler::AssociationCommand* incomingCommand = getCommand(compound->getCommandPool());

    wns::service::dll::UnicastAddress sendingTo = incomingCommand->peer.dst;

    if (!(sendingTo == layer2->getDLLAddress()))
        return;

    wns::service::dll::UnicastAddress comingFrom = incomingCommand->peer.src;

    if(incomingCommand->peer.myCompoundType == CompoundType::association_req())
    {
        MESSAGE_BEGIN(NORMAL, logger, m, "Received ");
        m << incomingCommand->peer.mode << separator
          << "association_req from "
          << layer2->getStationManager()->getStationByMAC(incomingCommand->peer.src)->getName();
        MESSAGE_END();

        //safe the new users duplex group
        duplexGroups[incomingCommand->peer.user] = incomingCommand->peer.duplexGroup;
        MESSAGE_SINGLE(NORMAL, logger, "Stored duplex group=" << duplexGroups[incomingCommand->peer.user] << " for user=" << layer2->getStationManager()->getStationByMAC(incomingCommand->peer.user)->getName());

        if(!(incomingCommand->peer.user == comingFrom)) // forwarded
        {
            // inform associationsProxy
            lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs =
                associationsProxy->associatedPerMode(incomingCommand->peer.user, incomingCommand->peer.src, incomingCommand->peer.mode);

            // notify observers
            notifyOnAssociated(incomingCommand->peer.user, comingFrom);

            // send association_ack
            connector->activate(friends.cpDispatcher);
            createAssociation_ack(comingFrom, incomingCommand->peer.user, incomingCommand->peer.mode, _controlPlaneFlowIDs);
        }
        else // directly
        {
            // inform associationsProxy
            lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs =
                associationsProxy->associatedPerMode(incomingCommand->peer.user, layer2->getDLLAddress(), incomingCommand->peer.mode);

            // notify observers
            notifyOnAssociated(incomingCommand->peer.user, layer2->getDLLAddress());

            connector->activate(friends.bchBuffer);
            createAssociation_ack(comingFrom, incomingCommand->peer.user, mode, _controlPlaneFlowIDs);
        }
    }
    else if(incomingCommand->peer.myCompoundType == CompoundType::disassociation_req())
    {
        MESSAGE_BEGIN(NORMAL, logger, m, "received ");
        m << incomingCommand->peer.mode << separator << "disassociation_req from DLL address " << incomingCommand->peer.src;
        MESSAGE_END();

        bool preserve = false;
        if ((incomingCommand->peer.targetRAP).isValid())
            preserve = associationsProxy->inMyREC(incomingCommand->peer.targetRAP);

        // inform AssociationsProxy
        associationsProxy->disassociatedPerMode(incomingCommand->peer.user,
                                                incomingCommand->peer.targetRAP,
                                                incomingCommand->peer.mode);

        if(incomingCommand->peer.user == comingFrom) // UT to BS (directly)
        {
            // directly
            dll::ILayer2* ut = layer2->getStationManager()->getStationByMAC(incomingCommand->peer.user);
            //dll::ILayer2* ut = layer2->getStationManager()->getStationByMAC(incomingCommand->peer.user);
            associationService->releaseClient(ut);

	    boost::function<void()> callback;
	    callback = boost::bind(&lte::controlplane::associationHandler::AssociationHandler::notifyOnDisassociated, this, incomingCommand->peer.user, layer2->getDLLAddress());
            connector->activate(friends.cpDispatcher);
            createDisassociation_ack(comingFrom, incomingCommand->peer.user, preserve, mode, callback);
        }
        else // forwarded via a RN (UT->RN->BS)
        {
            // notify observers
            notifyOnDisassociated(incomingCommand->peer.user, comingFrom);

	    boost::function<void()> callback;
            connector->activate(friends.cpDispatcher);
            createDisassociation_ack(comingFrom, incomingCommand->peer.user, preserve, incomingCommand->peer.mode, callback);
        }
    }
    else
	assure(false, "received association compound with wrong type!");
}

void
AssociationHandlerBS::createAssociation_ack(wns::service::dll::UnicastAddress destination,
                                            wns::service::dll::UnicastAddress user,
                                            std::string perMode,
                                            lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs) const
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr associationCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    /** activate and set MACg and command */
    lte::macg::MACgCommand* macgCommand = dynamic_cast<lte::macg::MACgCommand*>(macgReader->activateCommand(associationCompound->getCommandPool()));
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = destination;

    // activate RLC command
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(associationCompound->getCommandPool()));
    rlcCommand->peer.flowID = _controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    AssociationCommand* outgoingCommand = this->activateCommand(associationCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::association_ack();
    outgoingCommand->peer.src = layer2->getDLLAddress();
    outgoingCommand->peer.dst = destination;
    outgoingCommand->peer.user = user;
    outgoingCommand->peer.mode = perMode;
    outgoingCommand->peer.duplexGroup = duplexGroups.find(user)->second;
    outgoingCommand->peer.controlPlaneFlowIDs = _controlPlaneFlowIDs;
    outgoingCommand->magic.bs = layer2->getDLLAddress();
    outgoingCommand->magic.sender = this; // for debugging and assures

    /** send the compound */
    if (getConnector()->hasAcceptor(associationCompound))
    {
        getConnector()->getAcceptor(associationCompound)->sendData(associationCompound);
        MESSAGE_SINGLE(NORMAL, logger, "sent "
                       << outgoingCommand->peer.mode << separator << "associationAck to "
                       << A2N(outgoingCommand->peer.dst));
    }
    else
        assure(false, "Lower FU is not accepting scheduled association_ack compound but is supposed to do so");
}

int
AssociationHandlerBS::getMyDuplexGroup(int frameNr, bool isDL)
{
    int duplexGroup = 0;
    if (phyUser->isFddCapable())
    {
        for (int i=0; i < frameNr; i++)
        {
            {
                duplexGroup = (duplexGroup + 1) % 2;
                //just alternating everytime we are RAP...
            }
        }
        // for UL:
        if (!isDL) {
            duplexGroup = (duplexGroup + 1) % 2;
            // UL will be the other group
        }
        return duplexGroup+1;
        // plus one, because we have defined halfduplex groups as 1 and 2...
    }
    else
    {
        return duplexGroup;
    }
}

int
AssociationHandlerBS::getPeerDuplexGroup(wns::service::dll::UnicastAddress user)
{
    return duplexGroups.find(user)->second;
}

void
AssociationHandlerBS::createDisassociation_ack(wns::service::dll::UnicastAddress destination,
                                               wns::service::dll::UnicastAddress user,
                                               bool preserve, std::string perMode,
                                               boost::function<void()> callback) const
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr associationCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    /** activate and set MACg and command */
    lte::macg::MACgCommand* macgCommand = dynamic_cast<lte::macg::MACgCommand*>(macgReader->activateCommand(associationCompound->getCommandPool()));
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = destination;

    // activate RLC command
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(associationCompound->getCommandPool()));
    lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs =
        friends.flowManager->getControlPlaneFlowIDs(destination);
    rlcCommand->peer.flowID = _controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    AssociationCommand* outgoingCommand = this->activateCommand(associationCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::disassociation_ack();
    outgoingCommand->peer.src = layer2->getDLLAddress();
    outgoingCommand->peer.dst = destination;
    outgoingCommand->peer.user = user;
    outgoingCommand->peer.preserve = preserve;
    outgoingCommand->peer.mode = perMode;
    outgoingCommand->magic.bs = layer2->getDLLAddress();
    outgoingCommand->magic.sender = this; // for debugging and assures

    if (!callback.empty())
    {
        /** activate Phy User's command */
        lte::macr::PhyCommand* outgoingPhyCommand = phyUser->activateCommand(associationCompound->getCommandPool());
        outgoingPhyCommand->local.onAirCallback = callback;
    }

    /** send the compound */
    if (getConnector()->hasAcceptor(associationCompound))
    {
        getConnector()->getAcceptor(associationCompound)->sendData(associationCompound);
        MESSAGE_SINGLE(NORMAL, logger, "Sent "
                       << outgoingCommand->peer.mode << separator << "disassociation_ack to "
                       << layer2->getStationManager()->getStationByMAC(outgoingCommand->peer.dst)->getName());
    }
    else
        assure(false, "Lower FU is not accepting scheduled disassociation_ack compound but is supposed to do so");
}

void
AssociationHandlerBS::disassociationOnTimeout(const wns::service::dll::UnicastAddress adr, const std::string perMode)
{
    // notify observers
    wns::service::dll::UnicastAddress rapAdr = associationsProxy->getRAPforUserPerMode(adr, perMode);
    notifyOnDisassociated(adr, rapAdr);

    // inform AssociationsProxy
    associationsProxy->disassociatedPerMode(adr, wns::service::dll::UnicastAddress(), perMode);
}
