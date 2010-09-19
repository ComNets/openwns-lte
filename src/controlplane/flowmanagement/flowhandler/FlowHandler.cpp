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

#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandler.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>
#include <LTE/macg/MACg.hpp>
#include <LTE/helper/Keys.hpp>

#include <DLL/StationManager.hpp>
#include <DLL/Layer2.hpp>

#include <WNS/service/dll/StationTypes.hpp>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace lte::controlplane::flowmanagement::flowhandler;

FlowHandler::FlowHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
  wns::ldk::CommandTypeSpecifier<FlowHandlerCommand>(fun),
  wns::ldk::HasReceptor<>(),
  wns::ldk::HasDeliverer<>(),
  helper::HasModeName(config),
  fun(fun),
  layer2(fun->getLayer<dll::ILayer2*>()),
  macg(NULL),
  rlcReader(NULL),
  flowManager(NULL),
  associationService(NULL),
  logger(config.get("logger")),
  commandSize(config.get<Bit>("commandSize")),
  lowerFlowGate(NULL),
  flowSeparatorNames()
{
  for(int i = 0; i < config.len("flowSeparators"); ++i)
    {
      std::string fsName = config.get<std::string>("flowSeparators", i);
      flowSeparatorNames.push_back(fsName);

      MESSAGE_BEGIN(NORMAL, logger, m, " " );
      m << "Added Flowseparator: " << fsName;
      MESSAGE_END();
    }
}

FlowHandler::~FlowHandler()
{
}

void
FlowHandler::onFUNCreated()
{
  associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);
  assure(associationService, "AssociationService not set.");
  //get flow separators from config
  for(std::list<std::string>::iterator iter = flowSeparatorNames.begin(); iter != flowSeparatorNames.end(); ++iter)
    {
      MESSAGE_SINGLE(NORMAL, logger, "Added FlowSeparator for: "<< *iter);
      wns::ldk::FlowSeparator* flowSeparator = fun->findFriend<wns::ldk::FlowSeparator*>(*iter);
      flowSeparators.push_back(flowSeparator);
    }

  std::string flowmanagername="FlowManager";
  if (layer2->getStationType() == wns::service::dll::StationTypes::UE()) {
    flowmanagername += "UT";
  } else { // BS
    flowmanagername += "BS";
  }
  flowManager = layer2->getControlService<lte::controlplane::flowmanagement::FlowManager>(flowmanagername);
  assure(flowManager, "FlowManager not set.");
  rlcReader = fun->getCommandReader("rlc");
  assure(rlcReader, "rlc not set.");

  dll::services::control::Association* associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);

  // Register as Observer at the association info provider
  // Do not register in RN in UT task. Would lead to double calls to FlowManagerRN::onDisassociated()
  // We want to be informed about a completed disassociationAck, so that we can remove the controlPlaneFlowIds.
  if (!((layer2->getStationType() == wns::service::dll::StationTypes::FRS()) && (taskID == "UT"))) {
    MESSAGE_SINGLE(NORMAL, logger, "Starting to observe AssociationInfoProvider. taskID=\""<<taskID<<"\", mode=\""<<mode<<"\"");
    //dll::Layer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(modeBase);
    dll::ILayer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(mode);
    dll::ILayer2::AssociationInfoContainer::const_iterator iter = ais.begin();
    for (; iter != ais.end(); ++iter)
      this->startObserving(*iter);
  } else {
    MESSAGE_SINGLE(NORMAL, logger, "Refusing to observe AssociationInfoProvider. taskID=\""<<taskID<<"\"");
  }
} // onFUNCreated

void
FlowHandler::doSendData(const wns::ldk::CompoundPtr&)
{
  assure(false, "Should never be called!");
}

bool
FlowHandler::doIsAccepting(const wns::ldk::CompoundPtr&  /*compound*/) const
{
  return true;
}

void
FlowHandler::doWakeup()
{}

void
FlowHandler::calculateSizes(const wns::ldk::CommandPool* commandPool,
			    Bit& commandPoolSize,
			    Bit& dataSize) const
{
  fun->calculateSizes(commandPool, commandPoolSize, dataSize, this);

  // add my commandSize. No Data to add.
  commandPoolSize += this->commandSize;
}

void
FlowHandler::destroyFlow(wns::service::dll::FlowID flowID)
{
  wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(flowID));

  // delete instances of flowSeparators
  for (std::list<wns::ldk::FlowSeparator*>::iterator iter = flowSeparators.begin();
       iter != flowSeparators.end(); ++iter)
    {
      if ((*iter)->getInstance(key) != NULL)
	{
	  (*iter)->removeInstance(key);
	  lowerFlowGate->destroyFlow(key);
	  MESSAGE_SINGLE(NORMAL, logger, "Deleted lower FlowSeparator and FlowGate for FlowID="<< flowID);
	}
      else
	{
	  MESSAGE_SINGLE(NORMAL, logger, "No instances to delete found for FlowID="<< flowID);
	}
    }
}

void
FlowHandler::closeFlow(wns::service::dll::FlowID flowID)
{
  wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(flowID));
  lowerFlowGate->closeFlow(key);
  MESSAGE_SINGLE(NORMAL, logger, "Lower Flow closed for FlowID: "<< flowID);
}

void
FlowHandler::onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
  assure(flowManager, "FlowManager not set.");
  flowManager->onAssociated(userAdr,dstAdr);
}

// called from AckOnAirCallback::callback() triggered by PhyUser sending the very last "AssociationAck" packet
void
FlowHandler::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
  MESSAGE_SINGLE(NORMAL, logger, "onDisassociated(from "<<A2N(userAdr)<<" to "<<A2N(dstAdr)<<")");
  assure(flowManager, "FlowManager not set.");
  if (dstAdr == layer2->getDLLAddress()) { // we are meant (dstAdr=RAP)
    MESSAGE_SINGLE(NORMAL, logger, "onDisassociated: calling flowManager->onDisassociated");
    flowManager->onDisassociated(userAdr,dstAdr);
  } else if (userAdr == layer2->getDLLAddress()) { // we are meant (userAdr==UT)
    assure(layer2->getStationType() == wns::service::dll::StationTypes::UE(),"wrong stationType");
    MESSAGE_SINGLE(NORMAL, logger, "onDisassociated: calling flowManager->onDisassociated");
    flowManager->onDisassociated(userAdr,dstAdr);
  } else {
    // we are BS in a Multihop case (dstAdr=via=RN) and do not delete the controlPlaneFlowId between BS and RN
    // therefore we don't call flowManager->onDisassociated(userAdr,dstAdr);
    //MESSAGE_SINGLE(NORMAL, logger, "onDisassociated(from "<<A2N(userAdr)<<" via "<<A2N(dstAdr)<<")");
  }
}
