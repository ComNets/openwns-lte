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

#ifndef LTE_MACG_MACGUT_HPP
#define LTE_MACG_MACGUT_HPP

#include <LTE/macg/MACg.hpp>
#include <LTE/helper/Route.hpp>

#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/RoundRobinReceptor.hpp>

namespace lte { namespace macg {
	class MACgUT;

	class MACgSchedulerUT :
	public MACgScheduler
	{
	    friend class MACgUT;

	public:
	    MACgSchedulerUT();

	    virtual bool hasAcceptor(const wns::ldk::CompoundPtr& compound);

	    virtual wns::ldk::IConnectorReceptacle*
	    getAcceptor(const wns::ldk::CompoundPtr& compound);
	};


	class MACgUT :
	public MACg,
	public wns::ldk::HasReceptor<wns::ldk::RoundRobinReceptor>,
	public wns::ldk::HasConnector<MACgSchedulerUT>,
	public wns::ldk::HasDeliverer<>,
	public wns::ldk::Forwarding<MACgUT>,
	public wns::Cloneable<MACgUT>
	{
	public:
	    MACgUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

	    virtual void
	    processOutgoing(const wns::ldk::CompoundPtr& compound);
	};
} }

#endif // LTE_MACG_MACGUT_HPP


