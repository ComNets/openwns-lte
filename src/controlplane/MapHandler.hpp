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

#ifndef LTE_CONTROLPLANE_MAPHANDLER_HPP
#define LTE_CONTROLPLANE_MAPHANDLER_HPP

#include <LTE/controlplane/MapHandlerInterface.hpp>
#include <LTE/timing/partitioning/PartitioningInfo.hpp>
#include <LTE/helper/HasModeName.hpp>

#include <DLL/services/control/Association.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/scheduler/MapInfoProviderInterface.hpp>
#include <WNS/scheduler/MapInfoEntry.hpp>
#include <WNS/scheduler/SchedulingMap.hpp>
#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/Subject.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/pyconfig/View.hpp>

namespace wns { namespace node { class Node; }}

namespace lte {
    namespace timing {
        class TimingScheduler;
        class SchedulerIncoming;
    }
    namespace macr {
        class PhyUser;
    }
    namespace controlplane {

        typedef std::vector<wns::scheduler::MapInfoCollectionPtr> MapInfoCollectionVector;
        typedef std::vector<wns::scheduler::SchedulingMapPtr> SchedulingMapCollectionVector;

        /** @brief content of a MAP controlplane packet: DL and UL map */
        class MapCommand
                : public wns::ldk::Command
        {
        public:
            MapCommand()
            {
            };
            struct Local {
            } local;
            /** @brief A MAP contains SchedulingMaps for DL and UL for distinct frameNumbers.
                frameNumbers contains the list of these numbers.
                Also the SchedulingMapCollectionVectors contain empty SmartPtrs at other frameNumbers.
                => frameNumbers could be obsolete? */
            struct Friends {
                std::vector<int> ulFrameNumbers;
                std::vector<int> dlFrameNumbers;

                SchedulingMapCollectionVector dlSchedulingMapVector;
                SchedulingMapCollectionVector ulSchedulingMapVector;
            } peer;
            struct Magic {
                wns::service::dll::UnicastAddress source;
            } magic;
        };

        /**
         * @brief Creates Map compounds and tries to deliver them.
         */
        class MapHandler :
                public virtual wns::ldk::FunctionalUnit,
                public wns::ldk::CommandTypeSpecifier<MapCommand>,
                public wns::ldk::HasReceptor<>,
                public wns::ldk::HasConnector<>,
                public wns::ldk::HasDeliverer<>,
                public wns::Cloneable<MapHandler>,
                public lte::helper::HasModeName,
                public wns::Subject<ResourceGrantNotificationInterface>,
                public IMapHandlerTiming,
		public IMapHandlerRS
        {
        public:

            MapHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
            virtual ~MapHandler();
            virtual void onFUNCreated();

            /** @brief compound handler interface */
            virtual void doSendData(const wns::ldk::CompoundPtr& /* compound */);
            virtual void doOnData(const wns::ldk::CompoundPtr& /* compound */);

            /** @brief UT asks for allowed resources (old method?). timing/ut/Events.cpp */
            virtual wns::scheduler::SchedulingMapPtr
            getMasterMapForSlaveScheduling(int frameNr);

            /** @brief PDU size information */
            virtual void calculateSizes(const wns::ldk::CommandPool* commandPool, Bit& _pciSize, Bit& _pduSize) const;

            /** @brief triggered by event startMap */
            void startMapTx(simTimeType duration, std::vector<int> dlFrameNumbers, std::vector<int> ulFrameNumbers);
            /** @brief triggered by event startMap */
            void startMapRx();
            /** @brief get+write statistics for prepared maps (on BS side) */
            void prepareMapOutput();
            void evaluateMaps();
            void closeMapOutput();

            /** @brief To provide the received Map Info inside the FUN */
            //wns::scheduler::MapInfoCollectionPtr getTxResources(int frameNr);

            /** @brief return free SubChannelSet to the RS */
            //lte::timing::SubChannelRangeSet getFreeDLResources(int frameNr, uint32_t groupNumber);
            //lte::timing::SubChannelRangeSet getFreeULResources(int frameNr, uint32_t groupNumber);

            // fetch scheduled Resources from RS and safe into SuperFrame map
            //void saveMap(int frameNr);
            /** @brief put scheduled resources from RS and save into SuperFrame map */
            virtual void
            saveDLMap(int frameNr, wns::scheduler::SchedulingMapPtr schedulingMap);
            /** @brief put scheduled resources from RS and save into SuperFrame map */
            virtual void
            saveULMap(int frameNr, wns::scheduler::SchedulingMapPtr schedulingMap);

            //return resourceDedication to RS
            //std::string getResourceDedication(int frameNr);

            //return scheduling information to the setExpectation method of RS
            //wns::scheduler::MapInfoCollectionPtr getFreeDLResources(int frameNr);
            //wns::scheduler::MapInfoCollectionPtr getFreeULResources(int frameNr);

            void resetResources(int frameNr);
            void setCurrentPhase();
        private:
            virtual bool doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;
            virtual void doWakeup();
            /** @brief clear all scheduling information once at the very beginning */
            void initSuperFrameMap();

            dll::ILayer2* layer2;
            wns::node::Interface* myUser;

            struct Friends {
                Friends() {dlrs=NULL; timer=NULL; phyUser=NULL; associationService=NULL; dlmip=NULL; ulmip=NULL;};
                lte::timing::SchedulerIncoming* dlrs;
                lte::timing::TimingScheduler* timer;
                lte::macr::PhyUser* phyUser;
                dll::services::control::Association* associationService;
                wns::scheduler::SchedulingMapProviderInterface* dlmip;
                wns::scheduler::SchedulingMapProviderInterface* ulmip;
            } friends;

            uint32_t pduSize;
            uint32_t lowestSubChannel;
            uint32_t highestSubChannel;
            int framesPerSuperFrame;

            /** @brief free resources (old format) */
            //std::vector<lte::timing::SubChannelRangeSet> freeDLResources;
            //std::vector<lte::timing::SubChannelRangeSet> freeULResources;
            /** @brief free resources (new format) */
            //std::vector<wns::scheduler::UsableSubChannelVector> dlUsableSubChannels;
            //std::vector<wns::scheduler::UsableSubChannelVector> ulUsableSubChannels;
            /** @brief SuperFrame map entries; index=frameNumber.
                initialized as vector[0..framesPerSuperFrame-1] */
            // old format:
            //MapInfoCollectionVector scheduledDLResources;
            //MapInfoCollectionVector scheduledULResources;
            // new format:
            SchedulingMapCollectionVector scheduledDLResources;
            SchedulingMapCollectionVector scheduledULResources;
            /** @brief resourceDedication (string "Feeder" or "Universal").
                index=frameNr. */
            std::vector<std::string> resourceDedication;

            lte::timing::partitioning::PartitioningInfo* partInfo;

            /** @brief modulation&coding used to transmit MAP */
            wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;
            /** @brief power used to transmit MAP */
            wns::Power txPower;
            /** @brief output file names */
            bool writeMapOutput;
            std::string mapOutputFileNameDL;
            std::string mapOutputFileNameUL;
            std::ofstream *mapFileDL;
            std::ofstream *mapFileUL;
            uint32_t currentPhase;
            wns::logger::Logger logger;
        };
    }} // namespace lte::controlplane


#endif // NOT defined LTE_CONTROLPLANE_MAPHANDLER_HPP


