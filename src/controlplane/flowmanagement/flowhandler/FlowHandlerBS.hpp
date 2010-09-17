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

#ifndef LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLER_FLOWHANDLERBS_HPP
#define LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLER_FLOWHANDLERBS_HPP

#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandler.hpp>
#include <LTE/helper/TransactionID.hpp>

#include <WNS/Cloneable.hpp>
#include <WNS/service/dll/Address.hpp>


namespace lte {

  namespace controlplane {

    namespace flowmanagement {
      class FlowManagerBS;

      namespace flowhandler {

	class FlowHandlerBS :
	  public FlowHandler,
	  public wns::Cloneable<FlowHandlerBS>
	{
	public:
	  FlowHandlerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	  virtual ~FlowHandlerBS(){};

	  virtual void
	  onFUNCreated();

	  virtual bool
	  doIsAccepting(const wns::ldk::CompoundPtr& /*compound*/) const
	  {
	    return true;
	  }

	  virtual void
	  doOnData(const wns::ldk::CompoundPtr& compound);

	  void
	  flowConfirm(lte::helper::TransactionID _transactionId,
		      wns::service::dll::FlowID _flowID,
		      wns::service::dll::UnicastAddress _utAddress);

	private:
	  void
	  createFlow_conf(lte::helper::TransactionID _transactionId,
			  wns::service::dll::UnicastAddress _destination,
			  wns::service::dll::FlowID _flowID,
			  wns::service::dll::UnicastAddress _utAddress);

	  void
	  createFlowReleaseAck(wns::service::dll::FlowID _flowID);

	  wns::ldk::fun::FUN* fun;
	  dll::ILayer2* bsLayer2;
	  lte::controlplane::flowmanagement::FlowManagerBS* flowManagerBS;
	  wns::container::Registry<lte::helper::TransactionID, wns::service::dll::UnicastAddress> TransactionIDToDestination;
	  wns::container::Registry<wns::service::dll::FlowID, wns::service::dll::UnicastAddress> FlowIDInToDestIn;
	};
      } //flowHandler
    } //flowmanagement
  } //controlplane
} //lte

#endif // LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLER_FLOWHANDLERBS_HPP
