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

#include <LTE/macr/Scorer.hpp>
#include <LTE/rlc/RLCCommand.hpp>

#include <WNS/ldk/CommandReaderInterface.hpp>

using namespace lte::macr;

Scorer::Scorer(const std::string& _modeName, const wns::pyconfig::View& config) :
	seen(),
	associationService(NULL),
	modeName(_modeName),
	logger(config.get("logger")),
	rlcReader(NULL)
{}

Scorer::~Scorer()
{}

lte::helper::Route
Scorer::score(const wns::ldk::CompoundPtr& compound)
{
	assure(associationService,"AssociationService has not been set in Scorer!");
	assure(rlcReader,"RLC command Reader has not been set in Scorer!");

	lte::rlc::RLCCommand* rlcCommand = rlcReader->readCommand<lte::rlc::RLCCommand>(compound->getCommandPool());
	lte::helper::Route route;
	route.modeName = this->modeName;

	MESSAGE_BEGIN(VERBOSE, logger, m, "score ");
	m << lte::rlc::PacketDirection::toString(rlcCommand->local.direction)
	  << " destination "
	  << rlcCommand->peer.destination;
	MESSAGE_END();

	if(rlcCommand->local.direction == lte::rlc::PacketDirection::UPLINK()) {
		if (associationService->hasAssociation()){
			route.score = 1.0;
			route.target = associationService->getAssociation();


			MESSAGE_SINGLE(NORMAL, logger, "uplink Route.target is: "<< route.target);


		} else {
			route.score = -1.0;
		}
	} else if(lte::rlc::PacketDirection::DOWNLINK()) {
		if (hasRoute(rlcCommand->peer.destination)){
			route.score = 1.0;
			route.target = seen[rlcCommand->peer.destination];


			MESSAGE_SINGLE(NORMAL, logger, "downlink Route.target is: "<< route.target);



		} else {
			route.score = -1.0;
		}
	} else {
		assure(false, "compound is neither in UPLINK, nor in DOWNLINK");
	}

	if (route.valid()) {
		MESSAGE_BEGIN(VERBOSE, logger, m, "score ");
		m << " direction " << lte::rlc::PacketDirection::toString(rlcCommand->local.direction)
		  << " -> score " << route.score
		  << " mode "    << route.modeName
		  << " target DLL Address " << route.target;
		MESSAGE_END();
	}

	return route;
}

void
Scorer::setRoute(const wns::service::dll::UnicastAddress& source,
		 const wns::service::dll::UnicastAddress& receivedFrom)
{
	seen[source] = receivedFrom;
}

void
Scorer::deleteRoute(const wns::service::dll::UnicastAddress& source)
{
	assure(hasRoute(source), "Could not delete route");
	seen.erase(source);
}

bool
Scorer::hasRoute(const wns::service::dll::UnicastAddress& source)
{
	if (seen.find(source) != seen.end())
		return true;
	else
		return false;
}

wns::service::dll::UnicastAddress
Scorer::getRoute(const wns::service::dll::UnicastAddress& source)
{
	assure(hasRoute(source),"route could not found!");
	return seen[source];
}

void
Scorer::setAssociationService(dll::services::control::Association* service)
{
	associationService = service;
}

void
Scorer::setRLC(wns::ldk::CommandReaderInterface* _rlcReader)
{
	rlcReader = _rlcReader;
}
