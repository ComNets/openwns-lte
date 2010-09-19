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

#ifndef LTE_TIMING_PARTITIONING_PARTITIONINGINFO
#define LTE_TIMING_PARTITIONING_PARTITIONINGINFO

#include <LTE/timing/partitioning/Scheme.hpp>
#include <LTE/timing/partitioning/Partition.hpp>

#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>

namespace lte { namespace timing { namespace partitioning {

            class PartitioningInfo :
            public wns::ldk::ControlService
            {
                Scheme scheme;
                wns::logger::Logger logger;
            public:
                PartitioningInfo(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
                virtual ~PartitioningInfo();

                /** @brief returns the resource partition (old) */
                SubChannelRangeSet
                getFreeResources(uint32_t phaseNumber, uint32_t groupNumber) const;

                /** @brief returns the resource partition (new) */
                wns::scheduler::UsableSubChannelVector
                getUsableSubChannels(uint32_t phaseNumber, uint32_t groupNumber) const;

                /** @brief 'Feeder' or 'Universal' ([pab]) */
                std::string
                getDedication(uint32_t phaseNumber, uint32_t groupNumber) const;
            };

        }}}
#endif // LTE_TIMING_PARTITIONING_PARTITIONINGINFO


