/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#include <LTE/upperconvergence/eNB.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/main/IRANG.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/RANG.hpp>

#ifndef WNS_NO_LOGGING
#include <DLL/StationManager.hpp>
#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()
#endif

using namespace lte::upperconvergence;

STATIC_FACTORY_REGISTER_WITH_CREATOR(ENBUpperConvergence,
				     wns::ldk::FunctionalUnit,
                                     "lte.eNBUpperConvergence",
				     wns::ldk::FUNConfigCreator);

ENBUpperConvergence::ENBUpperConvergence(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config) :
    dll::APUpperConvergence(_fun, config),
    layer2(NULL),
    rlcReader(NULL),
    flowManagementRang(NULL)
{
}

void
ENBUpperConvergence::onFUNCreated()
{
  layer2 = getFUN()->getLayer<dll::ILayer2*>();
  rlcReader   = getFUN()->getCommandReader("rlc");
  assure(rlcReader, "rlcReader not set");
  dll::UpperConvergence::onFUNCreated();
}

void
ENBUpperConvergence::registerHandler(wns::service::dll::protocolNumber proto,
				    wns::service::dll::Handler* dh)
{
  dll::APUpperConvergence::registerHandler(proto, dh);

  assureType(dh, lte::main::rang::IFlowManagement*);

  flowManagementRang = dynamic_cast<lte::main::rang::IFlowManagement*>(dh);
  assure(flowManagementRang, "ENBUpperConvergence failed to register lte::main::rang::IFlowManagment");
}

void
ENBUpperConvergence::sendData(
    const wns::service::dll::UnicastAddress& _peer,
    const wns::osi::PDUPtr& pdu,
    wns::service::dll::protocolNumber protocol,
    wns::service::dll::FlowID _dllFlowID)
{
    // received compound from RANG
    // _dllFlowID is valid between RANG and ENB
    MESSAGE_SINGLE(NORMAL, logger,"Sending Compound with RANG-DLL-FlowID="<< _dllFlowID<<" to: "<<A2N(_peer));
    dll::UpperConvergence::sendData(_peer, pdu, protocol, _dllFlowID);
}

void
ENBUpperConvergence::processIncoming(const wns::ldk::CompoundPtr& compound)
{
    // the FlowID is read out of the RLC command to be forwarded to the datahandler
    lte::rlc::RLCCommand* rlcCommand = rlcReader->readCommand<lte::rlc::RLCCommand>(compound->getCommandPool());
    assure(rlcCommand, "RlcCommand not set");

    MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
    m << ": doOnData(), forwarding packet from "<<A2N(rlcCommand->peer.source)<<" to RANG";
    MESSAGE_END();

    assure(dataHandler, "no data handler set");
    // as opposed to the UT upper convergence, we have to tell the RANG who we
    // are and where the Packet comes from.
    dll::UpperCommand* myCommand = getCommand(compound->getCommandPool());
    dataHandler->onData(compound->getData(),
                        myCommand->peer.sourceMACAddress,
                        this,
                        rlcCommand->rang.flowID);

    MESSAGE_BEGIN(VERBOSE, logger, m, getFUN()->getName());
    m << ": Compound backtrace"
      << compound->dumpJourney(); // JOURNEY
    MESSAGE_END();
}

// calls RANG to give a FlowID for the transactioinID
wns::service::dll::FlowID
ENBUpperConvergence::requestFlow(lte::helper::TransactionID _transactionId,
                                wns::service::dll::UnicastAddress utAddress)
{
    assure(flowManagementRang, "Flow management for RANG not set!");

    MESSAGE_SINGLE(NORMAL, logger, "requestFlow("<<A2N(utAddress)<<")");

    return flowManagementRang->onFlowRequest(_transactionId, this, utAddress);
}

void
ENBUpperConvergence::releaseFlow(wns::service::dll::FlowID flowID)
{
    assure(flowManagementRang, "Flow management for RANG not set!");

    flowManagementRang->onFlowRelease(flowID);
}

void
ENBUpperConvergence::deleteFlow(wns::service::dll::FlowID flowID)
{
    assure(flowManagementRang, "Flow management for RANG not set!");

    flowManagementRang->deleteFlow(flowID);
}
