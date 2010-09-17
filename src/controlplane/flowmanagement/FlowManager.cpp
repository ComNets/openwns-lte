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

#include <LTE/controlplane/flowmanagement/FlowManager.hpp>
#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandlerBS.hpp>
#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandlerUT.hpp>
#include <LTE/controlplane/AssociationsProxy.hpp>
#include <LTE/upperconvergence/eNB.hpp>
#include <LTE/upperconvergence/UE.hpp>
#include <LTE/rlc/RLCCommand.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/Assure.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/StaticFactory.hpp>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace lte::controlplane;
using namespace lte::controlplane::flowmanagement;
using namespace wns::service::dll;

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::flowmanagement::FlowManagerBS,
                                     wns::ldk::ControlServiceInterface,
                                     "lte.controlplane.flowmanagement.FlowManagerBS",
                                     wns::ldk::CSRConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::flowmanagement::FlowManagerUT,
                                     wns::ldk::ControlServiceInterface,
                                     "lte.controlplane.flowmanagement.FlowManagerUT",
                                     wns::ldk::CSRConfigCreator);

FlowManager::FlowManager(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
    layer2(),
    plainDisassociation(false),
    rlcReader(NULL),
    logger(config.get("logger")),
    flowIDPool(config.get<simTimeType>("flowIDUnbindDelay"), 1, 57343), // 0 is reserved for ControlPlane
    broadcastFlowIDPool(config.get<simTimeType>("flowIDUnbindDelay"), 57344, 65535), // Upper area is for broadcasting
    separator("_"),
    flowSeparatorNames(),
    flowSeparators(),
    upperSynchronizer(NULL),
    upperFlowGate(NULL),
    arqFlowGate(NULL)
{
    bchFlowID_ = broadcastFlowIDPool.suggestPort();
    broadcastFlowIDPool.bind(bchFlowID_);

    for (int i = 0; i < config.len("modeNames"); ++i) {
        std::string modeName = config.get<std::string>("modeNames", i);
        myModes.push_back(modeName);
    }

    for (int ii = 0; ii < config.len("flowSeparatorNames"); ++ii)
    {
        std::string FSName = config.get<std::string>("flowSeparatorNames", ii);
        flowSeparatorNames.push_back(FSName);
    }

    wns::pyconfig::View transactionIdConfig(config, "transactionIdDistribution");
    std::string tIdDisName = transactionIdConfig.get<std::string>("__plugin__");
    transactionIdDistribution =
        wns::distribution::DistributionFactory::creator(tIdDisName)->create(transactionIdConfig);
}

FlowManager::~FlowManager()
{
}

std::string
FlowManager::getFlowTable() const
{ // this base class prints ControlPlane FlowIDs only
    std::stringstream out;
    out << std::endl;
    out << "FlowTable (" << logger.getLoggerName() <<"):"<< std::endl;
    for (ControlPlaneFlowIdTable::const_iterator iter = controlPlaneFlowIdsForPeer.begin();
         iter != controlPlaneFlowIdsForPeer.end();
         ++iter)
    {
        wns::service::dll::UnicastAddress peerAdr = iter->first;
        ControlPlaneFlowIDs flowIDs = iter->second;
        out << printControlPlaneFlowIDs(flowIDs) << " for peer="<<A2N(peerAdr) << std::endl;
    }
    return out.str();
}

std::string
FlowManager::printControlPlaneFlowIDs(ControlPlaneFlowIDs flowIDs) const
{
    std::stringstream out;
    out << "ControlPlaneFlowIDs=(";

    for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin();
         iter != flowIDs.end();
         ++iter)
    {
        FlowID flowID = iter->second;
        out << flowID << ", ";
    }
    out << ")";
    return out.str();
}

bool
FlowManager::isControlPlaneFlowID(FlowID flowID,
                                  ControlPlaneFlowIDs flowIDs) const
{
    bool found = false;
    for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin();
         iter != flowIDs.end();
         ++iter)
    {
        if (iter->second == flowID)
        {
            found = true;
            break;
        }
    }
    return found;
}

bool
FlowManager::isControlPlaneFlowID(wns::service::dll::UnicastAddress peerAddress,
                                  wns::service::dll::FlowID flowID) const
{
    bool found = false;
    if (controlPlaneFlowIdsForPeer.knows(peerAddress)) {
        ControlPlaneFlowIDs flowIDs = controlPlaneFlowIdsForPeer.find(peerAddress);
        for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin(); iter != flowIDs.end(); ++iter)
        {
            if (iter->second == flowID) { found = true; break; }
        }
    }
    return found;
}

lte::helper::TransactionID
FlowManager::drawNewTransactionID() const {
    lte::helper::TransactionID transactionId=0;
    int count=10; // should not take longer than this #trials
    bool cond1,cond2,cond3;
    do { // find a new transactionId which is not known in any existing table
        transactionId = static_cast<int>((*transactionIdDistribution)());
        MESSAGE_SINGLE(NORMAL, logger, "drawNewTransactionID(): transactionId="<<transactionId);
        cond1 = TransactionIDToTlFlowID.knows(transactionId);
        cond2 = TransactionIDToOldFlowID.knows(transactionId);
        cond3 = TransactionIDToQoSClass.knows(transactionId);
    } while ((--count>0) && (cond1 || cond2 || cond3));
    assure(count>0,"timeout while drawNewTransactionID()");
    return transactionId;
}

FlowID
FlowManager::getBCHFlowID()
{
    return bchFlowID_;
}

FlowID
FlowManager::getFlowIDin(FlowID flowIDout)
{
    assure(flowIDout>0,"flowID="<<flowIDout<<", must be >0");
    bool foundFlowID = false;
    for (SwitchingTable::const_iterator iter = FlowIDInToFlowIDOut.begin();
         iter != FlowIDInToFlowIDOut.end();
         ++iter)
    {
        MESSAGE_SINGLE(NORMAL, logger, "switching table entries flowIn "<<iter->first<<" , out "<<iter->second);

        if(iter->second == flowIDout)
        {
            foundFlowID = true;
            MESSAGE_SINGLE(NORMAL, logger, "FlowID switched from out "<<flowIDout<<" to in: "<<iter->first);
            return iter->first;
        }
    }
    if(foundFlowID==false)
    {
        MESSAGE_SINGLE(NORMAL, logger, "No such FlowIDout: "<<flowIDout<<getFlowTable())
            assure(foundFlowID, "No such FlowIDout: "<<flowIDout);
        return FlowID(0);
    }
    return FlowID(0);
}

FlowID
FlowManager::getFlowIDout(FlowID flowIDin)
{
    assure(flowIDin>0,"flowID="<<flowIDin<<", must be >0");
    assure(FlowIDInToFlowIDOut.knows(flowIDin),"No such FlowIDin="<<flowIDin<<getFlowTable());

    MESSAGE_SINGLE(NORMAL, logger, "FlowID switched from in "<<flowIDin<<" to out: "<<FlowIDInToFlowIDOut.find(flowIDin));

    return FlowIDInToFlowIDOut.find(flowIDin);
}

void
FlowManager::insertFlowIDToUT(FlowID flowID, wns::service::dll::UnicastAddress utAddress)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    FlowIDToUT.insert(flowID, utAddress);
    MESSAGE_SINGLE(NORMAL, logger, "Registering FlowID="<<flowID<<" for "<<A2N(utAddress));
}

void
FlowManager::deleteFlowsForUT(wns::service::dll::UnicastAddress utAddress)
{
    MESSAGE_SINGLE(NORMAL, logger, "deleteFlowsForUT("<<A2N(utAddress)<<")");
    MESSAGE_SINGLE(NORMAL, logger, "FlowIDToUT.size()="<<FlowIDToUT.size());
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == utAddress)
        {
            MESSAGE_SINGLE(NORMAL, logger, "Found FlowID="<<iter->first<<" in FlowIDTable for "<<A2N(utAddress));
            MESSAGE_SINGLE(NORMAL, logger, "FlowID="<<iter->first<<" deleted from FlowIDTable for "<<A2N(utAddress));
            flowIDPool.unbind(iter->first);
            FlowIDToUT.erase(iter->first);
        }
    }
}

wns::service::dll::UnicastAddress
FlowManager::getUTForFlow(FlowID flowID)
{
    wns::service::dll::UnicastAddress utAddress;
    // maybe a user plane flow ID
    if (FlowIDToUT.knows(flowID))
    {
        utAddress = FlowIDToUT.find(flowID);
    }
    else // or maybe a control plane flow ID
    {
        for (ControlPlaneFlowIdTable::const_iterator iter = controlPlaneFlowIdsForPeer.begin();
             iter != controlPlaneFlowIdsForPeer.end();
             ++iter)
        {
            if (isControlPlaneFlowID(flowID, iter->second)) // found
            {
                utAddress = iter->first; break;
            }
        }
    }
    assure(utAddress.getInteger()>0,"getUTForFlow("<<flowID<<") cannot be found");
    MESSAGE_SINGLE(NORMAL, logger, "getUTForFlow("<<flowID<<")="<<A2N(utAddress));
    return utAddress;
}

FlowManager::FlowIdList
FlowManager::getFlowsForUT(wns::service::dll::UnicastAddress utAddress)
{
    FlowIdList flowIdList;
    MESSAGE_SINGLE(NORMAL, logger, "getFlowsForUT("<<A2N(utAddress)<<")");
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == utAddress)
        {
            FlowID flowId = iter->first;
            MESSAGE_SINGLE(NORMAL, logger, "Found FlowID="<<flowId<<" in FlowIDTable for "<<A2N(utAddress));
            flowIdList.push_back(flowId);
        }
    }
    return flowIdList;
}

bool
FlowManager::hasFlowIDout(FlowID flowIDout)
{
    assure(flowIDout>0,"flowID="<<flowIDout<<", must be >0");
    bool foundFlowID = false;
    for (SwitchingTable::const_iterator iter = FlowIDInToFlowIDOut.begin();
         iter != FlowIDInToFlowIDOut.end();
         ++iter)
    {
        if(iter->second == flowIDout)
        {
            foundFlowID = true;
        }
    }
    return foundFlowID;
}

// asked in wns::ldk::flowseparator::CreateOnValidFlow
// RN overloads this method, see below
bool
FlowManager::isValidFlow(const wns::ldk::ConstKeyPtr& key) const
{
    const lte::helper::key::FlowID* _key = dynamic_cast<const lte::helper::key::FlowID*>(key.getPtr());
    bool knows = FlowIDToUT.knows(_key->flowID);
    if (!knows) {
        //MESSAGE_SINGLE(VERBOSE, logger, "isValidFlow("<<key->str()<<"="<<_key->flowID<<") == false"<<getFlowTable());
        MESSAGE_SINGLE(VERBOSE, logger, "isValidFlow("<<key->str()<<"="<<_key->flowID<<") == false");
    }
    return knows;
}

bool
FlowManager::isValidFlowId(FlowID flowID) const
{
    if (flowID==0) { return true; } // temporary Controlplane flowID for RNs at simulation start
    bool valid = FlowIDToUT.knows(flowID);
    if (!valid)
    { // can it be a controlPlane FlowId?
        for (ControlPlaneFlowIdTable::const_iterator iter = controlPlaneFlowIdsForPeer.begin();
             iter != controlPlaneFlowIdsForPeer.end();
             ++iter)
        {
            if (isControlPlaneFlowID(flowID, iter->second)) // found
            {
                valid=true; break;
            }
        }
    }
    valid &= !hasPreserved(flowID); // in case of intra-REC HO of a UT from/via RN to its BS
    MESSAGE_SINGLE(NORMAL, logger, "isValidFlowId("<<flowID<<")="<<valid<<(hasPreserved(flowID)?" preserved":""));
    return valid;
}

int
FlowManager::countFlows() const
{
    int count=0; // =FlowIDToUT.size(); ?
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin(); iter != FlowIDToUT.end(); ++iter) count++;
    assure(count==FlowIDToUT.size(),"FlowManager::countFlows(): error counting flows");
    return FlowIDToUT.size();
}

int
FlowManager::countFlows(wns::service::dll::UnicastAddress utAddress) const
{
    int count=0;
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin(); iter != FlowIDToUT.end(); ++iter)
    {
        if(iter->second == utAddress) { count++; }
    }
    return count;
}

void
FlowManager::deleteSwitchingTableForUT(wns::service::dll::UnicastAddress utAddress)
{
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == utAddress)
        {
            FlowIDInToFlowIDOut.erase(iter->first);
            MESSAGE_SINGLE(NORMAL, logger, "FlowID="<<iter->first<<" deleted from SwitchingTable for "<<A2N(utAddress));
        }
    }
}

void
FlowManager::deleteFlowSeparator(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(flowID));
    // delete user's (downlink and uplink) flows from the flowseparators
    for (std::list<wns::ldk::FlowSeparator*>::iterator itr = flowSeparators.begin(); itr != flowSeparators.end(); ++itr)
    {
        if((*itr)->getInstance(key))
            (*itr)->removeInstance(key);
        MESSAGE_SINGLE(NORMAL, logger, "Upper FlowSeparator deleted for FlowID="<<flowID);
    }
}

void
FlowManager::deleteAllFlowSeparators(wns::service::dll::UnicastAddress utAddress)
{
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == utAddress)
        {
            wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(iter->first));
            for (std::list<wns::ldk::FlowSeparator*>::iterator itr = flowSeparators.begin(); itr != flowSeparators.end(); ++itr)
            {
                if((*itr)->getInstance(key))
                    (*itr)->removeInstance(key);
                MESSAGE_SINGLE(NORMAL, logger, "Upper FlowSeparator deleted for FlowID="<<iter->first);
            }
        }
    }
}

void
FlowManager::deleteAllUpperFlows(wns::service::dll::UnicastAddress Adr)
{
    // first clean up the upperSynchronizer's buffer
    assure(upperSynchronizer, "upperSynchronizer not set.");
    assure(arqFlowGate, "arqFlowGate not set.");
    assure(upperFlowGate, "upperFlowGate not set.");

    wns::ldk::CompoundPtr compound = upperSynchronizer->hasSomethingToSend();
    if(compound != wns::ldk::CompoundPtr()){
        lte::rlc::RLCCommand* rlcCommand = rlcReader->readCommand<lte::rlc::RLCCommand>(compound->getCommandPool());
        if(rlcCommand->peer.destination == Adr)
        {
            // getSomethingToSend() takes the compund out of the Synchronizer
            compound = upperSynchronizer->getSomethingToSend();
            MESSAGE_SINGLE(NORMAL, logger, "Drop outgoing compound in buffer of upperSynchronizer!");
        }
    }

    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == Adr)
        {
            wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(iter->first));
            // destroy Flow in gates
            upperFlowGate->destroyFlow(key);
            arqFlowGate->destroyFlow(key);
            MESSAGE_SINGLE(NORMAL, logger, "Upper Flow deleted for FlowID="<<iter->first);
        }
    }
}

void
FlowManager::closeUpperFlows(wns::service::dll::UnicastAddress userAdr)
{
    assure(arqFlowGate, "arqFlowGate not set.");
    assure(upperFlowGate, "upperFlowGate not set.");

    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == userAdr)
        {
            wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(iter->first));
            upperFlowGate->closeFlow(key);
            arqFlowGate->closeFlow(key);
            MESSAGE_SINGLE(NORMAL, logger, "FlowID="<<iter->first<<" closed for UT="<<userAdr);
        }
    }
}

void
FlowManager::closeUpperFlow(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    assure(arqFlowGate, "arqFlowGate not set.");
    assure(upperFlowGate, "upperFlowGate not set.");

    wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(flowID));
    upperFlowGate->closeFlow(key);
    arqFlowGate->closeFlow(key);
    MESSAGE_SINGLE(NORMAL, logger, "FlowID="<<flowID<<" closed.");
}

void
FlowManager::openUpperFlow(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    assure(arqFlowGate, "arqFlowGate not set.");
    assure(upperFlowGate, "upperFlowGate not set.");

    wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(flowID));
    upperFlowGate->openFlow(key);
    arqFlowGate->openFlow(key);
    MESSAGE_SINGLE(NORMAL, logger, "FlowID="<<flowID<<" opened in Upper Gates.");
}

bool
FlowManager::hasPreserved(wns::service::dll::UnicastAddress userAdr) const
{
    std::set<wns::service::dll::UnicastAddress>::const_iterator found = preservedUsers.find(userAdr);
    return (found != preservedUsers.end());
}

bool
FlowManager::hasPreserved(FlowID flowID) const
{
    assure(flowID>=0,"flowID="<<flowID<<", must be >=0");
    std::set<FlowID>::const_iterator found = preservedFlowIDs.find(flowID);
    return (found != preservedFlowIDs.end());
}

void
FlowManager::registerPreservedUser(wns::service::dll::UnicastAddress userAdr)
{
    preservedUsers.insert(userAdr);
    MESSAGE_SINGLE(NORMAL, logger, "Registered Preserved User: "<<userAdr);
}

void
FlowManager::registerPreservedFlowID(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    preservedFlowIDs.insert(flowID);
    MESSAGE_SINGLE(NORMAL, logger, "Registered Preserved FlowID="<<flowID);
}

void
FlowManager::deletePreservedUser(wns::service::dll::UnicastAddress userAdr)
{
    preservedUsers.erase(userAdr);
    MESSAGE_SINGLE(NORMAL, logger, "Erased Preserved User: "<<userAdr);
}



///////////////////////////
//                       //
//     FlowManagerBS     //
//                       //
///////////////////////////

FlowManagerBS::FlowManagerBS(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
    FlowManager(csr, config),
    ControlService(csr)
{
}

FlowManagerBS::~FlowManagerBS()
{}

wns::service::qos::QoSClass
FlowManagerBS::getQoSClassForBSFlowID(FlowID dllFlowID) const
{
    if (dllFlowID == bchFlowID_)
    {
        return lte::helper::QoSClasses::PBCH();
    }
    // flowID=0 is the rare case of RN-to-BS at the beginning
    if (dllFlowID==0) {
        return lte::helper::QoSClasses::PCCH();
    }
    assure(DllFlowIDToQoSClass.knows(dllFlowID),"expected the QoSClass to be known for flowID="<<dllFlowID<<getFlowTable());
    return DllFlowIDToQoSClass.find(dllFlowID);
}

wns::service::qos::QoSClass
FlowManagerBS::getQoSClassForUTFlowID(FlowID /*dllFlowID*/) const
{
    assure(false,"FlowManagerBS::getQoSClassForUTFlowID(): does not exist");
}

void
FlowManagerBS::setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs)
{
    // can overwrite old entry, but must be the same content
    if (controlPlaneFlowIdsForPeer.knows(peerAddress)) {
        ControlPlaneFlowIDs oldFlowIDs = controlPlaneFlowIdsForPeer.find(peerAddress);
        assure(flowIDs == oldFlowIDs,"setControlPlaneFlowIDs("<<A2N(peerAddress)<<","<<printControlPlaneFlowIDs(flowIDs)<<") overwrites oldFlowID="<<printControlPlaneFlowIDs(oldFlowIDs)<<getFlowTable());
    } else { // new
        MESSAGE_SINGLE(NORMAL, logger, "setControlPlaneFlowIDs("<<A2N(peerAddress)<<",flowIDs="<<printControlPlaneFlowIDs(flowIDs)<<")");
        controlPlaneFlowIdsForPeer.insert(peerAddress, flowIDs);

        int qosClass = lte::helper::QoSClasses::PHICH();
        assure(flowIDs.size()== 3, "Expected exactly 3 control plane FlowIDs (PBCH, PCCH, DCCH, PHICH), but got " << flowIDs.size());
        for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin();
             iter != flowIDs.end();
             ++iter)
        {
            FlowID flowID = iter->second;
            assure(!DllFlowIDToQoSClass.knows(flowID),"DllFlowIDToQoSClass("<<flowID<<") already registered");
            // qosClass matches the QoS class ENUM from lte::helper::QoSClasses, i.e. PCCH=1, DCCH=2, PHICH=3
            DllFlowIDToQoSClass.insert(flowID, qosClass);
			qosClass++;
        }
    }
}

std::string
FlowManagerBS::getFlowTable() const
{
    std::stringstream out;
    out << FlowManager::getFlowTable(); // base class prints ControlPlane FlowIDs
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        FlowID flowID = iter->first;
        wns::service::dll::UnicastAddress peerAdr = iter->second;
        wns::service::qos::QoSClass qosClass = lte::helper::QoSClasses::UNDEFINED();
        if (DllFlowIDToQoSClass.knows(flowID))
            qosClass = DllFlowIDToQoSClass.find(flowID);
        out << "D-FID="<<flowID<<": peer="<<A2N(peerAdr)<<", QoS="<<lte::helper::QoSClasses::toString(qosClass) << std::endl;
    }
    return out.str();
}

void
FlowManagerBS::buildFlow(wns::service::tl::FlowID /*flowID*/, wns::service::qos::QoSClass /*qosClass*/)
{}

void
FlowManagerBS::flowBuilt(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    openUpperFlow(flowID);
    //erase preserved FlowID:
    preservedFlowIDs.erase(flowID);
}

FlowManager::ControlPlaneFlowIDs
FlowManagerBS::getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress)
{
    ControlPlaneFlowIDs controlPlaneFlowIDs;
    if (controlPlaneFlowIdsForPeer.knows(peerAddress)) {
        controlPlaneFlowIDs = controlPlaneFlowIdsForPeer.find(peerAddress);
        MESSAGE_SINGLE(VERBOSE, logger, "getControlPlaneFlowIDs("<<A2N(peerAddress)<<"): flowIDs="<<printControlPlaneFlowIDs(controlPlaneFlowIDs));
    }
    else
    {
        for (int qosClass = lte::helper::QoSClasses::PHICH();
             qosClass <= lte::helper::QoSClasses::DCCH();
             ++qosClass)
        {
            // flow IDs for PCCH, DCCH and PHICH are assigned now
            FlowID flowID = flowIDPool.suggestPort();
            flowIDPool.bind(flowID);
            controlPlaneFlowIDs[qosClass] = flowID;

            if (!DllFlowIDToQoSClass.knows(flowID))
                DllFlowIDToQoSClass.insert(flowID, qosClass);
        }
        controlPlaneFlowIdsForPeer.insert(peerAddress, controlPlaneFlowIDs);
        MESSAGE_SINGLE(NORMAL, logger, "getControlPlaneFlowIDs("<<A2N(peerAddress)<<"): flowIDs="<<printControlPlaneFlowIDs(controlPlaneFlowIDs)<<" NEW");
    }
    return controlPlaneFlowIDs;
}

void
FlowManagerBS::forwardFlowRequest(lte::helper::TransactionID _transactionID,
                                  lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* _flowHandlerBS,
                                  wns::service::dll::UnicastAddress utAddress,
                                  FlowID oldFlowID,
                                  wns::service::qos::QoSClass qosClass)
{
    MESSAGE_SINGLE(NORMAL, logger, "FlowManagerBS::forwardFlowRequest(UT="<<A2N(utAddress)<<",TID="<<_transactionID<<",oldFID="<<oldFlowID<<",QoS="<<lte::helper::QoSClasses::toString(qosClass)<<")");
    if(hasPreserved(oldFlowID))
    {
        //(no new flow id if preserved) and reconfigure switching table (oldFlowID)
        //FlowConfirm...
        MESSAGE_SINGLE(NORMAL, logger, "Intra-REC Handover (preserved) for User:"<<A2N(utAddress)<<" Confirming FlowID="<<oldFlowID);
        _flowHandlerBS->flowConfirm(_transactionID, oldFlowID, utAddress);
        assure(qosClass == DllFlowIDToQoSClass.find(oldFlowID),"oldFlowID does not know qosClass"<<getFlowTable());
    }
    else
    {
        //select (outgoing) transactionID for request to RANG
        lte::helper::TransactionID transactionIdBS = drawNewTransactionID();
        //save transactionID's
        TransactionIdOutToIn.insert(transactionIdBS, _transactionID);
        //save FlowHandler to answer to later
        TransactionIdOutToFlowHandler.insert(transactionIdBS, _flowHandlerBS);
        TransactionIDToQoSClass.insert(_transactionID, qosClass); // other transactionID here!
        //call BSUpperConvergence toget a flowID from the RANG
        FlowID _flowIDout = bsUpperConvergence->requestFlow(transactionIdBS, utAddress);

	assure(_flowIDout>0,"flowID="<<_flowIDout<<", must be >0");
	// _flowIDout is the FlowID chosen by the RANG
	// we've got to choose a new FlowIDIn for the lower hop and save this
	// pair (FlowIDIn, FlowIDout) in the switching table of the BS.

	// choose FlowID for UT or RN
	FlowID flowIDin = flowIDPool.suggestPort();
	flowIDPool.bind(flowIDin);
	assure(!FlowIDToUT.knows(flowIDin), "This FlowID="<<flowIDin<<" is already in the FlowIDTable."<<getFlowTable());
	MESSAGE_SINGLE(NORMAL, logger, "registerFlowID(): Newly chosen flowIDin="<<flowIDin);

	// create new closed flow
	upperFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(flowIDin)));
	arqFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(flowIDin)));

	insertFlowIDToUT(flowIDin, utAddress);

	// get TransactionIdin for TransactionIdOut
	assure(TransactionIdOutToIn.knows(transactionIdBS), "TransactionID not known!");
	lte::helper::TransactionID transactionIdIn = TransactionIdOutToIn.find(transactionIdBS);
	
	assure(TransactionIDToQoSClass.knows(transactionIdIn),"TransactionIDToQoSClass(transactionIdIn="<<transactionIdIn<<") unknown"<<getFlowTable());
	wns::service::qos::QoSClass qosClass = TransactionIDToQoSClass.find(transactionIdIn);
	TransactionIDToQoSClass.erase(transactionIdIn);
	assure(qosClass!=lte::helper::QoSClasses::UNDEFINED(),"undefined qosClass");
	DllFlowIDToQoSClass.insert(flowIDin,qosClass);

	FlowIDInToFlowIDOut.insert(flowIDin, _flowIDout);
	MESSAGE_SINGLE(NORMAL, logger, "Updating Switching table: New (FlowIDin, FlowIDout): ("<<flowIDin<<", "<<_flowIDout<<")");

	// get the right FlowHandlerBS
	assure(TransactionIdOutToFlowHandler.knows(transactionIdBS),"No FlowHandlerBS registered for transactionID : "<<transactionIdIn);
	lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* flowHandlerBS =
	  TransactionIdOutToFlowHandler.find(transactionIdBS);

	// delete transactionIDs from containers
	TransactionIdOutToIn.erase(transactionIdBS);
	TransactionIdOutToFlowHandler.erase(transactionIdBS);

	// trigger FlowHandlerBS to Flow_Confirm
	flowHandlerBS->flowConfirm(transactionIdIn, flowIDin, utAddress);
    }
}

void
FlowManagerBS::onCSRCreated()
{
    layer2 = dynamic_cast<dll::Layer2*>(getCSR()->getLayer());

    for( std::list<std::string>::iterator iter = flowSeparatorNames.begin(); iter != flowSeparatorNames.end(); ++iter)
    {
        wns::ldk::FlowSeparator* fs = layer2->getFUN()->findFriend<wns::ldk::FlowSeparator*>(*iter);
        flowSeparators.push_back(fs);
    }

    //get handle to the AssociationsProxyBS
    wns::ldk::ControlServiceInterface* csi = layer2->getControlService<wns::ldk::ControlServiceInterface>("AssociationsProxy");
    aProxyBS = dynamic_cast<lte::controlplane::AssociationsProxyBS*>(csi);
    assureNotNull(aProxyBS);

    rlcReader = layer2->getFUN()->getCommandReader("rlc");
    bsUpperConvergence = layer2->getFUN()->findFriend<lte::upperconvergence::ENBUpperConvergence*>
        ("upperConvergence");
    assure(bsUpperConvergence, "No BSUpperConvergence set.");

    upperSynchronizer = layer2->getFUN()->findFriend<wns::ldk::tools::Synchronizer*>("upperSynchronizer");
    upperFlowGate = layer2->getFUN()->findFriend<wns::ldk::FlowGate*>("upperFlowGate");
    arqFlowGate = layer2->getFUN()->findFriend<wns::ldk::FlowGate*>("arqFlowGate");
    assure(upperSynchronizer, "No upperSynchronizer set.");
    assure(upperFlowGate, "No upperFlowGate set.");
    assure(arqFlowGate, "No arqFlowGate set.");
}

void
FlowManagerBS::releaseFlow(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    //the FlowID entries in the switching table have to be deleted
    //the FlowID has also to be unbound from the FLowIDPool
    //so this FlowID could be chosen again in future FlowRequests.
    //finally we have to tell the RANG to delete this flow too.
    //RANG has to be informed of the FlowRelease
    FlowID flowIDOut = getFlowIDout(flowID);
    bsUpperConvergence->releaseFlow(flowIDOut);

    FlowIDInToFlowIDOut.erase(flowID);

    MESSAGE_SINGLE(NORMAL, logger, "Updating Switching table: Erasing (FlowIDin, FlowIDout): ("<<flowID<<", "<<flowIDOut<<")");

    flowIDPool.unbind(flowID);
}

void
FlowManagerBS::deleteFlowsInRang(wns::service::dll::UnicastAddress userAdr)
{
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == userAdr)
        {
	  bsUpperConvergence->deleteFlow(getFlowIDout(iter->first));
        }
    }
}

void
FlowManagerBS::deleteLowerFlow(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    //destroy lower flows for myModes
    for(std::list<std::string>::iterator iter = myModes.begin(); iter != myModes.end(); ++iter)
    {
        ModeName mode = *iter;
        lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* flowHandlerBS =
            layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS*>
            (mode+"_FlowHandler");

        flowHandlerBS->destroyFlow(flowID);
        MESSAGE_SINGLE(NORMAL, logger, "Lower Flow deleted for FlowID="<<flowID);
    }
}

void
FlowManagerBS::deleteAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode)
{
    //destroy lower flows for mode
    lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* flowHandlerBS =
        layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS*>
        (mode+"_FlowHandler");

    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == userAdr)
        {
            flowHandlerBS->destroyFlow(iter->first);
        }
    }
}

void
FlowManagerBS::preserveFlows(wns::service::dll::UnicastAddress userAdr)
{
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        if(iter->second == userAdr)
        {
            FlowID flowID = iter->first;
            closeUpperFlow(flowID);
            MESSAGE_SINGLE(NORMAL, logger, "preserveFlows("<<userAdr<<") for FlowID="<<flowID<<" for Intra-REC HO.");
            assure(!hasPreserved(flowID), "FlowID has already been preserved!");
            registerPreservedFlowID(flowID);
            assure(hasPreserved(flowID), "preserve not correctly!");
        }
    }
}

void
FlowManagerBS::onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, bool preserved)
{
    if(preserved) // can keep state in macg FUs (e.g. ARQ state), because intra-REC handover
    {
        MESSAGE_SINGLE(NORMAL, logger, "onDisassociationReq("<<A2N(userAdr)<<","<<mode<<","<<preserved<<"): Preserving context of " << A2N(userAdr));
        //preserve upper flows
        preserveFlows(userAdr);
        deleteAllLowerFlows(userAdr, mode);
    }
    else
    {
        MESSAGE_SINGLE(NORMAL, logger, "onDisassociationReq("<<A2N(userAdr)<<","<<mode<<","<<preserved<<"): Deleting context of " << A2N(userAdr));
        deleteAllFlowSeparators(userAdr); // deletes all upper flow separators for this user
        deleteAllUpperFlows(userAdr); // deletes all upper flow gates for this user and if nnecessary the packet in the upper synchronizer
        deleteFlowsInRang(userAdr); // deletes the flows of this user in the RANG
        deleteAllLowerFlows(userAdr, mode); // deletes the mode-dependent lower flow separators and gates for this user by means of the FlowHandler
        deleteSwitchingTableForUT(userAdr); // deletes flows in FlowIDInToFlowIDOut for this user
        deleteFlowsForUT(userAdr);   // deletes flows in FlowIDToUT for this user and frees the FlowIDs in the flowIDPool
    }
}

// called from FlowHandler, which was notified by AssociationObserver
void
FlowManagerBS::onAssociated(wns::service::dll::UnicastAddress /*userAdr*/, wns::service::dll::UnicastAddress /*dstAdr*/)
{}

// called from FlowHandler, which was notified by AssociationObserver at the very end.
void
FlowManagerBS::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    MESSAGE_SINGLE(NORMAL, logger, "onDisassociated(from "<<A2N(userAdr)<<" to "<<A2N(dstAdr)<<")"); // "via" case is filtered out in FlowHandler
    assure(dstAdr == layer2->getDLLAddress(),"not for me ?!");
    assure(controlPlaneFlowIdsForPeer.knows(userAdr),"controlPlaneFlowIdsForPeer("<<A2N(userAdr)<<") not known"<<getFlowTable());
    ControlPlaneFlowIDs flowIDs = controlPlaneFlowIdsForPeer.find(userAdr);
    for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin();
         iter != flowIDs.end();
         ++iter)
    {
        FlowID flowID = iter->second;
        assure(DllFlowIDToQoSClass.knows(flowID),"DllFlowIDToQoSClass("<<flowID<<") unknown");
        DllFlowIDToQoSClass.erase(flowID);
    }
    controlPlaneFlowIdsForPeer.erase(userAdr);
}

////////////////////////////
//                        //
//    FlowManagerUT       //
//                        //
////////////////////////////

FlowManagerUT::FlowManagerUT(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
    FlowManager(csr, config),
    ControlService(csr)
{
}

FlowManagerUT::~FlowManagerUT()
{}

wns::service::qos::QoSClass
FlowManagerUT::getQoSClassForBSFlowID(FlowID /*dllFlowID*/) const
{
    assure(false,"FlowManagerUT::getQoSClassForBSFlowID(): does not exist");
}

wns::service::qos::QoSClass
FlowManagerUT::getQoSClassForUTFlowID(FlowID dllFlowID) const
{
    if (dllFlowID == bchFlowID_)
    {
        return lte::helper::QoSClasses::PBCH();
    }
    assure(dllFlowID>0,"flowID="<<dllFlowID<<", must be >0");
    assure(DllFlowIDToQoSClass.knows(dllFlowID),"expected the QoSClass to be known for flowID="<<dllFlowID<<getFlowTable());
    return DllFlowIDToQoSClass.find(dllFlowID);
}

void
FlowManagerUT::setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs)
{
    // can overwrite old entry, but must be the same content
    if (controlPlaneFlowIdsForPeer.knows(peerAddress)) {
        ControlPlaneFlowIDs oldFlowIDs = controlPlaneFlowIdsForPeer.find(peerAddress);
        assure(flowIDs == oldFlowIDs,"setControlPlaneFlowIDs("<<A2N(peerAddress)<<","<<printControlPlaneFlowIDs(flowIDs)<<") overwrites oldFlowIDs="<<printControlPlaneFlowIDs(oldFlowIDs)<<getFlowTable());
    } else { // new
        MESSAGE_SINGLE(NORMAL, logger, "setControlPlaneFlowIDs("<<A2N(peerAddress)<<", flowIDs="<<printControlPlaneFlowIDs(flowIDs)<<")");
        controlPlaneFlowIdsForPeer.insert(peerAddress, flowIDs);

        int qosClass = lte::helper::QoSClasses::PHICH();
        assure(flowIDs.size()== 3, "Expected exactly 3 control plane FlowIDs (PCCH, DCCH, PHICH), but got " << flowIDs.size());
        for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin();
             iter != flowIDs.end();
             ++iter)
        {
            FlowID flowID = iter->second;
            assure(!DllFlowIDToQoSClass.knows(flowID),"DllFlowIDToQoSClass("<<flowID<<") already registered");
            DllFlowIDToQoSClass.insert(flowID, qosClass);
			qosClass++;
        }
    }
}

std::string
FlowManagerUT::getFlowTable() const
{
    std::stringstream out;
    out << FlowManager::getFlowTable(); // base class prints ControlPlane FlowIDs
    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        FlowID flowID = iter->first;
        wns::service::dll::UnicastAddress peerAdr = iter->second;
        wns::service::qos::QoSClass qosClass = lte::helper::QoSClasses::UNDEFINED();
        if (DllFlowIDToQoSClass.knows(flowID))
            qosClass = DllFlowIDToQoSClass.find(flowID);
        out << "FID="<<flowID<<": peer="<<A2N(peerAdr)<<", QoS="<<lte::helper::QoSClasses::toString(qosClass) << std::endl;
    }
    return out.str();
}

void
FlowManagerUT::buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass)
{
    // get an association from the associationProxy.
    if(aProxyUT->hasAssociation())
    {
        wns::service::dll::UnicastAddress destinationAddress = aProxyUT->getBestDetected().rapAdr;
        std::string usedMode = aProxyUT->getBestDetected().mode;
        //select a mode and get the FlowHandler for this mode(association)
        lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT* flowHandlerUT =
            layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*>
            (usedMode+"_FlowHandler");

        TlFlowIDToFlowHandler.insert(flowID, flowHandlerUT);

        lte::helper::TransactionID transactionId = drawNewTransactionID();
        TransactionIDToTlFlowID.insert(transactionId, flowID);
        TransactionIDToQoSClass.insert(transactionId, qosClass);
        MESSAGE_SINGLE(NORMAL, logger, "buildFlow(tl::FlowID="<<flowID<<"," // tl::flowID is not an int here!
                       <<lte::helper::QoSClasses::toString(qosClass)
                       <<"): dest="<<A2N(destinationAddress)
                       <<", transactionId="<<transactionId<<", mode="<<usedMode);
        assure(qosClass!=lte::helper::QoSClasses::UNDEFINED(),"undefined qosClass for FlowID="<<flowID);
        flowHandlerUT->flowReq(transactionId, destinationAddress, 0/* means unused */, qosClass);
    }
    else
    {
        MESSAGE_SINGLE(NORMAL, logger, "Terminal currently not associated. Waiting for new Association.");
    }
}

void
FlowManagerUT::getAssociations()
{
    std::string mode = aProxyUT->getBestDetected().mode;
    selectedMode = mode;
}

void
FlowManagerUT::flowBuilt(lte::helper::TransactionID _transactionId,  FlowID _dllFlowID)
{
    assure(_dllFlowID>0,"flowID="<<_dllFlowID<<", must be >0");
    bool rebuiltFlow = false;
    if(TransactionIDToOldFlowID.knows(_transactionId))
    {
        rebuiltFlow = true;
    }

    if(rebuiltFlow) // new flowID for existing Flow either preserved or not:
    {
        if(hasPreserved(_dllFlowID)) // Intra-REC
        {
            MESSAGE_SINGLE(NORMAL, logger, "Opening UpperFlowGates for preserved FlowID="<<TransactionIDToOldFlowID.find(_transactionId));

            // erase preserved FlowID
            preservedFlowIDs.erase(_dllFlowID);

            // delete TransactionID-stuff:
            TransactionIDToOldFlowID.erase(_transactionId);
            TransactionIDToTlFlowID.erase(_transactionId);
            TransactionIDToQoSClass.erase(_transactionId);
            assure(DllFlowIDToQoSClass.knows(_dllFlowID),"expected the QoSClass to be known for flowID="<<_dllFlowID<<getFlowTable());

            // open Gates
            upperFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
            arqFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
        }
        else // Inter-REC or PlainDisassociation
        {
            if(!DroppedFlowIDs.knows(TransactionIDToOldFlowID.find(_transactionId))) // Inter-REC
            {

                MESSAGE_SINGLE(NORMAL, logger, "Flow_rebuilt for old FlowID="<<TransactionIDToOldFlowID.find(_transactionId)<<", new FlowID="<< _dllFlowID);

                assure(TransactionIDToTlFlowID.knows(_transactionId),"There is no Transaction with transactionId=" << _transactionId);
                assure(TransactionIDToQoSClass.knows(_transactionId),"expected the QoSClass to be known for transactionId="<<_transactionId);

                //insert into table
                wns::service::tl::FlowID    tlFlowID = TransactionIDToTlFlowID.find(_transactionId);
                wns::service::qos::QoSClass qosClass = TransactionIDToQoSClass.find(_transactionId);

                //erase old entries:
                TlFlowIDToDllFlowID.erase(tlFlowID);
                FlowID oldFlowID = TransactionIDToOldFlowID.find(_transactionId);
                DllFlowIDToTlFlowID.erase(oldFlowID);
                DllFlowIDToQoSClass.erase(oldFlowID);
                MESSAGE_SINGLE(NORMAL, logger, "Erasing old FlowID="<<TransactionIDToOldFlowID.find(_transactionId)<<" from FlowIDTable.");
                FlowIDToUT.erase(TransactionIDToOldFlowID.find(_transactionId));

                //destroy flowseparator instances for old flowid:
                deleteFlowSeparator(TransactionIDToOldFlowID.find(_transactionId));

                // clear UpperSynchronizer for old flowid:
                if(upperSynchronizer->hasSomethingToSend()){
                    wns::ldk::CompoundPtr compound = upperSynchronizer->getSomethingToSend();
                    MESSAGE_SINGLE(NORMAL, logger, "Drop outgoing compound in buffer of upperSynchronizer!");
                }

                // destroy closed old gates:
                upperFlowGate->destroyFlow(wns::ldk::ConstKeyPtr(
                                               new lte::helper::key::FlowID(TransactionIDToOldFlowID.find(_transactionId))));

                arqFlowGate->destroyFlow(wns::ldk::ConstKeyPtr(
                                             new lte::helper::key::FlowID(TransactionIDToOldFlowID.find(_transactionId))));

                // add new entries:
                TlFlowIDToDllFlowID.insert(tlFlowID, _dllFlowID);
                DllFlowIDToTlFlowID.insert(_dllFlowID, tlFlowID);
                DllFlowIDToQoSClass.insert(_dllFlowID, qosClass);
                insertFlowIDToUT(_dllFlowID, layer2->getDLLAddress());

                // delete TransactionID-stuff:
                TransactionIDToOldFlowID.erase(_transactionId);
                TransactionIDToTlFlowID.erase(_transactionId);
                TransactionIDToQoSClass.erase(_transactionId);

                // inform UpperConvergence of built flow
                utUpperConvergence->onFlowBuilt(tlFlowID, _dllFlowID, false);
                // create new gates
                upperFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
                arqFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
                upperFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
                arqFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
            }
            else // plainDisassociation
            {
                MESSAGE_SINGLE(NORMAL, logger, "Flow_rebuilt for dropped FlowID: "<<TransactionIDToOldFlowID.find(_transactionId)<<", new FlowID: "<< _dllFlowID);

                assure(TransactionIDToTlFlowID.knows(_transactionId),"There is no Transaction with transactionId=" << _transactionId);
                assure(TransactionIDToQoSClass.knows(_transactionId),"expected the QoSClass to be known for transactionId="<<_transactionId);

                // insert into table
                wns::service::tl::FlowID    tlFlowID = TransactionIDToTlFlowID.find(_transactionId);
                wns::service::qos::QoSClass qosClass = TransactionIDToQoSClass.find(_transactionId);

                // erase old entries:
                TlFlowIDToDllFlowID.erase(tlFlowID);
                FlowID oldFlowID = TransactionIDToOldFlowID.find(_transactionId);
                DllFlowIDToTlFlowID.erase(oldFlowID);
                DllFlowIDToQoSClass.erase(oldFlowID);
                MESSAGE_SINGLE(NORMAL, logger, "Erasing old FlowID: "<<TransactionIDToOldFlowID.find(_transactionId)<<" from DroppedFlowIDs-Table.");
                DroppedFlowIDs.erase(TransactionIDToOldFlowID.find(_transactionId));

                // add new entries:
                TlFlowIDToDllFlowID.insert(tlFlowID, _dllFlowID);
                DllFlowIDToTlFlowID.insert(_dllFlowID, tlFlowID);
                DllFlowIDToQoSClass.insert(_dllFlowID, qosClass);
                insertFlowIDToUT(_dllFlowID, layer2->getDLLAddress());

                // delete TransactionID-stuff:
                TransactionIDToOldFlowID.erase(_transactionId);
                TransactionIDToTlFlowID.erase(_transactionId);
                TransactionIDToQoSClass.erase(_transactionId);

                // inform UpperConvergence of built flow
                utUpperConvergence->onFlowBuilt(tlFlowID, _dllFlowID, false);
                // create new gates
                upperFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
                arqFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
                upperFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
                arqFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
            }
        }
    }
    else  // Totally new Flow
    {
        MESSAGE_SINGLE(NORMAL, logger, "New Flow built. FlowID: "<<_dllFlowID);

//         assure(TransactionIDToTlFlowID.knows(_transactionId),"There is no Transaction with transactionId=" << _transactionId);
//         assure(TransactionIDToQoSClass.knows(_transactionId),"expected the QoSClass to be known for transactionId="<<_transactionId);

        if (TransactionIDToTlFlowID.knows(_transactionId)) {
            // insert into table
            wns::service::tl::FlowID tlFlowID = TransactionIDToTlFlowID.find(_transactionId);
            TlFlowIDToDllFlowID.insert(tlFlowID, _dllFlowID);
            DllFlowIDToTlFlowID.insert(_dllFlowID, tlFlowID);
            wns::service::qos::QoSClass qosClass = TransactionIDToQoSClass.find(_transactionId);
            DllFlowIDToQoSClass.insert(_dllFlowID, qosClass);

            TransactionIDToTlFlowID.erase(_transactionId);
            TransactionIDToQoSClass.erase(_transactionId);

            insertFlowIDToUT(_dllFlowID, layer2->getDLLAddress());

            //inform UpperConvergence of built flow
            utUpperConvergence->onFlowBuilt(tlFlowID, _dllFlowID, true);

            // create new flow
            upperFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
            arqFlowGate->createFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
            upperFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
            arqFlowGate->openFlow(wns::ldk::ConstKeyPtr(new lte::helper::key::FlowID(_dllFlowID)));
        }
    }
    // inform the AssociationsProxy for probe measurement purposes and that
    // the contingently the handover is finished
    aProxyUT->flowBuilt();
    MESSAGE_SINGLE(NORMAL, logger, "Probes written: "<<_dllFlowID);
} // FlowManagerUT::flowBuilt()

FlowManager::ControlPlaneFlowIDs
FlowManagerUT::getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress)
{
    ControlPlaneFlowIDs flowIDs;
    if (controlPlaneFlowIdsForPeer.knows(peerAddress))
    {
        flowIDs = controlPlaneFlowIdsForPeer.find(peerAddress);
    }
    else
    { // can only be the (one and only) RAP we are associated to, i.e. there is only one entry
        assure(controlPlaneFlowIdsForPeer.begin() != controlPlaneFlowIdsForPeer.end(),"controlPlaneFlowIdsForPeer is empty"<<getFlowTable());
        flowIDs = controlPlaneFlowIdsForPeer.begin()->second;
    }
    MESSAGE_SINGLE(VERBOSE, logger, "getControlPlaneFlowIDs("<<A2N(peerAddress)<<"): flowIDs="<<printControlPlaneFlowIDs(flowIDs));
    return flowIDs;
}

void
FlowManagerUT::releaseFlow(wns::service::tl::FlowID flowID)
{
    assure(TlFlowIDToDllFlowID.knows(flowID), "There is no Flow for this FlowID="<<flowID<<" available."<<getFlowTable());
    lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT* flowHandlerUT = TlFlowIDToFlowHandler.find(flowID);
    assure(flowHandlerUT, "FlowHandler not found.");
    flowHandlerUT->releaseFlow(TlFlowIDToDllFlowID.find(flowID));
}

void
FlowManagerUT::flowReleased(FlowID flowID)
{
    assure(flowID>0,"flowID="<<flowID<<", must be >0");
    wns::service::tl::FlowID tmpTlFlowID = DllFlowIDToTlFlowID.find(flowID);
    TlFlowIDToFlowHandler.erase(tmpTlFlowID);
    MESSAGE_SINGLE(NORMAL, logger, "Flow released for FlowID="<<flowID);
}

std::string
FlowManagerUT::selectMode()
{
    return selectedMode;
}

void
FlowManagerUT::onCSRCreated()
{
    layer2 = dynamic_cast<dll::Layer2*>(getCSR()->getLayer());

    for( std::list<std::string>::iterator iter = flowSeparatorNames.begin(); iter != flowSeparatorNames.end(); ++iter)
    {
        wns::ldk::FlowSeparator* fs = layer2->getFUN()->findFriend<wns::ldk::FlowSeparator*>(*iter);
        flowSeparators.push_back(fs);
    }

    //get handle to the AssociationsProxyUT
    wns::ldk::ControlServiceInterface* csi = layer2->getControlService<wns::ldk::ControlServiceInterface>("AssociationsProxy");
    aProxyUT = dynamic_cast<lte::controlplane::AssociationsProxyUT*>(csi);
    assureNotNull(aProxyUT);

    rlcReader = layer2->getFUN()->getCommandReader("rlc");

    upperSynchronizer = layer2->getFUN()->findFriend<wns::ldk::tools::Synchronizer*>("upperSynchronizer");
    upperFlowGate = layer2->getFUN()->findFriend<wns::ldk::FlowGate*>("upperFlowGate");
    arqFlowGate = layer2->getFUN()->findFriend<wns::ldk::FlowGate*>("arqFlowGate");

    utUpperConvergence = layer2->getFUN()->findFriend<lte::upperconvergence::UEUpperConvergence*>
        ("upperConvergence");

    assure(upperFlowGate, "No upperFlowGate set.");
    assure(arqFlowGate, "No arqFlowGate set.");
    assure(utUpperConvergence, "No UpperConvergence set.");
}

// after a plaindisassociation, the packets from the existing flows are dropped.
// layer 2 has to know that there are packets being dropped.
void
FlowManagerUT::insertDroppedFlowIDs()
{
    MESSAGE_SINGLE(NORMAL, logger, "FLOWIDFORUTSIZE: "<<FlowIDToUT.size());

    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        FlowID dllFlowID     = iter->first;
        FlowID droppedFlowID = dllFlowID + 65536; // outside the normal bounds
        wns::service::tl::FlowID tlFlowID       = DllFlowIDToTlFlowID.find(dllFlowID);
        wns::service::qos::QoSClass qosClass    = DllFlowIDToQoSClass.find(dllFlowID);
        MESSAGE_SINGLE(NORMAL, logger, "Deleted FlowID="<<dllFlowID<<", inserted FlowIDtoBeDropped="<< droppedFlowID<< " for TLFlowID="<<tlFlowID);
        DllFlowIDToTlFlowID.erase(dllFlowID);
        DllFlowIDToTlFlowID.insert(droppedFlowID, tlFlowID);
        DllFlowIDToQoSClass.erase(dllFlowID);
        DllFlowIDToQoSClass.insert(droppedFlowID, qosClass);
        DroppedFlowIDs.insert(droppedFlowID, layer2->getDLLAddress());
        TlFlowIDToDllFlowID.erase(tlFlowID);
        TlFlowIDToDllFlowID.insert(tlFlowID, droppedFlowID);
        utUpperConvergence->onFlowBuilt(tlFlowID, droppedFlowID, false);
    }
    FlowIDToUT.clear();
}

// called by AssociationProxy
void
FlowManagerUT::onAssociatedPerMode(wns::service::dll::UnicastAddress rapAdr, bool preserved)
{
    if(preserved)
    {
        MESSAGE_SINGLE(NORMAL, logger, "AssociatedPerMode: Intra_Rec Handover.");
    }
    else
    {
        MESSAGE_SINGLE(NORMAL, logger, "AssociatedPerMode: Inter_Rec Handover.");
    }
    reBuildFlows(rapAdr, preserved);
}

// called by AssociationsProxy
void
FlowManagerUT::disassociating(ModeName mode)
{
    MESSAGE_SINGLE(NORMAL, logger, "About to Disassociate. Close UL-Flows.");
    closeUpperFlows(layer2->getDLLAddress());
    closeLowerFlows(mode);
}

// disassociation due to following handover (called by AssociationsProxy)
void
FlowManagerUT::onDisassociatedPerMode(wns::service::dll::UnicastAddress /*bsAdr*/, ModeName mode, bool preserved)
{
    if(preserved)
    {
        MESSAGE_SINGLE(NORMAL, logger, "DisassociatedPerMode: Intra-REC Handover.");
        for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
             iter != FlowIDToUT.end();
             ++iter)
        {
            MESSAGE_SINGLE(NORMAL, logger, "Preserving FlowID="<<iter->first);
            registerPreservedFlowID(iter->first);
        }
    }
    else
    {
        MESSAGE_SINGLE(NORMAL, logger, "DisassociatedPerMode: Inter-REC Handover.");
    }
    deleteAllLowerFlows(mode);
}

// disassociation from old RAP without any new RAP (called by AssociationsProxy)
void
FlowManagerUT::onPlainDisassociation(ModeName mode)
{
    plainDisassociation = true;
    MESSAGE_SINGLE(NORMAL, logger, "PlainDisassociation: Deleting all Flows. All packets are being dropped till new Association.");
    deleteAllFlowSeparators(layer2->getDLLAddress());
    deleteAllUpperFlows(layer2->getDLLAddress());
    deleteAllLowerFlows(mode);
    insertDroppedFlowIDs();
}

// notified by AssociationObserver
void
FlowManagerUT::onAssociated(wns::service::dll::UnicastAddress /*userAdr*/, wns::service::dll::UnicastAddress /*dstAdr*/)
{}

// notified by AssociationObserver at the very end.
void
FlowManagerUT::onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr)
{
    MESSAGE_SINGLE(NORMAL, logger, "onDisassociated(from "<<A2N(userAdr)<<" to "<<A2N(dstAdr)<<")");
    assure(userAdr == layer2->getDLLAddress(),"not for me ?!");
    assure(controlPlaneFlowIdsForPeer.knows(dstAdr),"controlPlaneFlowIdsForPeer("<<A2N(dstAdr)<<") not known"<<getFlowTable());
    ControlPlaneFlowIDs flowIDs = controlPlaneFlowIdsForPeer.find(dstAdr);
    for (ControlPlaneFlowIDs::const_iterator iter = flowIDs.begin();
         iter != flowIDs.end();
         ++iter)
    {
        FlowID flowID = iter->second;
        assure(DllFlowIDToQoSClass.knows(flowID),"DllFlowIDToQoSClass("<<flowID<<") unknown");
        DllFlowIDToQoSClass.erase(flowID);
    }
    controlPlaneFlowIdsForPeer.erase(dstAdr);
}

void
FlowManagerUT::reBuildFlows(wns::service::dll::UnicastAddress /*bsAdr*/, bool preserved)
{
    wns::service::dll::UnicastAddress destinationAddress = aProxyUT->getBestDetected().rapAdr;
    std::string usedMode = aProxyUT->getBestDetected().mode;

    //select a mode and get the FlowHandler for this mode(association)
    lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT* flowHandlerUT =
        layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*>
        (usedMode+"_FlowHandler");

    if(preserved)
    {
        for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
             iter != FlowIDToUT.end();
             ++iter)
        {
            FlowID dllFlowID = iter->first;
            MESSAGE_SINGLE(NORMAL, logger, "Flow_Establishment triggered for preserved FlowID="<<dllFlowID);
            lte::helper::TransactionID transactionId = drawNewTransactionID();
            TransactionIDToOldFlowID.insert(transactionId, dllFlowID);
            TransactionIDToTlFlowID.insert(transactionId, DllFlowIDToTlFlowID.find(dllFlowID));
            assure(DllFlowIDToQoSClass.knows(dllFlowID),"expected the QoSClass to be known for flowID="<<dllFlowID<<getFlowTable());
            wns::service::qos::QoSClass qosClass = DllFlowIDToQoSClass.find(dllFlowID);
            TransactionIDToQoSClass.insert(transactionId, qosClass);
            flowHandlerUT->flowReq(transactionId, destinationAddress, dllFlowID, qosClass);
        }
    }
    else
    {
        if(!plainDisassociation)
        {
            for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
                 iter != FlowIDToUT.end();
                 ++iter)
            {
                FlowID dllFlowID = iter->first;
                MESSAGE_SINGLE(NORMAL, logger, "New Flow_Establishment triggered for old FlowID="<<dllFlowID);
                lte::helper::TransactionID transactionId = drawNewTransactionID();
                TransactionIDToOldFlowID.insert(transactionId, dllFlowID);

                TransactionIDToTlFlowID.insert(transactionId, DllFlowIDToTlFlowID.find(dllFlowID));
                assure(DllFlowIDToQoSClass.knows(dllFlowID),"expected the QoSClass to be known for flowID="<<dllFlowID<<getFlowTable());
                wns::service::qos::QoSClass qosClass = DllFlowIDToQoSClass.find(dllFlowID);
                TransactionIDToQoSClass.insert(transactionId, qosClass);
                flowHandlerUT->flowReq(transactionId, destinationAddress, dllFlowID, qosClass);
            }
        }
        else
        {
            for (FlowIDTable::const_iterator iter = DroppedFlowIDs.begin();
                 iter != DroppedFlowIDs.end();
                 ++iter)
            {
                FlowID dllFlowID = iter->first;
                MESSAGE_SINGLE(NORMAL, logger, "New Flow_Establishment triggered for existing but yet dropped FlowID="<<dllFlowID);
                lte::helper::TransactionID transactionId = drawNewTransactionID();
                TransactionIDToOldFlowID.insert(transactionId, dllFlowID);
                TransactionIDToTlFlowID.insert(transactionId, DllFlowIDToTlFlowID.find(dllFlowID));
                assure(DllFlowIDToQoSClass.knows(dllFlowID),"expected the QoSClass to be known for flowID="<<dllFlowID<<getFlowTable());
                wns::service::qos::QoSClass qosClass = DllFlowIDToQoSClass.find(dllFlowID);
                TransactionIDToQoSClass.insert(transactionId, qosClass);
                flowHandlerUT->flowReq(transactionId, destinationAddress, dllFlowID, qosClass);
            }
            plainDisassociation = false;
        }
    }
}

void
FlowManagerUT::deleteLowerFlow(FlowID flowID)
{
    //destroy lower flows for myModes
    for(std::list<std::string>::iterator iter = myModes.begin(); iter != myModes.end(); ++iter)
    {
        ModeName mode = *iter;
        lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT* flowHandlerUT =
            layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*>
            (mode+"_FlowHandler");
        flowHandlerUT->destroyFlow(flowID);
        MESSAGE_SINGLE(NORMAL, logger, "Lower Flow deleted for FlowID="<<flowID);
    }
}

void
FlowManagerUT::deleteAllLowerFlows(ModeName mode)
{
    lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT* flowHandlerUT =
        layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*>
        (mode+"_FlowHandler");

    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        flowHandlerUT->destroyFlow(iter->first);
        MESSAGE_SINGLE(NORMAL, logger, "Lower Flow deleted for FlowID="<<iter->first);
    }
}

void
FlowManagerUT::closeLowerFlows(ModeName mode)
{
    //close lower flows for mode
    lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT* flowHandlerUT =
        layer2->getFUN()->findFriend<lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*>
        (mode+"_FlowHandler");

    for (FlowIDTable::const_iterator iter = FlowIDToUT.begin();
         iter != FlowIDToUT.end();
         ++iter)
    {
        flowHandlerUT->closeFlow(iter->first);
    }
}
