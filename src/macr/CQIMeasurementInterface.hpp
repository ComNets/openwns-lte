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

#ifndef LTE_MACR_CQIMEASUREMENTINTERFACE_HPP
#define LTE_MACR_CQIMEASUREMENTINTERFACE_HPP

#include <LTE/helper/HasModeName.hpp>

#include <WNS/ldk/CommandReaderInterface.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>

#include <WNS/service/phy/ofdma/Measurements.hpp>
#include <WNS/service/phy/ofdma/MeasurementHandler.hpp>
#include <WNS/service/phy/power/OFDMAMeasurement.hpp>

#include <WNS/service/phy/power/PowerMeasurement.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>
#include <WNS/distribution/Distribution.hpp>
#include <WNS/PowerRatio.hpp>
#include <DLL/services/control/Association.hpp>

/* deleted by chen */
// namespace dll {
//     class StationManager;
// }
namespace lte {
//     namespace controlplane {
//         class CQIHandler;
//     }
    namespace macr {
//         class PhyUser;

/* deleted by chen */
//         class CQIMeasurement :
//                 virtual public wns::ldk::FunctionalUnit,
//                 public wns::ldk::CommandTypeSpecifier<>,
//                 public wns::ldk::Processor<CQIMeasurement>,
//                 public wns::ldk::HasReceptor<>,
//                 public wns::ldk::HasConnector<>,
//                 public wns::ldk::HasDeliverer<>,
//                 public wns::Cloneable<CQIMeasurement>,
//                 public lte::helper::HasModeName,
//                 virtual public wns::service::phy::ofdma::MeasurementHandler
/* inserted by chen */
        class CQIMeasurement

        {
/* deleted by chen */
            /** @brief pointer to the phy command reader */
//             wns::ldk::CommandReaderInterface* phyCommandReader;
//             /** @brief mapping of ordered phyModes and SINR ranges */
//             wns::service::phy::phymode::PhyModeMapperInterface* phyModeMapper;
// 
//             /** @brief Association service access is needed to correctly
//              * address the packets */
//             dll::services::control::Association* associationService;
// 
//             /** @brief pointer to the CQI Handler */
//             lte::controlplane::CQIHandler* cqi;
// 
//             /** @brief pointer to the station Manager info service */
//             dll::StationManager* stationManager;
// 
//             /** @brief do we need add measurement noise to the measured value? */
//             bool noiseEnabled;
// 
//             /** @brief the Distribution which is used to create noise value[dB] */
//             wns::distribution::Distribution* noiseDistribution;
// 
//             /** @brief if we want to have measurements of broadcast compounds.*/
//             bool useBroadcastPDU;
// 
//             /** @brief if we want to have measurements of broadcast compounds,  we should
//                 compensate interference for those broadcast transmissions (The easiest way is just to use the
//                 measurement of last unicast )*/
//             wns::Power lastValidInterference;
// 
//             /** @brief service from the PHY layer (measurements from OFDMAPhy) */
//             wns::service::phy::ofdma::Measurements* measurementService;
// 
//             /** @brief my Logger */
//             wns::logger::Logger logger;

/* deleted by chen */
//         public:
//             CQIMeasurement(wns::ldk::fun::FUN* fun,
//                            const wns::pyconfig::View& config);
// 
//             virtual
//             ~CQIMeasurement();

//             virtual void
//             onFUNCreated();
// 
//             virtual void
//             processIncoming(const wns::ldk::CompoundPtr& compound);
// 
//             virtual void
//             processOutgoing(const wns::ldk::CompoundPtr&){}
// 
//             virtual void
//             setMeasurementService(wns::service::Service* phy);
// 
//             virtual wns::service::phy::ofdma::Measurements*
//             getMeasurementService() const;
// 
//             virtual void
//             onMeasurementUpdate(wns::node::Interface* source, wns::service::phy::power::OFDMAMeasurementPtr rxPowerMeasurement);

/* inserted by chen */
        public:

/* deleted by chen */
//             virtual void
//             onFUNCreated() = 0;
// 
//             virtual void
//             processIncoming(const wns::ldk::CompoundPtr& compound) = 0;
// 
//             virtual void
//             processOutgoing(const wns::ldk::CompoundPtr&) = 0;

            virtual void
            setMeasurementService(wns::service::Service* phy) = 0;

/* deleted by chen */
//             virtual wns::service::phy::ofdma::Measurements*
//             getMeasurementService() const = 0;
// 
//             virtual void
//             onMeasurementUpdate(wns::node::Interface* source, wns::service::phy::power::OFDMAMeasurementPtr rxPowerMeasurement) = 0;
        };

    } }

#endif // not defined LTE_MACR_CQIMEASUREMENTINTERFACE_HPP


