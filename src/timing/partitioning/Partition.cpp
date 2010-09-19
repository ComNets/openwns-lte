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

#include <LTE/timing/partitioning/Partition.hpp>

#include <iostream>

using namespace lte::timing::partitioning;

Partition::Partition(const wns::pyconfig::View& config) :
    numberOfSubChannels(config.get<int>("numberOfSubChannels")),
    grouping(),
    dedication(),
    logger(config.get("logger"))
{
    MESSAGE_SINGLE(NORMAL, logger,"Partition::Partition():");
    for (int gg = 0; gg<config.len("groups.keys()"); ++gg)
    {
        std::stringstream dictEntry;
        dictEntry << "groups[groups.keys()[" << gg << "]]";

        wns::pyconfig::View groupView = config.get(dictEntry.str());

        uint32_t groupNumber = groupView.get<uint32_t>("number");

        SubChannelRangeSet group;
        usableSubChannelsPerGroup[groupNumber] = wns::scheduler::UsableSubChannelVector(numberOfSubChannels,false);
        assure(numberOfSubChannels==usableSubChannelsPerGroup[groupNumber].size(),"size error");

        for (int bb = 0; bb<groupView.len("bands"); ++bb)
        {
            wns::pyconfig::View bandView = groupView.get("bands",bb);

            int fMin = bandView.get<int>("fMin");
            int fMax = bandView.get<int>("fMax");

            SubChannelRange band = SubChannelRange::FromIncluding(fMin).ToIncluding(fMax);
            group.push_back(band);
            assure(fMax<numberOfSubChannels,"fMax="<<fMax<<" must be < numberOfSubChannels="<<numberOfSubChannels);
            for(int subChannel=fMin; subChannel<=fMax; ++subChannel)
            {
                usableSubChannelsPerGroup[groupNumber][subChannel] = true;
            }
        }
        grouping[groupNumber] = group;
        dedication[groupNumber] = groupView.get<std::string>("dedication");
        //MESSAGE_SINGLE(NORMAL, logger,"groups["<<gg<<"="<<dictEntry.str()<<"]: groupNumber="<<groupNumber<<", dedication="<<dedication[groupNumber]);
        MESSAGE_SINGLE(NORMAL, logger,"groups["<<gg<<"]: groupNumber="<<groupNumber<<", dedication="<<dedication[groupNumber]);
    }
    MESSAGE_SINGLE(NORMAL, logger, "Partition = "<<doToString());
}

Partition::~Partition()
{}

bool
Partition::hasResources(uint32_t groupNumber) const
{
    //return (grouping.find(groupNumber) != grouping.end());
    return (usableSubChannelsPerGroup.find(groupNumber) != usableSubChannelsPerGroup.end());
}

lte::timing::partitioning::SubChannelRangeSet
Partition::getFreeResources(uint32_t groupNumber) const
{
    if (grouping.find(groupNumber) == grouping.end())
        assure(false, "Requested resource info for unknown group!");

    return grouping.find(groupNumber)->second;
}

// return const reference only (efficient); origin does not change
wns::scheduler::UsableSubChannelVector
Partition::getUsableSubChannels(uint32_t groupNumber)
{
    if (usableSubChannelsPerGroup.find(groupNumber) != usableSubChannelsPerGroup.end())
    {
        return usableSubChannelsPerGroup[groupNumber];
    } else {
        MESSAGE_SINGLE(NORMAL, logger, "WARNING: cannot find usableSubChannelsPerGroup[groupNumber="<<groupNumber<<"]");
        static wns::scheduler::UsableSubChannelVector empty(numberOfSubChannels,false);
        //=wns::scheduler::UsableSubChannelVector(numberOfSubChannels,false);
        return empty;
    }
}

std::string
Partition::getDedication(uint32_t groupNumber) const
{
    if (dedication.find(groupNumber) == dedication.end())
        assure(false, "Requested resource info for unknown group!");

    return dedication.find(groupNumber)->second;
}

std::string
Partition::doToString() const
{
    std::stringstream s;
    s << "---\n";
    for (GroupContainer::const_iterator iter = grouping.begin();
         iter != grouping.end();
         ++iter)
    {
        s << "GroupNumber=" << iter->first << ": Dedication=" << getDedication(iter->first) << ", sc=";
		for (lte::timing::partitioning::SubChannelRangeSet::const_iterator sciter = iter->second.begin();
             sciter != iter->second.end();
             ++sciter)
        {
            s << sciter->min() << ".." << sciter->max();
            if (sciter != iter->second.end()) s << std::endl;
        }
    }
    return s.str();
}


