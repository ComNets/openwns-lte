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

#include <LTE/controlplane/associationHandler/AssociationHandler.hpp>

#include <DLL/StationManager.hpp>
#include <DLL/Layer2.hpp>

#include <WNS/service/dll/StationTypes.hpp>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::controlplane::associationHandler;

AssociationHandler::AssociationHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
  wns::ldk::CommandTypeSpecifier<AssociationCommand>(fun),
  wns::ldk::HasReceptor<>(),
  wns::ldk::HasConnector<lte::helper::SwitchConnector>(),
  wns::ldk::HasDeliverer<>(),
  helper::HasModeName(config),
  fun(fun),
  layer2(fun->getLayer<dll::ILayer2*>()),
  associationService(NULL),
  connector(dynamic_cast<lte::helper::SwitchConnector*>(getConnector())),
  commandSize(config.get<Bit>("commandSize")),
  flowSeparatorNames(),
  logger(config.get("logger"))
{
  for(int i = 0; i < config.len("flowSeparators"); ++i)
    {
      std::string fsName = config.get<std::string>("flowSeparators", i);
      flowSeparatorNames.push_back(fsName);

      MESSAGE_BEGIN(NORMAL, logger, m, " " );
      m << "added flowseparatorname to flowseperator: " << fsName;
      MESSAGE_END();
    }

  friends.rachDispatcher = NULL;
  friends.bchBuffer = NULL;
  friends.cpDispatcher = NULL;
  macgReader = NULL;
  rlcReader = NULL;
  friends.flowManager = NULL;

  // post myself as an AssociationInfo provider to the Layer2
  layer2->addAssociationInfoProvider(mode, this);
}

AssociationHandler::~AssociationHandler()
{
  layer2 = NULL;
}

void
AssociationHandler::onFUNCreated()
{
  associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);

  if (mode == modeBase)
    macgReader = fun->getCommandReader("macg");
  else
    macgReader = fun->getCommandReader("macg"+separator+taskID);

  //get flow separators from config
  for(std::list<std::string>::iterator iter = flowSeparatorNames.begin(); iter != flowSeparatorNames.end(); ++iter)
    {
      wns::ldk::FlowSeparator* flowSeparator = fun->findFriend<wns::ldk::FlowSeparator*>(*iter);
      flowSeparators.push_back(flowSeparator);
    }

  std::string flowmanagername="FlowManager";

  if (layer2->getStationType() == wns::service::dll::StationTypes::UE()) {
    flowmanagername += "UT";
  } else { // BS
    flowmanagername += "BS";
  }
  // in the BS we don't activate the connector since we don't send anything

  friends.flowManager = layer2->getControlService<lte::controlplane::flowmanagement::IFlowSwitching>(flowmanagername);
  assure(friends.flowManager, "FlowManager not set.");

  rlcReader = fun->getCommandReader("rlc");
}

void
AssociationHandler::doSendData(const wns::ldk::CompoundPtr&)
{
  assure(false, "Should never be called!");
}

bool
AssociationHandler::doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const
{
  return false;
}

void
AssociationHandler::doWakeup()
{}

void
AssociationHandler::calculateSizes(const wns::ldk::CommandPool* commandPool,
				   Bit& commandPoolSize,
				   Bit& dataSize) const
{
  fun->calculateSizes(commandPool, commandPoolSize, dataSize, this);

  // add my commandSize. No Data to add.
  commandPoolSize += this->commandSize;
}

void
AssociationHandler::notifyOnAssociated(wns::service::dll::UnicastAddress user,
				       wns::service::dll::UnicastAddress dst)
{
  MESSAGE_SINGLE(NORMAL, logger, "notifyOnAssociated(from "<<A2N(user)<<" to "<<A2N(dst)<<")");
  dll::services::control::AssociationFunctor doNotification(&dll::services::control::AssociationObserverInterface::onAssociated,
							    user,
							    dst);
  this->forEachObserver(doNotification);
}

void
AssociationHandler::notifyOnDisassociated(wns::service::dll::UnicastAddress user,
					  wns::service::dll::UnicastAddress dst)
{
  MESSAGE_SINGLE(NORMAL, logger, "notifyOnDisassociated(from "<<A2N(user)<<" to "<<A2N(dst)<<")");
  dll::services::control::AssociationFunctor doNotification(&dll::services::control::AssociationObserverInterface::onDisassociated,
							    user,
							    dst);
  this->forEachObserver(doNotification);
}
