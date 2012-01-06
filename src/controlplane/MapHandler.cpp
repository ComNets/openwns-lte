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

#include <LTE/controlplane/MapHandler.hpp>

#include <LTE/main/Layer2.hpp>
#include <LTE/timing/TimingScheduler.hpp>
#include <LTE/timing/ResourceScheduler.hpp>
#include <LTE/macr/PhyUser.hpp>

#include <DLL/StationManager.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/service/phy/ofdma/Pattern.hpp>

#include <fstream>
#include <boost/bind.hpp>

using namespace lte;
using namespace lte::controlplane;
using namespace wns;
using namespace wns::ldk;

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

STATIC_FACTORY_REGISTER_WITH_CREATOR(MapHandler, FunctionalUnit, "lte.controlplane.MapHandler", FUNConfigCreator);

MapHandler::MapHandler(wns::ldk::fun::FUN* fun, const pyconfig::View& config) :
    CommandTypeSpecifier<MapCommand>(fun),
    HasReceptor<>(),
    HasConnector<>(),
    HasDeliverer<>(),
    Cloneable<MapHandler>(),
    helper::HasModeName(config),
    layer2(NULL),
    myUser(NULL),
    friends(),
    pduSize(config.get<uint32_t>("pduSize")),
    lowestSubChannel(config.get<uint32_t>("lowestMapChannel")),
    highestSubChannel(config.get<uint32_t>("highestMapChannel")),
    framesPerSuperFrame(config.get<int>("framesPerSuperFrame")),
    scheduledDLResources(framesPerSuperFrame),
    scheduledULResources(framesPerSuperFrame),
    phyModePtr(wns::service::phy::phymode::createPhyMode(config.getView("phyMode"))),
    txPower(config.get<wns::Power>("txPower")),
    sinrProbe(NULL),
    writeMapOutput(config.get<bool>("writeMapOutput")),
    currentPhase(0),
    logger(config.get("logger"))
{
    if (writeMapOutput) {
        mapOutputFileNameDL = config.get<std::string>("mapOutputFileNameDL");
        mapOutputFileNameUL = config.get<std::string>("mapOutputFileNameUL");
    } // else they are None

    dll::ILayer2* layer2 = fun->getLayer<dll::ILayer2*>();
    wns::node::Interface* node = layer2->getNode();
    wns::probe::bus::ContextProviderCollection localIDs(&node->getContextProviderCollection());
    sinrProbe = new wns::probe::bus::ContextCollector(localIDs, "lte.MAP_SINR");
}

MapHandler::~MapHandler()
{
    // better would be in a method onSimulationEnd()
    if (writeMapOutput) closeMapOutput();
    // cleanup:
    scheduledDLResources.clear();
    scheduledULResources.clear();
    delete sinrProbe; sinrProbe = NULL;
}

void
MapHandler::onFUNCreated()
{
    layer2 = getFUN()->getLayer<dll::ILayer2*>();
    myUser = dynamic_cast<lte::main::Layer2*>(getFUN()->getLayer())->getNode();
    friends.associationService = layer2->getControlService<dll::services::control::Association>("ASSOCIATION"+modeBase);
    friends.timer = layer2->getManagementService<lte::timing::TimingScheduler>(mode+separator+"Timer");
    friends.dlrs = getFUN()->findFriend<lte::timing::SchedulerIncoming*>(mode+separator+rsNameSuffix+"TX");
    friends.phyUser = getFUN()->findFriend<lte::macr::PhyUser*>(modeBase+separator+"phyUser");
    iCache = layer2->getManagementService<dll::services::management::InterferenceCache>("INTERFERENCECACHE"+modeBase);
    // obtain pointers to the schedulers that have prepared DL and UL Maps
    // (only needed in RAPs that perform master scheduling)
    try {
        friends.dlmip = getFUN()->findFriend<wns::scheduler::SchedulingMapProviderInterface*>(mode+separator+rsNameSuffix+"TX");
    } catch(...) {
        friends.dlmip = NULL;
    }
    try {
        friends.ulmip = getFUN()->findFriend<wns::scheduler::SchedulingMapProviderInterface*>(mode+separator+rsNameSuffix+"RX");
    } catch(...) {
        friends.ulmip = NULL;
    }
    // remember resource partitioning information service
    partInfo = layer2->getControlService<lte::timing::partitioning::PartitioningInfo>("PARTITIONINGINFO"+modeBase);
    initSuperFrameMap();
    if (writeMapOutput) prepareMapOutput();
}

bool
MapHandler::doIsAccepting(const CompoundPtr& /* compound */) const
{
    // will never be called since nobody is likely to downconnect on the MapHandler
    return false;
} // isAccepting

void
MapHandler::doSendData(const CompoundPtr& /* compound */)
{
    assure(false, "sendData of MapHandler");
} // doSendData

void
MapHandler::saveDLMap(int frameNr, wns::scheduler::SchedulingMapPtr schedulingMap)
{
    assure(frameNr < (int)scheduledDLResources.size(), "invalid frameNumber");
    scheduledDLResources[frameNr] = schedulingMap;
    MESSAGE_SINGLE(NORMAL, logger, "MapHandler::saveDLMap");
    MESSAGE_SINGLE(VERBOSE, logger, "MapHandler::saveDLMap schedulingMAP=" << scheduledDLResources[frameNr]->toString());
}

void
MapHandler::saveULMap(int frameNr, wns::scheduler::SchedulingMapPtr schedulingMap)
{
    assure(frameNr < (int)scheduledULResources.size(), "invalid frameNumber");
    scheduledULResources[frameNr] = schedulingMap;
    MESSAGE_SINGLE(NORMAL, logger, "MapHandler::saveULMap");
    MESSAGE_SINGLE(VERBOSE, logger, "MapHandler::saveULMap schedulingMAP=" << scheduledULResources[frameNr]->toString());
}
void
MapHandler::resetResources(int frameNr)
{
    assure(frameNr < (int)scheduledDLResources.size(), "invalid frameNumber");
    assure(frameNr < (int)scheduledULResources.size(), "invalid frameNumber");
    scheduledDLResources[frameNr] = wns::scheduler::SchedulingMapPtr();
    scheduledULResources[frameNr] = wns::scheduler::SchedulingMapPtr();

    int groupNumber = dynamic_cast<lte::timing::ResourceScheduler*>(friends.dlmip)->getPartitionGroup();
    int phaseNumber  = friends.timer->phaseNumberAtFrame(frameNr);
}

void
MapHandler::initSuperFrameMap()
{
    MESSAGE_SINGLE(NORMAL, logger,"initSuperFrameMap(frames 0.."<<framesPerSuperFrame-1<<")");

    int groupNumber = dynamic_cast<lte::timing::ResourceScheduler*>(friends.dlmip)->getPartitionGroup();
    assure(friends.timer, "MapHandler requires a friend with name: "+mode+separator+"Timer");
    friends.timer->initStationTaskPhases(); // needed before timer->phaseNumberAtFrame(i)
    for (int i=0; i<framesPerSuperFrame; i++)
    {
        int phaseNumber  = friends.timer->phaseNumberAtFrame(i);
        scheduledDLResources[i] = wns::scheduler::SchedulingMapPtr(); // empty map
        scheduledULResources[i] = wns::scheduler::SchedulingMapPtr(); // empty map
    }
}

// incoming MAP from RAP to RUT
void
MapHandler::doOnData(const CompoundPtr& compound)
{
    /** process incoming DL/UL Map transmission (from BS to us: UT,RN)*/
    assure (compound, "Invalid Map compound received!");

    MapCommand* myCommand = getCommand( compound->getCommandPool());

    /** Check if this is the Map we're supposed to listen to */
    wns::service::dll::UnicastAddress listeningTo;

    if (friends.associationService->hasAssociation())
        listeningTo = friends.associationService->getAssociation();
    wns::service::dll::UnicastAddress comingFrom = myCommand->magic.source;

    // this is for me...
    if (listeningTo == comingFrom && friends.timer->stationTaskAtOffset(0.0/*now*/) == timing::StationTasks::UT() )
    {
        MESSAGE_SINGLE(NORMAL, logger,"doOnData(): pdu from "<<A2N(comingFrom)<<" contains MAP");
        using namespace wns::scheduler;

        lte::macr::PhyCommand* phyCommand = friends.phyUser->getCommand(compound->getCommandPool());
        sinrProbe->put(compound, phyCommand->local.rxPowerMeasurementPtr->getSINR().get_dB());

        // Store SINR value in interference cache 
        wns::simulator::getEventScheduler()->scheduleDelay(boost::bind(
            &dll::services::management::InterferenceCache::storeMeasurements,
            iCache,
            myUser,
            phyCommand->local.rxPowerMeasurementPtr,
            dll::services::management::InterferenceCache::Remote,
            WIDEBAND), 0.004);

        // get latest received UL+DL-MAP:
        SchedulingMapCollectionVector scheduledULResourceMAP = myCommand->peer.ulSchedulingMapVector;
        SchedulingMapCollectionVector scheduledDLResourceMAP = myCommand->peer.dlSchedulingMapVector;

        std::vector<int> dlFrameNumbers = myCommand->peer.dlFrameNumbers;
        std::vector<int> ulFrameNumbers = myCommand->peer.ulFrameNumbers;

        // check if at least one of the frames contains granted ul resources
        bool ulResourcesAvailable = false;

        // multi-frame maps (support half duplex FDD [sdr])
        for (size_t i=0; i<dlFrameNumbers.size(); i++)
        {
            int frameNumber = dlFrameNumbers[i];
            assure(scheduledDLResourceMAP[i]!=wns::scheduler::SchedulingMapPtr(),"empty DLMAP["<<i<<" = frame"<<frameNumber<<"]");
            MESSAGE_SINGLE(NORMAL, logger,"Received DL MAP for frameOffset="<<i
                           <<", DLsize="<<scheduledDLResourceMAP[i]->getResourceUsage()*100.0<<"%"
                           <<")");
            // At this point we could make a deep_copy ! (see below)
            scheduledDLResources[frameNumber] = scheduledDLResourceMAP[i];
            MESSAGE_SINGLE(NORMAL, logger,"DLMAP[frame="<<frameNumber<<"]="<<scheduledDLResources[frameNumber]->toString());
        } // for all DL frames in MAP

        for (size_t i=0; i<ulFrameNumbers.size(); i++)
        {
            int frameNumber = ulFrameNumbers[i];
            // UL+DL MAPs (ULMAP is important for UT.RS-TX but DLMAP is not needed anymore)
            assure(scheduledULResourceMAP[i]!=wns::scheduler::SchedulingMapPtr(),"empty ULMAP["<<i<<" = frame"<<frameNumber<<"]");
            MESSAGE_SINGLE(NORMAL, logger,"Received UL MAP for frameOffset="<<i
                           <<" (ULsize="<<scheduledULResourceMAP[i]->getResourceUsage()*100.0<<"%"
                           <<")");
            // At this point we could make a deep_copy ! (see below)
            scheduledULResources[frameNumber] = scheduledULResourceMAP[i];
            if (scheduledULResources[frameNumber]->hasResourcesForUser(wns::scheduler::UserID(myUser)))
            {
                ulResourcesAvailable = true; // assume always true when a MAP comes
                // At this point we should make a deep_copy !
            }
            MESSAGE_SINGLE(NORMAL, logger,"ULMAP[frame="<<frameNumber<<"]="<<scheduledULResources[frameNumber]->toString());
        } // for all UL frames in MAP

        // Inform all interested observers whether or not resources have been granted
        if (ulResourcesAvailable) {
            MESSAGE_SINGLE(NORMAL, logger, "doOnData(): ulResourcesAvailable => sendNotifies...");
        }
        // this calls resourcesGranted(bool) in RRHandler + CQIHandler:
        this->sendNotifies(&ResourceGrantNotificationInterface::resourcesGranted, ulResourcesAvailable);
        // if (ulResourcesAvailable==true): MapHandler says: "choosing cpDispatcher" instead of RACH
    } else {
        // received compound from a source (RAP) we are not associated with -> ignore it
        return;
    }
} // doOnData

// UT asks for allowed resources (old method?). timing/ut/Events.cpp
wns::scheduler::SchedulingMapPtr
MapHandler::getMasterMapForSlaveScheduling(int frameNr)
{
    // This is a SmartPtr copy. We are in UT here.
    // It is still the original datastructure written by BS-RS-TX
    return scheduledULResources[frameNr];
}
// called for every frame by rap/Events.cpp: rap::StartMap::execute
void
MapHandler::setCurrentPhase()
{
    //needed to reset resources at the and of a frame
    currentPhase = friends.timer->phaseNumberAtOffset(0.0/*relative to now*/);
}

void
MapHandler::doWakeup()
{
}

void
MapHandler::calculateSizes(const CommandPool* commandPool, Bit& _commandPoolSize, Bit& _dataSize) const
{
    getFUN()->getProxy()->calculateSizes(commandPool, _commandPoolSize, _dataSize, this);
    _dataSize += this->pduSize;
}

// start sending the (UL+DL) MAPs in DL towards user terminals
void
MapHandler::startMapTx(simTimeType duration, std::vector<int> dlFrameNumbers, std::vector<int> ulFrameNumbers)
{
    MESSAGE_SINGLE(NORMAL, logger,"MapHandler::startMapTx(d="<<duration<<"s): sending maps for "<<ulFrameNumbers.size()<<" UL frames, "<< dlFrameNumbers.size()<<" DL frames ...");
    // when Map Tx starts, the phy has to be switched to RAP-like operation
    friends.phyUser->rapTuning();

    // prepare vectors with resource information
    SchedulingMapCollectionVector dlSchedulingMapVectorInMap;
    SchedulingMapCollectionVector ulSchedulingMapVectorInMap;
    for (size_t i=0; i<dlFrameNumbers.size(); i++)
    {
        int frameNumber = dlFrameNumbers[i];
        assure(frameNumber < (int)scheduledDLResources.size(), "invalid frameNumber="<<frameNumber);
        MESSAGE_SINGLE(NORMAL, logger,"MapHandler::startMapTx(): frameNumber["<<i<<"]="<<frameNumber
                       <<": map size: DL="<<scheduledDLResources[frameNumber]->getResourceUsage()*100.0);
        // Prepare sufficient copies of the map pdu for all subcarriers assigned to this station
        dlSchedulingMapVectorInMap.push_back(scheduledDLResources[frameNumber]);
    }

    for (size_t i=0; i<ulFrameNumbers.size(); i++)
    {
        int frameNumber = ulFrameNumbers[i];
        assure(frameNumber < (int)scheduledULResources.size(), "invalid frameNumber="<<frameNumber);
        MESSAGE_SINGLE(NORMAL, logger,"MapHandler::startMapTx(): frameNumber["<<i<<"]="<<frameNumber
                       <<"%, UL="<<scheduledULResources[frameNumber]->getResourceUsage()*100.0<<"%");
        // Prepare sufficient copies of the map pdu for all subcarriers assigned to this station
        ulSchedulingMapVectorInMap.push_back(scheduledULResources[frameNumber]);
    }

    MESSAGE_BEGIN(NORMAL, logger,m,"MapHandler::startMapTx(): dlSchedulingMapVectorInMap.size="<<dlSchedulingMapVectorInMap.size()<<", ulSchedulingMapVectorInMap.size="<<ulSchedulingMapVectorInMap.size()<<"\n");
    {
        for (size_t i=0; i<dlSchedulingMapVectorInMap.size(); i++)
        { // forall frames that are signaled in a map
            m<<"dlSchedulingMapVectorInMap[i="<<i<<"]="<<dlSchedulingMapVectorInMap[i]->getResourceUsage()*100.0<<"%\n";
        }
        for (size_t i=0; i<ulSchedulingMapVectorInMap.size(); i++)
        {
            m<<"ulSchedulingMapVectorInMap[i="<<i<<"]="<<ulSchedulingMapVectorInMap[i]->getResourceUsage()*100.0<<"%\n";
        }
        m<<"Map is broadcasted on SubChannels=" << lowestSubChannel << "..." << highestSubChannel;
    }
    MESSAGE_END();
    // put tempVectors into the commands and pass them to phy
    for (uint32_t subBand = lowestSubChannel; subBand <= highestSubChannel; ++subBand){
        CompoundPtr mapCompound = CompoundPtr(new Compound(getFUN()->createCommandPool()));
        MapCommand* myCommand = activateCommand(mapCompound->getCommandPool());

        myCommand->peer.dlSchedulingMapVector = dlSchedulingMapVectorInMap; // this is sent to the peer
        myCommand->peer.ulSchedulingMapVector = ulSchedulingMapVectorInMap; // this is sent to the peer
        myCommand->peer.dlFrameNumbers = dlFrameNumbers;
        myCommand->peer.ulFrameNumbers = ulFrameNumbers;
        myCommand->magic.source = layer2->getDLLAddress();

        // set PhyUser Command
        lte::macr::PhyCommand* phyCommand = dynamic_cast<lte::macr::PhyCommand*>(
            getFUN()->getProxy()->activateCommand( mapCompound->getCommandPool(), friends.phyUser ));

        simTimeType startTime = wns::simulator::getEventScheduler()->getTime(); // now

        phyCommand->local.beamforming = false;
        phyCommand->local.pattern = wns::service::phy::ofdma::PatternPtr(); // NULL Pointer
        phyCommand->local.start = startTime;
        phyCommand->local.stop = startTime + duration;
        phyCommand->local.subBand = subBand;
        phyCommand->local.modeRxTx = lte::macr::PhyCommand::Tx;
        phyCommand->local.phyModePtr = phyModePtr;
        phyCommand->magic.txp = txPower;
        phyCommand->magic.source = getFUN()->getLayer<dll::ILayer2*>()->getNode();
        phyCommand->magic.destination = NULL; // broadcast
        if (getConnector()->hasAcceptor(mapCompound)) {
            getConnector()->getAcceptor(mapCompound)->sendData(mapCompound);
            MESSAGE_SINGLE(NORMAL, logger, "startMapTx: prepared and sent new outgoing DL/UL MapInfo burst.");
        }
        else
            assure(false, "Lower FU is not accepting scheduled Map PDU but is supposed to do so");
    }
    // do this for DL and UL maps
    evaluateMaps();
} // startMapTx()

// prepare UT or RNUT to receive the MAPs from BS or RNBS
void
MapHandler::startMapRx()
{
    // when Map Rx starts, the phy has to be switched to UT-like operation
    friends.phyUser->utTuning();
}

void
MapHandler::prepareMapOutput()
{
    if (!writeMapOutput) return; // nothing to do
    MESSAGE_SINGLE(NORMAL, logger, "prepareMapOutput(): opening output file "<<mapOutputFileNameDL<<" and "<<mapOutputFileNameUL);
    mapFileDL = new std::ofstream(mapOutputFileNameDL.c_str());
    mapFileUL = new std::ofstream(mapOutputFileNameUL.c_str());
    assure(mapFileDL!=NULL,"cannot write to mapFile "<<mapOutputFileNameDL);
    assure(mapFileUL!=NULL,"cannot write to mapFile "<<mapOutputFileNameUL);
    (*mapFileDL) << "% contents=\""<<logger.getLoggerName()<<" DL map\"\n";
    (*mapFileUL) << "% contents=\""<<logger.getLoggerName()<<" UL map\"\n";
}

// analyze MAPs to get performance values from it.
// Probe output and file output for Matlab
void
MapHandler::evaluateMaps()
{
    if (!writeMapOutput) return; // nothing to do
    MESSAGE_SINGLE(NORMAL, logger, "evaluateMaps(): writing to "<<mapOutputFileNameDL<<" and "<<mapOutputFileNameUL);
    assure(mapFileDL!=NULL,"cannot write to mapFile "<<mapOutputFileNameDL);
    assure(mapFileUL!=NULL,"cannot write to mapFile "<<mapOutputFileNameUL);
    // problem: we have 1..N maps here for DL/UL. They can overlap.
    // TODO: write table output to file for postprocessing with Matlab
    // TODO: write a probe "ResourceUsage" here, namely only for frame frameNumbers[0]
}

void
MapHandler::closeMapOutput()
{
    if (!writeMapOutput) return; // nothing to do
    MESSAGE_SINGLE(NORMAL, logger, "closeMapOutput(): closing "<<mapOutputFileNameDL<<" and "<<mapOutputFileNameUL);
    if (*mapFileDL) mapFileDL->close();
    if (*mapFileUL) mapFileUL->close();
}

