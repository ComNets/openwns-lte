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

#ifndef LTE_TIMING_RESOURCESCHEDULERNONE_HPP
#define LTE_TIMING_RESOURCESCHEDULERNONE_HPP

#include <LTE/timing/ResourceScheduler.hpp>
#include <WNS/ldk/Forwarding.hpp>

namespace lte { namespace timing {

    /**
     * @dummy FU to satisfy the FUN structure
     */
    class ResourceSchedulerNone :
        virtual public SchedulerFUInterface,
        public wns::ldk::Forwarding<ResourceSchedulerNone>,
        public wns::ldk::CommandTypeSpecifier<SchedulerCommand>,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<>,
        public wns::ldk::HasDeliverer<>,
        public wns::Cloneable<ResourceSchedulerNone>
    {
    public:

        ResourceSchedulerNone(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

        virtual ~ResourceSchedulerNone(){};
    private:

        /**
         * @brief Scheduler FU Interface completion
         */
        virtual void
        onNodeCreated() {};

        /**
         * @brief overloaded CompoundHandler Methods to avoid accidental Use
         */
        virtual bool
        doIsAccepting(const wns::ldk::CompoundPtr&){ return false; };

        virtual void
        doSendData(const wns::ldk::CompoundPtr&){};

        virtual void
        doOnData(const wns::ldk::CompoundPtr&){};

        virtual void
        doWakeup(){};
    };
}
}


#endif // NOT defined ...


