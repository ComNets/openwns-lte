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

#ifndef LTE_HELPER_IDPROVIDER_CELLID_HPP
#define LTE_HELPER_IDPROVIDER_CELLID_HPP

#include <WNS/probe/bus/ContextProvider.hpp>
#include <DLL/services/control/Association.hpp>
#include <DLL/Layer2.hpp>


namespace lte { namespace helper { namespace idprovider {

    /** @brief Provides Connection-based distance calculation */
    class CellId :
        virtual public wns::probe::bus::ContextProvider
    {
	public:
    	CellId(dll::Layer2*, std::string modeName);

        virtual
        ~CellId();

    private:
        virtual void
        doVisit(wns::probe::bus::IContext& c) const;

        const std::string key_;
        const std::string modeName_;
        bool isBS_;

        dll::services::control::Association* associationService_;
        dll::Layer2* layer2_;
    };

}}}

#endif // not defined LTE_HELPER_IDPROVIDER_CELLID_HPP


