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

#include <LTE/controlplane/bch/BCHUnit.hpp>
#include <LTE/timing/ResourceScheduler.hpp>
#include <LTE/controlplane/associationHandler/AssociationHandler.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>
#include <LTE/rlc/RLCCommand.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/services/control/Association.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/helper/FakePDU.hpp>

using namespace lte;
using namespace lte::controlplane::bch;
using namespace wns;
using namespace wns::ldk;
using namespace wns::ldk::fun;

STATIC_FACTORY_REGISTER_WITH_CREATOR(LTEBCHUnitRAP, FunctionalUnit, "lte.controlplane.BCHUnit.RAP", FUNConfigCreator);
STATIC_FACTORY_REGISTER_WITH_CREATOR(LTEBCHUnitUT, FunctionalUnit, "lte.controlplane.BCHUnit.UT", FUNConfigCreator);
STATIC_FACTORY_REGISTER_WITH_CREATOR(NoBCH, FunctionalUnit, "lte.controlplane.BCHUnit.No", FUNConfigCreator);


LTEBCHUnit::LTEBCHUnit(wns::ldk::fun::FUN* fun, const pyconfig::View& config) :
  CommandTypeSpecifier<LTEBCHCommand>(fun),
  HasReceptor<>(),
  HasConnector<>(),
  HasDeliverer<>(),
  lte::helper::HasModeName(config),

  logger(config.get("logger")),
  layer2(NULL),
  commandSize(config.get<int>("commandSize"))
{
    friends.scheduler = NULL;
    friends.macg = NULL;

    rlcReader = NULL;
}


LTEBCHUnitRAP::LTEBCHUnitRAP(wns::ldk::fun::FUN* fun, const pyconfig::View& config) :
  LTEBCHUnit(fun, config),
  Cloneable<LTEBCHUnitRAP>(),
  txPower(config.get<wns::Power>("txPower")),
  accepting(false),
  compound(CompoundPtr()),
  rlcName(config.get<std::string>("rlcName")),
  macgName(config.get<std::string>("macgName")),
  flowManagerName(config.get<std::string>("flowManagerName"))
{
}

LTEBCHUnitUT::LTEBCHUnitUT(wns::ldk::fun::FUN* fun, const pyconfig::View& _config) :
  LTEBCHUnit(fun, _config),
  Cloneable<LTEBCHUnitUT>(),

  bchService(NULL),
  associationService(NULL),
  config(_config),
  sinrProbe(NULL),
  rxpProbe(NULL),
  interfProbe(NULL),
  schedulerName(config.get<std::string>("schedulerName"))
{
  MESSAGE_SINGLE(NORMAL, logger, "LTEBCHUnit for mode " << modeBase << " started.");
}

LTEBCHUnit::~LTEBCHUnit()
{
  layer2 = NULL;
}

LTEBCHUnitRAP::~LTEBCHUnitRAP()
{
  compound = CompoundPtr();
}

LTEBCHUnitUT::~LTEBCHUnitUT()
{
  associationService = NULL;
  bchService = NULL;

  delete sinrProbe; sinrProbe = NULL;
  delete rxpProbe; rxpProbe = NULL;
  delete interfProbe; interfProbe = NULL;
}

void
LTEBCHUnit::onFUNCreated()
{
    layer2 = getFUN()->getLayer<dll::ILayer2*>();
}

void
LTEBCHUnitRAP::onFUNCreated()
{
    LTEBCHUnit::onFUNCreated();
    rlcReader = getFUN()->getCommandReader(rlcName);
    friends.macg = getFUN()->findFriend<lte::macg::MACg*>(macgName);
}

void
LTEBCHUnitUT::onFUNCreated()
{
  LTEBCHUnit::onFUNCreated();

  bchService = layer2->getControlService<lte::controlplane::bch::BCHService>("BCHSERVICE"+separator+mode);
  associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);

  wns::node::Interface* node = layer2->getNode();
  wns::probe::bus::ContextProviderCollection localIDs(&node->getContextProviderCollection());

  friends.scheduler = getFUN()->findFriend<lte::timing::ResourceScheduler*>(schedulerName);

  // read the localIDs from the config
  for (int ii = 0; ii<config.len("localIDs.keys()"); ++ii)
    {
      std::string key = config.get<std::string>("localIDs.keys()",ii);
      uint32_t value  = config.get<uint32_t>("localIDs.values()",ii);
      localIDs.addProvider(wns::probe::bus::contextprovider::Constant(key, value));
      MESSAGE_SINGLE(VERBOSE, logger, "Using Local IDName '"<<key<<"' with value: "<<value);
    }
  sinrProbe   = new wns::probe::bus::ContextCollector(localIDs, "lte.BCH_SINR");
  rxpProbe    = new wns::probe::bus::ContextCollector(localIDs, "lte.BCH_RxPower");
  interfProbe = new wns::probe::bus::ContextCollector(localIDs, "lte.BCH_Interference");
}


bool
LTEBCHUnitRAP::doIsAccepting(const CompoundPtr& /* compound */) const
{
  return accepting;
} // isAccepting

bool
LTEBCHUnitUT::doIsAccepting(const CompoundPtr& /* compound */) const
{
  return false;
}

void
LTEBCHUnitRAP::doSendData(const CompoundPtr& _compound)
{
  LTEBCHCommand* myCommand;
  if (compound == CompoundPtr()) // if first Association ACK
    {
      compound = CompoundPtr(new Compound(getFUN()->createCommandPool()));
      myCommand = activateCommand(compound->getCommandPool());
      myCommand->magic.source = layer2->getDLLAddress();
      myCommand->peer.acknowledgement = true;
      myCommand->peer.ackedUTs.push_back(_compound);
    }
  else // use the existing BCH compound and just append the Association ACK into it
    {
      myCommand = getCommand(compound->getCommandPool());
      myCommand->peer.ackedUTs.push_back(_compound);
    }
} // doSendData

void
LTEBCHUnitUT::doSendData(const CompoundPtr&)
{}

void
LTEBCHUnitRAP::doOnData(const CompoundPtr&)
{
}

void
LTEBCHUnitUT::doOnData(const CompoundPtr& compound)
{
  /** process incoming BCH transmission */
    lte::timing::SchedulerCommand* schedulerCommand = friends.scheduler->getCommand(compound->getCommandPool());
  LTEBCHCommand* myCommand = getCommand(compound->getCommandPool());

  wns::service::dll::UnicastAddress comingFrom = myCommand->magic.source;
  /** Inform the LTEBCHService about the Measurement */
  assure(bchService, "LTEBCHUnit unexpectedly received a BCH compound.");
  bchService->storeMeasurement(comingFrom, schedulerCommand->local.phyMeasurementPtr, schedulerCommand->local.distance, schedulerCommand->local.subBand);

  { // Probe measurement Values
    assure(associationService, "Trying to probe BCH Measurement in a RAP.");

    if (associationService->hasAssociation())
      if (associationService->getAssociation() == comingFrom)
	{
        wns::service::phy::power::PowerMeasurementPtr phyMeasurementPtr = schedulerCommand->local.phyMeasurementPtr;
	  sinrProbe->put(compound, phyMeasurementPtr->getSINR().get_dB());
	  rxpProbe->put(compound, phyMeasurementPtr->getRxPower().get_dBm());
	  interfProbe->put(compound, phyMeasurementPtr->getInterferencePower().get_dBm());
	}
  }

  if (myCommand->peer.acknowledgement)
    {
      assure(!myCommand->peer.ackedUTs.empty(), "BCH compound does not contain Association ACKs, but should do so!");
      // send the Association ACKs tp the AssociationHandler FU. The filtering by the MAC address will be done there
      for(std::list<wns::ldk::CompoundPtr>::const_iterator iter = myCommand->peer.ackedUTs.begin(); iter != myCommand->peer.ackedUTs.end(); ++iter)
	{
	  getDeliverer()->getAcceptor(compound)->onData(*iter);
	}
      myCommand->peer.ackedUTs.clear();
    }
  // else the compound is dropped here
} // doOnData

void
LTEBCHUnitRAP::doWakeup()
{
  getReceptor()->wakeup();
} // wakeup

void
LTEBCHUnitUT::doWakeup()
{
}

void
LTEBCHUnit::calculateSizes(const CommandPool* commandPool, Bit& _commandPoolSize, Bit& _dataSize) const
{
  getFUN()->getProxy()->calculateSizes(commandPool, _commandPoolSize, _dataSize, this);
  _commandPoolSize += this->commandSize;
}

void LTEBCHUnitRAP::sendBCH(simTimeType duration)
{
  assure(rlcReader, "You may not use the  'sendBCH' Event without an RLC in your FUN");

  MESSAGE_SINGLE(NORMAL, logger, "sendBCH(): Creating BCH");

  accepting = true;
  // if there are pending Association ACKs in the BCH buffer, they will be stored in peer.ackedUTs now
  this->wakeup();
  accepting = false;

  if (compound == CompoundPtr()) //no compound from associationHandler FU
    {
      compound = CompoundPtr(new Compound(getFUN()->createCommandPool(),
                                          wns::ldk::helper::FakePDUPtr(new wns::ldk::helper::FakePDU(1))));
      LTEBCHCommand* myCommand = activateCommand(compound->getCommandPool());
      myCommand->magic.source = layer2->getDLLAddress();
      myCommand->peer.acknowledgement = false;
      myCommand->peer.ackedUTs.clear();
    }

    // set RLC Command
    lte::rlc::RLCCommand* rlcCommand = dynamic_cast<lte::rlc::RLCCommand*>(rlcReader->activateCommand(compound->getCommandPool()));

    // Downlink
    rlcCommand->local.direction = 2;
    rlcCommand->peer.source = layer2->getDLLAddress();
    rlcCommand->peer.destination = wns::service::dll::UnicastAddress();
    rlcCommand->peer.qosClass = lte::helper::QoSClasses::PBCH();
    rlcCommand->peer.flowID = layer2->getControlService<lte::controlplane::flowmanagement::FlowManager>(flowManagerName)->getBCHFlowID();

    // set MACg Command
    lte::macg::MACgCommand* macgCommand =
        dynamic_cast<lte::macg::MACgCommand*>(getFUN()->getProxy()->activateCommand(compound->getCommandPool(), friends.macg ));
    macgCommand->peer.source = layer2->getDLLAddress();
    macgCommand->peer.dest = wns::service::dll::UnicastAddress();

  if (getConnector()->hasAcceptor(compound)){
    MESSAGE_SINGLE(NORMAL, logger, "sendBCH(): BCH created");
    /** @todo: convert int numbers modulation, coding to string (operator<<) */
    //MESSAGE_SINGLE(NORMAL, logger, "sendBCH(): BCH created (PhyMode " << modulation << "-" << coding <<")");
    getConnector()->getAcceptor(compound)->sendData(compound);
  }
  else
      assure(false, "Lower FU is not accepting scheduled PDU but is supposed to do so");

  compound = CompoundPtr();
}
