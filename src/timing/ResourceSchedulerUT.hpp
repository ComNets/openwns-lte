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

#ifndef LTE_TIMING_RESOURCESCHEDULERUT_HPP
#define LTE_TIMING_RESOURCESCHEDULERUT_HPP

#include <LTE/timing/ResourceScheduler.hpp>

namespace lte { namespace timing {

    class ResourceSchedulerUT :
        virtual public SlaveScheduler,
        public ResourceScheduler,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<>,
        public wns::ldk::HasDeliverer<>,
        public wns::Cloneable<ResourceSchedulerUT>
    {
    public:

        ResourceSchedulerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

        virtual ~ResourceSchedulerUT(){};

        virtual void
        onFUNCreated();

        /**
         * @brief trigger slave scheduling
         */
        virtual void
        startCollection(int frameNr);

        virtual void
        onDisassociated(wns::service::dll::UnicastAddress userAdr,
                        wns::service::dll::UnicastAddress dstAdr);

        virtual void
        onAssociated(wns::service::dll::UnicastAddress userAdr,
                     wns::service::dll::UnicastAddress dstAdr);

        wns::scheduler::QueueStatusContainer
        getQueueStatus() const;

    private:
        void
        deletePacketsToFrom(wns::node::Interface* destination,
                            wns::service::dll::UnicastAddress source);

        simTimeType symbolDuration;

        wns::node::Interface* myMasterUserID;
    };
}}

#endif // NOT defined ...
