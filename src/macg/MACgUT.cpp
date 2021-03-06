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

#include <LTE/macg/MACgUT.hpp>

#include <DLL/Layer2.hpp>

#include <WNS/ldk/IConnectorReceptacle.hpp>

using namespace lte::macg;
using namespace wns::ldk;

STATIC_FACTORY_REGISTER_WITH_CREATOR(MACgUT,
				     FunctionalUnit,
				     "lte.macg.UserTerminal",
				     FUNConfigCreator);

MACgSchedulerUT::MACgSchedulerUT() :
    MACgScheduler()
{}


bool
MACgSchedulerUT::hasAcceptor(const CompoundPtr& compound)
{
    lte::helper::Route route = getRoute(compound);

    MESSAGE_BEGIN(VERBOSE, *logger, m, "");
    m << "hasAcceptor(compound) ->";
    if(!route.valid()) {
	m << " false";
    } else {
	m << " mode " << route.modeName
	  << " target DLL Address " << route.target
	  << " accepting " << (layers.at(route.mode)->isAccepting(compound) ? "true" : "false");
    }
    MESSAGE_END();

    return (route.valid() && layers.at(route.mode)->isAccepting(compound));
} // hasAcceptor


wns::ldk::IConnectorReceptacle*
MACgSchedulerUT::getAcceptor(const CompoundPtr& compound)
{
    lte::macg::MACgCommand* macgCommand = macg->getCommand(compound->getCommandPool());

    MESSAGE_BEGIN(NORMAL, *logger, m, "");
    m << "getAcceptor(compound) ->"
      << " mode " << macgCommand->local.modeName
      << " target DLL Address " << macgCommand->peer.dest
      << " accepting " << (layers.at(macgCommand->local.modeID)->isAccepting(compound) ? "true" : "false");
    MESSAGE_END();

    return layers.at(macgCommand->local.modeID);
} // getAcceptor


MACgUT::MACgUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    MACg(fun, config),
    HasReceptor<wns::ldk::RoundRobinReceptor>(),
    HasConnector<MACgSchedulerUT>(),
    HasDeliverer<>(),
    Forwarding<MACgUT>(),
    wns::Cloneable<MACgUT>()
{
    // Resolve issues with the Connector - set pointer to myself
    getConnector()->setMACg(this);
    // and fire up its logger
    getConnector()->setLogger(config.get("logger"));
} // MACgUT


void
MACgUT::processOutgoing(const CompoundPtr& compound)
{
    MACgCommand* macgCommand = activateCommand(compound->getCommandPool());

    // set source MACg Identification
    dll::ILayer2* layer2 = getFUN()->getLayer<dll::ILayer2*>();
    //source
    macgCommand->peer.source   = layer2->getDLLAddress();
    //mode: the macr with the best value
    lte::helper::Route  bestRoute = getConnector()->getRoute(compound);
    macgCommand->local.modeID          = bestRoute.mode;
    macgCommand->local.modeName        = bestRoute.modeName;
    //destination
    macgCommand->peer.dest   = bestRoute.target;
    fun->getProxy()->commitSizes(compound->getCommandPool(), this);
} // processOutgoing



