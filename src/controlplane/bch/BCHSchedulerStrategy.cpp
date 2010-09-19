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

#include <LTE/controlplane/bch/BCHSchedulerStrategy.hpp>
#include <WNS/scheduler/MapInfoEntry.hpp>

using namespace lte::controlplane::bch;

STATIC_FACTORY_REGISTER_WITH_CREATOR(BCHSchedulerStrategy,
                                     wns::scheduler::strategy::staticpriority::SubStrategyInterface,
                                     "lte.BCH.BCHSchedulerStrategy",
                                     wns::PyConfigViewCreator);

BCHSchedulerStrategy::BCHSchedulerStrategy(const wns::pyconfig::View& config):
    logger_(config.get("logger"))
{
    availableInSubframes_.resize(config.len("availableInSubframes"));
    for (int ii=0; ii<config.len("availableInSubframes"); ++ii)
    {
        availableInSubframes_[ii] = config.get<int>("availableInSubframes", ii);
    }
    numberOfSubchannels_ = config.get<int>("numberOfSubchannels");
}

BCHSchedulerStrategy::~BCHSchedulerStrategy()
{
}

void
BCHSchedulerStrategy::initialize()
{
}

wns::scheduler::MapInfoCollectionPtr
BCHSchedulerStrategy::doStartSubScheduling(wns::scheduler::strategy::SchedulerStatePtr schedulerState,
                                           wns::scheduler::SchedulingMapPtr schedulingMap)
{
    std::vector<int>::iterator result;
    result = std::find(availableInSubframes_.begin(), availableInSubframes_.end(), schedulingMap->getFrameNr());

    if (result == availableInSubframes_.end() )
    {
        MESSAGE_SINGLE(NORMAL, logger_, "BCH is not active in this frame (frameNr = " << schedulingMap->getFrameNr()  << ")");
    }

    wns::scheduler::MapInfoCollectionPtr mapInfoCollection = wns::scheduler::MapInfoCollectionPtr(new wns::scheduler::MapInfoCollection); // result datastructure

    wns::scheduler::ConnectionSet &currentConnections = schedulerState->currentState->activeConnections;

    assure(currentConnections.size() == 1, "There should only be one CID for the BCH");

    if(colleagues.queue->queueHasPDUs( (*currentConnections.begin()) ))
    {
        int pduCounter = 0;

        scheduleCid(schedulerState,schedulingMap,(*currentConnections.begin()),pduCounter, 10000, mapInfoCollection);
    }

    return mapInfoCollection;
}
