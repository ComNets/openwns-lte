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

#ifndef LTE_TIMING_PARTITIONING_PARTITION
#define LTE_TIMING_PARTITIONING_PARTITION

#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/IOutputStreamable.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/Interval.hpp>
#include <vector>
#include <map>

namespace lte { namespace timing { namespace partitioning {

            typedef uint32_t SubChannelNumber;

            typedef wns::Interval<SubChannelNumber> SubChannelRange;
            typedef std::vector<SubChannelRange> SubChannelRangeSet;

            /** @brief print the complete SubChannelRangeSet */
            inline std::string
            printSubChannelRangeSet(const SubChannelRangeSet& subChannelRangeSet)
            {
                std::stringstream s;
                for ( SubChannelRangeSet::const_iterator iter = subChannelRangeSet.begin(); iter != subChannelRangeSet.end(); /*nothing*/)
                {
                    s << (*iter).min() << "-" << (*iter).max();
                    if (++iter != subChannelRangeSet.end()) s << ",";
                }
                return s.str();
            } // printSubChannelRangeSet

            class Scheme;

            class Partition :
            public wns::IOutputStreamable
            {
                /** @brief partitionGroupNumber -> SubChannelRangeSet */
                typedef std::map<uint32_t, SubChannelRangeSet> GroupContainer;
                typedef std::map<uint32_t, std::string> GroupDedication;
                typedef std::map<uint32_t, wns::scheduler::UsableSubChannelVector> UsableSubChannelsGroupContainer;
                /** @brief this is kept constant for the whole simulation */
                int numberOfSubChannels;
                GroupContainer grouping;
                UsableSubChannelsGroupContainer usableSubChannelsPerGroup;
                GroupDedication dedication;
                wns::logger::Logger logger;
            public:
                Partition(const wns::pyconfig::View& config);
                virtual ~Partition();

                /** @brief tells if a resource partition is available for this frame */
                bool
                hasResources(uint32_t groupNumber) const;

                /** @brief returns the resource partition (old) */
                SubChannelRangeSet
                getFreeResources(uint32_t groupNumber) const;

                /** @brief returns the resource partition (new) */
                wns::scheduler::UsableSubChannelVector
                getUsableSubChannels(uint32_t groupNumber);

                /** @brief 'Feeder' or 'Universal' ([pab]) */
                std::string
                getDedication(uint32_t groupNumber) const;

            private:
                virtual std::string
                doToString() const;

            };

        }}}
#endif // LTE_TIMING_PARTITIONING_PARTITION


