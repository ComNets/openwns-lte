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

#ifndef LTE_RLC_RLCINTERFACE_HPP
#define LTE_RLC_RLCINTERFACE_HPP

#include <LTE/helper/QoSClasses.hpp>

#include <DLL/Layer2.hpp>

#include <WNS/logger/Logger.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/Forwarding.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Classifier.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/service/dll/FlowID.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/Enum.hpp>
#include <set>

namespace lte { namespace rlc {

/* deleted by chen */
//         ENUM_BEGIN(PacketDirection);
//         ENUM(UNKNOWN, 0);
//         ENUM(UPLINK, 1);
//         ENUM(DOWNLINK, 2);
//         ENUM_END();

/* deleted by chen */
        /** @brief Command contributed by the RLC functional units. */
//         class RLCCommand :
//         public wns::ldk::Command
/* inserted by chen */
        class RLCCommand

        {
        public:
/* deleted by chen */
//             RLCCommand()
//             {
//                 local.direction = lte::rlc::PacketDirection::UNKNOWN();
//                 peer.destination = wns::service::dll::UnicastAddress();
//                 peer.flowID = -1;
//                 peer.qosClass = lte::helper::QoSClasses::UNDEFINED();
//             }

            /** @brief The locally used part of the RLC command */
            struct Rang
            {
                /** @brief The FlowID used between RANG and Basestations*/
                wns::service::dll::FlowID flowID;
            };
            Rang rang;
            /** @brief The locally used part of the RLC command */
            struct Local
            {
                /** @brief The direction of the compound, 1 means UPLINK (towards
                 * the core network), 2 means DOWNLINK (coming from the core network)*/
                int direction;
            };
            Local local;
            /** @brief The part of the RLC command signalled to the peer entitity */
            struct Peer
            {
                /** @brief source Address from an End-to-end Point of View */
                wns::service::dll::UnicastAddress source;
                /** @brief destination Address from an End-to-end Point of View, in the
                 * uplink this is not set because packets are always sent to the
                 * "Association".
                 */
                wns::service::dll::UnicastAddress destination; // E2E address
                wns::service::dll::FlowID flowID;
                wns::service::qos::QoSClass qosClass; // only here to support probe context
            };
            Peer peer;
            /** @brief The "magic" part of the RLC command, used for
             * simulator-internal modelling only. */
            struct Magic
            {
            };
            Magic magic;
        };

/* deleted by chen */
        /** @brief Base class for BS and UT RLC implementation */
//         class RLC :
//         virtual public wns::ldk::FunctionalUnit,
//         public wns::ldk::CommandTypeSpecifier<RLCCommand>,
//         public wns::ldk::HasReceptor<>,
//         public wns::ldk::HasConnector<>,
//         public wns::ldk::HasDeliverer<>
//         {
// /* deleted by chen */
// //         public:
// //             RLC(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// //             virtual ~RLC() {}
// // 
// //             virtual void
// //             onFUNCreated();
// 
// /* inserted by chen */
//         public:
// 
//             virtual void
//             onFUNCreated() = 0;
// 
// /* deleted by chen */
// //         protected:
// //             dll::ILayer2* layer2;
// // 
// //             wns::logger::Logger logger;
//         };
    }
}

#endif // NOT defined LTE_RLC_RLCINTERFACE_HPP
