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

#ifndef LTE_CONTROLPLANE_BCH_BCHSCHEDULERSTRATEGY_HPP
#define LTE_CONTROLPLANE_BCH_BCHSCHEDULERSTRATEGY_HPP

#include <WNS/scheduler/strategy/staticpriority/SubStrategy.hpp>
#include <WNS/scheduler/strategy/SchedulerState.hpp>

namespace lte { namespace controlplane { namespace bch {

class BCHSchedulerStrategy:
    public wns::scheduler::strategy::staticpriority::SubStrategy
{
public:
    BCHSchedulerStrategy(const wns::pyconfig::View& config);

    virtual ~BCHSchedulerStrategy();

    virtual void
    initialize();

    virtual wns::scheduler::MapInfoCollectionPtr
    doStartSubScheduling(wns::scheduler::strategy::SchedulerStatePtr schedulerState,
                         wns::scheduler::SchedulingMapPtr schedulingMap);

private:
    wns::logger::Logger logger_;

    std::vector<int> availableInSubframes_;

    int numberOfSubchannels_;
};

} // bch
} // controlplane
} // lte

#endif // LTE_CONTROLPLANE_BCH_BCHSCHEDULERSTRATEGY_HPP
