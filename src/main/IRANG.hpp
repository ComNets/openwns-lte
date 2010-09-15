/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#ifndef LTE_MAIN_IRANG_HPP
#define LTE_MAIN_IRANG_HPP

namespace lte { namespace upperconvergence { class ENBUpperConvergence; }}

namespace lte { namespace main { namespace rang {

class IFlowManagement
{
public:
  virtual wns::service::dll::FlowID
  onFlowRequest(lte::helper::TransactionID _transactionId,
		lte::upperconvergence::ENBUpperConvergence* _eNBUpperConvergence,
		wns::service::dll::UnicastAddress utAddress) = 0;

  virtual void
  onFlowRelease(wns::service::dll::FlowID flowID) = 0;

  virtual void
  deleteFlow(wns::service::dll::FlowID flowID) = 0;
};

} // rang
} // main
} // lte
#endif // LTE_MAIN_IRANG_HPP
