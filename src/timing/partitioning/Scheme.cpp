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

#include <LTE/timing/partitioning/Scheme.hpp>

using namespace lte::timing::partitioning;

Scheme::Scheme(const wns::pyconfig::View& config) :
    partList()
{
    for (int ii=0; ii< config.len("partitions"); ++ii)
    {
        partList.push_back(Partition(config.get("partitions",ii)));
    }
}

Scheme::~Scheme()
{}

bool
Scheme::hasPartition(uint32_t frameIndex) const
{
    return (frameIndex < partList.size());
}

Partition
Scheme::getPartition(uint32_t frameIndex) const
{
    return partList.at(frameIndex);
}


std::string
Scheme::doToString() const
{
    std::stringstream str;
    str << "List of partitions:\n";
    for (PartList::const_iterator iter = this->partList.begin();
         iter != this->partList.end();
         ++iter)
        str << "#####\n" << (*iter);
    return str.str();
}


