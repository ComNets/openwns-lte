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

#include <LTE/main/RANG.hpp>
#include <LTE/upperconvergence/eNB.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/StationManager.hpp>

#include <WNS/module/Base.hpp>

using namespace lte;
using namespace lte::main;

STATIC_FACTORY_REGISTER_WITH_CREATOR(RANG,
				     wns::node::component::Interface,
				     "lte.RANG",
				     wns::node::component::ConfigCreator);

RANG::RANG(wns::node::Interface* node, const wns::pyconfig::View& _config) :
    dll::RANG(node, _config),
    config(_config),
    logger(config.get("logger")),
    flowIDPool(config.get<simTimeType>("flowIDUnbindDelay"), 1, 65535)
{
    MESSAGE_SINGLE(NORMAL, logger, "LTE::RANG created");
}

void
RANG::sendData(
	       const wns::service::dll::UnicastAddress& _peer,
	       const wns::osi::PDUPtr& pdu,
	       wns::service::dll::protocolNumber protocol,
	       wns::service::dll::FlowID _dllFlowID)
{
    MESSAGE_SINGLE(NORMAL, logger, "Outgoing PDU with FlowID: "<<_dllFlowID<<" for UT: "<<_peer);
    if(flowIDForUT.knows(_dllFlowID))
	{
	    if(flowIDForUT.find(_dllFlowID)==_peer)
		dll::RANG::sendData(_peer, pdu, protocol, _dllFlowID);
	    else
		{
		    MESSAGE_SINGLE(NORMAL, logger, "FlowID: "<<_dllFlowID<<" is not Flow of UT: "<<_peer);
		    MESSAGE_SINGLE(NORMAL, logger, "Dropping PDU.");
		}
	}
    else
	{
	    MESSAGE_SINGLE(NORMAL, logger, "FlowID: "<<_dllFlowID<<" unknown.");
	    MESSAGE_SINGLE(NORMAL, logger, "Dropping PDU.");
	}
}

wns::service::dll::FlowID
RANG::onFlowRequest(lte::helper::TransactionID _transactionId,
		    lte::upperconvergence::ENBUpperConvergence* _bsUpperConvergence,
		    wns::service::dll::UnicastAddress utAddress)
{
    // choose a FlowID
    wns::service::dll::FlowID flowID = flowIDPool.suggestPort();
    flowIDPool.bind(flowID);
    MESSAGE_SINGLE(NORMAL, logger, "FlowRequest received for UT: "<<utAddress<< " confirming new FlowID: "<<flowID);

    //insert FlowID and the apropriate BS's upperconvergence to a list
    assure(_bsUpperConvergence, "_bsUpperConvergence not set!");
    FlowIDToBS.insert(flowID, _bsUpperConvergence);

    //save FlowIDForUT:
    flowIDForUT.insert(flowID, utAddress);

    // tell the BS the chosen FlowID
    return flowID;
}

void
RANG::onFlowRelease(wns::service::dll::FlowID flowID)
{
    assure(FlowIDToBS.knows(flowID), "There is no Flow with this FlowID to release. FlowID: "<<flowID);

    FlowIDToBS.erase(flowID);
    flowIDPool.unbind(flowID);

    MESSAGE_SINGLE(NORMAL, logger, "Flow released with FlowID: "<<flowID);

    // delete flows at ip
    ruleControl->onFlowRemoved(flowID);
}

void
RANG::deleteFlow(wns::service::dll::FlowID flowID)
{
    assure(FlowIDToBS.knows(flowID), "There is no Flow with this FlowID to release. FlowID: "<<flowID);
    FlowIDToBS.erase(flowID);
    flowIDForUT.erase(flowID);
    flowIDPool.unbind(flowID);
    MESSAGE_SINGLE(NORMAL, logger, "Flow released with FlowID: "<<flowID);

    // delete flows at IPTables
    ruleControl->onFlowRemoved(flowID);
}

void
RANG::registerIRuleControl(wns::service::dll::IRuleControl* rControl)
{
    assure(rControl, "IRuleControl not set.");
    ruleControl = rControl;
}
