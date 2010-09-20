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

#ifndef LTE_MACR_PHYCOMMAND_HPP
#define LTE_MACR_PHYCOMMAND_HPP

#include <LTE/helper/MIProviderInterface.hpp>

#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/phy/power/PowerMeasurement.hpp>
#include <WNS/node/Node.hpp>
#include <WNS/service/phy/ofdma/Pattern.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/CandI.hpp>

#include <boost/function.hpp>

namespace dll {
	class ILayer2;
	namespace services {
		namespace management {
			class InterferenceCache;
		}
	}
}

namespace lte {
	namespace macr {

		/** @brief The PhyCommand, for communication of other FUs with the PhyUser
		 */
		class PhyCommand :
			public wns::ldk::Command,
			public lte::helper::MIProviderInterface
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

			PhyCommand(){
				// initialize with meaningless default values
				local.subBand = -1;
				local.beamforming = false;
				local.beam = 0;
				local.pattern = wns::service::phy::ofdma::PatternPtr();
				local.start = 0.0;
				local.stop = 0.0;
				local.modeRxTx = Rx;

				// these are all included in rxPowerMeasurementPtr
				//local.mib = 0.0;
				//local.sinr = wns::Ratio();
				//local.interference = wns::Power();
				//local.rxp = wns::Power();
				//local.distance = 0.0;

				magic.source = NULL;
				magic.destination = NULL;
				magic.remoteCache = NULL;
				magic.txp = wns::Power();
				magic.estimatedSINR = wns::CandI();
                magic.isRetransmission = false;
			};
			/** @brief completion of the wns::ldk::ErrorRateProvider Interface */
			virtual double
			getMutualInformation() const { return local.rxPowerMeasurementPtr->getMIB(); };

			/**< @brief Parameters to control the PhyUser 's behaviour.
			 *
			 * Parameters to control the PhyUser 's behaviour in the
			 * outgoing case and to transport measurements in the
			 * incoming case. They are filled in by the scheduler, MapHandler, or whoever
			 * prepares the Compounds in the outgoing direction. The
			 * measurements are filled in by the PhyUser in the
			 * incoming direction. */
			struct Local {
				 /** @name local info for the outgoing case */
				//@{
				/** @brief OFDMA subband to be used for this compound */
				int subBand;
				/** @brief determine whether beamforming should be switched on/off */
				bool beamforming;
				/** @brief MIMO beam */
				int beam;
				/** @brief pointer identifying the antenna beam pattern to be used for this compound */
				wns::service::phy::ofdma::PatternPtr pattern;
				/** @brief start time for this compound's transmission */
				simTimeType start;
				/** @brief stop time for this compound's transmission */
				simTimeType stop;
				/** @brief determine whether PHY should the be prepared for transmission or reception */
				ModeRxTx modeRxTx;
				//@}

				/** @name local info used in incoming and outgoing case */
				//@{
				/** @brief Modulation&Coding scheme used. @see rise::plmapping */
				wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;
				//wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;
				//@}

				/** @name local info used in the incoming case */
				//@{
				/** @brief measurement information for CQI (contains MI,SINR,...) */
				wns::service::phy::power::PowerMeasurementPtr rxPowerMeasurementPtr;
				/** @brief stored MI/bit (MIB) measurement */
				//double mib;
				/** @brief stored SINR measurement */
				//wns::Ratio sinr;
				/** @brief stored pathloss estimation */
				//wns::Ratio pathloss;
				/** @brief stored Interference measurement */
				//wns::Power interference;
				/** @brief stored RxPower measurement */
				//wns::Power rxp;
				/** @brief stored distance measurement */
				double distance;

				/** @brief Used to callback upper FUs when a PDU is put on Air */
			        boost::function<void()> onAirCallback;
			} local;
			/** @brief empty, since PhyUser doesn't do any signalling */
			struct Peer {} peer;
			/** @brief for simulator internal use @todo node pointers should
			 *  better be replaced by MAC addresses */
			struct Magic {
				wns::node::Interface* source;
				wns::node::Interface* destination;
				dll::services::management::InterferenceCache* remoteCache;
				//wns::PositionableInterface* mobility; // can calculate distances
				/** @brief stored TxPower setting */
				wns::Power txp;
				/** @brief For debugging purposes: Original SINR Estimation
				 * (from grouper) */
				wns::CandI estimatedSINR;
                bool isRetransmission;
			} magic;

		};
	} // macr
} // lte

#endif // NOT defined LTE_MACR_PHYCOMMAND_HPP


