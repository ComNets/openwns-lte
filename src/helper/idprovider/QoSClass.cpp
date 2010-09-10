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

#include <LTE/helper/idprovider/QoSClass.hpp>
#include <LTE/helper/QoSClasses.hpp>
/* deleted by chen */
// #include <LTE/rlc/RLC.hpp>
/* inserted by chen */
// #include <LTE/rlc/RLCInterface.hpp>
#include <LTE/lteDummy.hpp>

#include <WNS/service/Service.hpp>
#include <WNS/ldk/CommandReaderInterface.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/isClass.hpp>


using namespace lte::helper::idprovider;

QoSClass::QoSClass(wns::ldk::fun::FUN* fun) :
    rlcCommandReader(fun->getCommandReader("rlc")),
    key("MAC.QoSClass")
{
}

QoSClass::~QoSClass()
{
}

void
QoSClass::doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const
{
    assure(compound, "Received NULL CompoundPtr");
    int qosClassInt = 0;

    if (rlcCommandReader->commandIsActivated(compound->getCommandPool())) {

/* deleted by chen */
//         rlc::RLCCommand* rlcCommand = rlcCommandReader->readCommand<rlc::RLCCommand>(compound->getCommandPool());
/* inserted by chen */
        lte::lteDummy* rlcCommand = rlcCommandReader->readCommand<lte::lteDummy>(compound->getCommandPool());

/* deleted by chen */
//         wns::service::qos::QoSClass qosClass = rlcCommand->peer.qosClass;
/* inserted by chen */
        wns::service::qos::QoSClass qosClass = rlcCommand->RLCCommand::peer.qosClass;

        // No assure here, because assure is already in lte::helper::ThroughputProbe
        // Maybe we do NOT want to sort by the QoSClass and do NOT use lte::helper::ThroughputProbe,
        // but wns::probe::bus::Window/Packet, then QoSClass may not be set
        if (qosClass!=lte::helper::QoSClasses::UNDEFINED())
        {
            c.insertInt(this->key, qosClass);
        }
    }
}
