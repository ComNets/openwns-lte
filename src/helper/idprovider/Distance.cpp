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

#include <LTE/helper/idprovider/Distance.hpp>

#include <DLL/StationManager.hpp>

#include <WNS/service/Service.hpp>
#include <WNS/ldk/CommandReaderInterface.hpp>
#include <WNS/ldk/Compound.hpp>
#include <LTE/macr/PhyCommand.hpp>


using namespace lte::helper::idprovider;

Distance::Distance(wns::ldk::fun::FUN* fun, wns::ldk::CommandReaderInterface* _cmdReader) :
	cmdReader(_cmdReader),
	key("MAC.Distance")
{}


Distance::~Distance()
{
	cmdReader = NULL;
}

void
Distance::doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const
{
    assure(compound, "Received NULL CompoundPtr");

    if (cmdReader->commandIsActivated(compound->getCommandPool())) 
    {

        macr::PhyCommand* phyCommand = cmdReader->readCommand<macr::PhyCommand>(compound->getCommandPool());
    	double distance = phyCommand->local.rxPowerMeasurementPtr->getDistance();

    	assure(distance > 0, "PANIC: negative distance (" << distance << ") in lte::helper::idprovider::Distance");

    	c.insertInt( this->key, int(distance) );
    }
}


