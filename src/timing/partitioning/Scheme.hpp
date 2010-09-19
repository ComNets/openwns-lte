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

#ifndef LTE_TIMING_PARTITIONING_SCHEME
#define LTE_TIMING_PARTITIONING_SCHEME

#include <LTE/timing/partitioning/Partition.hpp>
#include <WNS/IOutputStreamable.hpp>
#include <WNS/pyconfig/View.hpp>
#include <vector>
#include <map>

namespace lte { namespace timing { namespace partitioning {

            namespace tests {
                class SchemeTest;
            }

            class Scheme :
            public wns::IOutputStreamable
            {
                typedef std::vector<Partition> PartList;
                PartList partList;

            public:
                Scheme(const wns::pyconfig::View& config);
                virtual ~Scheme();

                bool
                hasPartition(uint32_t frameIndex) const;

                Partition
                getPartition(uint32_t frameIndex) const;

            private:
                virtual std::string
                doToString() const;
            };

        }}}
#endif // LTE_TIMING_PARTITIONING_SCHEME


