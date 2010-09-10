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

#ifndef LTE_HELPER_IDPROVIDER_DISTANCE_HPP
#define LTE_HELPER_IDPROVIDER_DISTANCE_HPP

#include <WNS/probe/bus/CompoundContextProvider.hpp>
#include <WNS/Positionable.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <DLL/Layer2.hpp>

namespace lte { namespace helper { namespace idprovider {

	/** @brief Provides Connection-based distance calculation */
	class Distance :
		virtual public wns::probe::bus::CompoundContextProvider
	{
		/** @brief points to this nodes mobility component */
		wns::PositionableInterface* myPosition;
		/** @brief StationManager Handle */
		dll::StationManager* stationManager;
		const std::string key;
	public:
		Distance(wns::ldk::fun::FUN* fun, dll::StationManager* sm);

		virtual
		~Distance();

		virtual const std::string&
		getKey() const
		{
			return key;
		}
    private:
        virtual void
        doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const;

	};

}}}

#endif // not defined LTE_HELPER_IDPROVIDER_DISTANCE_HPP


