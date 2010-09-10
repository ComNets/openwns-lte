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

#include <WNS/ldk/FUNConfigCreator.hpp>

#include <LTE/timing/events/Base.hpp>
#include <LTE/timing/TimingScheduler.hpp>
/* deleted by chen */
// #include <LTE/macr/PhyUser.hpp>
// #include <LTE/timing/ResourceScheduler.hpp>
// #include <LTE/controlplane/MapHandler.hpp>
// #include <LTE/macr/RACH.hpp>

using namespace lte::timing;
using namespace lte::timing::events;

events::Base::Base(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
	HasModeName(config),
	duration(config.get<simTimeType>("duration")),
    rxTxSetter(_fun->findFriend<lte::macr::IRxTxSettable*>(modeBase+separator+"phyUser")),
	timer(NULL),
	fun(_fun),
	logger(config.get("logger"))
{
}

void
events::Base::setTimer(lte::timing::TimingScheduler* _timer)
{
	timer = _timer;
}

void
events::Base::setStateRxTx(lte::macr::IRxTxSettable::StateRxTx stateRxTx)
{
	assure(timer, "Timer has not been set in Event!");
	if (timer->duplex == lte::timing::DuplexSchemes::FDD())
	{
		rxTxSetter->setStateRxTx(lte::macr::IRxTxSettable::BothRxTx);
		MESSAGE_SINGLE(NORMAL, logger, "setState=FDD");
	}else if (timer->duplex == lte::timing::DuplexSchemes::TDD())
		rxTxSetter->setStateRxTx(stateRxTx);
	else
		assure(0, "undefined duplex mode");
}





