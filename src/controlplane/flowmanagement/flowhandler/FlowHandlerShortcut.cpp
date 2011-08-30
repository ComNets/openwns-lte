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

#include <LTE/controlplane/flowmanagement/flowhandler/FlowHandlerShortcut.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::FlowHandlerShortcutUT,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.FlowHandler.ShortcutUT",
                                     wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::FlowHandlerShortcutBS,
                                     wns::ldk::FunctionalUnit,
                                     "lte.controlplane.FlowHandler.ShortcutBS",
                                     wns::ldk::FUNConfigCreator);

using namespace lte::controlplane;

FlowHandlerShortcut::FlowHandlerShortcut(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyco):
    wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, FlowHandlerShortcut*>(fun, pyco),
    wns::ldk::CommandTypeSpecifier<>(fun),
    helper::HasModeName(pyco),
    fun_(fun),
    layer2_(NULL)
{
}

FlowHandlerShortcut::~FlowHandlerShortcut()
{
}

void
FlowHandlerShortcut::onFUNCreated()
{
    layer2_ = fun_->getLayer<dll::ILayer2*>();

    if (mode == modeBase) {
        friends.macg = fun_->findFriend<lte::macg::MACg*>("macg");
    } else {
        // taskID is inherited from HasModeName class
        friends.macg = fun_->findFriend<lte::macg::MACg*>("macg"+separator+taskID);
    }
    wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, FlowHandlerShortcut*>::onFUNCreated();
}

FlowHandlerShortcutUT::FlowHandlerShortcutUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyco):
    FlowHandlerShortcut(fun, pyco)
{
}

FlowHandlerShortcutUT::~FlowHandlerShortcutUT()
{
}

bool
FlowHandlerShortcutUT::isReceiver()
{
    return true;
}

wns::service::dll::UnicastAddress
FlowHandlerShortcutUT::getSourceAddress()
{
    assure(layer2_, "No layer2 instance was set");
    return layer2_->getDLLAddress();
}

wns::service::dll::UnicastAddress
FlowHandlerShortcutUT::getDestinationAddress(const wns::ldk::CompoundPtr& compound)
{
    lte::macg::MACgCommand* macgCommand = friends.macg->getCommand(compound->getCommandPool());

    return macgCommand->peer.dest;
}

bool
FlowHandlerShortcutUT::isBroadcast(const wns::ldk::CompoundPtr&)
{
    return false;
}

FlowHandlerShortcutBS::FlowHandlerShortcutBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyco):
    FlowHandlerShortcut(fun, pyco)
{
}

FlowHandlerShortcutBS::~FlowHandlerShortcutBS()
{
}

bool
FlowHandlerShortcutBS::isReceiver()
{
    return true;
}

wns::service::dll::UnicastAddress
FlowHandlerShortcutBS::getSourceAddress()
{
    assure(layer2_, "No layer2 instance was set");
    return layer2_->getDLLAddress();
}

wns::service::dll::UnicastAddress
FlowHandlerShortcutBS::getDestinationAddress(const wns::ldk::CompoundPtr& compound)
{
    lte::macg::MACgCommand* macgCommand = friends.macg->getCommand(compound->getCommandPool());

    return macgCommand->peer.dest;
}

bool
FlowHandlerShortcutBS::isBroadcast(const wns::ldk::CompoundPtr&)
{
    return false;
}
