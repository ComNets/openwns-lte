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

#include <LTE/helper/QueueProxy.hpp>
#include <LTE/timing/RegistryProxy.hpp>
#include <WNS/probe/bus/utils.hpp>

using namespace lte::helper;
using namespace wns::scheduler;
using namespace wns::scheduler::queue;

STATIC_FACTORY_REGISTER_WITH_CREATOR(QueueProxy,
                                     wns::scheduler::queue::QueueInterface,
                                     "QueueProxy",
                                     wns::HasReceptorConfigCreator);

QueueProxy::QueueProxy(wns::ldk::HasReceptorInterface* hasReceptor, const wns::pyconfig::View& _config)
    : QueueInterface(),
      queue(NULL), //queue(config),
      rrhandler(NULL),
      rrStorage(NULL),
      queueIsExternal(true),
      logger(_config.get("logger")),
      config(_config),
      probeContextProviderForCid(NULL),
      probeContextProviderForPriority(NULL)
{
    // make a real queue if it is required so (internal, not external):
    if (!queueIsExternal) queue = new wns::scheduler::queue::SimpleQueue(hasReceptor, config);
    MESSAGE_SINGLE(NORMAL, logger, "QueueProxy::QueueProxy()");
}

QueueProxy::~QueueProxy()
{
    MESSAGE_SINGLE(VERBOSE, logger, "QueueProxy::~QueueProxy()");
    if (queue!=NULL && !queueIsExternal) delete queue;
    if (probeContextProviderForCid) { delete probeContextProviderForCid; }
    if (probeContextProviderForPriority) { delete probeContextProviderForPriority; }
}

void
QueueProxy::setColleagues(RegistryProxyInterface* _registry) {
    colleagues.registry = _registry;
    assure(colleagues.registry != NULL,"colleagues.registry==NULL");
    if (queue!=NULL) {
        // Forward call to internal queue
        queue->setColleagues(_registry);
    }
}

void
QueueProxy::setFUN(wns::ldk::fun::FUN* fun)
{
    MESSAGE_SINGLE(NORMAL, logger, "QueueProxy::setFUN()");
    assure(fun!=NULL, "fun==NULL");
    assure(queue!=NULL || rrhandler!=NULL,"need at least queue or rrhandler");
    if (queue!=NULL) queue->setFUN(fun);
    if (rrhandler!=NULL) { // only needed in this case; queue has its own probes
        // Initialization of probes...
        wns::probe::bus::ContextProviderCollection localContext(&fun->getLayer()->getContextProviderCollection());
        for (int ii = 0; ii<config.len("localIDs.keys()"); ++ii)
        {
            std::string key = config.get<std::string>("localIDs.keys()",ii);
            uint32_t value  = config.get<uint32_t>("localIDs.values()",ii);
            localContext.addProvider( wns::probe::bus::contextprovider::Constant(key, value) );
        }
        probeContextProviderForCid  = new wns::probe::bus::contextprovider::Variable("cid", 0);
        probeContextProviderForPriority  = new wns::probe::bus::contextprovider::Variable("MAC.QoSClass", 0);
        localContext.addProvider(wns::probe::bus::contextprovider::Container(probeContextProviderForCid));
        localContext.addProvider(wns::probe::bus::contextprovider::Container(probeContextProviderForPriority));
        std::string sizeProbeName = config.get<std::string>("sizeProbeName");
        sizeProbeBus = wns::probe::bus::ContextCollectorPtr(new wns::probe::bus::ContextCollector(localContext, sizeProbeName));
    }
}

void
QueueProxy::setQueue(wns::scheduler::queue::QueueInterface* _queue)
{
    assure(rrhandler==NULL,"you can either set Queue or RRHandler. Not both!");
    if (queue!=NULL) delete queue; // old object
    queueIsExternal = true; // don't delete in destructor later
    queue=_queue;
    MESSAGE_SINGLE(NORMAL, logger, "configured as proxy to queue");
}

void
QueueProxy::setRRHandler(lte::controlplane::RRHandler* _rrhandler)
{
    assure(queue==NULL,"you can either set Queue or RRHandler. Not both!");
    rrhandler = dynamic_cast<lte::controlplane::RRHandlerBS*>(_rrhandler);
    assure(rrhandler!=NULL,"rrhandler is invalid");
    MESSAGE_SINGLE(NORMAL, logger, "configured as proxy to RRHandler");
    rrStorage = rrhandler->getRRStorage();
    assure(rrStorage!=NULL,"rrStorage is invalid");
}

bool
QueueProxy::isAccepting(const wns::ldk::CompoundPtr&  compound ) const
{
    assure(rrhandler==NULL,"isAccepting() cannot be used in RRHandler proxy mode");
    assure(queue!=NULL,"queue==NULL");
    return queue->isAccepting(compound);
}

void
QueueProxy::put(const wns::ldk::CompoundPtr&  compound ) {
    assure(rrhandler==NULL,"put() cannot be used in RRHandler proxy mode. This compound's journey:\n"<<compound->dumpJourney());
    assure(queue!=NULL,"queue==NULL");
    queue->put(compound);
    // no probing required in this case
}

wns::scheduler::UserSet
QueueProxy::getQueuedUsers() const
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->getQueuedUsers();
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->getActiveUsers();
    }
}

wns::scheduler::ConnectionSet
QueueProxy::getActiveConnections() const
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->getActiveConnections();
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->getActiveConnections();
    }
}

void
QueueProxy::writeProbe(wns::scheduler::ConnectionID cid, unsigned int priority) const
{
    if (probeContextProviderForCid && probeContextProviderForPriority && sizeProbeBus) {
        probeContextProviderForCid->set(cid /*int context*/);
        probeContextProviderForPriority->set(priority);
        double bits = numBitsForCid(cid);
        sizeProbeBus->put(bits); // absolute
    }
}

unsigned long int
QueueProxy::numCompoundsForCid(wns::scheduler::ConnectionID cid) const
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->numCompoundsForCid(cid);
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->numCompoundsForCid(cid);
    }
}

unsigned long int
QueueProxy::numBitsForCid(wns::scheduler::ConnectionID cid) const
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->numBitsForCid(cid);
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->numBitsForCid(cid);
    }
}

wns::scheduler::QueueStatusContainer
QueueProxy::getQueueStatus() const
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->getQueueStatus();
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->getQueueStatus();
    }
}

wns::ldk::CompoundPtr
QueueProxy::getHeadOfLinePDU(wns::scheduler::ConnectionID cid)
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->getHeadOfLinePDU(cid);
    } else {
        // this case is deprecated. We support dynamicSegmentation, so why should we be asked for an arbitrary pdu?
        assure(rrhandler!=NULL,"rrhandler is invalid");
        assure(rrStorage!=NULL,"rrStorage is invalid");
        //wns::scheduler::UserID destinationUser = colleagues.registry->getUserForCID(cid);
        wns::service::dll::UnicastAddress destinationAddress
            = colleagues.registry->getPeerAddressForCID(cid);
        //= layer2->getStationManager()->getStationByNode(user)->getDLLAddress();
        uint32_t bitsPerPDU = rrhandler->getDefaultBitsPerPDU();
        wns::service::dll::FlowID flowID = cid;
        int servedBits = rrStorage->decrementRequest(flowID, bitsPerPDU);
        return rrhandler->createFakePDU(destinationAddress, servedBits, flowID);
    }
}

int
QueueProxy::getHeadOfLinePDUbits(wns::scheduler::ConnectionID cid)
{
    assure(queue!=NULL || rrhandler!=NULL,"queue==NULL && rrhandler==NULL");
    if (queue!=NULL) {
        return queue->getHeadOfLinePDUbits(cid);
    } else {
        assure(rrhandler!=NULL,"rrhandler is invalid");
        assure(rrStorage!=NULL,"rrStorage is invalid");
        uint32_t defaultBitsPerPDU = rrhandler->getDefaultBitsPerPDU(); // constant. No concrete knowledge available
        uint32_t currentBits = rrStorage->numBitsForCid(cid);
        uint32_t headOfLinePDUbits = currentBits;
        MESSAGE_SINGLE(NORMAL, logger, "getHeadOfLinePDUbits(cid="<<cid<<"): default="<<defaultBitsPerPDU<<", current="<<currentBits<<" => "<<headOfLinePDUbits<<" bits");
        return headOfLinePDUbits;
    }
}

bool
QueueProxy::isEmpty() const
{
    if (queue!=NULL) {
        return queue->isEmpty();
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->isEmpty();
    }
}

bool
QueueProxy::hasQueue(wns::scheduler::ConnectionID cid)
{
    if (queue!=NULL) {
        return queue->hasQueue(cid);
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->knowsFlow(cid);
    }
}

bool
QueueProxy::queueHasPDUs(wns::scheduler::ConnectionID cid) const
{
    if (queue!=NULL) {
        return queue->queueHasPDUs(cid);
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return (rrStorage->numCompoundsForCid(cid) > 0);
    }
}

wns::scheduler::ConnectionSet
QueueProxy::filterQueuedCids(wns::scheduler::ConnectionSet connections)
{
    if (queue!=NULL) {
        return queue->filterQueuedCids(connections);
    } else {
        assure(rrStorage!=NULL,"rrStorage is invalid");
        return rrStorage->filterActiveConnections(connections);
    }
}

QueueInterface::ProbeOutput
QueueProxy::resetAllQueues()
{
    if (queue!=NULL) {
        ProbeOutput probeOutput = queue->resetAllQueues();
        //probeOutput.bits = p.bits;
        //probeOutput.compounds = p.compounds;
        return probeOutput;
    } else {
        ProbeOutput probeOutput;
        wns::scheduler::ConnectionSet allActiveConnections = rrStorage->getActiveConnections();
        for (ConnectionSet::const_iterator iter = allActiveConnections.begin(); iter != allActiveConnections.end(); )
        {
            wns::scheduler::ConnectionID flowId = (*iter);
            int priority = colleagues.registry->getPriorityForConnection(flowId);
            writeProbe(flowId, priority);
            probeOutput.bits += numBitsForCid(flowId);
            probeOutput.compounds += numCompoundsForCid(flowId);
            rrStorage->resetFlow(flowId);
        }
        return probeOutput;
    }
}

QueueInterface::ProbeOutput
QueueProxy::resetQueues(wns::scheduler::UserID user)
{
    if (queue!=NULL) {
        ProbeOutput probeOutput = queue->resetQueues(user);
        return probeOutput;
    } else {
        ProbeOutput probeOutput;
        //probeOutput.bits = numBitsForUser(user);
        //probeOutput.compounds = numCompoundsForUser(user);
        probeOutput.bits = 0;
        probeOutput.compounds = 0;
        assure(rrStorage!=NULL,"rrStorage is invalid");
        rrStorage->resetUser(user);
        return probeOutput;
    }
}

QueueInterface::ProbeOutput
QueueProxy::resetQueue(wns::scheduler::ConnectionID cid)
{
    if (queue!=NULL) {
        ProbeOutput probeOutput = queue->resetQueue(cid);
        //probeOutput.bits = p.bits;
        //probeOutput.compounds = p.compounds;
        probeOutput.bits = 0;
        probeOutput.compounds = 0;
        return probeOutput;
    } else {
        ProbeOutput probeOutput;
        probeOutput.bits = numBitsForCid(cid);
        probeOutput.compounds = numCompoundsForCid(cid);
        assure(rrStorage!=NULL,"rrStorage is invalid");
        rrStorage->resetFlow(cid);
        return probeOutput;
    }
}

void
QueueProxy::frameStarts()
{
    if (queue!=NULL) {
        queue->frameStarts();
    }
}

std::string
QueueProxy::printAllQueues()
{
    if (queue!=NULL)
        return queue->printAllQueues();
    else if (rrhandler!=NULL && rrStorage!=NULL)
        return rrStorage->printQueueStatus();
    else
        return "ERROR";
}

bool
QueueProxy::supportsDynamicSegmentation() const
{
    MESSAGE_SINGLE(NORMAL, logger, "supportsDynamicSegmentation() = "<<(rrhandler!=NULL));
    return (rrhandler!=NULL); // true for UL Master Scheduler
}

wns::ldk::CompoundPtr
QueueProxy::getHeadOfLinePDUSegment(wns::scheduler::ConnectionID cid, int bits)
{
    if (queue!=NULL) {
        throw wns::Exception("getHeadOfLinePDUSegment() is unsupported");
        return wns::ldk::CompoundPtr();
    } else {
        assure(rrhandler!=NULL,"rrhandler is invalid");
        assure(rrStorage!=NULL,"rrStorage is invalid");
        wns::service::dll::UnicastAddress destinationAddress
            = colleagues.registry->getPeerAddressForCID(cid);
        int servedBits = rrStorage->decrementRequest(cid, bits);
        wns::service::dll::FlowID flowID = cid;
        MESSAGE_SINGLE(NORMAL, logger, "getHeadOfLinePDUSegment(cid="<<cid<<",bits="<<bits<<"): "<<servedBits<<" bits");
        //return rrhandler->createFakePDU(destinationAddress, servedBits, flowID);
        // Try to send NULL (empty compound):
        return wns::ldk::CompoundPtr();
    }
}

int
QueueProxy::getMinimumSegmentSize() const
{
    if (queue!=NULL) {
        throw wns::Exception("getMinimumSegmentSize() is unsupported");
        return 0;
    } else {
        assure(rrhandler!=NULL,"rrhandler is invalid");
        return rrhandler->getDefaultBitsPerPDU();;
    }
}

std::queue<wns::ldk::CompoundPtr> 
QueueProxy::getQueueCopy(wns::scheduler::ConnectionID cid)
{ 
    wns::Exception("You should not call getQueueCopy of the QueueProxy."); 
}

