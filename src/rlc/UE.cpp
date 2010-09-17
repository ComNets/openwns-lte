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

#include <LTE/rlc/UE.hpp>
#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>

#include <DLL/Layer2.hpp>

using namespace lte::rlc;
using namespace wns::ldk;

STATIC_FACTORY_REGISTER_WITH_CREATOR(UERLC, FunctionalUnit, "lte.rlc.UE", FUNConfigCreator);

UERLC::UERLC(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
  wns::ldk::CommandTypeSpecifier<RLCCommand>(fun),
  wns::ldk::Processor<UERLC>(),
  destination(),
  logger(config.get("logger"))
{
    friends.flowswitching = NULL;
    upperConvergenceReader = NULL;
}

void
UERLC::onFUNCreated()
{
    upperConvergenceReader = getFUN()->getCommandReader("upperConvergence");

    wns::ldk::ControlServiceInterface* csi = getFUN()->getLayer<dll::ILayer2*>()->getControlService<wns::ldk::ControlServiceInterface>("FlowManagerUT");
    friends.flowswitching = dynamic_cast<lte::controlplane::flowmanagement::IFlowSwitching*>(csi);
    assureNotNull(friends.flowswitching);

    MESSAGE_SINGLE(VERBOSE, logger, "onFUNCreated(): complete");
}

UERLC::~UERLC()
{
}

void
UERLC::processOutgoing(const CompoundPtr& compound)
{
    assure(upperConvergenceReader, "No reader for upper convergence set!");
    dll::UpperCommand* upper = upperConvergenceReader->readCommand<dll::UpperCommand>(compound->getCommandPool());
    assure(upper, "Erroneous Upper Convergence Command!");

    RLCCommand* command = activateCommand(compound->getCommandPool());
    assure(command, "No RLC-Command set!");

    command->local.direction = PacketDirection::UPLINK();
    command->peer.source = getFUN()->getLayer<dll::ILayer2*>()->getDLLAddress();
    command->peer.destination = destination;

    command->peer.flowID = upper->local.dllFlowID;
    command->rang.flowID = 0;

    wns::service::qos::QoSClass qosClass = friends.flowswitching->getQoSClassForUTFlowID(upper->local.dllFlowID);
    command->peer.qosClass = qosClass;

    MESSAGE_SINGLE(NORMAL, logger, "processOutgoing(): FlowID="<<command->peer.flowID<<",QoS="<<lte::helper::QoSClasses::toString(qosClass)<<". Sending to eNB: "<< destination);

} // processOutgoing

void
UERLC::processIncoming(const CompoundPtr& compound)
{
}

CommandPool*
UERLC::createReply(const CommandPool* original) const
{
    MESSAGE_BEGIN(NORMAL, logger, m, "createReply");
    MESSAGE_END();

    CommandPool* commandPool = getFUN()->createCommandPool();
    RLCCommand* inRLCCommand = getCommand(original);
    RLCCommand* outRLCCommand = activateCommand(commandPool);

    outRLCCommand->local.direction = PacketDirection::UPLINK();
    outRLCCommand->peer.source = inRLCCommand->peer.destination;
    outRLCCommand->peer.destination = inRLCCommand->peer.source;
    outRLCCommand->peer.flowID = inRLCCommand->peer.flowID;
    outRLCCommand->rang.flowID = 0; // ControlPlaneFlowID

    MESSAGE_SINGLE(NORMAL, logger, " inRLCCommand FlowID: "<<inRLCCommand->peer.flowID);

    return commandPool;
} // createReply

void
UERLC::setDestination(wns::service::dll::UnicastAddress dst)
{
    destination = dst;
}

wns::service::dll::UnicastAddress
UERLC::getDestination()
{
    return destination;
}
