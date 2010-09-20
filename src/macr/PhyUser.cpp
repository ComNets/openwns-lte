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

#include <LTE/macr/PhyUser.hpp>
/* deleted by chen */
// #include <LTE/macr/WINNERSAR.hpp>
/* inserted by chen */
#include <boost/bind.hpp>

#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/StaticFactory.hpp>
#include <WNS/ldk/helper/FakePDU.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/sar/Soft.hpp>
#include <WNS/pyconfig/View.hpp>

#include <DLL/services/management/InterferenceCache.hpp>
#include <DLL/Layer2.hpp>
#include <DLL/StationManager.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace lte::macr;

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

STATIC_FACTORY_REGISTER_WITH_CREATOR(PhyUser, wns::ldk::FunctionalUnit, "lte.macr.PhyUser", wns::ldk::FUNConfigCreator);

PhyUser::PhyUser(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyConfigView) :
    wns::ldk::CommandTypeSpecifier<PhyCommand>(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasDeliverer<>(),
    wns::Cloneable<PhyUser>(),
    lte::helper::HasModeName(pyConfigView),
    layer2(NULL),
    stateRxTx(Rx),
    logger(pyConfigView.get("logger")),
    activeSubBands(),
    fddCapable((pyConfigView.get<std::string>("plm.mac.duplex") == "FDD")),
    safetyFraction(pyConfigView.get<simTimeType>("plm.mac.safetyFraction")),
    es(wns::simulator::getEventScheduler()),
    iCache(NULL),
    bfTransmission(NULL),
    transmission(NULL),
    notificationService(NULL),
    mobility(NULL),
    stationManager(NULL),
    measurementDelay_(pyConfigView.get<wns::simulator::Time>("measurementDelay"))
{
    MESSAGE_BEGIN(NORMAL, logger, m, "PhyUser() created.");
    if (fddCapable) m << " fddCapable";
    MESSAGE_END();
} // PhyUser


PhyUser::~PhyUser()
{
    iCache = NULL;
    bfTransmission = NULL;
    transmission = NULL;
    notificationService = NULL;
    mobility = NULL;
    stationManager = NULL;
} // ~PhyUser

void
PhyUser::onFUNCreated()
{
    wns::ldk::fun::FUN* fun = getFUN();

    layer2 = fun->getLayer<dll::ILayer2*>();

    iCache = layer2->getManagementService<dll::services::management::InterferenceCache>("INTERFERENCECACHE"+modeBase);

    stationManager =
        layer2->getStationManager();
}

bool
PhyUser::doIsAccepting(const wns::ldk::CompoundPtr& /*compound*/) const
{
    return true;
} // isAccepting


void
PhyUser::doSendData(const wns::ldk::CompoundPtr& compound)
{
    assure(compound, "sendData called with an invalid compound.");
    assure(getFUN()->getProxy()->commandIsActivated( compound->getCommandPool(), this),
           "PhyCommand not specified. PhyUser can not handle this compound!");

    // finally commit CommandPool Size
    this->commitSizes(compound->getCommandPool());

    PhyCommand* myCommand = getCommand(compound->getCommandPool());
    if (myCommand->local.modeRxTx == lte::macr::PhyCommand::Tx)
    {
        MESSAGE_SINGLE(NORMAL, logger,"doSendData(Tx): start="<< myCommand->local.start <<"s..stop=" << myCommand->local.stop <<"s => d="<<(myCommand->local.stop-myCommand->local.start)*1e6<<"us, subBand=" << myCommand->local.subBand << ", len="<<compound->getLengthInBits() << "bits");
        simTimeType startTime = myCommand->local.start;
        // TODO: check/assure that we don't send FakePDUPtr.
        // this can happen with compounds from RS-RX, created in RRHandler
        //assure(dynamic_cast<wns::ldk::helper::FakePDU*>(compound->getData().getPtr())==NULL,"FakePDU in Tx mode");
        es->schedule(StartTxEvent(compound, this), startTime);
        // Inform FUs that have added a callback that the compound is on air now
	if (!myCommand->local.onAirCallback.empty())
	    {
		myCommand->local.onAirCallback();
	    }
    }
    else
    { // reception (Rx)
        //MESSAGE_SINGLE(NORMAL, logger,"doSendData(Rx): startTime="<< myCommand->local.start <<", stopTime=" << myCommand->local.stop << ", subBand=" << myCommand->local.subBand << ", len="<<compound->getLengthInBits() << "bits");
        MESSAGE_SINGLE(NORMAL, logger,"doSendData(Rx): startTime="<< myCommand->local.start <<", stopTime=" << myCommand->local.stop << ", subBand=" << myCommand->local.subBand << ", len="<<compound->getLengthInBits() << "bits" << " SHALL NOT OCCUR");
        //es->schedule(StartRxEvent(compound, this), myCommand->local.start);
        assure(false,"doSendData(Rx): startTime="<< myCommand->local.start <<", stopTime=" << myCommand->local.stop << ", subBand=" << myCommand->local.subBand << ", len="<<compound->getLengthInBits() << "bits" << " SHALL NOT OCCUR");
    }

    // stamp link to my InterferenceCache into the Command
    myCommand->magic.remoteCache = iCache;
} // doSendData


void
PhyUser::doOnData(const wns::ldk::CompoundPtr& compound)
{
    assure(compound, "onData called with an invalid compound.");
    getDeliverer()->getAcceptor(compound)->onData(compound);
} // doOnData


void
PhyUser::onData(wns::osi::PDUPtr pdu, wns::service::phy::power::PowerMeasurementPtr rxPowerMeasurement)
{
    MESSAGE_BEGIN(VERBOSE, logger, m, "DATAInd");
    m << " while in state: " << getStateRxTx();
    MESSAGE_END();

    // If we are not in receiving state this PDU can't be meant for us.
    if (!(stateRxTx==Rx || stateRxTx==BothRxTx))
        return;

    assure(wns::dynamicCast<wns::ldk::Compound>(pdu), "Wrong type of PDU!");

    // take a copy!
    wns::ldk::CompoundPtr compound = wns::staticCast<wns::ldk::Compound>(pdu)->copy();
    PhyCommand* myCommand = getCommand(compound->getCommandPool());

    //const
    wns::node::Interface* source = rxPowerMeasurement->getSourceNode();
    assure(source == myCommand->magic.source,"measurement source mismatch");
    // set receiver Ptr in command pool, for later Probing
    compound->getCommandPool()->setReceiver(getFUN());

    wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = rxPowerMeasurement->getPhyMode();
    assure(myCommand->local.phyModePtr == phyModePtr, "getPhyMode error");

    bool broadcast = (myCommand->magic.destination == NULL);
    if (broadcast) {
        MESSAGE_SINGLE(NORMAL,logger, "RECEIVED BROADCAST COMPOUND FROM " <<source->getName());
    }

    // perform the first part of the LL mapping, the second part (Mapping from
    // MIB to PER) is performed when all segments belonging to a code-block are
    // re-assembled (SAR-FU).
    // Store measurements in the phyCommand, e.g. for evaluation in the BCHUnit
    myCommand->local.rxPowerMeasurementPtr = rxPowerMeasurement;
    //assure(myCommand->local.subBand == rxPowerMeasurement->getSubChannel(),"subChannel mismatch");

    /** magic distance calculation. In a real world, the receiver would have to
     * use info about the timing advance or the synchronization for this, but we
     * can use this shortcut.
     */
    // TODO [rs]: shift this functionality into rxPowerMeasurement and get rid of myCommand->local.distance
    assure(mobility!=NULL,"mobility==NULL");
    myCommand->local.distance = mobility->getDistance( source->getService<wns::PositionableInterface*>("mobility") );
    //myCommand->local.distance = rxPowerMeasurement->getDistance();
    // myCommand->magic.source is a wns::node::Interface*
    // wns::PositionableInterface* mobility we get by PhyUser::setMobility() called by ILayer2 class:
    // phyUser->setMobility( getNode()->getService<wns::PositionableInterface*>("mobility") );

    // estimate pathloss (including tx and rx antenna gains)
    //wns::Ratio pathloss = rxPowerMeasurement->getPathLoss();
    //myCommand->local.pathloss = rxPowerMeasurement->getPathLoss();

    MESSAGE_BEGIN(NORMAL, logger, m, "DataInd ");
    wns::service::dll::UnicastAddress sourceAddress =
        stationManager->getStationByNode(source)->getDLLAddress();
    m << "from " << A2N(sourceAddress) << ":";
    m << " SubBand=" << myCommand->local.subBand;
    //m << " SubBand=" << rxPowerMeasurement->getSubChannel();
    m << ", PhyMode=" << *phyModePtr;
    m << std::fixed << std::setprecision( 1 ); // Nachkommastellen
    m << ", "<<rxPowerMeasurement->getString();
    m << std::fixed << std::setprecision( 2 ); // Nachkommastellen
    m << ", MIB=" << rxPowerMeasurement->getMIB();
    MESSAGE_END();

    // During Broadcast Phases, Interference is not representative, therefore we
    // do not store it
    if (broadcast == false && !myCommand->magic.isRetransmission)
    {
        // store pathloss measurement for source station in local InterferenceCache
        /*iCache->storePathloss( source,
                               rxPowerMeasurement->getLoss(),
                               dll::services::management::InterferenceCache::Local );*/
        /* ^ this is ok, but doing the following seems to mix up UL and DL vached values:
           iCache->storeMeasurements( source,
           rxPowerMeasurement,
           dll::services::management::InterferenceCache::Local );
        */
        // "magically" store my measurements in remote InterferenceCache
        /*myCommand->magic.remoteCache->storeMeasurements( getFUN()->getLayer<dll::ILayer2*>()->getNode(),
                                                         rxPowerMeasurement,
                                                         dll::services::management::InterferenceCache::Remote,
                                                         myCommand->local.subBand );*/

        wns::simulator::getEventScheduler()->scheduleDelay(boost::bind(
            &dll::services::management::InterferenceCache::storeMeasurements,
            myCommand->magic.remoteCache,
            getFUN()->getLayer<dll::ILayer2*>()->getNode(),
            rxPowerMeasurement,
            dll::services::management::InterferenceCache::Remote,
            myCommand->local.subBand), measurementDelay_);



        wns::Ratio sinrEstimation = wns::Ratio::from_dB(0.0);
        if (myCommand->magic.estimatedSINR.I.get_mW() != 0.0){
            sinrEstimation = myCommand->magic.estimatedSINR.C / myCommand->magic.estimatedSINR.I;
            MESSAGE_BEGIN(NORMAL, logger, m, "DataInd: ");
            m << "SINR Estimation was: (S=" << myCommand->magic.estimatedSINR.C
              << ", I+N=" << myCommand->magic.estimatedSINR.I
              << ", Error=" << sinrEstimation-rxPowerMeasurement->getSINR() << ")\n";
            MESSAGE_END();
        }
    }
    /* Use braodcast to determine pathloss */
    else
    {
        /*myCommand->magic.remoteCache->storePathloss(getFUN()->getLayer<dll::ILayer2*>()->getNode(),
                                                         rxPowerMeasurement->getLoss(),
                                                         dll::services::management::InterferenceCache::Remote );*/
    }

    // deliver compound
    doOnData(compound);
}

void
PhyUser::doWakeup()
{
    // calls wakeup method of upper functional unit(s)
    getReceptor()->wakeup();
} // wakeup


void
PhyUser::stopTransmission(wns::osi::PDUPtr pdu, int subBand)
{
    transmission->stopTransmission(pdu, subBand);
    activeSubBands.remove(std::pair<wns::osi::PDUPtr, int>(pdu, subBand));
}

void
PhyUser::setReceiveAntennaPattern(wns::node::Interface* destination, wns::service::phy::ofdma::PatternPtr pattern)
{
    assure(pattern, "No Beamforming Pattern set."); // set correct pattern in PhyUser
    bfTransmission->insertReceivePattern(destination, pattern);
}

void
PhyUser::deleteReceiveAntennaPatterns()
{
    // Delete old Rx Antenna Patterns
    std::map<wns::node::Interface*, wns::service::phy::ofdma::PatternPtr> emptyMap;
    bfTransmission->setCurrentReceivePatterns(emptyMap);
}

bool
PhyUser::checkIdle()
{
    return ( activeSubBands.empty() && !transmission->isReceiving() );
}

void
PhyUser::setStateRxTx(StateRxTx _state)
{
    switch (_state)
    {
    case Tx:
        assure(!transmission->isReceiving(), "Tried to switch to Tx while still receiving!");
        notificationService->disableReception();
        break;
    case Rx:
        assure(activeSubBands.empty(), "Tried to switch to Rx while still transmitting!");
        // Cleanup old Antenna Patterns
        this->deleteReceiveAntennaPatterns();
        notificationService->enableReception();
        MESSAGE_SINGLE(VERBOSE,logger,"Cleared old Antenna Patterns");
        break;
    case BothRxTx:
        assure(fddCapable, "only FDD capable stations can set 'state' to 'BothRxTx'!");
        notificationService->enableReception();
        break;
    default:
        assure(false, "You tried to set an unknown state!");
        break;
    }

    stateRxTx=_state;

    MESSAGE_SINGLE(VERBOSE,logger,"Set PhyUser State to: "+getStateRxTx());
}

std::string
PhyUser::getStateRxTx() const
{
    switch (stateRxTx)
    {
    case Tx:
        return "Tx";
        break;
    case Rx:
        return "Rx";
        break;
    case BothRxTx:
        return "BothRxTx";
        break;
    default:
        assure(false, "Unknown stateTxRx in PhyUser!");
        return "";
    }
    return "";
}

void
PhyUser::rapTuning()
{
    if (getFUN()->getLayer<dll::ILayer2*>()->getStationType() == wns::service::dll::StationTypes::FRS())
        transmission->setTxRxSwap(false);
}

void
PhyUser::utTuning()
{
    if (getFUN()->getLayer<dll::ILayer2*>()->getStationType() == wns::service::dll::StationTypes::FRS())
        transmission->setTxRxSwap(true);
}

void
PhyUser::startTransmission(const wns::ldk::CompoundPtr& compound)
{
    assure(getFUN()->getProxy()->commandIsActivated( compound->getCommandPool(), this),
           "PhyCommand not specified. PhyUser can not handle this compound!");
    PhyCommand* myCommand = getCommand(compound->getCommandPool());
    MESSAGE_SINGLE(NORMAL, logger,"startTransmission(): start="<< myCommand->local.start <<"s..stop=" << myCommand->local.stop <<"s => d="<<myCommand->local.stop-myCommand->local.start<<"s, subBand=" << myCommand->local.subBand);

    wns::Power txPower = myCommand->magic.txp;

    int subBand = myCommand->local.subBand;
    int beam = myCommand->local.beam;
    wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = myCommand->local.phyModePtr;
    simTimeType duration = myCommand->local.stop - myCommand->local.start; // duration should be multiple of OFDM symbol length
    assure(myCommand->local.start == es->getTime(), "myCommand->local.start is not now");
    assure(phyModePtr->dataRateIsValid(),"!dataRateIsValid for "<<*phyModePtr);
    int capacity = phyModePtr->getBitCapacityFractional(duration);
    MESSAGE_SINGLE(NORMAL, logger,"PhyMode="<<*phyModePtr<<" supports "<<phyModePtr->getBitCapacityFractional(1.0)<<" bit/s/subChannel");
    MESSAGE_BEGIN(NORMAL, logger, m, "startTransmission on subBand=");
    m << subBand
        //<< std::ios::fixed // puts "4" into output!?
      << ", PhyMode=" << *phyModePtr
        //<< std::ios::floatfield << std::setprecision(3) // puts "260" into output!?
      << ", D=" << duration*1e6 << "us"
      << ", " << compound->getLengthInBits() << " bit"
      << ", cap=" << capacity << " bit"
      << ", source=" << myCommand->magic.source->getName()
      << ", dest=" << (myCommand->magic.destination == NULL ? "BROADCAST" : myCommand->magic.destination->getName())
      << ", P=" << txPower;
    MESSAGE_END();

    assure(compound->getLengthInBits() <= capacity , "SDU too long: len="<<compound->getLengthInBits()<<" <= cap="<<capacity<<" ("<<phyModePtr->getString()<<", D="<<duration<<"s)");

    if (myCommand->magic.destination == 0) {
        // no destination, send broadcast
        assure(beam==0,"broadcast is only possible with beam==0, but beam="<<beam);
        transmission->startBroadcast(compound, subBand, txPower, phyModePtr);
    } else {
        // we have a destination, this is not a broadcast
        if (myCommand->local.beamforming == true){
            assure(myCommand->local.pattern, "No Beamforming Pattern set.");
            // call startTransmission method of dataTransmission service
            // which is located in WNS/service/phy/ofdma/Station.cpp: Station::startTransmission
            bfTransmission->startTransmission(compound,
                                              myCommand->magic.destination,
                                              subBand,
                                              //beam, // <- cannot be used here
                                              myCommand->local.pattern,
                                              txPower,
                                              phyModePtr);
        } else {
            transmission->startUnicast(compound,
                                       myCommand->magic.destination,
                                       subBand,
                                       //beam, // for MIMO
                                       txPower,
                                       phyModePtr);
        }
    }
    activeSubBands.push_back( std::pair<wns::osi::PDUPtr, int>(compound, subBand) );
    // prepare the stopEvent
    es->schedule(StopTxEvent(compound, subBand, this), myCommand->local.stop-safetyFraction);
} // startTransmission

void
PhyUser::setDataTransmissionService(wns::service::Service* phy)
{
    assure(phy, "must be non-NULL");
    assureType(phy, wns::service::phy::ofdma::DataTransmission*);

    wns::service::phy::ofdma::DataTransmission* tmp =
        dynamic_cast<wns::service::phy::ofdma::DataTransmission*>(phy);

    bfTransmission = tmp;
    transmission   = tmp;
}

wns::service::phy::ofdma::DataTransmission*
PhyUser::getDataTransmissionService() const
{
    assure(transmission, "no ofdma::DataTransmission set. Did you call setDataTransmission()?");
    assureType(transmission, wns::service::phy::ofdma::DataTransmission*);
    return dynamic_cast<wns::service::phy::ofdma::DataTransmission*>(transmission);
}

// called by ILayer2 class in lte:
void
PhyUser::setNotificationService(wns::service::Service* phy)
{
    MESSAGE_SINGLE(NORMAL, logger, "PhyUser::setNotificationService() called");
    assure(phy, "must be non-NULL");
    assureType(phy, wns::service::phy::ofdma::Notification*);
    notificationService = dynamic_cast<wns::service::phy::ofdma::Notification*>(phy);
    notificationService->registerHandler(this);
}

// currently not called anywhere:
wns::service::phy::ofdma::Notification*
PhyUser::getNotificationService() const
{
    assure(notificationService, "no ofdma::Notification set. Did you call setNotificationService()?");
    return notificationService;
}

void
PhyUser::setMACAddress(const wns::service::dll::UnicastAddress& _address)
{
    address = _address;
    MESSAGE_SINGLE(NORMAL, logger, "setting my MAC address to: " << address);
}

void
PhyUser::setMobility(wns::PositionableInterface* _mobility)
{
    assure(_mobility, "Trying to set invalid mobility Object Pointer");
    mobility = _mobility;
}

bool
PhyUser::isFddCapable() const
{
    return fddCapable;
}




