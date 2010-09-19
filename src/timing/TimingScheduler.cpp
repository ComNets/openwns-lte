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

#include <LTE/timing/TimingScheduler.hpp>
#include <LTE/timing/events/Base.hpp>
#include <LTE/timing/events/rap/Events.hpp>
#include <LTE/timing/events/ut/Events.hpp>
#include <LTE/main/Layer2.hpp>
/* deleted by chen */
// #include <LTE/controlplane/associationHandler/AssociationHandler.hpp>
//#include <LTE/controlplane/bch/LTEBCHUnit.hpp>

#include <LTE/controlplane/bch/BCHUnitInterface.hpp>


#include <DLL/StationManager.hpp>

#include <sstream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::timing;
using namespace lte::timing::events;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    TimingScheduler,
    wns::ldk::ManagementServiceInterface,
    "lte.timing.TimingScheduler",
    wns::ldk::MSRConfigCreator);

TimingScheduler::TimingScheduler(wns::ldk::ManagementServiceRegistry* msr,
                                 wns::pyconfig::View _config) :
    wns::ldk::ManagementService(msr),
    lte::helper::HasModeName(_config),
    layer2(NULL),
    fun(NULL),
    config(_config),
    logger(config.get("logger")),
    superFrameLength(config.get<simTimeType>("mac.superFrameLength")),
    superFrameStartTime(0.0),
    eventContainer(),
    stationTaskPhaseContainer(),
    peerTiming(),
    peerTimingSchedulers(),
    es(wns::simulator::getEventScheduler()),
    startOfFirstFrame(config.get<simTimeType>("startOfFirstFrame")),
    frameLength(config.get<simTimeType>("frameLength")),
    schedulingOffset(config.get<int>("mac.schedulingOffset")),
    numberOfFramesToSchedule(config.get<int>("mac.numberOfFramesToSchedule")),
    framesPerSuperFrame(config.get<int>("mac.framesPerSuperFrame")),
    switchingPointOffset(config.get<simTimeType>("mac.switchingPointOffset")),
    duplex(DuplexSchemes::fromString(config.get<std::string>("mac.duplex")))
{
    MESSAGE_BEGIN(VERBOSE, logger, m, "TimingScheduler() constructed: ");
    m << "superFrameLength=" << superFrameLength << " s\n";
    m << "superFrameStartTime=" << superFrameStartTime << " s\n";
    m << "switchingPointOffset=" << switchingPointOffset << " s";
    MESSAGE_END();

} // SuperFrameScheduler

TimingScheduler::~TimingScheduler()
{
    while (!eventContainer.empty())
    {
        delete eventContainer.begin()->event;
        eventContainer.erase(eventContainer.begin());
    }
}

void
TimingScheduler::onMSRCreated()
{
    MESSAGE_BEGIN(NORMAL, logger, m, "onFUNCreated(): Starting Timing for superFrames with interval of ");
    m << superFrameLength << " s";
    MESSAGE_END();

    // Obtain FUN pointer
    layer2 = dynamic_cast<dll::ILayer2*>(getMSR()->getLayer());
    assure(layer2, "WinProSt TimingScheduler could not retrieve dll::ILayer2 pointer");
    fun = layer2->getFUN();

    dll::ILayer2::AssociationInfoContainer ais = layer2->getAssociationInfoProvider(mode);
    dll::ILayer2::AssociationInfoContainer::const_iterator iter = ais.begin();
    for (; iter != ais.end(); ++iter)
        this->startObserving(*iter);

    // this is my own frame structure (sequence of Events for one superFrameLength)
    readStationTaskPhases("stationTaskPhases");
    // ^ too late here for MapHandler::initSuperFrameMap() which needs stationTaskPhases
    // NOTE: readEvents has to be re-called each time the Task phases of a
    // station are altered
    assure(wns::simulator::getEventScheduler()->getTime()==0.0,"this must be called at simTime 0.0");
    readEvents("phases");

    startPeriodicTimeout(superFrameLength, 0);
}

void
TimingScheduler::onWorldCreated()
{
    // only for static associations:
    for (int i=0; i< config.len("peers"); ++i) // foreach peer
    {
        wns::service::dll::UnicastAddress peerAddress(config.get<int32_t>("peers",i));
        MESSAGE_SINGLE(NORMAL, logger, "onWorldCreated(): getting timingScheduler of peer "<<A2N(peerAddress));
        dll::ILayer2* peer = layer2->getStationManager()->getStationByMAC(peerAddress);

        lte::timing::TimingScheduler* peerTimingScheduler = NULL;

	peerTimingScheduler =
	  peer->
	  getManagementService<lte::timing::TimingScheduler>(getTimerName());

        assure(peerTimingScheduler,"No valid peerTimingScheduler!");
        peerTimingSchedulers.insert(peerAddress, peerTimingScheduler);
    }
}

// SuperFrameStart; called by simulation event scheduler
void
TimingScheduler::periodically()
{
    MESSAGE_SINGLE(NORMAL, logger, "========== Start of SuperFrame ==========");
    superFrameStartTime = es->getTime();

    // execution of the stored events
    for (unsigned int i = 0; i < eventContainer.size(); ++i)
    {
        lte::timing::events::Base* event = eventContainer.at(i).event;
        simTimeType timeOffset = eventContainer.at(i).timeOffset;
        simTimeType at = es->getTime() + timeOffset;
        es->schedule(boost::bind(&lte::timing::events::Base::operator(), event), at);
    }

    // tell all observers that new superframe starts now
    this->superFrameTrigger();
} // periodically

int
TimingScheduler::getSchedulingOffset() const
{
    return schedulingOffset;
}

int
TimingScheduler::getNumberOfFramesToSchedule() const
{
    return numberOfFramesToSchedule;
}

lte::timing::StationTask
TimingScheduler::stationTaskAtFrame(int frameNr /* absolute compared to superFrame start */) const
{
    StationTaskPhase taskPhase;
    taskPhase = stationTaskPhaseContainer.at( phaseNumberAtFrame(frameNr)/*absolute*/ );
    assure(taskPhase.task != StationTasks::INVALID(),"stationTaskAt called with an offset out of bounds");
    MESSAGE_SINGLE(VERBOSE, logger, "stationTaskAtFrame(frameNr="<<frameNr<<") = "<<StationTasks::toString(taskPhase.task));
    return taskPhase.task;
}

lte::timing::StationTask
TimingScheduler::stationTaskAtOffset(const simTimeType offset/*relative*/) const /* offset compared to now */
{
    assure(offset >= 0.0,"stationTaskAtOffset called with a negative offset"); // only into future

    StationTaskPhase taskPhase;
    taskPhase = stationTaskPhaseContainer.at( phaseNumberAtOffset(offset)/*relative*/ );
    assure(taskPhase.task != StationTasks::INVALID(),"stationTaskAtOffset called with an offset out of bounds");
    MESSAGE_SINGLE(VERBOSE, logger, "stationTaskAtOffset("<<offset<<") = "<<StationTasks::toString(taskPhase.task));
    return taskPhase.task;
}

uint32_t
TimingScheduler::phaseNumberAtFrame(int frameNr /* absolute compared to superFrame start */) const
{
    simTimeType offset = startOfFirstFrame + frameNr * frameLength;
    assure(stationTaskPhaseContainer.size()>0, "stationTaskPhaseContainer not ready");

    for (uint32_t index = 0; index < stationTaskPhaseContainer.size(); ++index)
    {
        StationTaskPhase taskPhase = stationTaskPhaseContainer.at(index);
        if (taskPhase.startTime + taskPhase.duration > offset)
        {
            MESSAGE_SINGLE(VERBOSE, logger, "phaseNumberAtFrame(frameNr="<<frameNr<<",offset=" << offset << "): taskPhase=" << index <<", task=" << lte::timing::StationTasks::toString(stationTaskPhaseContainer.at(index).task)<< "");
            return index;
        }
    }
    assure(false, "phaseNumberAtFrame("<<frameNr<<"): called with an offset out of bounds");
    return 0; // to please the compiler
}

uint32_t
TimingScheduler::phaseNumberAtOffset(const simTimeType offset /* compared to now */) const
{
    simTimeType now = es->getTime();
    simTimeType currentOffsetInSuperFrame = now - superFrameStartTime;
    simTimeType offsetInSuperFrame = currentOffsetInSuperFrame + offset;
    MESSAGE_SINGLE(VERBOSE, logger,  "phaseNumber("<<offset<<"=@"<<offsetInSuperFrame<<")");
    assure(stationTaskPhaseContainer.size()>0, "stationTaskPhaseContainer not ready");

    for (uint32_t index = 0; index < stationTaskPhaseContainer.size(); ++index)
    { // forall tasks
        StationTaskPhase taskPhase = stationTaskPhaseContainer.at(index);
        simTimeType endTime = taskPhase.startTime + taskPhase.duration;
        if (endTime > offsetInSuperFrame)
        { // found
            MESSAGE_SINGLE(VERBOSE, logger, "phaseNumberAtOffset(offset=" << offset << "): taskPhase=" << index <<", task=" << lte::timing::StationTasks::toString(stationTaskPhaseContainer.at(index).task)<< "");
            return index;
        }
    }
    assure(false, "phaseNumber("<<offset<<") called with an offset out of bounds");
    return 0; // to please the compiler
}

void
TimingScheduler::addPeerTimingScheduler(wns::service::dll::UnicastAddress peerAddress, TimingScheduler* _timingScheduler)
{
    peerTimingSchedulers.insert(peerAddress, _timingScheduler);
    MESSAGE_SINGLE(VERBOSE, logger, "Added peer Timing of "<<A2N(peerAddress));
}

void
TimingScheduler::removePeerTimingScheduler(wns::service::dll::UnicastAddress peerAddress)
{
    if (peerTimingSchedulers.knows(peerAddress))
    {
        peerTimingSchedulers.erase(peerAddress);
        MESSAGE_SINGLE(VERBOSE, logger, "Successfully removed peer Timing of "<<A2N(peerAddress));
    }
    else
    {
        MESSAGE_SINGLE(VERBOSE, logger, "Nothing to be done to remove peer Timing of "<<A2N(peerAddress));
    }
}

// Can a RN receive a map?
bool
TimingScheduler::canReceiveMapNow(const wns::service::dll::UnicastAddress& peerAddress)
{	// the station is identified by its DLL Address
    assure(peerTimingSchedulers.knows(peerAddress), "No knowledge about requested Peer's ("<<peerAddress<<") timing!");
    bool reachable = (peerTimingSchedulers.find(peerAddress)->stationTaskAtOffset(0.0) == lte::timing::StationTasks::UT());
    MESSAGE_SINGLE(VERBOSE, logger, "canReceiveMapNow(user=" << A2N(peerAddress) << "): " << (reachable ? "Yes" : "No"));
    return reachable;
}

// to get info about inferior stations' availability
bool
TimingScheduler::isPeerListeningAt(const wns::service::dll::UnicastAddress& peerAddress, const int frameNr)
{
    // the station is identified by its DLL Address
    assure(peerTimingSchedulers.knows(peerAddress), "No knowledge about requested Peer's ("<<peerAddress<<") timing!");
    MESSAGE_SINGLE(VERBOSE, logger, "isPeerListeningAt(User="<< A2N(peerAddress) <<",frameNr="<<frameNr<<") ?");
    bool isListening = (peerTimingSchedulers.find(peerAddress)->stationTaskAtFrame(frameNr/*absolute*/) == lte::timing::StationTasks::UT());
    MESSAGE_SINGLE(VERBOSE, logger, "isPeerListeningAt(User="<< A2N(peerAddress) <<",frameNr="<<frameNr<<"): "<< (isListening ? "Yes" : "No"));
    return isListening;
}

void
TimingScheduler::readEvents(std::string viewName)
{
    /* this is only called at the beginning of the simulation */
    // When it is called, this method assumes that readStationTaskPhases has
    // already been called
    for(int i = 0; i < config.len(viewName); ++i)
    {
        wns::pyconfig::View phaseConfig = config.get(viewName, i);
        std::string plugin = phaseConfig.get<std::string>("__plugin__");
        // a list of prototypes for events which are generated in pyconfig::Parser will be stored in an array
        TimingEvent e;
        e.event = NULL;
        e.timeOffset = phaseConfig.get<simTimeType>("time");
        // be sure that this is done only at now=superframe start time.
        // stationTaskAtOffset(offset is relative to now)
        if (this->stationTaskAtOffset(e.timeOffset) == StationTasks::RAP())
        {
            lte::timing::events::rap::EventCreator* c =
                lte::timing::events::rap::EventFactory::creator(plugin);
            e.event = c->create(fun, phaseConfig);
        }
        else if (this->stationTaskAtOffset(e.timeOffset) == StationTasks::UT())
        {
            lte::timing::events::ut::EventCreator* c =
                lte::timing::events::ut::EventFactory::creator(plugin);
            e.event = c->create(fun, phaseConfig);
        }
        else if (this->stationTaskAtOffset(e.timeOffset) == StationTasks::IDLE())
        {
            // do not create any events
        }
        else
            assure(false, "Unknown StationTask configured!");

        if (e.event!=NULL){
            e.event->setTimer(this);
            eventContainer.push_back( e );
        }

        MESSAGE_BEGIN(VERBOSE, logger, m, viewName);
        // print frame structure. For Winner i is from [0..24], 0=Preamble, +
        // 3*8 frames (MapTx, DataTx, DataRx)=BS_view, (MapRx, DataRx,
        // DataTx)=UTview
        simTimeType duration = phaseConfig.get<simTimeType>("duration");
        m << ": " << i << ", EvType=" << plugin << ", T=" << e.timeOffset << ", D=" << duration;
        MESSAGE_END();
    }
} // TimingScheduler::readEvents()

void
TimingScheduler::initStationTaskPhases()
{
    if (stationTaskPhaseContainer.size()==0) {
        // this is my own frame structure (sequence of Events for one superFrameLength)
        readStationTaskPhases("stationTaskPhases");
        // ^ structure needed in MapHandler::initSuperFrameMap()
    }
}

void
TimingScheduler::readStationTaskPhases(std::string viewName)
{
    if (stationTaskPhaseContainer.size()>0) {
        MESSAGE_SINGLE(NORMAL, logger, "readStationTaskPhases(): already processed ("<<stationTaskPhaseContainer.size()<<" elements)");
        return;
    }
    simTimeType durationSum = 0.0;
    // read in all stationTask items. There are (framesPerSuperFrame+1) elements.
    for(int i = 0; i < config.len(viewName); ++i)
    {
        wns::pyconfig::View phaseConfig = config.get(viewName, i);
        StationTaskPhase s;
        s.startTime = phaseConfig.get<simTimeType>("startTime");
        s.duration = phaseConfig.get<simTimeType>("duration");
        s.task = StationTasks::fromString(phaseConfig.get<std::string>("task"));
        stationTaskPhaseContainer.push_back( s );
        durationSum += s.duration;
        MESSAGE_BEGIN(NORMAL, logger, m, viewName);
        m << ": i=" << i << ", task=" << StationTasks::toString(s.task) << ", T=" << s.startTime << ", D=" << s.duration;
        MESSAGE_END();
    }
    MESSAGE_SINGLE(NORMAL, logger, "readStationTaskPhases(): "<<stationTaskPhaseContainer.size()<<" StationTaskPhases; duration = "<<durationSum);
    assure(fabs(durationSum - superFrameLength) < 1e-9 ,"StationTaskPhase durationSum="<<durationSum<<" does not match superFrameLength="<<superFrameLength);
    assure(stationTaskPhaseContainer.size()>0, "stationTaskPhaseContainer is empty");
}

void
TimingScheduler::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    //RAP task only, UT do nothing here
    if (layer2->getDLLAddress() == dstAdr)
    {
        removePeerTimingScheduler(userAdr);
        MESSAGE_SINGLE(NORMAL, logger, "Successfully removed Peer TimingScheduler for user=" << A2N(userAdr));
    }
}

void
TimingScheduler::onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    MESSAGE_SINGLE(NORMAL, logger, "TimingScheduler::onAssociated called (user=" << A2N(userAdr) << " dst=" << A2N(dstAdr) << ").");
    //RAP task only
    if (layer2->getDLLAddress() == dstAdr)
    {
        dll::ILayer2* newUser = layer2->getStationManager()->getStationByMAC(userAdr);

        TimingScheduler* userTimingScheduler = NULL;

	userTimingScheduler =
	  newUser->
	  getManagementService<lte::timing::TimingScheduler>(getTimerName());

        addPeerTimingScheduler(userAdr, userTimingScheduler);
        MESSAGE_SINGLE(NORMAL, logger, "Successfully added Peer TimingScheduler for user=" << A2N(userAdr));
    }
    else if (layer2->getDLLAddress() == userAdr)
    {
        // Notification in UT -> nothing to do
        MESSAGE_SINGLE(VERBOSE, logger, "Not adding Peer TimingScheduler for user with address " << A2N(userAdr) << " at Scheduler with Address " << A2N(userAdr));
    }
    else
    {
        // Notification in BS while association is with RN -> nothing to do
        MESSAGE_SINGLE(VERBOSE, logger, "Not adding Peer TimingScheduler for user with address " << A2N(userAdr) << " at Scheduler with Address " << A2N(layer2->getDLLAddress()));
    }
}

void
TimingScheduler::superFrameTrigger()
{
    this->wns::Subject<SuperFrameStartNotificationInterface>::sendNotifies(&SuperFrameStartNotificationInterface::onSuperFrameStart);
    /**
     * @todo dbn: Move this to the data phases and make it happen every 0th and 5th frame
     */
    try {
	/**
	 * @todo dbn: lterelease: Enable BCH trigger when BCH is available
	 */
        lte::controlplane::bch::IBCHTimingTx* bch = NULL;
	//bch = this->fun->findFriend<lte::controlplane::bch::IBCHTimingTx*>(mode+separator+"bch");
        if (bch != NULL)
        {
            bch->sendBCH(0.0005);
        }

    }
    catch (wns::ldk::fun::FindFriendException)
    {
    }
}

void
TimingScheduler::frameTrigger()
{
    this->wns::Subject<FrameStartNotificationInterface>::sendNotifies(&FrameStartNotificationInterface::onFrameStart);
}






