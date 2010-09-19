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

#ifndef LTE_MACR_RACHINTERFACE_HPP
#define LTE_MACR_RACHINTERFACE_HPP

#include <LTE/helper/HasModeName.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/PowerRatio.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/ldk/ShortcutFU.hpp>
#include <WNS/ldk/fu/Plain.hpp>
#include <WNS/service/dll/Address.hpp>

namespace lte {
    namespace macr {

    class IRachTimingTx
    {
    public:
       /** @brief Trigger method to start RACH Tx phase */
        virtual void
        startTx(simTimeType duration) = 0;

        /** @brief Trigger method to stop RACH Tx phase */
	virtual void
	stopTx() = 0;
    };

    class IRachTimingRx
    {
    public:
       /** @brief Trigger method to start RACH Rx phase */
        virtual void
        startRx(simTimeType duration) = 0;

        /** @brief Trigger method to stop RACH Rx phase */
	virtual void
	stopRx() = 0;
    };

    } // macr
} // lte

#endif // LTE_MACR_RACHINTERFACE_HPP


