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

#ifndef LTE_MACG_MACGCOMMAND_HPP
#define LTE_MACG_MACGCOMMAND_HPP

#include <WNS/service/dll/Address.hpp>

namespace lte { namespace macg {

    class MACgCommand :
       public wns::ldk::Command
    {
    public:
      MACgCommand()
      {
	local.modeID = -1;
	local.modeName = "";
	peer.source = wns::service::dll::UnicastAddress();
	peer.dest   = wns::service::dll::UnicastAddress();
	magic.hopCount = 1;
      }

      struct {
	int modeID;
	std::string modeName;
      } local;

      struct {
	wns::service::dll::UnicastAddress source;
	wns::service::dll::UnicastAddress dest; // next hop address
      } peer;

      struct {
	unsigned int hopCount;
      } magic;
    }; // MACgCommand

} // macg
} // lte

#endif // LTE_MACG_MACGCOMMAND_HPP
