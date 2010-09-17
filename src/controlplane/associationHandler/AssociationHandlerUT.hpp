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

#ifndef LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLERUT_HPP
#define LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLERUT_HPP

#include <LTE/controlplane/associationHandler/AssociationHandler.hpp>

#include <WNS/Cloneable.hpp>
#include <WNS/events/CanTimeout.hpp>

namespace lte {

  namespace controlplane {

    class AssociationsProxyUT;

    namespace associationHandler {

      class AssociationHandlerUT :
	public AssociationHandler,
	public wns::events::CanTimeout,
	public wns::Cloneable<AssociationHandlerUT>
      {

      public:
	AssociationHandlerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	virtual ~AssociationHandlerUT(){};

	virtual void
	onFUNCreated();

	virtual void
	doOnData(const wns::ldk::CompoundPtr&);

	virtual void
	onTimeout();

	void
	associateTo(wns::service::dll::UnicastAddress rapAdr);

	void
	disassociate(wns::service::dll::UnicastAddress rapAdr);

	void
	switchAssociationTo(wns::service::dll::UnicastAddress rapAdr);

	// the below two functions will only be called by BCHService, if used without MIH
	void
	bestRAP(wns::service::dll::UnicastAddress destination);

	void
	belowThreshold(wns::service::dll::UnicastAddress destination);

	int getMyDuplexGroup(int frameNr, bool isDL);
	int getPeerDuplexGroup(wns::service::dll::UnicastAddress user);
      private:
	void
	createAssociation_req(wns::service::dll::UnicastAddress destination) const;

	void
	createDisassociation_req(wns::service::dll::UnicastAddress target) const;

	double timeout;

	lte::controlplane::AssociationsProxyUT* associationsProxy;

	wns::service::dll::UnicastAddress scheduledRAP;

	void
	associate(wns::service::dll::UnicastAddress destination);

	//my own duplexgroup
	int duplexGroup;
      };
    }
  }
}
#endif // LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLERUT_HPP
