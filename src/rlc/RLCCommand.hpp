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

#ifndef LTE_RLC_RLCCOMMAND_HPP
#define LTE_RLC_RLCCOMMAND_HPP

#include <LTE/helper/QoSClasses.hpp>

#include <WNS/ldk/Command.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/Enum.hpp>

namespace lte { namespace rlc {

    ENUM_BEGIN(PacketDirection);
    ENUM(UNKNOWN, 0);
    ENUM(UPLINK, 1);
    ENUM(DOWNLINK, 2);
    ENUM_END();

    /** @brief Command contributed by the RLC functional units. */
    class RLCCommand :
       public wns::ldk::Command
    {
    public:
      RLCCommand()
      {
	local.direction = lte::rlc::PacketDirection::UNKNOWN();
	peer.destination = wns::service::dll::UnicastAddress();
	peer.flowID = -1;
	peer.qosClass = lte::helper::QoSClasses::UNDEFINED();
      }

      struct
      {
	/**
	 * @brief The FlowID used between RANG and Basestations
	 */
	wns::service::dll::FlowID flowID;
      } rang;

      struct
      {
	/**
	 * @brief The direction of the compound, 1 means UPLINK (towards
	 * the core network), 2 means DOWNLINK (coming from the core network)
	 */
	int direction;
      } local;


      struct
      {
	/**
	 * @brief source Address from an End-to-end Point of View
	 */
	wns::service::dll::UnicastAddress source;

	/** @brief destination Address from an End-to-end Point of View, in the
	 * uplink this is not set because packets are always sent to the
	 * "Association".
	 */
	wns::service::dll::UnicastAddress destination; // E2E address

	wns::service::dll::FlowID flowID;

	wns::service::qos::QoSClass qosClass; // only here to support probe context
      } peer;

      struct
      {
      } magic;

    };

} // rlc
} // lte

#endif // LTE_RLC_RLCCOMMAND_HPP
