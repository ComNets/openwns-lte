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

#include <LTE/timing/ResourceScheduler.hpp>
#include <LTE/controlplane/flowmanagement/FlowManager.hpp>

#include <LTE/macg/MACg.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/macr/PhyUser.hpp>
#include <LTE/controlplane/associationHandler/AssociationHandler.hpp>
#include <LTE/timing/RegistryProxy.hpp>
#include <LTE/timing/partitioning/PartitioningInfo.hpp>
#include <LTE/helper/QueueProxy.hpp>

#include <WNS/ldk/helper/FakePDU.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/utils.hpp>

#include <boost/bind.hpp>

#include <fstream>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace std;
using namespace lte::timing;


ResourceScheduler::ResourceScheduler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::CommandTypeSpecifier<SchedulerCommand>(fun),
    HasModeName(config),
    pyConfig(config),
    logger(config.get("logger")),
    layer2(NULL),
    associationService(NULL),
    friends(),
    colleagues(),
    framesPerSuperFrame(config.get<int>("framesPerSuperFrame")),
    slotDuration(config.get<simTimeType>("slotDuration")), // currently assumed constant for all timeSlots
    freqChannels(config.get<int>("freqChannels")),
    numberOfTimeSlots(config.get<int>("numberOfTimeSlots")),
    maxBeams(config.get<int>("maxBeams")),
    beamforming(config.get<bool>("grouper.beamforming")),
    partitionGroup(config.get<uint32_t>("group")),
    schedulingResultOfFrame(framesPerSuperFrame), // vector schedulingResultOfFrame[frameNr]
    scorer(mode,config),
    schedulerSpot(-1), // initialized to good value in setFriends
    IamUplinkMaster(config.get<bool>("uplinkMaster")), // uplink==true only for RS-RX
    writeMapOutput(config.get<bool>("writeMapOutput")),
    mapFile(NULL),
    transportBlockCounter(0), 
    metaScheduler(wns::scheduler::metascheduler::IMetaScheduler::getMetaScheduler(config.getView("metaScheduler"))),
    
    maxTxPower(config.get<wns::Power>("maxTxPower"))
{
    MESSAGE_SINGLE(NORMAL, logger,"Constructor: Strategy="<<config.get<std::string>("strategy.nameInStrategyFactory"));
    if (writeMapOutput)
    {
        std::string outputDir =
            wns::simulator::getConfiguration().get<std::string>("outputDir");
        mapOutputFileName = outputDir + "/" + config.get<std::string>("mapOutputFileName");
        MESSAGE_SINGLE(NORMAL, logger,"writing maps to "<<mapOutputFileName);
    } // else they are None

    colleagues.harq = STATIC_FACTORY_NEW_INSTANCE(wns::scheduler::harq::HARQInterface, 
        wns::PyConfigViewCreator, pyConfig.get("harq"), pyConfig.get("harq"));
    assure(colleagues.harq, "HARQ creation failed");

	wns::probe::bus::ContextProviderCollection* cpcParent = &fun->getLayer()->getContextProviderCollection();
    wns::probe::bus::ContextProviderCollection cpc(cpcParent);
    resUsageProbe_ = wns::probe::bus::collector(cpc, config, "resUsageProbeName");
    ulTBSizeProbe_ = wns::probe::bus::collector(cpc, config, "ulTBSizeProbeName");

} // ResourceScheduler


ResourceScheduler::~ResourceScheduler()
{
    MESSAGE_SINGLE(VERBOSE, logger, "ResourceScheduler::~ResourceScheduler()");
    if(colleagues.registry)
    {
        delete colleagues.registry; 
        colleagues.registry = NULL;
    }
    if(colleagues.strategy) 
    {
        delete colleagues.strategy; 
        colleagues.strategy = NULL;
    }
    if(colleagues.grouper)
    {
        delete colleagues.grouper;
        colleagues.grouper = NULL;
    }
    if(colleagues.queue)  
    {
        delete colleagues.queue;
        colleagues.queue = NULL;
    }
    for(int frame = 0; frame < framesPerSuperFrame; ++frame)
    {
        schedulingResultOfFrame[frame] = wns::scheduler::strategy::StrategyResultPtr();
    }
    if(writeMapOutput) 
        closeMapOutput();

    MESSAGE_SINGLE(VERBOSE, logger, "ResourceScheduler::~ResourceScheduler() ready");
} // ~ResourceScheduler

// for MapHandler
int
lte::timing::ResourceScheduler::getPartitionGroup()
{
    return partitionGroup;
}

// this is called by ILayer2::onNodeCreated() via fun->onFUNCreated();
void
ResourceScheduler::onFUNCreated()
{
    MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onFUNCreated()");
    wns::ldk::fun::FUN* fun = getFUN();
    assure(fun, "Could not get my FUN");

    layer2 = fun->getLayer<dll::ILayer2*>();
    
    assure(layer2, "could not get ILayer2 from FUN");

    std::string flowmanagername = "FlowManager";
    
    if(layer2->getStationType() == wns::service::dll::StationTypes::UE()) 
    {
        flowmanagername += "UT";
    } 
    else if(layer2->getStationType() == wns::service::dll::StationTypes::FRS()) 
    {
        flowmanagername += "RN";
    } 
    else if(layer2->getStationType() == wns::service::dll::StationTypes::eNB())
    { // BS
        flowmanagername += "BS";
    }

    // remember association service
    associationService = 
        layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);

    // Register as Observer at the association Info providers
    dll::ILayer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(mode);
    dll::ILayer2::AssociationInfoContainer::const_iterator iter = ais.begin();
    for(; iter != ais.end(); ++iter)
        this->startObserving(*iter);

    if (mode == modeBase)
        friends.macg = fun->findFriend<lte::macg::MACg*>("macg");
    else
        friends.macg = fun->findFriend<lte::macg::MACg*>("macg"+separator+taskID);

    friends.rlcReader = fun->getCommandReader("rlc");
    friends.phyUser = fun->findFriend<lte::macr::PhyUser*>(modeBase+separator+"phyUser");
    friends.mapHandler = 
        fun->findFriend<lte::controlplane::IMapHandlerRS*>(mode+separator+"mapHandler");

    friends.rrHandler = 
        fun->findFriend<lte::controlplane::RRHandler*>(mode+separator+"RRHandler");

    friends.timer = 
        layer2->getManagementService<lte::timing::TimingScheduler>(mode+separator+"Timer");

    friends.flowManager = 
        layer2->getControlService<lte::controlplane::flowmanagement::FlowManager>(flowmanagername);

    friends.partitioningInfo = 
        layer2->getControlService<lte::timing::partitioning::PartitioningInfo>("PARTITIONINGINFO"+modeBase);

    assure(friends.flowManager != NULL,"friends.flowManager == NULL");
    assure(friends.rrHandler != NULL,"friends.rrHandler == NULL");

    // inform my scorer about things it needs to know
    scorer.setRLC(friends.rlcReader);
    scorer.setAssociationService(associationService);

    std::string registryName = pyConfig.get<std::string>("registry.nameInRegistryProxyFactory");
    std::string strategyName = pyConfig.get<std::string>("strategy.nameInStrategyFactory");
    std::string grouperName = pyConfig.get<std::string>("grouper.nameInGrouperFactory");

    // the first thing to do is to set up the registry because other colleagues
    // may depend on it for their initialization
    wns::pyconfig::View registryView = pyConfig.get<wns::pyconfig::View>("registry");
    wns::scheduler::RegistryCreator* registryCreator = wns::scheduler::RegistryFactory::creator(registryName);
    colleagues.registry = dynamic_cast<lte::timing::RegistryProxy*>(registryCreator->create(fun, registryView));
    assure(colleagues.registry, "Registry creation failed");

    colleagues.registry->setFUN(fun); 
    colleagues.registry->setHARQ(colleagues.harq);
    colleagues.registry->setAssociationHandler(fun->findFriend<lte::controlplane::associationHandler::AssociationHandler*>(mode+separator+ "associationHandler"));
    
    // attach scheduler to metascheduler
    if(layer2->getStationType() == wns::service::dll::StationTypes::UE()) 
    { //UT
        metaScheduler->attachUT(& pyConfig,colleagues.registry); 
    } 
    else if(layer2->getStationType() == wns::service::dll::StationTypes::eNB())
    { // BS
        metaScheduler->attachBS(& pyConfig,colleagues.registry,IamUplinkMaster);
    } 
    
    // create the grouper
    wns::scheduler::grouper::SpatialGrouperCreator* grouperCreator = wns::scheduler::grouper::SpatialGrouperFactory::creator(grouperName);
    colleagues.grouper = grouperCreator->create(pyConfig.get<wns::pyconfig::View>("grouper"));
    assure(colleagues.grouper, "Grouper creation failed");
    colleagues.grouper->setColleagues(colleagues.registry);

    // create the scheduling strategy (subStrategies are set there)
    wns::scheduler::strategy::StrategyCreator* strategyCreator = wns::scheduler::strategy::StrategyFactory::creator(strategyName);
    colleagues.strategy = strategyCreator->create(pyConfig.get<wns::pyconfig::View>("strategy"));
    assure(colleagues.strategy, "Strategy module creation failed");

    if (IamUplinkMaster) 
    { 
        MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onFUNCreated(): Strategy creation finished. Now init RS-RX (UL master) specific tasks.");
        // create the queueProxy
        // The queueProxy knows the real queue if available (Tx) or the RRHandler (Rx)
        // It can also imitate a "dynamic segmentation" by sending specific commands...
        std::string queueProxyName = pyConfig.get<std::string>("queueProxy.nameInQueueFactory");
        MESSAGE_SINGLE(NORMAL, logger, "creating queueProxy of type " << queueProxyName);

        wns::pyconfig::View queueProxyView = pyConfig.get<wns::pyconfig::View>("queueProxy");
        wns::scheduler::queue::QueueCreator* queueProxyCreator = wns::scheduler::queue::QueueFactory::creator(queueProxyName);
        wns::scheduler::queue::QueueInterface* queueProxySimple = queueProxyCreator->create(NULL, queueProxyView);
        assure(queueProxySimple != NULL, "QueueProxySimple creation failed");
        colleagues.queue = queueProxySimple; 
        MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onFUNCreated(): QueueProxy created");

        lte::helper::QueueProxy* queueProxy = dynamic_cast<lte::helper::QueueProxy*>(queueProxySimple);
        assure(queueProxy != NULL, "QueueProxy creation failed");
        queueProxy->setRRHandler(friends.rrHandler);
        friends.rrHandler->setColleagues(colleagues.registry);

        // Create the HARQRetransmissionProxy for the uplink scheduler
        wns::scheduler::harq::HARQInterface* downlinkHARQ = NULL;
        downlinkHARQ = fun->findFriend<lte::timing::ResourceScheduler*>(
            pyConfig.get<std::string>("txSchedulerFUName"))->colleagues.harq;

        colleagues.harq->setDownlinkHARQ(downlinkHARQ);

        colleagues.strategy->setColleagues(colleagues.queue, 
                                           colleagues.grouper,
                                           colleagues.registry,
                                           colleagues.harq);

        queueProxy->setColleagues(colleagues.registry);
    } 
    else 
    {
        MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onFUNCreated(): Strategy creation finished. Now init RS-TX specific tasks.");
        // create the queues
        std::string queueName = pyConfig.get<std::string>("queue.nameInQueueFactory");
        MESSAGE_SINGLE(NORMAL, logger, "creating queue of type "<<queueName);
        wns::pyconfig::View queueView = pyConfig.get<wns::pyconfig::View>("queue");
        wns::scheduler::queue::QueueCreator* queueCreator = wns::scheduler::queue::QueueFactory::creator(queueName);
        colleagues.queue = queueCreator->create(NULL, queueView);
        assure(colleagues.queue, "Queue creation failed");

        colleagues.strategy->setColleagues(colleagues.queue,
                                           colleagues.grouper,
                                           colleagues.registry,
                                           colleagues.harq);
    }
    assure(colleagues.queue != NULL,"colleagues.queue==NULL");
    colleagues.queue->setColleagues(colleagues.registry);

    if (writeMapOutput) 
        prepareMapOutput();

    MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onFUNCreated() finished");
} // onFUNCreated

// called in ILayer2::onNodeCreated() to connect PHY service with spatial grouper
void
ResourceScheduler::onNodeCreated()
{
    MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onNodeCreated()");
    wns::service::phy::ofdma::DataTransmission* ofdmaProvider = friends.phyUser->getDataTransmissionService();
    colleagues.grouper->setFriends(ofdmaProvider);
    colleagues.strategy->setFriends(ofdmaProvider);
    colleagues.queue->setFUN(getFUN());
    schedulerSpot = colleagues.strategy->getSchedulerSpotType(); 
} // onNodeCreated(): setOFDMAService

void
ResourceScheduler::onFlowReleased(wns::service::dll::FlowID flowId, wns::scheduler::UserID user /*nextHop!*/)
{
    MESSAGE_SINGLE(NORMAL, logger, "ResourceScheduler::onFlowReleased("<<flowId<<")");
    colleagues.queue->resetQueue(flowId); 
    colleagues.registry->deregisterCID(flowId, user);
}

bool
ResourceScheduler::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
    return true;
} // isAccepting

void
ResourceScheduler::doSendData(const wns::ldk::CompoundPtr& compound)
{
    assure(compound, "sendData called with an invalid compound.");
    colleagues.queue->put(compound);
} // doSendData

void
ResourceScheduler::deliverSchedulingTimeSlot(
    bool canBeDecoded,
    const wns::scheduler::SchedulingTimeSlotPtr& schedulingTimeSlot,
    wns::service::phy::power::PowerMeasurementPtr& phyMeasurement,
    int subband)
{
    // Iterate over spatial domain
    for(wns::scheduler::PhysicalResourceBlockVector::iterator it = schedulingTimeSlot->physicalResources.begin();
        it != schedulingTimeSlot->physicalResources.end();
        ++it)
    {
        // Iterate over all contained compounds
        for (wns::scheduler::ScheduledCompoundsList::const_iterator compoundIt = it->scheduledCompoundsBegin();
             compoundIt != it->scheduledCompoundsEnd();
             ++compoundIt)
        {
            // There may be Non-HARQ transmissions in this SchedulingTimeSlot
            // For now these are error free and delivered regardless of canBeDecoded.
            // So: only check canBeDecoded flag for HARQ compounds
            // Do not deliver NON-HARQ compounds in a retry, they already have been
            // delivered.
            if ( (compoundIt->harqEnabled && (!canBeDecoded)) ||
                 (schedulingTimeSlot->harq.retryCounter > 0 && (!compoundIt->harqEnabled)))
            {
                continue;
            }

            MESSAGE_BEGIN(NORMAL, logger, m, "onData: ");
            if(friends.rlcReader->commandIsActivated(compoundIt->compoundPtr->getCommandPool())) 
            {
                lte::macg::MACgCommand* macgCommand = friends.macg->getCommand(compoundIt->compoundPtr->getCommandPool());
                lte::rlc::RLCCommand* rlcCommand = friends.rlcReader->readCommand<lte::rlc::RLCCommand>(compoundIt->compoundPtr->getCommandPool());
                m << "received PDU";
                m << " from source " << A2N(rlcCommand->peer.source)
                  << " via " << A2N(macgCommand->peer.source);
            } 
            else 
            {
                m << " rlcReader->commandIsActivated=false !!!";
            }
            MESSAGE_END();

            if(!getFUN()->getProxy()->commandIsActivated(compoundIt->compoundPtr->getCommandPool(), this))
            {
                this->activateCommand(compoundIt->compoundPtr->getCommandPool());
            }
            SchedulerCommand* myCommand = this->getCommand(compoundIt->compoundPtr->getCommandPool());
            myCommand->local.phyMeasurementPtr = phyMeasurement;
            myCommand->local.subBand = subband;

            getDeliverer()->getAcceptor(compoundIt->compoundPtr)->onData(compoundIt->compoundPtr);
        }

        it->clearScheduledCompounds();
    }
    schedulingTimeSlot->physicalResources.clear();
}

void
ResourceScheduler::deliverReceived()
{
    wns::scheduler::harq::HARQInterface::DecodeStatusContainer::iterator it;

    MESSAGE_SINGLE(NORMAL, logger, "Delivering " 
        << receivedNonHARQTimeslots_.size() 
        << " NON-HARQ transmissions");

    // First deliver all the non HARQ transmissions
    for (it = receivedNonHARQTimeslots_.begin();
         it!= receivedNonHARQTimeslots_.end(); ++it)
    {
      MESSAGE_SINGLE(NORMAL, logger, "Delivering NON-HARQ timeslot");
        deliverSchedulingTimeSlot(it->first->harq.successfullyDecoded,
                                  it->first,
                                  it->second.powerMeasurement_,
                                  it->second.sc_);
    }

    // Clear the non HARQ buffer
    receivedNonHARQTimeslots_.clear();

    // Now proceed with all that HARQ was able to decode
    wns::scheduler::harq::HARQInterface::DecodeStatusContainer compounds = colleagues.harq->decode();

    MESSAGE_SINGLE(NORMAL, logger, "Delivering " << compounds.size() << " HARQ transmissions");

    for (it=compounds.begin(); it!=compounds.end();++it)
    {
        deliverSchedulingTimeSlot(it->first->harq.successfullyDecoded,
                                  it->first,
                                  it->second.powerMeasurement_,
                                  it->second.sc_);
    }
}

// PDU or ResourceBlock from PHY came in ... just send up. 
// Someone else is responsible for the correct order (reordering).
void
ResourceScheduler::doOnData(const wns::ldk::CompoundPtr& compound)
{
    SchedulerCommand* myCommand = this->getCommand(compound->getCommandPool());
    lte::macr::PhyCommand* phyCommand = friends.phyUser->getCommand(compound->getCommandPool());

    wns::scheduler::SchedulingTimeSlotPtr ts = myCommand->magic.schedulingTimeSlotPtr;

    MESSAGE_SINGLE(NORMAL, logger, "Received timeslot");

    // after approx 3ms processing delay
    wns::simulator::getEventScheduler()->scheduleDelay(
        boost::bind(&ResourceScheduler::postDecoding, this, compound), 0.002999);
}

void
ResourceScheduler::postDecoding(const wns::ldk::CompoundPtr compound)
{
    assure(compound, "onData called with an invalid compound.");

    SchedulerCommand* myCommand = this->getCommand(compound->getCommandPool());
    lte::macr::PhyCommand* phyCommand = friends.phyUser->getCommand(compound->getCommandPool());

    wns::scheduler::SchedulingTimeSlotPtr ts = myCommand->magic.schedulingTimeSlotPtr;

    // A ResourceBlock has arrived (not a single compound/packet)
    if (ts != NULL)
    {
        MESSAGE_SINGLE(NORMAL, logger, "Received compound " <<  myCommand->magic.schedulingTimeSlotPtr->toString());

        wns::scheduler::harq::HARQInterface::TimeSlotInfo ti(phyCommand->local.rxPowerMeasurementPtr,
                                                             phyCommand->local.subBand);
        // Process Non-HARQ protected compounds first
        if (!ts->isHARQEnabled())
        {
            /**
             * @todo dbn,rs: Currently there is only an error model for the HARQ
             * transmissions. For other transmissions an error model would be nice,
             * too. And it can be added here.
             */
	        ts->harq.successfullyDecoded = true;
            wns::scheduler::harq::HARQInterface::DecodeStatusContainerEntry entry(ts, ti);
	        receivedNonHARQTimeslots_.push_back(entry);

	        MESSAGE_SINGLE(NORMAL, logger, "Stored NON-HARQ transmission");
            return;
        }
	    else
	    {
	        if (colleagues.harq != NULL)
	        {
	            colleagues.harq->onTimeSlotReceived(ts, ti);
            }
	    }
    }
} // doOnData

void
ResourceScheduler::doWakeup()
{
} // wakeup

// asked by MapHandler when it prepares the MAPs
wns::scheduler::SchedulingMapPtr
ResourceScheduler::getSchedulingMap(int frameNr) const
{
    return schedulingResultOfFrame[frameNr]->schedulingMap;
}

// triggered by TimingScheduler Event:
void
ResourceScheduler::startCollection(int frameNr,
                                   simTimeType _slotDuration)
{
    MESSAGE_SINGLE(NORMAL, logger,"startCollection(" << wns::scheduler::SchedulerSpot::toString(schedulerSpot)
        <<": frameNr=" << frameNr << ", d=" << _slotDuration*1e6<<"us)");
    assure(frameNr<framesPerSuperFrame,"invalid frameNr="<<frameNr);

    // PREPARE STRATEGY INPUT NOW:
    // due to maxBeams, this is already "MIMO-ready" [rs]
    // generic call for master or slave scheduling
    
    wns::scheduler::strategy::StrategyInput strategyInput(
        freqChannels, double(_slotDuration), numberOfTimeSlots, maxBeams,metaScheduler, NULL);
    strategyInput.beamforming = beamforming;
    strategyInput.setFrameNr(frameNr);
    
    wns::scheduler::SchedulingMapPtr inputSchedulingMap;
    if(schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave())
    {
        colleagues.registry->setDL(false);
        // inputSchedulingMap should be a deep_copy of the original SchedulingMap from BS.
        // Currently it is not!
        // But it works well and this saves a lot of time.
        inputSchedulingMap = friends.mapHandler->getMasterMapForSlaveScheduling(frameNr);
        if (inputSchedulingMap == wns::scheduler::SchedulingMapPtr()) 
        {
            MESSAGE_SINGLE(NORMAL, logger, "SlaveScheduling with null inputSchedulingMap: nothing to do");
            return;
        } 
        else if(inputSchedulingMap->isEmpty()) 
        { // isEmpty is O(C*S) operation
            MESSAGE_SINGLE(NORMAL, logger, "SlaveScheduling with empty inputSchedulingMap: nothing to do");
            return;
        }
        MESSAGE_SINGLE(NORMAL, logger, "SlaveScheduling with inputSchedulingMap");
        
        // This SchedulingMap counts as the "master mask" that the slave scheduler must obeye to.
        assure(schedulingResultOfFrame[frameNr] == wns::scheduler::strategy::StrategyResultPtr(),"ULSlave must not have stored schedulingMaps");
        
        // workaround for wrong slotDuration from Python
        _slotDuration = inputSchedulingMap->getSlotLength(); 
    } 
    else if(schedulerSpot == wns::scheduler::SchedulerSpot::ULMaster()) 
    {
        colleagues.registry->setDL(false);
        MESSAGE_SINGLE(NORMAL, logger, "MasterScheduling for UL");
    } 
    else if(schedulerSpot == wns::scheduler::SchedulerSpot::DLMaster()) 
    {
        colleagues.registry->setDL(true);
        MESSAGE_SINGLE(NORMAL, logger, "MasterScheduling for DL");
    } 

    // prepare INPUT schedulingMap; take care of previous results for this frame or UL-master-maps.
    if (schedulingResultOfFrame[frameNr] != wns::scheduler::strategy::StrategyResultPtr())
    { 
        // there already was a result of this frame (previous round or static allocation)
        // only possible in BS-RS-RX+TX (Master Scheduler)
        assure(schedulerSpot != wns::scheduler::SchedulerSpot::ULSlave(), 
            "wrong schedulerSpot = " << wns::scheduler::SchedulerSpot::toString(schedulerSpot));

        MESSAGE_SINGLE(NORMAL, logger,"schedulingResultOfFrame[frameNr="<<frameNr<<"] is not empty");
        inputSchedulingMap = schedulingResultOfFrame[frameNr]->schedulingMap;
    } 
    else 
    { // fresh new resource grid
        MESSAGE_SINGLE(NORMAL, logger,"schedulingResultOfFrame[frameNr="
            << frameNr <<"] is empty. Creating new one.");
        if(schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave()) 
        {
            assure(schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave(),
                "wrong schedulerSpot = " << wns::scheduler::SchedulerSpot::toString(schedulerSpot));
            assure(inputSchedulingMap != wns::scheduler::SchedulingMapPtr(),
                "inputSchedulingMap must not be empty in slave mode");
        } 
        else 
        {
            inputSchedulingMap = strategyInput.getPreDefinedSchedulingMap();
            int phaseNrAtFrameNr = friends.timer->phaseNumberAtFrame(frameNr);
            std::string resourceDedication  = friends.partitioningInfo->getDedication(phaseNrAtFrameNr, partitionGroup);
            MESSAGE_SINGLE(NORMAL, logger,"Get partitioning: frameNr="
                << frameNr << ", phaseNr=" 
                << phaseNrAtFrameNr << ", d=\""<<resourceDedication<<"\"");

            // get partitioning (may depend on DL/UL), i.e. usable subchannels
            if(schedulerSpot == wns::scheduler::SchedulerSpot::ULMaster())
            { 
                wns::scheduler::UsableSubChannelVector usableSubChannels
                    = friends.partitioningInfo->getUsableSubChannels(phaseNrAtFrameNr, partitionGroup);
                assure(usableSubChannels.size() >= freqChannels,
                    "#usableSubChannels=" << usableSubChannels.size());

                MESSAGE_BEGIN(NORMAL, logger, m, "usableSubChannels: ");
                for(int ii=0; ii< usableSubChannels.size(); ++ii)
                {
                    m << (usableSubChannels[ii] ? "1" : "0");
                }
                MESSAGE_END()

                inputSchedulingMap->maskOutSubChannels(usableSubChannels); // arg is just a const ref
            }
            else if(schedulerSpot==wns::scheduler::SchedulerSpot::DLMaster())
            { 
                const wns::scheduler::UsableSubChannelVector& usableSubChannels
                    = friends.partitioningInfo->getUsableSubChannels(phaseNrAtFrameNr, partitionGroup);
                assure(usableSubChannels.size()>=freqChannels,
                    "#usableSubChannels=" << usableSubChannels.size());

                inputSchedulingMap->maskOutSubChannels(usableSubChannels);
            }
 
            // used in filterReachable(), which must be changed anyway.
            if (resourceDedication == "Feeder")
                colleagues.registry->setRelaysOnly(); 
        } // if master/slave
    } // if empty/nonempty pre-scheduling input
    strategyInput.setInputSchedulingMap(inputSchedulingMap);

    // DO THE SCHEDULING (CALL STRATEGY):
    wns::scheduler::strategy::StrategyResult strategyResult = // copy small container
        colleagues.strategy->startScheduling(strategyInput);
    MESSAGE_SINGLE(VERBOSE, logger, "strategyResult.schedulingMap " << strategyResult.schedulingMap->toString());

    MESSAGE_SINGLE(NORMAL, logger, "startCollection finished");

    assure(_slotDuration == strategyResult.schedulingMap->getSlotLength(),
           "mismatch in slotLength: slotDuration="
            << slotDuration << " vs getSlotLength()="
            << strategyResult.schedulingMap->getSlotLength());

    if(schedulerSpot == wns::scheduler::SchedulerSpot::ULMaster())
    {
        MESSAGE_SINGLE(VERBOSE, logger, "deleteCompoundsInBursts()+deleteCompounds()");

        // MapInfoCollection is not needed anymore
        strategyResult.deleteCompoundsInBursts(); 
        
        // compounds are not needed anymore
        strategyResult.schedulingMap->deleteCompounds(); 

        // full time length on used subchannels (only for resourceUsage probe and plot)
        strategyResult.schedulingMap->grantFullResources(); 
    }

    // save result per frame
    // perhaps more? TODO: antennaPatterns
    schedulingResultOfFrame[frameNr] = wns::scheduler::strategy::StrategyResultPtr(
        new wns::scheduler::strategy::StrategyResult(strategyResult));

    // adjustment for filterReachable in RegistyProxy:
    colleagues.registry->setAllStations(); // => relaysOnly = false;

    if(schedulerSpot == wns::scheduler::SchedulerSpot::DLMaster()) 
    {
        friends.mapHandler->saveDLMap(frameNr,strategyResult.schedulingMap); 
    } 
    else if(schedulerSpot == wns::scheduler::SchedulerSpot::ULMaster()) 
    {
        friends.mapHandler->saveULMap(frameNr,strategyResult.schedulingMap);
    }
} // startCollection()

// called from rap::Events for RAP or ResourceSchedulerUT for UT:
// this method means: really send out the packets to PhyUser...
void
ResourceScheduler::finishCollection(int frameNr, simTimeType _startTime) {
    // startTime is absolute time (==now)
    assure(frameNr < framesPerSuperFrame, "invalid frameNr="<<frameNr);
    transportBlockCounter++;

    if(transportBlockCounter == 0)
    {
        transportBlockCounter++;
    }

    if(schedulingResultOfFrame[frameNr] == wns::scheduler::strategy::StrategyResultPtr())
    {
        MESSAGE_SINGLE(NORMAL, logger,"finishCollection(frameNr="
            << frameNr << ", startTime="
            << _startTime <<"): empty schedulingResult. nothing to do");
        return;
    }
    assure(schedulingResultOfFrame[frameNr] != wns::scheduler::strategy::StrategyResultPtr(),
        "schedulingResultOfFrame[frameNr="<<frameNr<<"]==NULL");

    wns::scheduler::SchedulingMapPtr schedulingMap = schedulingResultOfFrame[frameNr]->schedulingMap;
    double resourceUsage = schedulingMap->getResourceUsage();

    MESSAGE_SINGLE(NORMAL, logger,"finishCollection(frameNr="<<frameNr<<", startTime="<<_startTime<<"): "
                   <<"schedulingMap="<<resourceUsage*100.0<<"% full");

    if(resUsageProbe_->hasObservers())
        probeResourceUsage(schedulingMap);

    wns::scheduler::UserID myOwnUserID = wns::scheduler::UserID(layer2->getNode());
    if (schedulerSpot == wns::scheduler::SchedulerSpot::ULMaster())
    { // only beamforming etc.
        // RS-RX does not need to transmit pdus
        wns::scheduler::GroupingPtr sdmaGrouping = schedulingResultOfFrame[frameNr]->sdmaGrouping;
        if (sdmaGrouping != wns::scheduler::GroupingPtr()) {
            MESSAGE_SINGLE(NORMAL, logger,"finishCollection(frameNr="<<frameNr<<", startTime="<<_startTime<<"): setting antennaPatterns...");
            wns::scheduler::AntennaPatternsPerUser& antennaPatternsPerUser = sdmaGrouping->patterns;
            // foreach user in sdmaGrouping:
            for (wns::scheduler::AntennaPatternsPerUser::const_iterator iter = antennaPatternsPerUser.begin(); iter != antennaPatternsPerUser.end(); ++iter)
            {
                wns::scheduler::UserID user = iter->first;
                wns::service::phy::ofdma::PatternPtr antennaPattern = iter->second;
                MESSAGE_SINGLE(NORMAL, logger,"finishCollection(frameNr="<<frameNr<<"): setReceiveAntennaPattern(user="<<user.getName()<<")");
                friends.phyUser->setReceiveAntennaPattern(user.getNode(), antennaPattern);
            } // foreach user in sdmaGrouping
        } 
        else // if not sdmaGrouping used
        {
            MESSAGE_SINGLE(NORMAL, logger,"finishCollection(frameNr="<<frameNr<<", startTime="<<_startTime<<"): nothing to do");
        }
    } 
    else // RS-TX (BS-DL or UT-UL)
    { 
        // process finished schedulingMap in order to store ResourceBlocks into HARQ processes...
        if (colleagues.harq != NULL)
        { 
            for(wns::scheduler::SubChannelVector::iterator iterSubChannel = 
                    schedulingMap->subChannels.begin(); 
                iterSubChannel != schedulingMap->subChannels.end(); 
                ++iterSubChannel)
            {
                wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
                // TDMA can be easily supported...
                for(wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = 
                        subChannel.temporalResources.begin();
                    iterTimeSlot != subChannel.temporalResources.end(); 
                    ++iterTimeSlot)
                {
                    wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;
                    if (timeSlotPtr->countScheduledCompounds() > 0)
                    {
                        if(timeSlotPtr->harq.transportBlockID == 0)
                        {
                            timeSlotPtr->harq.transportBlockID = transportBlockCounter;
                        }
                        // store ResourceBlock
                        colleagues.harq->storeSchedulingTimeSlot(transportBlockCounter, timeSlotPtr);
                    }
                }
            }
        } // if HARQ

        applyPowerLimitation(schedulingMap);

        // currently the "SDMA antenna patterns" are set for each packet by bfTransmission->startTransmission() in PhyUser
        // but it would be more efficient to set it once, like this:
        // set SDMA antenna patterns first
        wns::scheduler::GroupingPtr sdmaGrouping = schedulingResultOfFrame[frameNr]->sdmaGrouping;
        wns::scheduler::AntennaPatternsPerUser* antennaPatternsPerUser = NULL;
        if (sdmaGrouping != wns::scheduler::GroupingPtr()) 
        {
            antennaPatternsPerUser = &(sdmaGrouping->patterns);
            // currently not needed because there's no support for that in PhyUser and OFDMAPhy            
            if(false) 
            {
                // foreach user in sdmaGrouping:
                for(wns::scheduler::AntennaPatternsPerUser::const_iterator iter = antennaPatternsPerUser->begin(); 
                    iter != antennaPatternsPerUser->end(); 
                    ++iter)
                {
                    wns::scheduler::UserID user = iter->first;
                    wns::service::phy::ofdma::PatternPtr antennaPattern = iter->second;
                    MESSAGE_SINGLE(NORMAL, logger,"finishCollection(frameNr=" << frameNr 
                        << "): setTransmitAntennaPattern(user=" << user.getName() << ")");

                    // There is currently no support for that in PhyUser and OFDMAPhy
                    //friends.phyUser->setTransmitAntennaPattern(user, antennaPattern); 
                } // foreach user in sdmaGrouping
            }
        } // if sdmaGrouping used

        // write schedulingMap to file for visualization purposes:
        if(writeMapOutput)
        { 
            evaluateMap(schedulingMap,frameNr);
        }

        wns::scheduler::SubChannelVector& subChannels  = schedulingMap->subChannels;

        double _slotDuration = schedulingMap->getSlotLength();

        // now send compounds ...
        // iterate over all compounds in schedulingMap
        for(wns::scheduler::SubChannelVector::iterator iterSubChannel = subChannels.begin();
              iterSubChannel != subChannels.end(); 
            ++iterSubChannel)
        {
            wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
            for(wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = 
                    subChannel.temporalResources.begin();
                iterTimeSlot != subChannel.temporalResources.end(); 
                ++iterTimeSlot)
            {
                wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;

                // Skip empty time slots
                if(timeSlotPtr->isEmpty())
                {
                    continue;
                }

                // Skip if no compound inside
                if(timeSlotPtr->countScheduledCompounds()==0)
                {
                    MESSAGE_SINGLE(NORMAL, logger, "timeSlotPtr->countScheduledCompounds()==0");
                    continue;
                }

                assure(timeSlotPtr->physicalResources[0].hasScheduledCompounds()>0,"PRB[0] has zero compounds");
                wns::scheduler::UserID TSuser = timeSlotPtr->getUserID();
                wns::scheduler::UserID PRBuser = timeSlotPtr->physicalResources[0].getUserID();
                assure( TSuser.isValid(), "Invalid TSuser=NULL");
                assure( PRBuser.isValid(), "Invalid PRBuser=NULL");
                assure(TSuser==PRBuser,"finishCollection("
                    << myOwnUserID.getName() << "): PRB on subchannel=" << subChannel.subChannelIndex
                    << " has timeSlotPtr->getUserID()=" << TSuser.getName()
                    << " but physicalResources[0]->getUserID()=" << PRBuser.getName());

                // ^ In the UT these TSuser and PRBuser are still the UT's name (myself)
                // That is important, because the libwns scheduler needs to know where it may put more compounds into;
                // it shall not use PRBs tagged for other UTs. IF ther would be an "BSx" tag,
                // then the UTs could not know which PRB is available for them.
                // Please keep it and don't think about changing this [rs].
                wns::scheduler::UserID user = timeSlotPtr->physicalResources[0].getUserIDOfScheduledCompounds();
                /**
                 * @todo dbn: The scheduling map is inconsistent in the uplink. S
                 * cheduled Compounds have a different user id than the timeslots. 
                 * Under bad channel conditions (400m distance) the assure Invalid user below fires.
                 */
                assure(user.isValid(), "Invalid !user.isValid()");

                // this is not a resource I'm responsible for (other UT)
                if((schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave()) 
                    && (TSuser != myOwnUserID))
                { 
                    MESSAGE_SINGLE(NORMAL, logger, "skipping PRB on subchannel=" << subChannel.subChannelIndex
                                   << " has timeSlotPtr->userID=" << TSuser.getName()
                                   << " and PRB->userID=" << PRBuser.getName()
                                   << " and compound->userID=" << user.getName());
                    continue;
                } 
                else 
                {
                    MESSAGE_SINGLE(NORMAL, logger, "PRB on subchannel=" << subChannel.subChannelIndex
                                   << " has timeSlotPtr->userID=" << TSuser.getName()
                                   << " and PRB->userID=" << PRBuser.getName()
                                   << " and compound->userID=" << user.getName());
                }

                int netDataBits = timeSlotPtr->getNetBlockSizeInBits();

                // (this is one "resource block" for HARQ retransmission)
                wns::ldk::CompoundPtr containerPdu = wns::ldk::CompoundPtr(
                    new wns::ldk::Compound(getFUN()->createCommandPool(),
                        wns::ldk::helper::FakePDUPtr(new wns::ldk::helper::FakePDU(netDataBits)))); 

                // set the scheduler command (contained in myPDU)
                SchedulerCommand* myCommand = this->activateCommand(containerPdu->getCommandPool());

                if (timeSlotPtr->getTxPower() == wns::Power())
                {
                    MESSAGE_SINGLE(NORMAL, logger, "WARNING: 0 dBm transmit power. I will not send this");
                    continue;
                }
                myCommand->magic.schedulingTimeSlotPtr = wns::scheduler::SchedulingTimeSlotPtr(
                    new wns::scheduler::SchedulingTimeSlot(*timeSlotPtr));;

		        // Check for HARQ validity
		        if (timeSlotPtr->isHARQEnabled())
		        {
		            assure(!timeSlotPtr->harq.ackCallback.empty(), 
                        "Trying to retransmit resource block with empty ack callback in HARQUplinkSlaveRetransmission");

		            assure(!timeSlotPtr->harq.nackCallback.empty(),
                        "Trying to retransmit resource block with empty nack callback in HARQUplinkSlaveRetransmission");
		        }

                lte::macr::PhyCommand* phyCommand = dynamic_cast<lte::macr::PhyCommand*>(
                    getFUN()->getProxy()->activateCommand( containerPdu->getCommandPool(), friends.phyUser ));

                // don't send to myself.
                assure(user != myOwnUserID, "Invalid user=" << user.getName()); 

                // currently assumed constant slotDuration for all timeSlots:
                // later TODO: sum (0..timeSlotPtr->timeSlotIndex) timeSlotPtr->slotLength
                simTimeType timeSlotOffset = (timeSlotPtr->timeSlotIndex * slotDuration);
                phyCommand->magic.source = myOwnUserID.getNode();

                if (user.isBroadcast())
                {
                    phyCommand->magic.destination = NULL;
                }
                else
                {
                    phyCommand->magic.destination = user.getNode();
                }
                if(myCommand->magic.schedulingTimeSlotPtr->harq.retryCounter > 0 
                    && myCommand->magic.schedulingTimeSlotPtr->harq.NDI == false)
                {
                    phyCommand->magic.isRetransmission = true;
                }
                else
                {
                    phyCommand->magic.isRetransmission = false;
                }

                // TxPower to used for sending
                phyCommand->magic.txp         = timeSlotPtr->getTxPower(); 
                phyCommand->local.subBand     = timeSlotPtr->subChannelIndex;
                
                if(timeSlotPtr->hasResourcesForUser(myOwnUserID))
                    phyCommand->magic.estimatedSINR = timeSlotPtr->getEstimatedCQI(myOwnUserID);
                
                if(timeSlotPtr->hasResourcesForUser(user))
                {
                    assure(!timeSlotPtr->hasResourcesForUser(myOwnUserID), 
                        "Can't have resources for Tx and Rx");

                    phyCommand->magic.estimatedSINR = timeSlotPtr->getEstimatedCQI(user);
                }

                // just as placeholder; the real phyModes are inside
                phyCommand->local.phyModePtr  = timeSlotPtr->physicalResources[0].getPhyMode(); 

                // time relative to the start of the burst
                phyCommand->local.start       = timeSlotOffset; 

                // time relative to the start of the burst
                phyCommand->local.stop        = timeSlotOffset + _slotDuration;   
                phyCommand->local.start      += _startTime;
                phyCommand->local.stop       += _startTime;

                // safety
                phyCommand->local.stop       -= wns::scheduler::slotLengthRoundingTolerance; 

                // beams are inside
                phyCommand->local.beam        = 0; 
                phyCommand->local.beamforming = beamforming;
                
                if (beamforming) 
                {
                    assure(antennaPatternsPerUser != NULL,"antennaPatternsPerUser == NULL");
                    wns::service::phy::ofdma::PatternPtr antennaPattern = (*antennaPatternsPerUser)[user];
                    assure(antennaPattern != wns::service::phy::ofdma::PatternPtr(),"No Beamforming antennaPattern set.");
                    
                    // see wns::service::phy::ofdma::PatternPtr()
                    phyCommand->local.pattern = antennaPattern; 
                }

                // Rx is obsolete. Maybe completely obsolete
                phyCommand->local.modeRxTx = lte::macr::PhyCommand::Tx; 

                MESSAGE_BEGIN(NORMAL, logger, m, "PRB for ");
                    m << user.getName() << " scheduled on ";
                    m << "frame=" << frameNr << ", subChannel=" << timeSlotPtr->subChannelIndex;
                    m << ", T=[" << int(phyCommand->local.start*1e6+.5) << "-"; 
                    m << int(phyCommand->local.stop * 1e6+.5) << "us]";

                    if (beamforming || (maxBeams > 1)) 
                    { 
                        m << ", beam=" << phyCommand->local.beam; 
                    }

                    m << ", TxP="<<phyCommand->magic.txp << ", TBC="; 
                    m << myCommand->magic.schedulingTimeSlotPtr->harq.transportBlockID;
                    m << ", netBits=" << netDataBits << "/" << containerPdu->getLengthInBits();
                MESSAGE_END();

                assure(phyCommand->local.subBand < freqChannels, "Invalid frequency channel " 
                    << phyCommand->local.subBand);

                assure(phyCommand->local.stop > phyCommand->local.start, 
                    "stop-start = " << phyCommand->local.stop << " - "<< phyCommand->local.start);

                assure(phyCommand->local.phyModePtr->isValid(),
                    "cannot work when !phyMode.isValid()");

                assure(phyCommand->local.phyModePtr->dataRateIsValid(),
                    "cannot work when !phyMode.dataRateIsValid()");

                assure(colleagues.strategy->isTx(),"must be TxScheduler");

                assure(getConnector()->hasAcceptor(containerPdu),
                    "Lower FU is not accepting compound but is supposed to do so");

                getConnector()->getAcceptor(containerPdu)->sendData(containerPdu); // to PhyUser or HARQ
            } // forall timeSlots
        } // forall subchannels
    } // TxScheduler
    
    // clear result; not needed anymore; must be clean before next round
    schedulingResultOfFrame[frameNr] = 
        wns::scheduler::strategy::StrategyResultPtr(); 
} // finishCollection

void 
ResourceScheduler::probeResourceUsage(wns::scheduler::SchedulingMapPtr schedulingMap)
{
    wns::scheduler::SubChannelVector& subChannels  = schedulingMap->subChannels;

    double _slotDuration = schedulingMap->getSlotLength();

    unsigned int numResources = 0;
    unsigned int usedResources = 0;

    for ( wns::scheduler::SubChannelVector::iterator iterSubChannel = subChannels.begin();
          iterSubChannel != subChannels.end(); ++iterSubChannel)
    {
        wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
        for ( wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = subChannel.temporalResources.begin();
              iterTimeSlot != subChannel.temporalResources.end(); ++iterTimeSlot)
        {
            wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;
            // Only count resources for this UT in ULSlave
            if(schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave())
            {
                wns::scheduler::UserID myOwnUserID = wns::scheduler::UserID(layer2->getNode());
                if(timeSlotPtr->getUserID() == myOwnUserID)
                    numResources++;            
            }
            // Count all resources in master
            else
            {
                numResources++;
            }
            
            if(!timeSlotPtr->isEmpty())
               usedResources++;
        }
    }
    if(schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave())
        ulTBSizeProbe_->put(numResources);

    resUsageProbe_->put(double(usedResources)/double(numResources), 
        boost::make_tuple("SchedulerSpot", schedulerSpot));
}

void
ResourceScheduler::applyPowerLimitation(wns::scheduler::SchedulingMapPtr schedulingMap)
{
    // First pass: Find the sum TxPower
    wns::Power sum = wns::Power::from_mW(0.0);
    wns::scheduler::UserID myOwnUserID = wns::scheduler::UserID(layer2->getNode());

    for(wns::scheduler::SubChannelVector::iterator iterSubChannel = 
            schedulingMap->subChannels.begin(); 
        iterSubChannel != schedulingMap->subChannels.end(); 
        ++iterSubChannel)
    {
        wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
        for(wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = 
                subChannel.temporalResources.begin();
            iterTimeSlot != subChannel.temporalResources.end(); 
            ++iterTimeSlot)
        {
            wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;

            if(schedulerSpot==wns::scheduler::SchedulerSpot::ULSlave())
            {
                if(timeSlotPtr->getUserID() != myOwnUserID)
                {
                    // Not my transmission. Belongs to another user
                    continue;
                }
            }
            sum += timeSlotPtr->getTxPower();
        }
    }

    wns::Ratio downscale = wns::Ratio::from_factor(1.0);

    if (sum > maxTxPower)
    {
        downscale = maxTxPower / sum;

        MESSAGE_SINGLE(NORMAL, logger, "Power limitation reached, downscaling by " << downscale);

        // Second pass:
        for(wns::scheduler::SubChannelVector::iterator iterSubChannel = 
                schedulingMap->subChannels.begin(); 
            iterSubChannel != schedulingMap->subChannels.end(); 
            ++iterSubChannel)
        {
            wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
            for(wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = 
                    subChannel.temporalResources.begin();
                iterTimeSlot != subChannel.temporalResources.end(); 
                ++iterTimeSlot)
            {
                wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;

                if(schedulerSpot == wns::scheduler::SchedulerSpot::ULSlave())
                {
                    if(timeSlotPtr->getUserID() != myOwnUserID)
                    {
                        // Not my transmission. Belongs to another user
                        continue;
                    }
                }

                timeSlotPtr->setTxPower(timeSlotPtr->getTxPower() * downscale);
            }
        }
    }
}

void
ResourceScheduler::sendPendingHARQFeedback()
{
    colleagues.harq->sendPendingFeedback();
}

lte::helper::Route
ResourceScheduler::score(const wns::ldk::CompoundPtr& compound)
{
    return scorer.score(compound);
}


void
ResourceScheduler::prepareMapOutput()
{
    // nothing to do
    if(!writeMapOutput) 
        return; 

    MESSAGE_SINGLE(NORMAL, logger, "prepareMapOutput(): opening output file " << mapOutputFileName);

    mapFile = new std::ofstream(mapOutputFileName.c_str());
    assure(mapFile != NULL,"cannot write to mapFile " << mapOutputFileName);
    assure(mapFile->is_open(), "mapFile " << mapOutputFileName << " not open");

    (*mapFile) << "# contents=\"" << logger.getLoggerName() << " map\"" << std::endl;
    wns::scheduler::SchedulingMap::writeHeaderToFile(*mapFile);
}

// analyze MAPs to get performance values from it.
// Probe output and file output for Matlab
void
ResourceScheduler::evaluateMap(wns::scheduler::SchedulingMapPtr schedulingMap, int frameNr)
{
    // nothing to do
    if(!writeMapOutput) 
        return; 

    MESSAGE_SINGLE(NORMAL, logger, "evaluateMap(): writing to " << mapOutputFileName);

    assure(mapFile != NULL,"cannot write to mapFile " << mapOutputFileName);

    // write table output to file for postprocessing with Matlab/Gnuplot
    simTimeType now = wns::simulator::getEventScheduler()->getTime();
    *mapFile << "# t=" << now << ", frameNr=" << frameNr << std::endl;
    std::stringstream p;
    p << now << "\t";
    schedulingMap->writeToFile(*mapFile,p.str());
} // evaluateMap

void
ResourceScheduler::closeMapOutput()
{
    // nothing to do
    if (!writeMapOutput) 
        return; 

    MESSAGE_SINGLE(NORMAL, logger, "closeMapOutput(): closing "<<mapOutputFileName);

    if(*mapFile) 
        mapFile->close();
}

