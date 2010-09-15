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

#ifndef LTE_UPPERCONVERGENCE_ENB_HPP
#define LTE_UPPERCONVERGENCE_ENB_HPP

#include <LTE/helper/TransactionID.hpp>
#include <DLL/UpperConvergence.hpp>

namespace dll { class ILayer2; }
namespace lte { namespace main { namespace rang { class IFlowManagement;}}}

namespace lte { namespace upperconvergence {
    
class ENBUpperConvergence :
    //virtual public wns::ldk::FunctionalUnit,
    virtual public dll::APUpperConvergence
{
public:
  ENBUpperConvergence(wns::ldk::fun::FUN* fun,const wns::pyconfig::View& config);

  virtual void
  registerHandler(wns::service::dll::protocolNumber protocol,
		  wns::service::dll::Handler* _dh);

  void
  sendData(const wns::service::dll::UnicastAddress& _peer,
	   const wns::osi::PDUPtr& pdu,
	   wns::service::dll::protocolNumber protocol,
	   wns::service::dll::FlowID _dllFlowID = 0);

  void
  processIncoming(const wns::ldk::CompoundPtr& compound);

  void
  onFUNCreated();

  void
  onFlowBuilt(wns::service::tl::FlowID _flowID, wns::service::dll::FlowID _dllFlowID);

  wns::service::dll::FlowID
  requestFlow(lte::helper::TransactionID _transactionId,
	      wns::service::dll::UnicastAddress utAddress);

  void
  releaseFlow(wns::service::dll::FlowID flowID);

  void
  deleteFlow(wns::service::dll::FlowID flowID);

private:
  dll::ILayer2* layer2;

  wns::ldk::CommandReaderInterface* rlcReader;

  lte::main::rang::IFlowManagement* flowManagementRang;
};

} // upperconvergence
} // lte

#endif // LTE_UPPERCONVERGENCE_ENB_HPP
