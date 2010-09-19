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

#include <LTE/timing/partitioning/PartitioningInfo.hpp>
#include <WNS/StaticFactory.hpp>

using namespace lte::timing::partitioning;

STATIC_FACTORY_REGISTER_WITH_CREATOR(PartitioningInfo,
                                     wns::ldk::ControlServiceInterface,
                                     "lte.timing.partitioning.PartitioningInfo",
                                     wns::ldk::CSRConfigCreator);

PartitioningInfo::PartitioningInfo(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
    wns::ldk::ControlService(csr),
    scheme(config.get("scheme")),
    logger(config.get("logger"))
{
    MESSAGE_SINGLE(NORMAL, logger, "Started Resource Partitioning Info Service");
    MESSAGE_BEGIN(VERBOSE, logger, m, "Loaded Resource Partitioning Scheme:\n");
    m << scheme;
    MESSAGE_END();
}

PartitioningInfo::~PartitioningInfo()
{}

lte::timing::partitioning::SubChannelRangeSet
PartitioningInfo::getFreeResources(uint32_t phaseNumber, uint32_t groupNumber) const
{
    assure(scheme.hasPartition(phaseNumber),"Requested partitioning info for unknown phase number");

    if (scheme.getPartition(phaseNumber).hasResources(groupNumber))
    {
        MESSAGE_BEGIN(VERBOSE, logger, m, "Read Resources from Partitioning Scheme for phase=");
        m << phaseNumber << ", group=" << groupNumber << "\n" << scheme.getPartition(phaseNumber);
        MESSAGE_END();

        return scheme.getPartition(phaseNumber).getFreeResources(groupNumber);
    }

	lte::timing::partitioning::SubChannelRangeSet empty;
    return empty;
}

wns::scheduler::UsableSubChannelVector
PartitioningInfo::getUsableSubChannels(uint32_t phaseNumber, uint32_t groupNumber) const
{
    assure(scheme.hasPartition(phaseNumber),"Requested partitioning info for unknown phase number");

    MESSAGE_BEGIN(NORMAL, logger, m, "getUsableSubChannels(phaseNr="<<phaseNumber<< ", groupNr="<<groupNumber<<"): ");
    if (scheme.getPartition(phaseNumber).hasResources(groupNumber))
    {
        m << "\n" << scheme.getPartition(phaseNumber);
    } else {
        m << "empty";
    }
    MESSAGE_END();
    return scheme.getPartition(phaseNumber).getUsableSubChannels(groupNumber);
}

std::string
PartitioningInfo::getDedication(uint32_t phaseNumber, uint32_t groupNumber) const
{
    assure(scheme.hasPartition(phaseNumber),"Requested dedication info for unknown phase number");

    if (scheme.getPartition(phaseNumber).hasResources(groupNumber))
    {
        MESSAGE_BEGIN(VERBOSE, logger, m, "Read Resource Dedication from Partitioning Scheme for phase=");
        m << phaseNumber << ", group=" << groupNumber << "\n" << scheme.getPartition(phaseNumber);
        MESSAGE_END();

        return scheme.getPartition(phaseNumber).getDedication(groupNumber);
    }

    return "None";
}


