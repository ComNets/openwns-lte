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

#include <LTE/upperconvergence/UE.hpp>

#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>
#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/helper/Keys.hpp>

#ifndef WNS_NO_LOGGING
#include <DLL/StationManager.hpp>
#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()
#endif

using namespace lte::upperconvergence;

STATIC_FACTORY_REGISTER_WITH_CREATOR(UEUpperConvergence,
				     wns::ldk::FunctionalUnit,
                                     "lte.UEUpperConvergence",
				     wns::ldk::FUNConfigCreator);


UEUpperConvergence::UEUpperConvergence(wns::ldk::fun::FUN* _fun,
				       const wns::pyconfig::View& config) :
  dll::UTUpperConvergence(_fun, config),
  fun(_fun),
  rlcReader(NULL),
  layer2(NULL),
  flowManager(NULL),
  tlFlowHandler(NULL)
{
    assure(fun, "FUN not set");
}

void
UEUpperConvergence::onFUNCreated()
{
    layer2 = fun->getLayer<dll::Layer2*>();
    assure(layer2, "Layer2 not set");
    /**
     * @todo dbn: lterelease: enable flow manager when available
     */
    //flowManager = layer2->getControlService<lte::controlplane::flowmanagement::IFlowManagerUE>("FlowManagerUE");

    /**
     * @todo dbn: lterelease: enable rlc reader when available
     */
    //rlcReader   = fun->getCommandReader("rlc");
    //assure(rlcReader, "RlcReader not set");
    MESSAGE_SINGLE(NORMAL, logger,"onFUNCreated(): FlowManager set to: "<< flowManager);
}

void
UEUpperConvergence::sendData(const wns::service::dll::UnicastAddress& _peer,
			     const wns::osi::PDUPtr& pdu,
			     wns::service::dll::protocolNumber protocol,
			     wns::service::dll::FlowID _dllFlowID)
{
  wns::ldk::ConstKeyPtr key(new lte::helper::key::FlowID(_dllFlowID));
  if(flowManager->isValidFlow(key))
    {
      wns::ldk::CompoundPtr compound(new wns::ldk::Compound(getFUN()->createCommandPool(), pdu));
      if (_peer.getInteger()==-1) {
	MESSAGE_SINGLE(NORMAL, logger,"Sending Compound with FlowID: "<< _dllFlowID<<" to: ?");
      } else {
	MESSAGE_SINGLE(NORMAL, logger,"Sending Compound with FlowID: "<< _dllFlowID<<" to: "<<A2N(_peer));
      }
      dll::UpperConvergence::sendData(_peer, pdu, protocol, _dllFlowID);
    } else {
    if (_peer.getInteger()==-1) {
      MESSAGE_SINGLE(NORMAL, logger,"Dropped Compound with FlowID: "<< _dllFlowID<<" to: ?, because unknown FlowID.");
    } else {
      MESSAGE_SINGLE(NORMAL, logger,"Dropped Compound with FlowID: "<< _dllFlowID<<" to: "<<A2N(_peer)<<" , because unknown FlowID.");
    }
  }
}


void
UEUpperConvergence::processIncoming(const wns::ldk::CompoundPtr& compound)
{
  //the FlowID is read out of the RLC command to be forwarded to the datahandler
  lte::rlc::RLCCommand* rlcCommand = rlcReader->readCommand<lte::rlc::RLCCommand>(compound->getCommandPool());
  assure(rlcCommand, "RlcCommand not set");
  int protocol = wns::service::dll::protocolNumberOf(compound->getData());
  
  MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
  m << "UEUpperConv::processIncoming(), forwarding packet from "<<A2N(rlcCommand->peer.source)<<" to upper Component (IP), protocol "<<protocol;
  MESSAGE_END();

  if(protocol == 1)
    {
      dataHandlerRegistry.find(wns::service::dll::protocolNumberOf(compound->getData()))->onData(compound->getData(), rlcCommand->peer.flowID);
    }
  else
    {
      dataHandlerRegistry.find(wns::service::dll::protocolNumberOf(compound->getData()))->onData(compound->getData());
    }
  
  MESSAGE_BEGIN(VERBOSE, logger, m, getFUN()->getName());
  m << ": Compound backtrace"
    << compound->dumpJourney(); // JOURNEY
  MESSAGE_END();
}

void
UEUpperConvergence::registerHandler(wns::service::dll::protocolNumber protocol,
                                    wns::service::dll::Handler* dh)
{
  assureNotNull(dh);
  dataHandlerRegistry.insert(protocol, dh);
  
  MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
  m << ": UEUpperConv registered dataHandler for protocol number " << protocol;
  MESSAGE_END();
}

void
UEUpperConvergence::registerFlowHandler(wns::service::dll::FlowHandler* flowHandler)
{
  tlFlowHandler = flowHandler;
  MESSAGE_SINGLE(NORMAL, logger, "TL-FlowHandler Registered: "<<flowHandler);
}

void
UEUpperConvergence::establishFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass)
{
  MESSAGE_SINGLE(NORMAL, logger, "FlowEstablishment called from TL for: " <<flowID);
  assure(flowManager, "FlowManager not set!");
  // if the default QoS class UNDEFINED is set in the traffic generator then change it to BACKGROUND
  if (qosClass == lte::helper::QoSClasses::UNDEFINED())
    qosClass = lte::helper::QoSClasses::BACKGROUND();

  flowManager->buildFlow(flowID,qosClass);
} // establishFlow


//after succesfully build of layer2-flow notify the TL
void
UEUpperConvergence::onFlowBuilt(wns::service::tl::FlowID _flowID,
                                wns::service::dll::FlowID _dllFlowID,
                                bool newFlow)
{
  assure(tlFlowHandler, "TransportLayerFlowHandler not set!");
  if(newFlow)
    {
      MESSAGE_SINGLE(NORMAL, logger, "New Flow built: "<<_dllFlowID);
      tlFlowHandler->onFlowEstablished(_flowID, _dllFlowID);
    }
  else
    {
      MESSAGE_SINGLE(NORMAL, logger, "Flow changed for TLFlowID: "<<_flowID);
      tlFlowHandler->onFlowChanged(_flowID, _dllFlowID);
    }
}


void
UEUpperConvergence::releaseFlow(wns::service::tl::FlowID flowID)
{
  MESSAGE_SINGLE(NORMAL, logger, "Deleting Flow for TlFlowID: " <<flowID);
  assure(flowManager, "FlowManager not set!");
  flowManager->releaseFlow(flowID);
} // releaseFlow
