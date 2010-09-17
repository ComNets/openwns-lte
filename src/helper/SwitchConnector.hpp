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

#ifndef LTE_HELPER_SWITCHCONNECTOR_HPP
#define LTE_HELPER_SWITCHCONNECTOR_HPP

#include <LTE/helper/SwitchLink.hpp>

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Connector.hpp>
#include <WNS/ldk/IConnectorReceptacle.hpp>

namespace lte { namespace helper {

        /**
         * @brief Switchable Connector, picking only the activated
         * functional unit
         * @ingroup hasconnector
         *
         * This class
         * @li implements the Connector interface,
         * @li may be used as parameter to HasConnector.
         *
         */
        class SwitchConnector
            : public wns::ldk::Connector,
              public SwitchLink<wns::ldk::IConnectorReceptacle>
        {
        public:
            /**
             * @name Connector interface
             */
            //{@
            /**
             * @brief Checks if an acceptor for the given compound is available
             */
            virtual bool
            hasAcceptor(const wns::ldk::CompoundPtr& compound);

            /**
             * @brief Retrieve acceptor for a given compound
             *
             * @pre hasAcceptor must have returned true for this compound
             */
            virtual wns::ldk::IConnectorReceptacle*
            getAcceptor(const wns::ldk::CompoundPtr& compound);
            //@}
        };
    } // helper
} // lte


#endif // LTE_HELPER_SWITCHCONNECTOR_HPP

