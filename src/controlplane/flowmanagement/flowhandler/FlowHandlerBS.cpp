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

#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandlerBS.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>
#include <LTE/macg/MACg.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/helper/Keys.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/service/dll/FlowID.hpp>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace lte::controlplane::flowmanagement::flowhandler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(FlowHandlerBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.flowmanagement.flowhandler.FlowHandlerBS",
                                     wns::ldk::FUNConfigCreator);

FlowHandlerBS::FlowHandlerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    FlowHandler(fun, config),
    wns::Cloneable<FlowHandlerBS>(),
    fun(fun)
{
}

void
FlowHandlerBS::onFUNCreated()
{
    FlowHandler::onFUNCreated();
    bsLayer2 = getFUN()->getLayer<dll::Layer2*>();
    assure(bsLayer2==layer2,"obsolete?");
    flowManagerBS = bsLayer2->getControlService<lte::controlplane::flowmanagement::FlowManagerBS>("FlowManagerBS");
    assure(flowManagerBS == flowManager,"obsolete?");

    if (mode == modeBase)
        macg = fun->findFriend<lte::macg::MACg*>("macg");
    else
        macg = fun->findFriend<lte::macg::MACg*>("macg"+separator+taskID);

    lowerFlowGate = getFUN()->findFriend<wns::ldk::FlowGate*>(mode+separator+"lowerFlowGate");
    assure(lowerFlowGate, "FlowHandlerHandler requires: "+mode+separator+"lowerFlowGate");
}

//confirms Flow_req and assigns new FlowID
void
FlowHandlerBS::flowConfirm(lte::helper::TransactionID _transactionId,
                           wns::service::dll::FlowID _flowID,
                           wns::service::dll::UnicastAddress _utAddress)
{
    assure(TransactionIDToDestination.knows(_transactionId), "There is no such TransactionID");
    createFlow_conf(_transactionId, TransactionIDToDestination.find(_transactionId), _flowID, _utAddress);
}

void
FlowHandlerBS::createFlow_conf(lte::helper::TransactionID _transactionId,
                               wns::service::dll::UnicastAddress _destination,
                               wns::service::dll::FlowID _flowID,
                               wns::service::dll::UnicastAddress _utAddress)
{
    // create closed Flow in gate
    lowerFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_flowID)));

    /** generate an empty PDU */
    wns::ldk::CompoundPtr FlowHandlerCompound = wns::ldk::CompoundPtr(
        new wns::ldk::Compound(
            getFUN()->createCommandPool()));

    /** activate MacG command */
    lte::macg::MACgCommand* macgCommand = macg->activateCommand(FlowHandlerCompound->getCommandPool());
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = _destination;

    /** activate RLC command */
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(FlowHandlerCompound->getCommandPool()));
    rlcCommand->peer.source = layer2->getDLLAddress();
    //rlcCommand->peer.destination = ?
    // using _destinationAddress as peerAddress here is not fully correct
    // we must have the next hop address here to get the DCCH FlowID
    FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs = flowManager->getControlPlaneFlowIDs(_destination);
    rlcCommand->peer.flowID = controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    FlowHandlerCommand* outgoingCommand = this->activateCommand(FlowHandlerCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::flow_confirm();
    outgoingCommand->peer.flowID = _flowID;
    outgoingCommand->peer.transactionId = _transactionId;
    outgoingCommand->peer.user = _utAddress;

    if (getConnector()->hasAcceptor(FlowHandlerCompound))
    {
        getConnector()->getAcceptor(FlowHandlerCompound)->sendData(FlowHandlerCompound);
        MESSAGE_SINGLE(NORMAL, logger, "Sending FlowConfirm for flowID="<< outgoingCommand->peer.flowID<<" to "<<A2N(_destination));
    }
    else
        assure(false, "BS: Lower FU is not accepting scheduled FlowConfirm compound but is supposed to do so");

    //delete transactionID entry from container:
    TransactionIDToDestination.erase(_transactionId);
}

void
FlowHandlerBS::doOnData(const wns::ldk::CompoundPtr& compound)
{
    lte::controlplane::flowmanagement::flowhandler::FlowHandlerCommand* incomingCommand = getCommand(compound->getCommandPool());
    lte::macg::MACgCommand* macgCommand = macg->getCommand(compound->getCommandPool());

    if (incomingCommand->peer.myCompoundType == CompoundType::flow_req())
    {
        MESSAGE_SINGLE(NORMAL, logger, "FlowRequest received from "<<A2N(incomingCommand->peer.user)<<" with TransactionID="<< incomingCommand->peer.transactionId);
        TransactionIDToDestination.insert(incomingCommand->peer.transactionId, macgCommand->peer.source);
        // call FlowManager
        flowManagerBS->forwardFlowRequest(incomingCommand->peer.transactionId,
                                          this,
                                          incomingCommand->peer.user,
                                          incomingCommand->peer.oldFlowID,
                                          incomingCommand->peer.qosClass);
    }

    else if (incomingCommand->peer.myCompoundType == CompoundType::flow_rel())
    {
        MESSAGE_SINGLE(NORMAL, logger, "FlowReleaseRequest received from "<<A2N(incomingCommand->peer.user)<<" for FlowID=" << incomingCommand->peer.flowID);

        FlowIDInToDestIn.insert(incomingCommand->peer.flowID, macgCommand->peer.source);
        flowManagerBS->releaseFlow(incomingCommand->peer.flowID);
        createFlowReleaseAck(incomingCommand->peer.flowID);
    }

    else if (incomingCommand->peer.myCompoundType == CompoundType::flow_ack())
    {
        MESSAGE_SINGLE(NORMAL, logger, "FlowAck received from "<<A2N(incomingCommand->peer.user)<<" for FlowID=" << incomingCommand->peer.flowID);
        // open lower FlowGate:
        lowerFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(incomingCommand->peer.flowID)));
        flowManagerBS->flowBuilt(incomingCommand->peer.flowID);
    }

    else
        assure(false, "received FlowHandlerCompound with wrong type!");

}

void
FlowHandlerBS::createFlowReleaseAck(wns::service::dll::FlowID _flowID)
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr FlowHandlerCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    wns::service::dll::UnicastAddress destinationAddress = FlowIDInToDestIn.find(_flowID);

    /** activate MACg command */
    lte::macg::MACgCommand* macgCommand = macg->activateCommand(FlowHandlerCompound->getCommandPool());
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = destinationAddress;

    /** activate RLC command */
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(FlowHandlerCompound->getCommandPool()));
    rlcCommand->peer.source = layer2->getDLLAddress();
    //rlcCommand->peer.destination = ?
    // using _destinationAddress as peerAddress here is not fully correct
    // we must have the next hop address here to get the ControlPlaneFlowID
    FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs = flowManager->getControlPlaneFlowIDs(destinationAddress);
    rlcCommand->peer.flowID = controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    FlowHandlerCommand* outgoingCommand = this->activateCommand(FlowHandlerCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType =  CompoundType::flow_rel_ack();
    outgoingCommand->peer.flowID = _flowID;

    if (getConnector()->hasAcceptor(FlowHandlerCompound))
    {
        getConnector()->getAcceptor(FlowHandlerCompound)->sendData(FlowHandlerCompound);
        MESSAGE_SINGLE(NORMAL, logger, "Sending FlowReleaseAck " << outgoingCommand->peer.flowID);
    }
    else
        assure(false, "BS: Lower FU is not accepting scheduled FlowReleaseAck compound but is supposed to do so");

    FlowIDInToDestIn.erase(_flowID);
}
