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

#include <LTE/controlplane/associationHandler/AssociationHandlerUT.hpp>
#include <LTE/controlplane/AssociationsProxy.hpp>
#include <LTE/macg/MACgCommand.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/helper/Keys.hpp>

#include <DLL/services/control/Association.hpp>
#include <DLL/Layer2.hpp>
#include <DLL/StationManager.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/ldk/IConnectorReceptacle.hpp>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::controlplane::associationHandler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(AssociationHandlerUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.AssociationHandler.UserTerminal",
                                     wns::ldk::FUNConfigCreator);

AssociationHandlerUT::AssociationHandlerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    AssociationHandler(fun, config),
    wns::Cloneable<AssociationHandlerUT>(),
    timeout(config.get<double>("timeout")),
    associationsProxy(),
    scheduledRAP(),
    duplexGroup(config.get<int>("capabilities.duplexGroup"))
{
    MESSAGE_SINGLE(NORMAL, logger, "My duplexGroup=" << duplexGroup);
}

void
AssociationHandlerUT::onFUNCreated()
{
    AssociationHandler::onFUNCreated();

    associationsProxy = dynamic_cast<lte::controlplane::AssociationsProxyUT*>(layer2->getControlService<wns::ldk::ControlServiceInterface>("AssociationsProxy"));

    // get handles to outgoing FUs
    typedef std::list<wns::ldk::IConnectorReceptacle*> FUList;
    FUList connectorSet = connector->getFUs();
    for (FUList::const_iterator iter = connectorSet.begin();
         iter != connectorSet.end();
         ++iter)
    {
        size_t found = (*iter)->getFU()->getName().find("rachDispatcher");
        if (found != std::string::npos)
            friends.rachDispatcher = (*iter);
        found = (*iter)->getFU()->getName().find("controlPlaneDispatcher");
        if (found != std::string::npos)
            friends.cpDispatcher = (*iter);
    }

    assure(friends.rachDispatcher, "AssociationHandler requires a friend with name: "+mode+separator+"rachDispatcher");
    assure(friends.cpDispatcher, "AssociationHandler requires a friend with name: "+mode+separator+"controlPlaneDispatcher");
}

void
AssociationHandlerUT::doOnData(const wns::ldk::CompoundPtr& compound)
{
    lte::controlplane::associationHandler::AssociationCommand* incomingCommand = getCommand(compound->getCommandPool());

    wns::service::dll::UnicastAddress sendingTo = incomingCommand->peer.dst;

    /** Check if this is sent to me*/
    if (!(sendingTo == layer2->getDLLAddress()))
        return;

    wns::service::dll::UnicastAddress comingFrom = incomingCommand->peer.src;

    if (incomingCommand->peer.myCompoundType == CompoundType::disassociation_ack())
    {
        MESSAGE_BEGIN(NORMAL, logger, m, "Received " );
        m << mode << separator << "disassociation_ack from " << layer2->getStationManager()->getStationByMAC(incomingCommand->peer.src)->getName();
        MESSAGE_END();

        assure(associationService->hasAssociation(), "Received disassociation_ack for none association!");

        // needed to notify observers after releasing from master
        wns::service::dll::UnicastAddress dst = associationService->getAssociation();

        // ignore the unexpected disassociation_ack from unexpected RAP! e.g.:
        // send disassociation_req to station 1...timeout,
        // release itself from the station 1 and
        // send association_req to station 2...,
        // received association_ack from station 2...,
        // received delayed disassociation_ack from station 1 !
        if (!(dst == comingFrom)) return;

        cancelTimeout();

        // get key of flow separators and flow gates
        wns::service::dll::UnicastAddress bsAdr = associationsProxy->getBSforMode(mode);
        assure(bsAdr.isValid(), "Wrong BS address stored in AssociationsProxy!");

        // notify observers
        notifyOnDisassociated(layer2->getDLLAddress(), dst);

        // notify Association Service
        associationService->releaseFromMaster();

        // inform associationsProxy
        associationsProxy->disassociatedPerMode(incomingCommand->peer.src, mode, incomingCommand->peer.preserve);

        // Disassociation phase of Intra Mode Handover is done,
        // Association phase automatically
        if (scheduledRAP.isValid())
        {
            createAssociation_req(scheduledRAP);
            // set timeout for waiting for the acknowledgement
            setTimeout(timeout);
        }
    }
    else if (incomingCommand->peer.myCompoundType == CompoundType::association_ack())
    {
        assure(!(associationService->hasAssociation()), "Received association_ack for already existing association!");

        MESSAGE_BEGIN(NORMAL, logger, m, "Received " );
        m << mode << separator << "association_ack from " << layer2->getStationManager()->getStationByMAC(incomingCommand->peer.src)->getName();
        MESSAGE_END();

        cancelTimeout();

        // Intra Mode handover is done, reset the scheduledRAP
        if (scheduledRAP.isValid())
        {
            assure(comingFrom == scheduledRAP, "Received association_ack from unexpected RAP!");

            // reset scheduledRAP
            scheduledRAP = wns::service::dll::UnicastAddress();
        }

        // register association
        associate(comingFrom);

        lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs =
            incomingCommand->peer.controlPlaneFlowIDs;

        // inform associationsProxy
        associationsProxy->associatedPerMode(incomingCommand->peer.src, incomingCommand->magic.bs, mode, _controlPlaneFlowIDs);
    }
    else
        assure(false, "received association compound with wrong type!");
}

void
AssociationHandlerUT::onTimeout()
{
    MESSAGE_SINGLE(NORMAL, logger, "Timeout for waiting for acknowledgement!");

    if (associationService->hasAssociation())
    {
        wns::service::dll::UnicastAddress dst = associationService->getAssociation();
        wns::service::dll::UnicastAddress bsAdr = associationsProxy->getBSforMode(mode);

        // notify observers
        notifyOnDisassociated(layer2->getDLLAddress(), dst);

        // release the not reachable association
        associationService->releaseFromMaster();
        layer2->getControlService<lte::controlplane::AssociationsProxyUT>("AssociationsProxy")->disassociationOnTimeout(dst, mode);
    }

    if (scheduledRAP.isValid())
    {
        createAssociation_req(scheduledRAP);
        setTimeout(timeout);
    }
}

int
AssociationHandlerUT::getMyDuplexGroup(int /*frameNr*/, bool /*isDL*/)
{
    return duplexGroup;
}
int
AssociationHandlerUT::getPeerDuplexGroup(wns::service::dll::UnicastAddress /*user*/)
{
    // this is my duplexGroup,
    // but we know that the peer's duplexGroup is the same
    return duplexGroup;
}

void
AssociationHandlerUT::associateTo(wns::service::dll::UnicastAddress rapAdr)
{
    assure(!(associationService->hasAssociation()), "UT has still association!");

    // plain association, must be called without association
    createAssociation_req(rapAdr);
    setTimeout(timeout);

    // enable re-association on timeout
    scheduledRAP = rapAdr;
}

void
AssociationHandlerUT::disassociate(wns::service::dll::UnicastAddress rapAdr)
{
    assure(associationService->hasAssociation(), "UT has not associated!");

    // no scheduled RAP, plain disassociation
    createDisassociation_req(rapAdr);
    setTimeout(timeout);
}

void
AssociationHandlerUT::switchAssociationTo(wns::service::dll::UnicastAddress rapAdr)
{
    assure(associationService->hasAssociation(), "UT has not associated!");

    // For Intra-Mode Handover, controlled by associationHandler self
    createDisassociation_req(rapAdr);
    setTimeout(timeout);

    // enable association direct after disassociation
    scheduledRAP = rapAdr;
}

void
AssociationHandlerUT::bestRAP(wns::service::dll::UnicastAddress destination)
{
    MESSAGE_SINGLE(NORMAL, logger, "bestRAP("<<A2N(destination)<<")");
    // Maybe report the Associationsproxy directly...
    assure(associationsProxy!=NULL,"invalid associationsProxy");
    associationsProxy->modeDetected(mode, destination);
}

void
AssociationHandlerUT::belowThreshold(wns::service::dll::UnicastAddress destination)
{
    // only if this is UT's active association
    if ((associationService->hasAssociation())
        && (associationService->getAssociation() == destination))
    {
        MESSAGE_SINGLE(NORMAL, logger, "Requesting Disassociation due to bad BCH");
        associationsProxy->disassociationNeeded(mode, destination);
    }
    else
        MESSAGE_SINGLE(NORMAL, logger, "Station in outage. Best BCH is below threshold.");
}

void
AssociationHandlerUT::createAssociation_req(wns::service::dll::UnicastAddress destination) const
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr associationCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    /** activate my command */
    AssociationCommand* outgoingCommand = this->activateCommand(associationCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::association_req();
    outgoingCommand->peer.src = layer2->getDLLAddress();
    outgoingCommand->peer.dst = destination;
    outgoingCommand->peer.user = layer2->getDLLAddress();
    outgoingCommand->peer.mode = mode;
    outgoingCommand->peer.duplexGroup = duplexGroup;
    outgoingCommand->magic.sender = this; // for debugging and assures

    // activate RLC command
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(associationCompound->getCommandPool()));
    rlcCommand->peer.flowID = 0; // over RACH we don't need a correct flowID. Anyway, we don't have it yet.

    /** send the compound */
    connector->activate(friends.rachDispatcher);

    if (getConnector()->hasAcceptor(associationCompound))
    {
        getConnector()->getAcceptor(associationCompound)->sendData(associationCompound);
        MESSAGE_SINGLE(NORMAL, logger, "Sent "
                       << mode+separator << "associationReq to "
                       << layer2->getStationManager()->getStationByMAC(outgoingCommand->peer.dst)->getName());
    }
    else
        assure(false, "Lower FU is not accepting scheduled associationReq compound but is supposed to do so");
}

void
AssociationHandlerUT::createDisassociation_req(wns::service::dll::UnicastAddress target) const
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr associationCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    wns::service::dll::UnicastAddress destinationAddress = associationService->getAssociation();

    /** activate and set MACg and command */
    lte::macg::MACgCommand* macgCommand = dynamic_cast<lte::macg::MACgCommand*>(macgReader->activateCommand(associationCompound->getCommandPool()));
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = destinationAddress;

    // activate RLC command
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(associationCompound->getCommandPool()));
    lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs =
        friends.flowManager->getControlPlaneFlowIDs(destinationAddress);
    rlcCommand->peer.flowID = _controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    connector->activate(friends.cpDispatcher);

    AssociationCommand* outgoingCommand = this->activateCommand(associationCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::disassociation_req();
    outgoingCommand->peer.src = layer2->getDLLAddress();
    outgoingCommand->peer.dst = associationService->getAssociation();
    outgoingCommand->peer.user = layer2->getDLLAddress();
    outgoingCommand->peer.targetRAP = target;
    outgoingCommand->peer.mode = mode;
    outgoingCommand->magic.sender = this; // for debugging and assures

    /** send the compound */
    if (getConnector()->hasAcceptor(associationCompound))
    {
        getConnector()->getAcceptor(associationCompound)->sendData(associationCompound);
        MESSAGE_SINGLE(NORMAL, logger, "Sent "
                       << mode+separator << "disassociationReq to "
                       << layer2->getStationManager()->getStationByMAC(outgoingCommand->peer.dst)->getName());
    }
    else
        assure(false, "Lower FU is not accepting scheduled disassociationReq compound but is supposed to do so");
}

void
AssociationHandlerUT::associate(wns::service::dll::UnicastAddress destination)
{
    dll::ILayer2* newDestination = layer2->getStationManager()->getStationByMAC(destination);

    associationService->associate(layer2, newDestination);

    // notify observers
    notifyOnAssociated(layer2->getDLLAddress(), destination);
}
