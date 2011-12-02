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

#ifndef LTE_TIMING_RESOURCESCHEDULERTX_HPP
#define LTE_TIMING_RESOURCESCHEDULERTX_HPP

#include <LTE/timing/ResourceSchedulerInterface.hpp>
#include <LTE/macr/Scorer.hpp>
#include <LTE/helper/HasModeName.hpp>

#include <DLL/services/control/Association.hpp>

// Forward declarations would be sufficient here
// For convenience we include here.
#include <WNS/scheduler/MapInfoProviderInterface.hpp>
#include <WNS/scheduler/RegistryProxyInterface.hpp>
#include <WNS/scheduler/queue/QueueInterface.hpp>
#include <WNS/scheduler/harq/HARQInterface.hpp>
#include <WNS/scheduler/strategy/StrategyInterface.hpp>
#include <WNS/scheduler/grouper/SpatialGrouper.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <WNS/scheduler/metascheduler/IMetaScheduler.hpp>

#include <map>
#include <list>

namespace lte {

// Forward declarations
namespace rlc { class RLC; }
namespace macg { class MACg; }
namespace macr { class PhyUser; }
namespace controlplane { class IMapHandlerRS; }
namespace controlplane { class RRHandler; }
namespace controlplane { namespace flowmanagement { class FlowManager; } }
namespace controlplane { namespace associationHandler { class AssociationHandlerBS; } }
namespace timing { class TimingScheduler; class RegistryProxy;}
namespace timing { namespace partitioning { class PartitioningInfo;}}
namespace helper { class QueueProxy; }

namespace timing {

/**
 * @brief Base Class for Functional Units performing the resource scheduling,
 * i.e. allocation of resources in time and frequency (subchannels)
 */
class ResourceScheduler :
        virtual public SchedulerFUInterface,
        virtual public SchedulerOutgoing,
        virtual public SchedulerIncoming,
        virtual public wns::ldk::FunctionalUnit,
        public wns::ldk::CommandTypeSpecifier<SchedulerCommand>,
        virtual public wns::scheduler::SchedulingMapProviderInterface,
        virtual public macr::ScorerInterface,
        public lte::helper::HasModeName,
        public dll::services::control::AssociationObserver
{
    friend class controlplane::associationHandler::AssociationHandlerBS;

    /**
     * @brief the SeenTable is a dataStructure to support Layer2-Routing in
     * the Multihop case. It relates the DLL address of a packet's original
     * sender to the DLL address of the sender we actually received it from.
     */
    typedef std::map<wns::service::dll::UnicastAddress,
                     wns::service::dll::UnicastAddress> SeenTable;

    typedef std::list<wns::ldk::CompoundPtr> CompoundList;

public:
    ResourceScheduler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

    virtual ~ResourceScheduler();

    /** @name CompoundHandlerInterface */
    //@{

    /**
     * @brief upper FU asks this, whether SendData may be called
     */
    virtual bool
    doIsAccepting(const wns::ldk::CompoundPtr& compound) const;

    /**
     * @brief upper FU wants to give us a PDU (compound) for transmission
     * the compound is then put into colleagues.queue
     */
    virtual void
    doSendData(const wns::ldk::CompoundPtr& compound);

    /**
     * @brief lower FU wants to deliver us a PDU (compound), reception
     */
    virtual void
    doOnData(const wns::ldk::CompoundPtr& compound);

    /**
    * @brief Called 3ms after doOnData received data, to model decoding delay
    */
    void
    postDecoding(const wns::ldk::CompoundPtr compound);

    /**
     * @brief upper FU is notified that we have space again for new PDUs
     */
    virtual void
    doWakeup();

    //@}

    /** @name MapInfoProviderInterface */
    //@{

    /**
     * @brief return the SchedulingMap resulting from the scheduling performed
     * by colleagues.strategy. Called by MapHandler in Lte.
     */
    wns::scheduler::SchedulingMapPtr
    getSchedulingMap(int frameNr) const;
    //@}

    /** @name implementation of Scheduler Interface */
    //@{

    /**
     * @brief dequeue pdu=compounds from scheduledCompounds and
     * send them downstack using sendData(pdu);
     * startCollection = make scheduling decisions,
     * finishCollection = send scheduled PDUs
     */
    void
    finishCollection(int frameNr, simTimeType _startTime);
    //@}

    virtual void
    sendPendingHARQFeedback();

    /** @name implementation of SchedulerFUInterface */
    //@{
    /**
     *  @brief in ILayer2::onNodeCreated() the handle for the PHY service is
     *  set here; This is done for resourceScheduler and resourceSchedulerRX
     *  @see lte::ILayer2::ILayer2::onNodeCreated()
     */
    virtual void
    onNodeCreated();
    //@}

    /** @brief Handle Intra-FUN dependencies and create colleagues -
        Get pointer to rlc, macg, phyUser
        Make object instances for the following helper classes:
        create (a) grouper
        create (b) queues
        create (c) the scheduling strategy
        handle colleagues (the objects a-c above)
    */
    virtual void
    onFUNCreated();

    /**
     * @brief AssociationObserver interface
     */
    virtual void
    onAssociated(wns::service::dll::UnicastAddress userAdr,
                 wns::service::dll::UnicastAddress dstAdr) = 0;

    /**
     * @brief AssociationObserver interface
     */
    virtual void
    onDisassociated(wns::service::dll::UnicastAddress userAdr,
                    wns::service::dll::UnicastAddress dstAdr) = 0;

    virtual void
    onFlowReleased(wns::service::dll::FlowID flowId,
                   wns::scheduler::UserID user /*nextHop!*/);

    /** @name ScorerInterface */
    //@{
    /**
     * @brief Implementation of ScorerInterface, forwarding to internal
     * Scorer member
     */
    virtual lte::helper::Route
    score(const wns::ldk::CompoundPtr& compound);
    //@}

    /**
     * @brief MapHandler needs to find out the group for partitioning
     */
    int
    getPartitionGroup();

    /**
     * @brief get+write statistics for prepared maps (on BS side
     */
    void
    prepareMapOutput();

    void
    evaluateMap(wns::scheduler::SchedulingMapPtr schedulingMap, int frameNr);

    void
    closeMapOutput();

protected:
    /**
     * @brief Starts the scheduling.
     */
    void
    startCollection(int frameNr, simTimeType duration);

    /**
     * @brief Unpack a SchedulingTimeSlotPtr and send its contents
     * to upper layers.
     */
    void
    deliverSchedulingTimeSlot(bool canBeDecoded,
                              const wns::scheduler::SchedulingTimeSlotPtr& schedulingTimeSlot,
                              wns::service::phy::power::PowerMeasurementPtr&,
                              int subband);

    virtual void
    deliverReceived();

    virtual void
    applyPowerLimitation(wns::scheduler::SchedulingMapPtr);

    void 
    probeResourceUsage(wns::scheduler::SchedulingMapPtr schedulingMap);

    //MetaScheduler
    wns::scheduler::metascheduler::IMetaScheduler* 
    metaScheduler;
    
    /** @brief Python Config View */
    wns::pyconfig::View pyConfig;

    wns::logger::Logger logger;

    dll::ILayer2* layer2;

    /**
     * @briefpointer to associationService, to query about associations
     */
    dll::services::control::Association* associationService;

    /**
     * @brief friend classes. We need direct access to them.
     */
    struct Friends {
        Friends() :
            rlcReader(NULL),
            macg(NULL),
            phyUser(NULL),
            timer(NULL),
            flowManager(NULL),
            mapHandler(NULL),
            rrHandler(NULL),
            partitioningInfo(NULL)
            {
            };

        wns::ldk::CommandReaderInterface* rlcReader;
        lte::macg::MACg* macg;
        lte::macr::PhyUser* phyUser;
        lte::timing::TimingScheduler* timer;
        lte::controlplane::flowmanagement::FlowManager* flowManager;
        lte::controlplane::IMapHandlerRS* mapHandler;
        lte::controlplane::RRHandler* rrHandler;
        lte::timing::partitioning::PartitioningInfo* partitioningInfo;
    } friends;

    /**
     * @brief colleagues are objects that are no FU themselves but helpers
     */
    struct Colleagues {
        Colleagues() :
            registry(NULL), 
            strategy(NULL), 
            queue(NULL),
            grouper(NULL), 
            harq(NULL)
        {
        };

        wns::scheduler::strategy::StrategyInterface* strategy;
        lte::timing::RegistryProxy* registry;
        wns::scheduler::queue::QueueInterface* queue;
        wns::scheduler::grouper::GroupingProviderInterface* grouper;
        wns::scheduler::harq::HARQInterface* harq;
    } colleagues;

    /**
     * @brief usually 5..20 frames make one superframe
     */
    int framesPerSuperFrame;

    /**
     * @brief size of resources in time-direction
     */
    simTimeType slotDuration;

    /**
     * @brief size of resources in frequency-direction
     */
    unsigned int freqChannels;

    /**
     * @brief numer of slots in one frame (TDMA component)
     */

    unsigned int numberOfTimeSlots;

    /**
     * @brief size of resources in spatial direction.
     * This can be beamforming beams (available for WiMAC)
     * or MIMO paths (not yet available).
     */
    unsigned int maxBeams;

    /**
     * @brief beamforming on/off
     */
    bool beamforming;

    /**
     * @brief Indicates to which group this scheduler belongs
     * taken from PyConfig
     */
    int partitionGroup;

    /**
     * @brief the scheduling result is stored here for each frame [0..9]
     */
    wns::scheduler::strategy::StrategyResultHistory schedulingResultOfFrame;

    /**
     * @brief member providing the scoring for outgoing compounds
     */
    lte::macr::Scorer scorer;

    /**
     * @brief probe to record ratio of used resources
     */
    wns::probe::bus::ContextCollectorPtr resUsageProbe_;

    wns::probe::bus::ContextCollectorPtr ulTBSizeProbe_;

    wns::probe::bus::ContextCollectorPtr numRetransmissionsProbe_;

    wns::probe::bus::ContextCollectorPtr totalTxDelayProbe_;

    /**
     * @brief there are three positions for the scheduler...
     */
    wns::scheduler::SchedulerSpotType schedulerSpot;

    /**
     * @brief Master scheduler for DL or UL?
     * true only for RS-RX. Set in Python
     */
    bool IamUplinkMaster;

    /**
     * @brief output file names
     */
    bool writeMapOutput;

    std::string mapOutputFileName;

    std::ofstream *mapFile;

    long int transportBlockCounter;

    wns::scheduler::harq::HARQInterface::DecodeStatusContainer receivedNonHARQTimeslots_;

    wns::Power maxTxPower;
};
}
} // lte::timing

#endif // NOT defined ...
