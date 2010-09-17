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

#ifndef LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLERUT_HPP
#define LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLERUT_HPP

#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandler.hpp>
#include <LTE/helper/TransactionID.hpp>

#include <WNS/Cloneable.hpp>
#include <WNS/events/CanTimeout.hpp>
#include <WNS/service/dll/FlowID.hpp>

namespace lte {
  namespace controlplane {
    namespace flowmanagement {
      class FlowManagerUT;
      namespace flowhandler {
	class FlowHandlerUT :
	  public FlowHandler,
	  public wns::events::CanTimeout,
	  public wns::Cloneable<FlowHandlerUT>
	{
	public:
	  FlowHandlerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	  virtual ~FlowHandlerUT(){};

	  virtual void
	  onFUNCreated();

	  virtual void
	  doOnData(const wns::ldk::CompoundPtr& compound);

	  virtual bool
	  doIsAccepting(const wns::ldk::CompoundPtr& /*compound*/) const
	  {
	    return true;
	  }

	  virtual void
	  onTimeout();

	  void
	  flowReq(lte::helper::TransactionID _transactionId,
		  wns::service::dll::UnicastAddress _destinationAddress,
		  wns::service::dll::FlowID oldflowID,
		  wns::service::qos::QoSClass qosClass);

	  void
	  releaseFlow(wns::service::dll::FlowID flowID);

	  void
	  createFlow(wns::service::dll::FlowID flowID);

	private:
	  void
	  createFlow_req(lte::helper::TransactionID _transactionId,
			 wns::service::dll::UnicastAddress _destinationAddress,
			 wns::service::dll::FlowID oldflowID,
			 wns::service::qos::QoSClass qosClass) const;

	  void
	  createFlowReleaseReq(wns::service::dll::FlowID flowID);

	  void
	  createFlow_ack(wns::service::dll::FlowID _flowID,
			 wns::service::dll::UnicastAddress _destinationAddress) const;

	  void
	  flowReleased(wns::service::dll::FlowID flowID);

	  double timeout;
	  std::string commandname;

	  wns::ldk::fun::FUN* fun;
	  dll::ILayer2* utLayer2;
	  lte::controlplane::flowmanagement::FlowManagerUT* flowManagerUT;
	  wns::container::Registry<wns::service::dll::FlowID, wns::service::dll::UnicastAddress> FlowIDOutToDestOut;
	};
      } //flowHandler
    } //flowmanagement
  } //controlplane
} //lte

#endif // LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLERUT_HPP
