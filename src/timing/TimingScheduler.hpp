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

#ifndef LTE_TIMING_TIMINGSCHEDULER_HPP
#define LTE_TIMING_TIMINGSCHEDULER_HPP

#include <LTE/helper/HasModeName.hpp>
/* deleted by chen */
// #include <LTE/controlplane/MapHandler.hpp>
/* inserted by chen */
#include <DLL/services/control/Association.hpp>

#include <WNS/ldk/ManagementServiceInterface.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/pyconfig/Sequence.hpp>
#include <WNS/events/PeriodicTimeout.hpp>
#include <WNS/events/EventFactory.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/Enum.hpp>
#include <WNS/container/Registry.hpp>

#include <WNS/service/dll/Address.hpp>

#include <vector>
#include <cstdlib>

namespace lte {
    namespace controlplane {
        class MapHandler; /* inserted by chen */
        namespace associationHandler {
            class AssociationHandler;
        }
    }
    namespace timing {
        namespace events {
            class Base;
        }

        /** @brief StationTask is used for StationTaskPhase.
         * BSs are always RAPs, UTs are always UTs, RN toggle between RAP and	UT */
        // e.g. lte::timing::StationTasks::RAP()
        ENUM_BEGIN(StationTasks);
        ENUM(RAP, 1);
        ENUM(UT, 2);
        ENUM(IDLE, 3);
        ENUM(INVALID, 4);
        ENUM_END();
        typedef int StationTask;

        /** @brief DuplexScheme: TDD or FDD */
        ENUM_BEGIN(DuplexSchemes);
        ENUM(TDD, 1);
        ENUM(FDD, 2);
        ENUM_END();
        typedef int DuplexScheme;

        class FrameStartNotificationInterface
        {
        public:
            virtual
            ~FrameStartNotificationInterface(){}

            virtual void
            onFrameStart() = 0;
        };

        class SuperFrameStartNotificationInterface
        {
        public:
            virtual
            ~SuperFrameStartNotificationInterface(){}

            virtual void
            onSuperFrameStart() = 0;
        };

        /** @brief The TimingScheduler takes care of the superframe timing by
         * keeping a list of phases, for each of which an event is generated.
         * The superframe start is notified periodically by a discrete event */
        class TimingScheduler :
                public wns::ldk::ManagementService,
                public wns::events::PeriodicTimeout,
                public lte::helper::HasModeName,
                public dll::services::control::AssociationObserver,
                public wns::Subject<FrameStartNotificationInterface>,
                public wns::Subject<SuperFrameStartNotificationInterface>
        {
            friend class controlplane::associationHandler::AssociationHandler;

        public:
            TimingScheduler(wns::ldk::ManagementServiceRegistry* msr,
                            wns::pyconfig::View _config);

            virtual
            ~TimingScheduler();

            /** @brief  SuperFrameStart - Periodically Interface from class PeriodicTimeout */
            virtual void periodically();

            /** @brief  to get info about inferior stations' availability */
            bool isPeerListeningAt(const wns::service::dll::UnicastAddress& peerAddress, const int frameNr);

            /** @brief  is the peer reachable now? -> Can the relay node receive
             * the map?*/
            bool canReceiveMapNow(const wns::service::dll::UnicastAddress& address);

            /** @brief To create events which have intra-fun dependencies. */
            void onMSRCreated();

            /** @brief get peer TimingScheduler in this phase */
            void onWorldCreated();

            /** @brief used to find out whether we are in RAP or UT
             * phase. Static for BS and UT, dynamic for RN. The offset is
             * counted relative to the current simtime
             */
            lte::timing::StationTask
            stationTaskAtOffset(const simTimeType offset) const;

            /** @brief same as stationTaskAtOffset but with parameter absolute frameNr */
            lte::timing::StationTask
            stationTaskAtFrame(int frameNr) const;

            /** @brief read in phases (before use of phaseNumber() functions) */
            void
            initStationTaskPhases();

            /** @brief get phaseNumber at time offset, compared to now */
            uint32_t
            phaseNumberAtOffset(const simTimeType offset) const;

            /** @brief get phaseNumber at absolute frame number */
            uint32_t
            phaseNumberAtFrame(int frameNr) const;

            virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr,
                                         wns::service::dll::UnicastAddress dstAdr);

            virtual void onAssociated(wns::service::dll::UnicastAddress userAdr,
                                      wns::service::dll::UnicastAddress dstAdr);

            void
            superFrameTrigger();

            void
            frameTrigger();

            /** @brief get duplex group number (1=HD1,2=HD2)
                only to be used in RAP (BS|RN)! */
        private:
            dll::ILayer2* layer2;
            wns::ldk::fun::FUN* fun;
            wns::pyconfig::View config;
            wns::logger::Logger logger;

            /** @brief friends are rlc, macg, phyUser. We need direct access to them */
            struct Friends {
                lte::controlplane::MapHandler* mapHandler;
            } friends;

            /** @brief for SuperFrame managing */
            simTimeType superFrameLength;
            /** @brief begin of current SuperFrame */
            simTimeType superFrameStartTime;
            /** @brief for event scheduling */
            struct TimingEvent {
                simTimeType timeOffset;
                lte::timing::events::Base* event;
            };
            typedef std::vector<TimingEvent> EventContainer;
            struct StationTaskPhase {
                simTimeType startTime; // relative to superframe start
                simTimeType duration; // [s]
                StationTask task;
            };
            typedef std::vector<StationTaskPhase> StationTaskPhaseContainer;
            typedef wns::container::Registry<wns::service::dll::UnicastAddress, EventContainer> PeerTiming;
            typedef wns::container::Registry<wns::service::dll::UnicastAddress, TimingScheduler*> PeerTimingSchedulers;

            EventContainer eventContainer;
            StationTaskPhaseContainer stationTaskPhaseContainer;
            PeerTiming peerTiming;
            PeerTimingSchedulers peerTimingSchedulers;

            wns::events::scheduler::Interface* es;

            /** @brief Helper function to create the list of events. Needs to be
             * re-called after alterations to the taskPhases */
            void readEvents(std::string viewName);
            /** @brief Helper function to read a list of stationTaskPhases. */
            void readStationTaskPhases(std::string viewName);

            /** @brief Only used by AssociationDecision */
            void
            addPeerTimingScheduler(wns::service::dll::UnicastAddress peerAddress, TimingScheduler* _timingScheduler);
            void
            removePeerTimingScheduler(wns::service::dll::UnicastAddress peerAddress);

            simTimeType startOfFirstFrame;
            simTimeType frameLength;
            int schedulingOffset;
            int numberOfFramesToSchedule;
            int framesPerSuperFrame;
        public:
            /** @brief for event SwitchingPoint  */
            simTimeType switchingPointOffset;
            /** @brief "TDD" or "FDD" */
            lte::timing::DuplexScheme duplex;
            int getSchedulingOffset() const;
            int getNumberOfFramesToSchedule() const;
        }; // TimingScheduler
    } //timing
} // lte

#endif // NOT defined LTE_TIMING_TIMINGSCHEDULER_HPP
