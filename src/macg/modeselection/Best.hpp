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

#ifndef LTE_MACG_MODESELECTION_BEST_HPP
#define LTE_MACG_MODESELECTION_BEST_HPP

#include <LTE/macg/modeselection/Strategy.hpp>

namespace lte { namespace macg { namespace modeselection {

/**
 * @brief Mode Selection Strategy that always choses the Route via
 * the mode that has the best score.
*/
class Best :
	public Strategy
{
public:
    Best() {}

    virtual ~Best() {}

    virtual lte::helper::Route
    getRoute(const wns::ldk::CompoundPtr& compound,  
	     LayerContainer layers, 
	     ScorerContainer scorers);
    
};
	
} // modeselection
} // macg
} // lte

#endif // LTE_MACG_MODESELECTION_BEST_HPP


