/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#include <LTE/rlc/eNB.hpp>
#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>

#include <DLL/UpperConvergence.hpp>
#include <DLL/StationManager.hpp>
#include <DLL/Layer2.hpp>

using namespace lte::rlc;

STATIC_FACTORY_REGISTER_WITH_CREATOR(ENBRLC,
				     wns::ldk::FunctionalUnit,
				     "lte.rlc.eNB",
				     wns::ldk::FUNConfigCreator);


ENBRLC::ENBRLC(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
  wns::ldk::CommandTypeSpecifier<RLCCommand>(fun),
  wns::ldk::Processor<ENBRLC>(),
  logger(config.get("logger"))
{
    friends.flowswitching = NULL;
    upperConvergenceReader = NULL;
} // ENBRLC


ENBRLC::~ENBRLC()
{
}

void
ENBRLC::onFUNCreated()
{
    dll::ILayer2* layer2 = getFUN()->getLayer<dll::ILayer2*>();

    upperConvergenceReader = getFUN()->getCommandReader("upperConvergence");

    /**
     * @todo dbn: lterelease: Enable flow manager when it is available
     */
    // friends.flowswitching = layer2->getControlService<lte::controlplane::flowmanagement::IFlowSwitching>("FlowManagerBS");

    MESSAGE_SINGLE(VERBOSE, logger, "onFUNCreated(): complete");
}

void
ENBRLC::processIncoming(const wns::ldk::CompoundPtr& compound)
{
    lte::rlc::RLCCommand* command = getCommand(compound->getCommandPool());
    command->rang.flowID =  friends.flowswitching->getFlowIDout(command->peer.flowID);
} // processIncoming


void
ENBRLC::processOutgoing(const wns::ldk::CompoundPtr& compound)
{
    assure(upperConvergenceReader, "No reader for upper convergence set!");
    dll::UpperCommand* upper = upperConvergenceReader->readCommand<dll::UpperCommand>(compound->getCommandPool());
    assure(upper, "Erroneous Upper Convergence Command!");

    RLCCommand* command = activateCommand(compound->getCommandPool());
    wns::service::dll::FlowID flowID = friends.flowswitching->getFlowIDin(upper->local.dllFlowID /* RANG-to-BS */);
    wns::service::qos::QoSClass qosClass = friends.flowswitching->getQoSClassForBSFlowID(flowID);
    command->local.direction = PacketDirection::DOWNLINK();
    command->peer.source = getFUN()->getLayer<dll::ILayer2*>()->getDLLAddress();
    command->peer.destination = upper->peer.targetMACAddress;

    MESSAGE_SINGLE(NORMAL, logger, "processOutgoing(): incoming FlowID(DL)="<<upper->local.dllFlowID<<", outgoing FlowID="<<flowID<<", QoS="<<lte::helper::QoSClasses::toString(qosClass));
    command->peer.qosClass = qosClass;
    command->peer.flowID = flowID;
    command->rang.flowID = 0;
} // processOutgoing

wns::ldk::CommandPool*
ENBRLC::createReply(const wns::ldk::CommandPool* original) const
{
    wns::ldk::CommandPool* commandPool = getFUN()->createCommandPool();
    RLCCommand* inRLCCommand = getCommand(original);
    RLCCommand* outRLCCommand = activateCommand(commandPool);

    outRLCCommand->local.direction = PacketDirection::DOWNLINK();
    outRLCCommand->peer.source = getFUN()->getLayer<dll::ILayer2*>()->getDLLAddress();
    outRLCCommand->peer.destination = inRLCCommand->peer.source;

    outRLCCommand->peer.flowID = inRLCCommand->peer.flowID;
    outRLCCommand->rang.flowID = 0; // ControlPlaneFlowID

    return commandPool;
} // createReply
