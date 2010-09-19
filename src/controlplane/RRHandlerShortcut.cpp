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

#include <LTE/controlplane/RRHandlerShortcut.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::RRHandlerShortcutUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.RRHandler.ShortcutUT",
                                     wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::RRHandlerShortcutBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.RRHandler.ShortcutBS",
                                     wns::ldk::FUNConfigCreator);

using namespace lte::controlplane;

RRHandlerShortcut::RRHandlerShortcut(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyco):
    wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, RRHandlerShortcut*>(fun, pyco),
    wns::ldk::CommandTypeSpecifier<>(fun),
    helper::HasModeName(pyco),
    fun_(fun),
    layer2_(NULL)
{
}

RRHandlerShortcut::~RRHandlerShortcut()
{
}

void
RRHandlerShortcut::onFUNCreated()
{
    layer2_ = fun_->getLayer<dll::ILayer2*>();

    if (mode == modeBase) {
        friends.macg = fun_->findFriend<lte::macg::MACg*>("macg");
    } else {
        // taskID is inherited from HasModeName class
        friends.macg = fun_->findFriend<lte::macg::MACg*>("macg"+separator+taskID);
    }
    wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, RRHandlerShortcut*>::onFUNCreated();
}

RRHandlerShortcutUT::RRHandlerShortcutUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyco):
    RRHandlerShortcut(fun, pyco)
{
}

RRHandlerShortcutUT::~RRHandlerShortcutUT()
{
}

bool
RRHandlerShortcutUT::isReceiver()
{
    return false;
}

wns::service::dll::UnicastAddress
RRHandlerShortcutUT::getSourceAddress()
{
    assure(layer2_, "No layer2 instance was set");
    return layer2_->getDLLAddress();
}

wns::service::dll::UnicastAddress
RRHandlerShortcutUT::getDestinationAddress(const wns::ldk::CompoundPtr& compound)
{
    lte::macg::MACgCommand* macgCommand = friends.macg->getCommand(compound->getCommandPool());

    return macgCommand->peer.dest;
}

bool
RRHandlerShortcutUT::isBroadcast(const wns::ldk::CompoundPtr&)
{
    return false;
}

RRHandlerShortcutBS::RRHandlerShortcutBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyco):
    RRHandlerShortcut(fun, pyco)
{
}

RRHandlerShortcutBS::~RRHandlerShortcutBS()
{
}

bool
RRHandlerShortcutBS::isReceiver()
{
    return true;
}

wns::service::dll::UnicastAddress
RRHandlerShortcutBS::getSourceAddress()
{
    assure(layer2_, "No layer2 instance was set");
    return layer2_->getDLLAddress();
}

wns::service::dll::UnicastAddress
RRHandlerShortcutBS::getDestinationAddress(const wns::ldk::CompoundPtr& compound)
{
    lte::macg::MACgCommand* macgCommand = friends.macg->getCommand(compound->getCommandPool());

    return macgCommand->peer.dest;
}

bool
RRHandlerShortcutBS::isBroadcast(const wns::ldk::CompoundPtr&)
{
    return false;
}
