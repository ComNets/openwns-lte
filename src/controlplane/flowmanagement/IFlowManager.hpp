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

#ifndef LTE_CONTROLPLANE_FLOWMANAGER_IFLOWMANAGER_HPP
#define LTE_CONTROLPLANE_FLOWMANAGER_IFLOWMANAGER_HPP

#include <LTE/helper/TransactionID.hpp>

#include <WNS/service/dll/Address.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/service/tl/FlowID.hpp>

namespace lte { namespace controlplane { namespace flowmanagement {

class IFlowSwitching
{
public:
  typedef std::map<wns::service::qos::QoSClass,
		   wns::service::dll::FlowID> ControlPlaneFlowIDs;

  virtual ~IFlowSwitching () {};

  virtual wns::service::dll::FlowID
  getFlowIDin(wns::service::dll::FlowID flowIDout) = 0;

  virtual wns::service::dll::FlowID
  getFlowIDout(wns::service::dll::FlowID flowIDin) = 0;

  virtual wns::service::qos::QoSClass
  getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const = 0;

  virtual wns::service::qos::QoSClass
  getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const = 0;

  virtual ControlPlaneFlowIDs
  getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress) = 0;

  virtual void
  setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs) = 0;
};

class IFlowManagerUE:
public IFlowSwitching
{
public:
  typedef std::string ModeName;
  virtual bool
  isValidFlow(const wns::ldk::ConstKeyPtr& key) const = 0;

  virtual void
  buildFlow(wns::service::tl::FlowID,
	    wns::service::qos::QoSClass) = 0;

  virtual void
  releaseFlow(wns::service::tl::FlowID flowID) = 0;

  virtual void
  onAssociatedPerMode(wns::service::dll::UnicastAddress rapAdr, bool preserved) = 0;

  virtual void
  onPlainDisassociation(ModeName mode) = 0;

  virtual void
  onDisassociatedPerMode(wns::service::dll::UnicastAddress bsAdr, ModeName mode, bool preserved) = 0;

  virtual void
  disassociating(ModeName mode) = 0;

};

class IFlowManagerENB:
public IFlowSwitching
{
public:
  typedef std::string ModeName;
  virtual void
  registerFlowID(lte::helper::TransactionID _transactionId,
		 wns::service::dll::FlowID _flowIDout,
		 wns::service::dll::UnicastAddress utAddress) = 0;

  virtual void
  onFlowReleaseAck(wns::service::dll::FlowID flowIDout) = 0;
    
  virtual void
  onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, bool preserved) = 0;
};

} // flowmanager
} // controlplane
} // lte
#endif //LTE_CONTROLPLANE_IFLOWMANAGER_HPP
