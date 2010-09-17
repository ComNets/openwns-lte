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

#include <LTE/macg/MACg.hpp>
#include <LTE/main/Layer2.hpp>
#include <LTE/macg/modeselection/Strategy.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <sstream>

using namespace lte::macg;
using namespace wns::ldk;

MACgScheduler::~MACgScheduler()
{
    delete strategy;
    delete logger;
}

unsigned long int
MACgScheduler::size() const
{
	return layers.size();
} // size

void
MACgScheduler::setMACg(MACg* _macg)
{
    macg = _macg;
}

void
MACgScheduler::setStrategy(lte::macg::modeselection::Strategy* _strategy)
{
    strategy = _strategy;
}

void
MACgScheduler::add(wns::ldk::IConnectorReceptacle* it)
{
    layers.push_back(it);
}

void
MACgScheduler::addDestination(lte::macr::ScorerInterface* scorer)
{
    scorers.push_back(scorer);
}

lte::helper::Route
MACgScheduler::getRoute(const wns::ldk::CompoundPtr& compound)
{
    lte::helper::Route route = strategy->getRoute(compound, layers, scorers);
    return  route;
}

void
MACgScheduler::setLogger(const wns::pyconfig::View& config)
{
    assure(!logger, "MACgScheduler: Logger already set!");
    logger = new wns::logger::Logger(config);
}

MACg::MACg(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& _config) :
    CommandTypeSpecifier<MACgCommand>(_fun),
    fun(_fun),
    config(_config),
    logger(_config.get("logger"))
{}

void
MACg::onFUNCreated()
{
    // I can find all my scorer friends from a dict in my config
    int numModes = config.len("modes");
    std::string separator = config.get<std::string>("separator");
    std::string taskName = config.get<std::string>("task");
    for(int i=0; i<numModes; i++)
    {
	wns::pyconfig::View modeView = config.getView("modes",i);

	std::string modeName   = modeView.get<std::string>("modeName");
	std::string suffixName = modeView.get<std::string>("scorerSuffix");

	std::ostringstream scorerName;
	scorerName << modeName;
	if (taskName!="")
	    scorerName << separator << taskName;
	scorerName << separator << suffixName;

	lte::macr::ScorerInterface* scorer =
	    fun->findFriend<lte::macr::ScorerInterface*>(scorerName.str());

	assureType(getConnector(), lte::macg::MACgScheduler*);
	//add them to the container "macrs"
	static_cast<lte::macg::MACgScheduler*>(getConnector())->addDestination(scorer);
    }
    std::string plugin  = config.get<std::string>("strategy.__plugin__");
    modeselection::StrategyCreator* creator = modeselection::StrategyFactory::creator(plugin);
    modeselection::Strategy* strategy = creator->create();

    assureType(getConnector(), lte::macg::MACgScheduler*);
    static_cast<lte::macg::MACgScheduler*>(getConnector())->setStrategy(strategy);
} // onFUNCreated


wns::ldk::CommandPool*
MACg::createReply(const wns::ldk::CommandPool* original) const
{
    wns::ldk::CommandPool* reply = this->fun->createCommandPool();

    MACgCommand* originalCommand = getCommand(original);
    MACgCommand* replyCommand = activateCommand(reply);

    // set source MACg Identification
    assureType(this->fun->getLayer(), lte::main::Layer2*);
    lte::main::Layer2* layer2 = dynamic_cast<lte::main::Layer2*>(this->fun->getLayer());
    // set source
    replyCommand->peer.source = layer2->getDLLAddress();
    // copy dest from original source
    replyCommand->peer.dest = originalCommand->peer.source;
    // copy mode
    replyCommand->local.modeID  = originalCommand->local.modeID;

    return reply;
} // createReply


