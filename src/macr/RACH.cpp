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

#include <LTE/macr/RACH.hpp>
#include <LTE/macr/PhyUser.hpp>
#include <LTE/controlplane/RRHandler.hpp>
#include <LTE/controlplane/bch/BCHService.hpp>
#include <LTE/main/Layer2.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/services/control/Association.hpp>
#include <WNS/StaticFactory.hpp>

using namespace lte::macr;

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::macr::RACHUT,
				     wns::ldk::FunctionalUnit,
				     "lte.macr.RACH.UT",
				     wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::macr::RACHBS,
				     wns::ldk::FunctionalUnit,
				     "lte.macr.RACH.BS",
				     wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::macr::RACHShortcutUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.macr.RACH.ShortcutUT",
                                     wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::macr::RACHShortcutBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.macr.RACH.ShortcutBS",
                                     wns::ldk::FUNConfigCreator);

RACH::RACH(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
	wns::ldk::CommandTypeSpecifier<>(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<>(),
	helper::HasModeName(config),
	associationService(NULL),
	phyModePtr(wns::service::phy::phymode::createPhyMode(config.getView("phyMode"))),
	logger(config.get("logger"))
{
	MESSAGE_SINGLE(NORMAL, logger, "RACHUnit: will use PhyMode " << *phyModePtr );
}

RACH::~RACH()
{}

void
RACH::onFUNCreated()
{
	wns::ldk::fun::FUN* fun = getFUN();

	dll::ILayer2* layer2 = fun->getLayer<dll::ILayer2*>();
	associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);

	// The phyUser is needed to set its command
	friends.phyUser = fun->findFriend<lte::macr::PhyUser*>(modeBase+separator+"phyUser");
}

void
RACHUT::onFUNCreated()
{
	RACH::onFUNCreated();
	rrh = getFUN()->findFriend<lte::controlplane::RRHandlerUT*>(mode+separator+"RRHandler");
	bchService = NULL;
}

void
RACHBS::doSendData(const wns::ldk::CompoundPtr&)
{
	MESSAGE_SINGLE(NORMAL, logger, "doSendData()");
	assure(false, "RACHBS: sendData should never be called!");
}

void
RACHUT::doSendData(const wns::ldk::CompoundPtr& compound)
{
	// set PhyUser Command
	lte::macr::PhyCommand* phyCommand =
		dynamic_cast<lte::macr::PhyCommand*>(
			getFUN()->getProxy()->activateCommand( compound->getCommandPool(),
							       friends.phyUser ));

	simTimeType startTime = wns::simulator::getEventScheduler()->getTime(); // now

	phyCommand->local.beamforming = false;
	phyCommand->local.pattern = wns::service::phy::ofdma::PatternPtr(); // NULL Pointer
	phyCommand->local.start = startTime;
	phyCommand->local.stop = stopTime;
	phyCommand->local.subBand = subBandCounter++;
	phyCommand->local.modeRxTx = lte::macr::PhyCommand::Tx;
	phyCommand->local.phyModePtr = phyModePtr;
	phyCommand->magic.destination = NULL;
	phyCommand->magic.source = getFUN()->getLayer<dll::ILayer2*>()->getNode();
	phyCommand->magic.txp = txPower;


	if (getConnector()->hasAcceptor(compound)){
		assure(phyModePtr->dataRateIsValid(),"invalid PhyMode dataRate");
		MESSAGE_SINGLE(NORMAL, logger, "sent RACH compound ("<< *phyModePtr <<")");
		getConnector()->getAcceptor(compound)->sendData(compound);
	}
	else
	    assure(false, "Lower FU is not accepting scheduled PDU but is supposed to do so");

}

void
RACHBS::doOnData(const wns::ldk::CompoundPtr& compound)
{
	MESSAGE_SINGLE(NORMAL, logger, "doOnData()");
	getDeliverer()->getAcceptor(compound)->onData(compound);
}

void
RACHUT::doOnData(const wns::ldk::CompoundPtr&)
{
	MESSAGE_SINGLE(NORMAL, logger, "doOnData()");
	assure(false, "RACHUT: onData should never be called!");
}

void
RACHUT::startTx(simTimeType duration)
{
	if (bchService == NULL)
		bchService = getFUN()->getLayer<dll::ILayer2*>()->getControlService<lte::controlplane::bch::BCHService>("BCHSERVICE_"+modeBase);

	MESSAGE_SINGLE(NORMAL, logger, "startTx(): RACH opened");

	// We have to wait for a RACH reception first
	lte::controlplane::bch::BCHRecordPtr best = bchService->getBest().entry;

	// if there is no 'best', i.e. no BCH at all:
	if (best == lte::controlplane::bch::BCHRecordPtr()) {
		MESSAGE_SINGLE(NORMAL, logger, "startTx(): never received a BCH");
		return;
	}

	assure(friends.phyUser, "You may not use the  'RACH::startTx' method without a PhyUser in your FUN");
	assure(friends.phyUser->checkIdle(), "PhyUser not ready!");

	// when RACH Tx starts, the phy has to be switched to UT-like operation
	friends.phyUser->utTuning();

	if (!rrh->hasResourcesGranted()) {
		MESSAGE_SINGLE(NORMAL, logger, "startTx(): RACH triggers createRRCompound...");
		rrh->createRRCompound();
	}
	subBandCounter = best->subBand;
	accepting = true;
	wns::events::scheduler::Interface* es = wns::simulator::getEventScheduler();
	stopTime = es->getTime()+duration;
	this->wakeup();

	// queue stopEvent
	es->schedule(StopEvent(this), stopTime);
}

void
RACHUT::stopTx()
{
	accepting = false;
}

void
RACHBS::startRx(simTimeType /*duration*/)
{
	assure(friends.phyUser, "You may not use the  'RACH::startRx' method without a PhyUser in your FUN");
	assure(friends.phyUser->checkIdle(), "PhyUser not ready!");

	// when RACH Rx starts, the phy has to be switched to RAP-like operation
	friends.phyUser->rapTuning();
}

void
RACHBS::stopRx()
{
}

bool
RACHBS::doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const
{
	return false;
} // doIsAccepting

bool
RACHUT::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
	// Check whether we are outside the RAP phase
	if (accepting == false) return false;

	// how much of the overall RACH duration is left?
	simTimeType duration = stopTime - wns::simulator::getEventScheduler()->getTime();
	assure(duration >= 0, "RACH Timing mismatch.");

	// Check whether the packet can be transmitted in the remaining time of
	// this RACH phase
	assure(phyModePtr->dataRateIsValid(),"!dataRateIsValid");
	int capacity = phyModePtr->getBitCapacityFractional(duration);
	bool compoundFits = (capacity >= compound->getLengthInBits());

	if (compoundFits == true)
	{
		return true;
	}
	else
	{
		MESSAGE_BEGIN(NORMAL, logger, m, "Deferring RACH compound until next RACH phase (not enough capacity left).");
		m << " Capacity needed=" << compound->getLengthInBits();
		m << ", capacity available=" << capacity;
		MESSAGE_END();
		return false;
	}
} // doIsAccepting


void
RACHBS::doWakeup()
{} // doWakeup

void
RACHUT::doWakeup()
{
	if(!inWakeup) {
		inWakeup = true;
		getReceptor()->wakeup();
		inWakeup = false;
	}
} // doWakeup

RACHShortcut::RACHShortcut(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config):
    wns::ldk::CommandTypeSpecifier<>(fun),
    wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, RACHShortcut*>(fun, config)
{
}

RACHShortcut::~RACHShortcut()
{
}

wns::service::dll::UnicastAddress
RACHShortcut::getSourceAddress()
{
    lte::main::Layer2* layer = dynamic_cast<lte::main::Layer2*>(getFUN()->getLayer());
    assure(layer!=NULL, "No layer 2 found.");
    return  layer->getDLLAddress();
}

wns::service::dll::UnicastAddress
RACHShortcut::getDestinationAddress(const wns::ldk::CompoundPtr&)
{
    assure(false, "All RACH compounds are broadcast. Cannot get a Destination adddress");
}

bool
RACHShortcut::isBroadcast(const wns::ldk::CompoundPtr&)
{
    return true;
}

RACHShortcutBS::RACHShortcutBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config):
    RACHShortcut(fun, config)
{
}

bool
RACHShortcutBS::isReceiver()
{
    return true;
}

RACHShortcutUT::RACHShortcutUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config):
    RACHShortcut(fun, config)
{
}

bool
RACHShortcutUT::isReceiver()
{
    return false;
}
