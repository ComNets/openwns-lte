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

#ifndef LTE_TIMING_RESOURCESCHEDULERINTERFACE_HPP
#define LTE_TIMING_RESOURCESCHEDULERINTERFACE_HPP

#include <WNS/scheduler/MapInfoEntry.hpp>
#include <WNS/scheduler/SchedulingMap.hpp>
#include <WNS/service/phy/power/PowerMeasurement.hpp>

namespace lte { namespace timing {

    /** @brief "command" PDU exchanged with peer schedulers */
	class SchedulerCommand :
	public wns::ldk::Command
     {
     public:
          SchedulerCommand()
              {
             }
  
          SchedulerCommand(const SchedulerCommand& other)
             {
                 local.distance = other.local.distance;
                 local.subBand = other.local.subBand;
                 local.phyMeasurementPtr = other.local.phyMeasurementPtr;
                 if (other.magic.burst == wns::scheduler::MapInfoEntryPtr())
                 {
                     magic.burst = other.magic.burst;
                 }
                 else
                 {
                     magic.burst = wns::scheduler::MapInfoEntryPtr(
                         new  wns::scheduler::MapInfoEntry(*other.magic.burst));
                 }
                 if (other.magic.schedulingTimeSlotPtr == wns::scheduler::SchedulingTimeSlotPtr())
                 {
                     magic.schedulingTimeSlotPtr = other.magic.schedulingTimeSlotPtr;
                 }
                 else
                 {
                     magic.schedulingTimeSlotPtr = wns::scheduler::SchedulingTimeSlotPtr(
                         new  wns::scheduler::SchedulingTimeSlot(*other.magic.schedulingTimeSlotPtr));
                 }
             }
 
         virtual ~SchedulerCommand() {}

         struct Local{
             double distance;
             int subBand;
             wns::service::phy::power::PowerMeasurementPtr phyMeasurementPtr;
         };
 
         struct Peer {};
 
         /**
          * @brief magic elements that usually don't belong into a packet.
          * All elements are SmartPtr.
          */
         struct Magic
         {
             /**
              * @brief this contains the complete information element
              * around this packet
              */
             wns::scheduler::MapInfoEntryPtr burst;
 
             /**
              * @brief this contains a complete "PhysicalResource" unit.
              * Used for HARQ. In this case it is type SchedulingTimeSlotPtr,
              * which is more or less a chunk.
              */
             wns::scheduler::SchedulingTimeSlotPtr schedulingTimeSlotPtr;
         };
 
         Local local;
 
         Peer peer;
 
         Magic magic;
    };

    class SchedulerOutgoing
    {
    public:

        /**
         * @brief Hands the result of the Scheduling Round down to the PHY
         * user, assuming _startTime to be the absolute Start Time of the
         * Scheduling Period
         */
        virtual void
        finishCollection(int frameNr, simTimeType _startTime) = 0;
    };

    class SchedulerIncoming
    {
    public:

        virtual void deliverReceived() = 0;

	virtual void
	sendPendingHARQFeedback() = 0;

    };

    class MasterScheduler :
        virtual public SchedulerOutgoing,
        virtual public SchedulerIncoming
    {
    public:

        /**
         * @brief trigger master scheduling (from FU TimingScheduler that calls
         * e.g. StartMapTx in timing/Events.cpp) */
        virtual void
        startCollection(int frameNr) = 0;

        virtual void
        resetHARQScheduledPeerRetransmissions() = 0;
    };

    class SlaveScheduler :
        virtual protected SchedulerOutgoing,
        virtual public SchedulerIncoming
    {
    public:

        /**
         * @brief trigger slave scheduling. calls
         * colleagues.strategy->startScheduling
         */
        virtual void
        startCollection(int frameNr) = 0;
    };

    /**
     * @brief Abstract interface class for ResourceScheduler FU
     */
    class SchedulerFUInterface
    {
    public:

        /**
         * @brief For Intra-Node dependency resolution.
         */
        virtual void
        onNodeCreated() = 0;
    };
} }


#endif // NOT defined LTE_TIMING_RESOURCESCHEDULERINTERFACE


