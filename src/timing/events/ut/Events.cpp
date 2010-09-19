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

#include <WNS/ldk/FUNConfigCreator.hpp>

#include <LTE/timing/events/ut/Events.hpp>
#include <LTE/timing/TimingScheduler.hpp>

#include <LTE/timing/ResourceSchedulerInterface.hpp>
#include <LTE/controlplane/MapHandlerInterface.hpp>
#include <LTE/macr/RACHInterface.hpp>

using namespace lte::timing;
using namespace lte::timing::events;

// objects of these classes are created by name:
// the names are specified in Timing.py as __plugin__
STATIC_FACTORY_REGISTER_WITH_CREATOR(ut::StartBCH,  ut::EventBase, "lte.timing.BCH", wns::ldk::FUNConfigCreator);
STATIC_FACTORY_REGISTER_WITH_CREATOR(ut::StartRACH, ut::EventBase, "lte.timing.RACH", wns::ldk::FUNConfigCreator);
STATIC_FACTORY_REGISTER_WITH_CREATOR(ut::StartMap,  ut::EventBase, "lte.timing.Map", wns::ldk::FUNConfigCreator);
STATIC_FACTORY_REGISTER_WITH_CREATOR(ut::StartData, ut::EventBase, "lte.timing.Data", wns::ldk::FUNConfigCreator);


ut::StartRACH::StartRACH(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    EventBase(_fun, config),
    rach(_fun->findFriend<lte::macr::IRachTimingTx*>(mode+separator+"rach"))
{}

void
ut::StartRACH::execute()
{
    assure(timer, "Timer has not been set in Event!");
    MESSAGE_SINGLE(NORMAL, logger, "Event StartRACH (as UT)");
    setStateRxTx(lte::macr::PhyUser::Tx);
    rach->startTx(duration);
}

ut::StartBCH::StartBCH(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    EventBase(_fun, config)
{}

void
ut::StartBCH::execute()
{
    assure(timer, "Timer has not been set in Event!");
    MESSAGE_SINGLE(NORMAL, logger, "Event StartBCH (as UT)");
    setStateRxTx(lte::macr::PhyUser::Rx);
}


ut::StartMap::StartMap(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    EventBase(_fun, config),
    frameNr(config.get<int>("frameNr")),
    useMapResourcesInUL(config.get<bool>("useMapResourcesInUL")),
    mapHandler(_fun->findFriend<lte::controlplane::IMapHandlerTiming*>(mode+separator+"mapHandler")),
    rstx(_fun->findFriend<lte::timing::SlaveScheduler*>(mode+separator+rsNameSuffix+"TX"))
{}

void
ut::StartMap::execute()
{
    assure(timer, "Timer has not been set in Event!");
    MESSAGE_SINGLE(NORMAL, logger, "Event StartMap (as UT) on current frame="<<frameNr);
    setStateRxTx(lte::macr::PhyUser::Rx);

    // needed to reset resources at the and of a frame
    mapHandler->setCurrentPhase();

    timer->frameTrigger();

    mapHandler->startMapRx();

    // Only valid for FDD
    if (useMapResourcesInUL)
    {
        MESSAGE_SINGLE(NORMAL, logger, "Starting UL transmission during Map phase!");
        // this is now done within rstxSlave->startCollection():
        //wns::scheduler::MapInfoCollectionPtr ulMapInfo = mapHandler->getTxResources(frameNr);
        setStateRxTx(lte::macr::PhyUser::Tx);
        //rstx->startCollection(frameNr, ulMapInfo); // start+finish inclusive
        rstx->startCollection(frameNr); // start+finish inclusive
    }
}


ut::StartData::StartData(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    EventBase(_fun, config),
    rstx(_fun->findFriend<lte::timing::SlaveScheduler*>(mode+separator+rsNameSuffix+"TX")),
    mapHandler(_fun->findFriend<lte::controlplane::IMapHandlerTiming*>(mode+separator+"mapHandler")),
    mySwitchingPointEvent(_fun, config),
    myStopDataEvent(_fun, config),
    frameNr(config.get<int>("frameNr")),
    framesPerSuperFrame(config.get<int>("framesPerSuperFrame")),
    useMapResourcesInUL(config.get<bool>("useMapResourcesInUL"))
{}

ut::StartData::~StartData()
{
}

// wns::CloneableInterface*
// ut::StartData::clone() const
// {
//     // use default implementation of copy c'tor
//     ut::StartData* that = new ut::StartData(*this);
//     // and make deep copies of the contained events
//     that->mySwitchingPointEvent = mySwitchingPointEvent;
//     that->myStopDataEvent = myStopDataEvent;
//     return that;
// }

void
ut::StartData::setTimer(lte::timing::TimingScheduler* _timer) {
    EventBase::setTimer(_timer);
    mySwitchingPointEvent.setTimer(_timer);
    myStopDataEvent.setTimer(_timer);
}

void
ut::StartData::execute()
{
    assure(timer, "Timer has not been set in Event!");
    assure(mapHandler, "mapHandler not set!");
    simTimeType now = wns::simulator::getEventScheduler()->getTime();

    MESSAGE_SINGLE(NORMAL, logger, "Event StartData (as UT). frameNr=" << frameNr);

    //setExpectation
    //rstx->setExpectations(frameNr); // obsolete
    setStateRxTx(lte::macr::PhyUser::Rx);

    // TDD, in case of FDD we don't have a switching point
    if (timer->switchingPointOffset > 0.0) {
        wns::simulator::getEventScheduler()->schedule(mySwitchingPointEvent,
                                                      now + timer->switchingPointOffset);
    }
    // FDD
    else {
        if(!useMapResourcesInUL)
        {
            MESSAGE_SINGLE(NORMAL, logger, "Not using Map Resources in UL!");
            // obsolete:
            //wns::scheduler::MapInfoCollectionPtr ulMapInfo = mapHandler->getTxResources(frameNr);
            setStateRxTx(lte::macr::PhyUser::Tx);
            //rstx->startCollection(frameNr, ulMapInfo); // start+finish inclusive
            rstx->startCollection(frameNr); // start+finish inclusive
        }
        mapHandler->resetResources(frameNr);
    }
    wns::simulator::getEventScheduler()->schedule(myStopDataEvent,
                                                  now + duration);
}

ut::StartData::SwitchingPoint::SwitchingPoint(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    EventBase(_fun, config),
    rstxSlave(_fun->findFriend<lte::timing::SlaveScheduler*>(mode+separator+rsNameSuffix+"TX")),
    rstxIncoming(_fun->findFriend<lte::timing::SchedulerIncoming*>(mode+separator+rsNameSuffix+"TX")),
    mapHandler(_fun->findFriend<lte::controlplane::IMapHandlerTiming*>(mode+separator+"mapHandler")),
    frameNr(config.get<int>("frameNr"))
{
}

void
ut::StartData::SwitchingPoint::execute()
{
    assure(timer, "Timer has not been set in Event!");
    assure(mapHandler, "mapHandler not set!");
    MESSAGE_SINGLE(NORMAL, logger, "SwitchingPoint reached (as UT). frameNr="<<frameNr);
    //rstxIncoming->deliverReceived(); // obsolete
    setStateRxTx(lte::macr::PhyUser::Tx);
    // this is now done within rstxSlave->startCollection():
    //wns::scheduler::MapInfoCollectionPtr ulMapInfo = mapHandler->getTxResources(frameNr);
    //rstxSlave->startCollection(frameNr, ulMapInfo); // start+finish inclusive
    rstxSlave->startCollection(frameNr); // start+finish inclusive
    mapHandler->resetResources(frameNr);
}

ut::StartData::StopData::StopData(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    EventBase(_fun, config),
    rstx(_fun->findFriend<lte::timing::SchedulerIncoming*>(mode+separator+rsNameSuffix+"TX"))
{}

void
ut::StartData::StopData::execute()
{
    MESSAGE_SINGLE(NORMAL, logger, "Event StopData");

    // HARQ, must be done BEFORE deliverReceived
    rstx->sendPendingHARQFeedback();
    
    // in case of a UT in TDD mode here deliverReceived() has no effect, because
    // it has already been called at the swtiching point
    rstx->deliverReceived(); // obsolete
}





