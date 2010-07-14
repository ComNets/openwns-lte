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

#ifndef LTE_PHY_PHYCOMMAND_HPP
#define LTE_PHY_PHYCOMMAND_HPP

#include <WNS/service/phy/ofdma/Pattern.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/phy/power/PowerMeasurement.hpp>
#include <WNS/CandI.hpp>

#include <WNS/ldk/Command.hpp>
#include <WNS/simulator/Time.hpp>

namespace wns { namespace node {
    class Interface;
}}

namespace dll
{
    namespace services
    {
        
        namespace management
        {
            
            class InterferenceCache;
        }
    }
}

namespace lte
{

namespace phy
{

/** @brief The PhyCommand, for communication of other FUs with the PhyUser
 */

class PhyCommand :
            public wns::ldk::Command
{
public:
    /** @brief to differentiate between outgoing compounds in the Tx and
     * Rx case.
     *
     * Outgoing compounds in Tx mode will really be transmitted by the
     * PHY. Outgoing compounds in Rx mode only carry control information
     * (and no payload) to set a certain receive antenna pattern in the
     * PHY station.
     */
    typedef enum { Tx, Rx } ModeRxTx;

    PhyCommand()
    {
        // initialize with meaningless default values
        local.subBand = -1;
        local.beamforming = false;
        local.beam = 0;
        local.pattern = wns::service::phy::ofdma::PatternPtr();
        local.start = 0.0;
        local.stop = 0.0;
        local.modeRxTx = Rx;

        magic.source = NULL;
        magic.destination = NULL;
        magic.remoteCache = NULL;
        magic.txp = wns::Power();
        magic.estimatedSINR = wns::CandI();
    };

    /** @brief Parameters to control the PhyUser 's behaviour.
     *
     * Parameters to control the PhyUser 's behaviour in the
     * outgoing case and to transport measurements in the
     * incoming case. They are filled in by the scheduler, MapHandler, or whoever
     * prepares the Compounds in the outgoing direction. The
     * measurements are filled in by the PhyUser in the
     * incoming direction. */

    struct Local
    {
        /** @brief OFDMA subband to be used for this compound */
        int subBand;

        /** @brief determine whether beamforming should be switched on/off */
        bool beamforming;

        /** @brief MIMO beam */
        int beam;

        /** @brief pointer identifying the antenna beam pattern to be used for
         *  this compound
         */
        wns::service::phy::ofdma::PatternPtr pattern;

        /** @brief start time for this compound's transmission */
        wns::simulator::Time start;

        /** @brief stop time for this compound's transmission */
        wns::simulator::Time stop;

        /** @brief determine whether PHY should the be prepared for transmission
         *  or reception
         */
        ModeRxTx modeRxTx;

        /** @brief Modulation&Coding scheme used. @see rise::plmapping */
        wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;

        /** @brief measurement information for CQI (contains MI,SINR,...) */
        wns::service::phy::power::PowerMeasurementPtr rxPowerMeasurementPtr;

        /** @brief stored distance measurement */
        double distance;
    } local;

    /** @brief empty, since PhyUser doesn't do any signalling */
    struct Peer {} peer;

    /** @brief for simulator internal use @todo node pointers should
     *  better be replaced by MAC addresses */
    struct Magic
    {
        wns::node::Interface* source;

        wns::node::Interface* destination;

        dll::services::management::InterferenceCache* remoteCache;

        /** @brief stored TxPower setting */
        wns::Power txp;

        /** @brief For debugging purposes: Original SINR Estimation
         * (from grouper) */
        wns::CandI estimatedSINR;
    } magic;

};
} // phy
} // lte

#endif // NOT defined LTE_PHY_PHYCOMMAND_HPP


