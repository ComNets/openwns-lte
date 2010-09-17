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

#ifndef LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLERAP_HPP
#define LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLERAP_HPP

#include <LTE/controlplane/associationHandler/AssociationHandler.hpp>
#include <LTE/macr/PhyUser.hpp>

#include <WNS/events/Callback.hpp>
#include <WNS/Cloneable.hpp>

namespace lte { namespace controlplane {

    class AssociationsProxyBS;

    namespace associationHandler {

      class AssociationHandlerBS :
	public AssociationHandler,
	public wns::Cloneable<AssociationHandlerBS>
      {
      public:
	AssociationHandlerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	virtual ~AssociationHandlerBS(){};

	virtual void
	onFUNCreated();

	virtual void
	doOnData(const wns::ldk::CompoundPtr& compound);

	virtual void
	disassociationOnTimeout(const wns::service::dll::UnicastAddress adr, const std::string perMode);

	int getMyDuplexGroup(int frameNr, bool isDL);
	int getPeerDuplexGroup(wns::service::dll::UnicastAddress user);
      private:
	class AckOnAirCallback:
	  public wns::events::Callback
	{
	public:
	  AckOnAirCallback(AssociationHandlerBS* _AssocHandler,
			   wns::service::dll::UnicastAddress _user,
			   wns::service::dll::UnicastAddress _ownAddress)
	  {
	    assure(_AssocHandler, "Invalid Association Handler given!");
	    assure(_user.isValid(), "Invalid User address given!");
	    assure(_ownAddress.isValid(), "Invalid User address given!");
	    this->AssocHandler = _AssocHandler;
	    this->user = _user;
	    this->ownAddress = _ownAddress;
	  }
	  virtual ~AckOnAirCallback(){};

	  virtual void
	  callback()
	  {
	    MESSAGE_BEGIN(NORMAL, this->AssocHandler->logger, m,"");
	    m << "AckOnAirCallback::callback(): disassociation_ack is on air";
	    MESSAGE_END();

	    // notify observers
	    AssocHandler->notifyOnDisassociated(this->user, this->ownAddress);

	    // Can be fired only once.
	    delete this;
	  }
	private:
	  AssociationHandlerBS* AssocHandler;
	  wns::service::dll::UnicastAddress user;
	  wns::service::dll::UnicastAddress ownAddress;
	};

	lte::controlplane::AssociationsProxyBS* associationsProxy;

	// Pointer to the Phy User, used in the
	// callback procedure
	lte::macr::PhyUser* phyUser;

	void
	createAssociation_ack(wns::service::dll::UnicastAddress destination,
			      wns::service::dll::UnicastAddress user,
			      std::string perMode,
			      lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs _controlPlaneFlowIDs) const;

	void
	createDisassociation_ack(wns::service::dll::UnicastAddress destination,
				 wns::service::dll::UnicastAddress user,
				 bool preserve, std::string perMode,
				 AckOnAirCallback* callback) const;

	//UTs´ duplex groups
	std::map<wns::service::dll::UnicastAddress, int> duplexGroups;
      };
    }
  }
}

#endif // LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLERBS_HPP
