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

#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandlerUT.hpp>
#include <LTE/macg/MACg.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/helper/Keys.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>

#include <WNS/StaticFactory.hpp>
#include <DLL/Layer2.hpp>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace lte::controlplane::flowmanagement::flowhandler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.flowmanagement.flowhandler.FlowHandlerUT",
                                     wns::ldk::FUNConfigCreator);

FlowHandlerUT::FlowHandlerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    FlowHandler(fun, config),
    wns::Cloneable<FlowHandlerUT>(),
    timeout(config.get<double>("timeout")),
    commandname(config.get<std::string>("commandname")),
    fun(fun)
{
}

void
FlowHandlerUT::onFUNCreated()
{
    FlowHandler::onFUNCreated();
    utLayer2 = fun->getLayer<dll::Layer2*>();
    assure(utLayer2==layer2,"obsolete?");
    flowManagerUT = utLayer2->getControlService<lte::controlplane::flowmanagement::FlowManagerUT>("FlowManagerUT");
    assure(flowManagerUT == flowManager,"obsolete?");

    if (mode == modeBase)
        macg = fun->findFriend<lte::macg::MACg*>("macg");
    else
        fun->findFriend<lte::macg::MACg*>("macg"+separator+taskID);

    lowerFlowGate = getFUN()->findFriend<wns::ldk::FlowGate*>(mode+separator+"lowerFlowGate");
    assure(lowerFlowGate, "FlowHandlerHandler required: "+mode+separator+"lowerFlowGate");
}

void
FlowHandlerUT::createFlow(wns::service::dll::FlowID flowID)
{
    lowerFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(flowID)));
    lowerFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(flowID)));
}

void
FlowHandlerUT::doOnData(const wns::ldk::CompoundPtr& compound)
{
    lte::macg::MACgCommand* macgCommand = macg->getCommand(compound->getCommandPool());

    lte::controlplane::flowmanagement::flowhandler::FlowHandlerCommand* incomingCommand = getCommand(compound->getCommandPool());
    if (incomingCommand->peer.myCompoundType == CompoundType::flow_confirm())
    {
        MESSAGE_SINGLE(NORMAL, logger, "FlowConfirm received for TransactionID="<< incomingCommand->peer.transactionId<<" with FlowID="<< incomingCommand->peer.flowID);

        //return ACK!
        createFlow_ack(incomingCommand->peer.flowID, macgCommand->peer.source);

        // inform FlowManager
        flowManagerUT->flowBuilt(incomingCommand->peer.transactionId, incomingCommand->peer.flowID);

        // create open Flow in gates
        createFlow(incomingCommand->peer.flowID);

        // delete if an old flowID:
        if(FlowIDOutToDestOut.knows(incomingCommand->peer.flowID))
            FlowIDOutToDestOut.erase(incomingCommand->peer.flowID);

        FlowIDOutToDestOut.insert(incomingCommand->peer.flowID, macgCommand->peer.source);

    }
    else if (incomingCommand->peer.myCompoundType == CompoundType::flow_rel_ack())
    {
        MESSAGE_SINGLE(NORMAL, logger, "FlowReleaseAck received for FlowID=" << incomingCommand->peer.flowID);
        flowReleased(incomingCommand->peer.flowID);
    }

    else //unknown CompoundType
    {
        MESSAGE_SINGLE(NORMAL, logger, "Unknown compound received. Type: " << incomingCommand->peer.myCompoundType);
        assure(false, "Received FlowHandler Compound with wrong type!");
    }
}

void
FlowHandlerUT::onTimeout()
{

    MESSAGE_SINGLE(NORMAL, logger, "FlowRequest TimeOut reached");
}

void
FlowHandlerUT::flowReq(lte::helper::TransactionID _transactionId, wns::service::dll::UnicastAddress _destinationAddress, wns::service::dll::FlowID oldFlowID, wns::service::qos::QoSClass qosClass)
{
    createFlow_req(_transactionId, _destinationAddress, oldFlowID, qosClass);
}

void
FlowHandlerUT::createFlow_req(lte::helper::TransactionID _transactionId, wns::service::dll::UnicastAddress _destinationAddress, wns::service::dll::FlowID oldFlowID, wns::service::qos::QoSClass qosClass) const
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr FlowHandlerCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    /** activate MACg command */
    lte::macg::MACgCommand* macgCommand = macg->activateCommand(FlowHandlerCompound->getCommandPool());
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = _destinationAddress;

    /** activate RLC command */
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(FlowHandlerCompound->getCommandPool()));
    rlcCommand->peer.source = layer2->getDLLAddress();
    //rlcCommand->peer.destination = ?
    // using _destinationAddress as peerAddress here is not fully correct
    // we must have the next hop address here to get the ControlPlaneFlowID
    FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs = flowManager->getControlPlaneFlowIDs(_destinationAddress);
    rlcCommand->peer.flowID = controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    FlowHandlerCommand* outgoingCommand = this->activateCommand(FlowHandlerCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::flow_req();
    outgoingCommand->peer.transactionId = _transactionId;
    outgoingCommand->peer.user = layer2->getDLLAddress();
    outgoingCommand->peer.oldFlowID = oldFlowID;
    outgoingCommand->peer.qosClass = qosClass;

    if (getConnector()->hasAcceptor(FlowHandlerCompound))
    {
        getConnector()->getAcceptor(FlowHandlerCompound)->sendData(FlowHandlerCompound);

        MESSAGE_SINGLE(NORMAL, logger, "Sending FlowRequest with TransactionID="<<outgoingCommand->peer.transactionId<<" to "<<A2N(_destinationAddress));
    }
    else
        assure(false, "UT: Lower FU is not accepting scheduled FlowRequest compound but is supposed to do so");
}

void
FlowHandlerUT::createFlow_ack(wns::service::dll::FlowID _flowID, wns::service::dll::UnicastAddress _destinationAddress) const
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr FlowHandlerCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    /** activate MACg command */
    lte::macg::MACgCommand* macgCommand = macg->activateCommand(FlowHandlerCompound->getCommandPool());
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = _destinationAddress;

    /** activate RLC command */
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(FlowHandlerCompound->getCommandPool()));
    rlcCommand->peer.source = layer2->getDLLAddress();
    //rlcCommand->peer.destination = ?
    // using _destinationAddress as peerAddress here is not fully correct
    // we must have the next hop address here to get the ControlPlaneFlowID
    FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs = flowManager->getControlPlaneFlowIDs(_destinationAddress);
    rlcCommand->peer.flowID = controlPlaneFlowIDs[lte::helper::QoSClasses::DCCH()];

    /** activate my command */
    FlowHandlerCommand* outgoingCommand = this->activateCommand(FlowHandlerCompound->getCommandPool());
    outgoingCommand->peer.myCompoundType = CompoundType::flow_ack();
    outgoingCommand->peer.flowID = _flowID;
    outgoingCommand->peer.user = layer2->getDLLAddress();

    if (getConnector()->hasAcceptor(FlowHandlerCompound))
    {
        getConnector()->getAcceptor(FlowHandlerCompound)->sendData(FlowHandlerCompound);

        MESSAGE_SINGLE(NORMAL, logger, "Sending FlowAck for FlowID=" << outgoingCommand->peer.flowID);
    }
    else
        assure(false, "UT: Lower FU is not accepting scheduled FlowAck compound but is supposed to do so");
}

void
FlowHandlerUT::createFlowReleaseReq(wns::service::dll::FlowID _flowID)
{
    /** generate an empty PDU */
    wns::ldk::CompoundPtr FlowHandlerCompound =
        wns::ldk::CompoundPtr(new wns::ldk::Compound(getFUN()->createCommandPool()));

    wns::service::dll::UnicastAddress destinationAddress = FlowIDOutToDestOut.find(_flowID);

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
    outgoingCommand->peer.myCompoundType = CompoundType::flow_rel();
    outgoingCommand->peer.flowID = _flowID;
    outgoingCommand->peer.user = layer2->getDLLAddress();

    if (getConnector()->hasAcceptor(FlowHandlerCompound))
    {
        getConnector()->getAcceptor(FlowHandlerCompound)->sendData(FlowHandlerCompound);

        MESSAGE_SINGLE(NORMAL, logger, "Sending FlowReleaseRequest for FlowID=" << outgoingCommand->peer.flowID);
    }
    else
        assure(false, "UT: Lower FU is not accepting scheduled FlowRelease_Request compound but is supposed to do so");
}

void
FlowHandlerUT::releaseFlow(wns::service::dll::FlowID flowID)
{
    //send FlowReleaseRequest to next RAP
    createFlowReleaseReq(flowID);
}

void
FlowHandlerUT::flowReleased(wns::service::dll::FlowID flowID)
{
    //destroyFlow(flowID);
    // Flow released -> inform FlowManager
    flowManagerUT->flowReleased(flowID);
}
