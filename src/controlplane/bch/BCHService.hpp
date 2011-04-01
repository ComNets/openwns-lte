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

#ifndef LTE_CONTROLPLANE_BCH_BCHSERVICE_HPP
#define LTE_CONTROLPLANE_BCH_BCHSERVICE_HPP

#include <WNS/events/PeriodicTimeout.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/ldk/ControlServiceInterface.hpp>

#include <LTE/controlplane/bch/BCHStorage.hpp>
#include <LTE/helper/HasModeName.hpp>
#include <LTE/macr/PhyCommand.hpp>

namespace lte {
	namespace macr {
		class PhyUser;
	}
	namespace controlplane {
		namespace associationHandler {
			class AssociationHandlerUT;
		}

	namespace bch {

	struct Best {
		BCHRecordPtr entry;
		double value;
	};

	class BCHService
		: public wns::ldk::ControlService,
		  public wns::events::PeriodicTimeout,
		  public lte::helper::HasModeName
	{
		BCHStorage<wns::service::dll::UnicastAddress> bchStorage;

        struct Criterion {
            Criterion(const wns::pyconfig::View& config):
                name(config.get<std::string>("name"))
            {
                if(name == "SINR" || name == "Pathloss")
                    ratioMargin = config.get<wns::Ratio>("margin");
                else if(name == "RxPower")
                    powerMargin = config.get<wns::Power>("margin");
                else if(name == "Distance")
                    distanceMargin == config.get<double>("margin");
                else if(name == "MAC_ID")
                {
                    assure(!config.isNone("address"), "MAC_ID criterion needs address of serving station");
                    address = wns::service::dll::UnicastAddress(config.get<int>("address"));              
                }                
                else
		            throw wns::Exception("Unsupported BCH Storage criterion");  
            }

    		std::string name;
            wns::Power powerMargin;
            wns::Ratio ratioMargin;
            double distanceMargin;
            wns::service::dll::UnicastAddress address;
        };

        Criterion criterion;

		double upperThreshold;
		double lowerThreshold;
		dll::ILayer2* layer2;
		lte::controlplane::associationHandler::AssociationHandlerUT* associationHandler;
		wns::logger::Logger logger;

		simTimeType timeout;
		bool triggersAssociation;
		bool triggersReAssociation;

	public:
		BCHService(wns::ldk::ControlServiceRegistry* csr,
				   const wns::pyconfig::View& config);
		virtual
		~BCHService(){}

		virtual void
		onCSRCreated(){}

		/** @brief Periodic Timeout Interface */
		virtual void
		periodically();
		/** @brief to store measurements */
        void
        storeMeasurement(const wns::service::dll::UnicastAddress& source,
                         const wns::service::phy::power::PowerMeasurementPtr&  phyMeasurementPtr,
                         int subband);
		void
		storeMeasurement(const wns::service::dll::UnicastAddress& source,
						 const lte::macr::PhyCommand& phyCommand);

		uint32_t
		getSubBand(const wns::service::dll::UnicastAddress& source) const;

		Best
		getBest() const;

	};

}}}

#endif // NOT defined LTE_CONTROLPLANE_BCH_BCHSERVICE_HPP


