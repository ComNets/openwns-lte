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

#ifndef LTE_TIMING_EVENTS_UT_EVENTS_HPP
#define LTE_TIMING_EVENTS_UT_EVENTS_HPP

#include <LTE/timing/events/Base.hpp>

namespace lte {
	namespace controlplane {
		class IMapHandlerTiming;
	}

	namespace macr {
		class IRachTimingTx;
	}

	namespace timing {
		class SlaveScheduler;
		class SchedulerIncoming;

		namespace events {
			namespace ut {

				/** @brief base class for RAP events to allow having them in a
				 * static factory of their own */
				class EventBase :
					public events::Base
				{
				public:
					/** @brief Forwarding Constructor */
					EventBase(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
						events::Base(_fun, config)
						{};
					virtual ~EventBase(){};
				};


				class StartBCH :
					public EventBase
                //public wns::Cloneable<StartBCH>
				{
				public:
					StartBCH(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
					virtual void execute();
				}; //StartBCH


				class StartRACH :
					public EventBase
                //public wns::Cloneable<StartRACH>
				{
					lte::macr::IRachTimingTx* rach;
				public:
					StartRACH(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
					virtual void execute();
				}; //StartRACH


				class StartMap :
					public EventBase
                //public wns::Cloneable<StartMap>
				{
					int frameNr;
					bool useMapResourcesInUL;
					lte::controlplane::IMapHandlerTiming* mapHandler;
					lte::timing::SlaveScheduler* rstx;
				public:
					StartMap(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
					virtual void execute();
				}; //StartMap


				class StartData :
					public EventBase
                //public wns::Cloneable<StartData>
				{
					/** @brief This event is not configured externally. It will
					 * automatically be generated at the end of the Rx Phase
					 */
					class SwitchingPoint :
						public EventBase
                    //public wns::Cloneable<SwitchingPoint>
					{
						lte::timing::SlaveScheduler* rstxSlave;
						lte::timing::SchedulerIncoming* rstxIncoming;
						lte::controlplane::IMapHandlerTiming* mapHandler;
						int frameNr;
					public:
						SwitchingPoint(wns::ldk::fun::FUN* _fun,
									   const wns::pyconfig::View& config);
						virtual void execute();
					}; // SwitchingPoint

					/** @brief This event is not configured externally. It will
					 * automatically be generated at the end of the Data Phase
					 */
					class StopData :
						public EventBase
                    //public wns::Cloneable<StopData>
					{
						lte::timing::SchedulerIncoming* rstx;
					public:
						StopData(wns::ldk::fun::FUN* _fun,
								 const wns::pyconfig::View& config);
						virtual void execute();
					}; // StopData

					lte::timing::SlaveScheduler* rstx;
					lte::controlplane::IMapHandlerTiming* mapHandler;
					SwitchingPoint mySwitchingPointEvent;
					StopData myStopDataEvent;
					int frameNr;
					int framesPerSuperFrame;
					bool useMapResourcesInUL;
				public:
					StartData(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
					virtual ~StartData();

					/**
					 * @brief need to make deep copies of the contained events
					 * when cloning
					 */
// 					virtual wns::CloneableInterface*
//                     clone() const;

					/** @brief Overloads setTimer from Eventbase because the pointer needs to be
					 *  passed to the internal events
					 */
					virtual void setTimer(lte::timing::TimingScheduler* _timer);
					virtual void execute();
				}; // StartData

				typedef wns::ldk::FUNConfigCreator<EventBase> EventCreator;
				typedef wns::StaticFactory<EventCreator> EventFactory;

			} //namespace ut
		} //namespace events
	} //timing
} // lte

#endif // LTE_TIMING_EVENTS_UT_EVENTS_HPP




