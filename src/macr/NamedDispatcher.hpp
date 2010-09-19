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

#ifndef LTE_MACR_NAMEDDISPATCHER_HPP
#define LTE_MACR_NAMEDDISPATCHER_HPP

#include <LTE/helper/HasModeName.hpp>

#include <WNS/ldk/multiplexer/Dispatcher.hpp>

namespace lte { namespace macr {

	class NamedDispatcher :
		public wns::ldk::multiplexer::Dispatcher,
		public lte::helper::HasModeName,
		public wns::Cloneable<NamedDispatcher>
	{
	public:
		NamedDispatcher(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
			wns::ldk::multiplexer::Dispatcher(fun, config),
			lte::helper::HasModeName(config)
		{}

		virtual
		~NamedDispatcher(){}

		virtual wns::CloneableInterface*
		clone() const
		{
			return new NamedDispatcher(*this);
		}
	};

    } // macr
} // lte

#endif // LTE_MACR_NAMEDDISPATCHER_HPP
