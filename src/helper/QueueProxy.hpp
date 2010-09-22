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

#ifndef LTE_HELPER_QUEUEPROXY_HPP
#define LTE_HELPER_QUEUEPROXY_HPP

#include <LTE/controlplane/RRHandler.hpp>
#include <WNS/scheduler/queue/QueueInterface.hpp>
#include <WNS/scheduler/queue/SimpleQueue.hpp>
#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>

namespace lte { namespace helper {

        /** @brief This class emulates the behaviour of a SimpleQueue,
            but can have different implementations behind it.
            In the colleages struct of a ResourceScheduler of function RS-TX
            it contains a SimpleQueue and simpy proxies all interfaces to it.
            In the function RS-RX it communicates with the RRHandler
            to emulate a real queue which does not exist here, but on the peer's transmitter side.
        */
        class QueueProxy :
            public wns::scheduler::queue::QueueInterface
        {
        private:
            wns::scheduler::queue::QueueInterface* queue;
            lte::controlplane::RRHandlerBS* rrhandler;
            struct Colleagues {
                Colleagues() : registry(NULL) {}
                wns::scheduler::RegistryProxyInterface* registry;
            } colleagues;
            lte::controlplane::RequestStorageInterface* rrStorage;
            /** @brief if queue has been set from outside, treat memory responsibility external (don't delete in destructor) */
            bool queueIsExternal;
            wns::logger::Logger logger;
            wns::pyconfig::View config;
            wns::probe::bus::contextprovider::Variable* probeContextProviderForCid;
            wns::probe::bus::contextprovider::Variable* probeContextProviderForPriority;
            wns::probe::bus::ContextCollectorPtr sizeProbeBus;
        public:
            QueueProxy(wns::ldk::HasReceptorInterface*, const wns::pyconfig::View& config);
            virtual ~QueueProxy();

            virtual void
            setColleagues(wns::scheduler::RegistryProxyInterface* _registry);

            virtual void
            setFUN(wns::ldk::fun::FUN* fun);

            /** @brief set queue from outside (colleague) */
            virtual void
            setQueue(wns::scheduler::queue::QueueInterface* _queue);

            /** @brief set ResourceRequestHandler from outside (colleague) */
            virtual void
            setRRHandler(lte::controlplane::RRHandler* _rrhandler);

            virtual bool
            isAccepting(const wns::ldk::CompoundPtr& compound) const;

            virtual void
            put(const wns::ldk::CompoundPtr& compound);

            virtual wns::scheduler::UserSet
            getQueuedUsers() const;

            virtual wns::scheduler::ConnectionSet
            getActiveConnections() const;

            virtual unsigned long int
            numCompoundsForCid(wns::scheduler::ConnectionID cid) const;

            virtual unsigned long int
            numBitsForCid(wns::scheduler::ConnectionID cid) const;

            virtual wns::scheduler::QueueStatusContainer
            getQueueStatus() const;

            virtual wns::ldk::CompoundPtr
            getHeadOfLinePDU(wns::scheduler::ConnectionID cid);

            virtual int
            getHeadOfLinePDUbits(wns::scheduler::ConnectionID cid);

            wns::simulator::Time 
            getHeadOfLinePDUWaitingTime(wns::scheduler::ConnectionID cid)
            {
                assure(false, "getHeadOfLinePDUWaitingTime not implemented.");
                return 0.0;
            };

            virtual bool
            isEmpty() const;

            virtual bool
            hasQueue(wns::scheduler::ConnectionID cid);

            virtual bool
            queueHasPDUs(wns::scheduler::ConnectionID cid) const;

            virtual wns::scheduler::ConnectionSet
            filterQueuedCids(wns::scheduler::ConnectionSet connections);

            virtual wns::scheduler::queue::QueueInterface::ProbeOutput
            resetAllQueues();

            virtual wns::scheduler::queue::QueueInterface::ProbeOutput
            resetQueues(wns::scheduler::UserID user);

            virtual wns::scheduler::queue::QueueInterface::ProbeOutput
            resetQueue(wns::scheduler::ConnectionID cid);

            virtual void frameStarts();

            /** @brief write queue contens (bits) into probe */
            virtual void
            writeProbe(wns::scheduler::ConnectionID cid, unsigned int priority) const;

            /** @brief true if getHeadOfLinePDUSegment() is supported */
            bool supportsDynamicSegmentation() const;
            /** @brief get compound out and do segmentation into #bits (gross) */
            wns::ldk::CompoundPtr getHeadOfLinePDUSegment(wns::scheduler::ConnectionID cid, int bits);
            /** @brief if supportsDynamicSegmentation, this is the minimum size of a segment in bits */
            int getMinimumSegmentSize() const;

            /** @brief Retrieves a copy of the queue for a CID. Makes no sense here so we throw
            **/
            virtual std::queue<wns::ldk::CompoundPtr> 
            getQueueCopy(wns::scheduler::ConnectionID cid);

            virtual std::string
            printAllQueues();
        };


    }} // namespace lte::helper
#endif // LTE_HELPER_PRIORITYQUEUE_HPP


