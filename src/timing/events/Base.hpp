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

#ifndef LTE_TIMING_EVENTS_BASE_HPP
#define LTE_TIMING_EVENTS_BASE_HPP

#include <LTE/helper/HasModeName.hpp>
#include <LTE/macr/PhyUser.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/events/EventFactory.hpp>
#include <WNS/Cloneable.hpp>

#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/ldk/fun/FUN.hpp>

namespace lte {

	namespace timing {
		class TimingScheduler;

		namespace events {

			/** @brief base class for the RAP task events during a super-/frame */
			class Base:
				public lte::helper::HasModeName
            //virtual public wns::CloneableInterface
			{
			protected:
				/** @brief the phase triggered by this event lasts for duration [seconds] */
				simTimeType duration;
				/** @brief needed for phyUser->setStateRxTx(Rx/Tx/BothRxTx) */
				lte::macr::IRxTxSettable* rxTxSetter;
				/** @brief only for timer->stationTaskAt() */
				lte::timing::TimingScheduler* timer;
				/** @brief store pointer to the FUN the event belongs to */
				wns::ldk::fun::FUN* fun;
				/** @brief for verbosity. This class is the basic cause of actions */
				wns::logger::Logger logger;
				/** @brief call phyUser->setStateRxTx with stateRxTx={ Tx, Rx, BothRxTx } */
				void
				setStateRxTx(lte::macr::PhyUser::StateRxTx stateRxTx);
			public:
				/** @brief used to inject to timer ptr */
				virtual void setTimer(lte::timing::TimingScheduler* _timer);
				/** @brief Constructor */
				Base(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);

				virtual void execute() = 0;

				virtual void operator()()
				{
					return this->execute();
				}

				/** @brief Destructor */
				virtual ~Base(){};
			};
		} //namespace events
	} //timing
} // lte

#endif // LTE_TIMING_EVENTS_BASE_HPP




