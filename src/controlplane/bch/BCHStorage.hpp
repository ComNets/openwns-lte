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

#ifndef LTE_CONTROLPLANE_BCH_BCHSTORAGE_HPP
#define LTE_CONTROLPLANE_BCH_BCHSTORAGE_HPP


#include <WNS/simulator/ISimulator.hpp>
#include <WNS/events/scheduler/Interface.hpp>

#include <WNS/Enum.hpp>
#include <WNS/SmartPtr.hpp>
#include <WNS/PowerRatio.hpp>
#include <WNS/container/Registry.hpp>
#include <WNS/pyconfig/View.hpp>

#include <WNS/service/dll/Address.hpp>

#include <vector>

namespace lte { namespace controlplane { namespace bch {

	class BCHRecord :
		virtual public wns::RefCountable
	{
	public:
		BCHRecord(wns::service::dll::UnicastAddress _address,
				  wns::Ratio _sinr,
				  wns::Ratio _pathloss,
				  wns::Power _rxpwr,
				  double _dist,
				  uint32_t _subBand) :
            source(_address),
			sinr(_sinr),
			pathloss(_pathloss),
			rxpower(_rxpwr),
			distance(_dist),
			subBand(_subBand),
			timeStamp(wns::simulator::getEventScheduler()->getTime())
		{}

		wns::service::dll::UnicastAddress source;
		wns::Ratio sinr;
		wns::Ratio pathloss;
		wns::Power rxpower;
		double distance;
		uint32_t subBand;

		simTimeType timeStamp;
	};

 	typedef wns::SmartPtr<BCHRecord> BCHRecordPtr;
 	typedef std::vector<BCHRecordPtr> BCHList;

	template <typename KEYTYPE>
	class BCHStorage
	{
		typedef std::map<KEYTYPE, BCHRecordPtr> BCHMap;
		typedef std::pair<KEYTYPE, BCHRecordPtr> BCHMapElement;
		typedef std::vector<KEYTYPE> BCHKeyList;

	public:
		BCHStorage(){};

		/** @brief store BCH measurement */
		void store(KEYTYPE key, BCHRecordPtr rec){
			// Store in Map, new entries automatically overwrite old ones
			bchMap[key] = rec;
		}

		/** @brief clear all measurement */
		void reset(){
			bchMap.clear();
		}

		/** @brief return the best BCH measurement according to sorting strategy T
		 */
		template <typename T>
		BCHRecordPtr
		getBest() const
		{
			if (bchMap.empty())
				return BCHRecordPtr();

			return std::max_element<typename BCHMap::const_iterator>(bchMap.begin(), bchMap.end(), T())->second;
		}

		/** @brief get Measuremment for a certain Station, identified by its
		 * address
		 */
		BCHRecordPtr
		get(const KEYTYPE& key) const {
			if (bchMap.find(key) != bchMap.end())
				return bchMap.find(key)->second;

			return BCHRecordPtr();
		}

		/** @brief get (unsorted) list of all measured BCHs
		 */
		BCHList
		getAll() const {
			BCHList copy;
			for (typename BCHMap::const_iterator it = bchMap.begin();
				 it != bchMap.end();
				 ++it)
				copy.push_back(it->second);
			return copy;
		}

		/** @brief get (unsorted) list of all Stations
		 */
		BCHKeyList
		getBCHKeys() const {
			BCHKeyList copy;
			for (typename BCHMap::const_iterator it = bchMap.begin();
				 it != bchMap.end();
				 ++it)
				copy.push_back(it->first);
			return copy;
		}


	private:
		BCHMap bchMap;
	};

	namespace compare {

	struct BestSINR
	{
		template <typename T>
		bool
		operator()(const T& e1, const T& e2) const
			{
				if (e1.second->sinr == e2.second->sinr)
					return e1.second->timeStamp < e2.second->timeStamp;

				return e1.second->sinr < e2.second->sinr;
			}
	};

	struct BestRXPWR
	{
		template <typename T>
		bool
		operator()(const T& e1, const T& e2) const
			{
				if (e1.second->rxpower == e2.second->rxpower)
					return e1.second->timeStamp < e2.second->timeStamp;

				return e1.second->rxpower < e2.second->rxpower;
			}
	};

	struct BestDIST
	{
		template <typename T>
		bool
		operator()(const T& e1, const T& e2) const
			{
				if (e1.second->distance == e2.second->distance)
					return e1.second->timeStamp < e2.second->timeStamp;

				return e1.second->distance > e2.second->distance;
			}
	};

	struct BestPathloss
	{
		template <typename T>
		bool
		operator()(const T& e1, const T& e2) const
			{
				if (e1.second->pathloss == e2.second->pathloss)
					return e1.second->timeStamp < e2.second->timeStamp;

				return e1.second->pathloss > e2.second->pathloss;
			}
	};

	}



} } }

#endif // NOT defined LTE_CONTROLPLANE_BCH_BCHSTORAGE_HPP


