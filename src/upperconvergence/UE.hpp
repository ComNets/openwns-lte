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

#ifndef LTE_UPPERCONVERGENCE_UE_HPP
#define LTE_UPPERCONVERGENCE_UE_HPP

#include <DLL/UpperConvergence.hpp>
#include <DLL/Layer2.hpp>

#include <WNS/ldk/FunctionalUnit.hpp>

namespace lte { namespace controlplane { namespace flowmanager { class IFlowManagerUE; }}}

namespace lte { namespace upperconvergence {

class UEUpperConvergence :
    virtual public wns::ldk::FunctionalUnit,
    virtual public dll::UTUpperConvergence
{
public:

  UEUpperConvergence(wns::ldk::fun::FUN* fun,
		     const wns::pyconfig::View& config);

  void
  sendData(const wns::service::dll::UnicastAddress& _peer,
	   const wns::osi::PDUPtr& pdu,
	   wns::service::dll::protocolNumber protocol,
	   wns::service::dll::FlowID _dllFlowID = 0);

  void
  processIncoming(const wns::ldk::CompoundPtr& compound);


  void
  onFUNCreated();

  virtual void
  registerHandler(wns::service::dll::protocolNumber protocol,
		  wns::service::dll::Handler* _dh);

  void
  registerFlowHandler(wns::service::dll::FlowHandler* flowHandler);

  void
  establishFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass);

  void
  releaseFlow(wns::service::tl::FlowID flowID);

  void
  onFlowBuilt(wns::service::tl::FlowID _flowID,
	      wns::service::dll::FlowID _dllFlowID,
	      bool newFlow);

private:

  wns::ldk::fun::FUN* fun;

  wns::ldk::CommandReaderInterface* rlcReader;
  
  dll::ILayer2* layer2;
  
  lte::controlplane::flowmanager::IFlowManagerUE* flowManager;
  
  wns::service::dll::FlowHandler* tlFlowHandler;
};

} // upperconvergence
} // lte

#endif // LTE_UPPERCONVERGENCE_UE_HPP
