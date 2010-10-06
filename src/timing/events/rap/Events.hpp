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

#ifndef LTE_TIMING_EVENTS_RAP_EVENTS_HPP
#define LTE_TIMING_EVENTS_RAP_EVENTS_HPP

#include <LTE/timing/events/Base.hpp>

namespace lte {
    namespace controlplane {
        namespace bch {
            class IBCHTimingTx;
        }
        class IMapHandlerTiming;
    }

    namespace macr {
        class IRachTimingRx;
    }

    namespace timing {
        class MasterScheduler;
        class SlaveScheduler;
        class SchedulerIncoming;

        namespace events {
            namespace rap {

                /** @brief base class for RAP events to allow having them in a
                 * static factory of their own */
                class EventBase :
                    public lte::timing::events::Base
                {
                public:
                    /** @brief Forwarding Constructor */
                    EventBase(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
                        lte::timing::events::Base(_fun, config)
                        {};
                    virtual ~EventBase(){};
                };


                class StartBCH :
                    public EventBase
                //public wns::Cloneable<StartBCH>
                {
                    lte::controlplane::bch::IBCHTimingTx* bch;
                public:
                    StartBCH(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
                    virtual void execute();
                }; //StartBCH


                class StartRACH :
                    public EventBase
                //public wns::Cloneable<StartRACH>
                {
                    lte::macr::IRachTimingRx* rach;
                public:
                    StartRACH(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
                    virtual void execute();
                }; //StartRACH


                class StartMap :
                    public EventBase
                //public wns::Cloneable<StartMap>
                {
                    lte::timing::MasterScheduler* rstx; // downlink ResourceScheduler
                    lte::timing::MasterScheduler* rsrx; // uplink ResourceScheduler
                    lte::controlplane::IMapHandlerTiming* mapHandler;
                    int frameNr;
                    int framesPerSuperFrame;
                    bool useMapResourcesInUL;
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
                        lte::timing::MasterScheduler* rsrx;
                        lte::controlplane::IMapHandlerTiming* mapHandler;
                    public:
                        SwitchingPoint(wns::ldk::fun::FUN* _fun,
                                       const wns::pyconfig::View& config);
//                      virtual ~SwitchingPoint(){};
                        virtual void execute();
                        int frameNr;
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
//                      virtual ~StopData(){};
                        virtual void execute();
                    }; // StopData

                    lte::timing::MasterScheduler* rstx;
                    lte::timing::MasterScheduler* rsrx;
                    SwitchingPoint mySwitchingPointEvent;
                    StopData myStopDataEvent;
                    lte::controlplane::IMapHandlerTiming* mapHandler;
                    int frameNr;
                    bool useMapResourcesInUL;
                public:
                    StartData(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
                    virtual ~StartData();
                    //virtual wns::CloneableInterface* clone() const;

                    /** @brief Overloads setTimer from Eventbase because the pointer needs to be
                     *  passed to the internal events
                     */
                    virtual void setTimer(lte::timing::TimingScheduler* _timer);
                    virtual void execute();
                }; // StartData

                typedef wns::ldk::FUNConfigCreator<EventBase> EventCreator;
                typedef wns::StaticFactory<EventCreator> EventFactory;

            } // namespace rap
        } // namespace events
    }// namespace timing
}// namespace lte

#endif // LTE_TIMING_EVENTS_RAP_EVENTS_HPP
