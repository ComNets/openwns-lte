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

#ifndef LTE_CONTROLPLANE_FLOWHANDLERSHORTCUT_HPP
#define LTE_CONTROLPLANE_FLOWHANDLERSHORTCUT_HPP

#include <LTE/helper/HasModeName.hpp>
#include <LTE/macg/MACg.hpp>
#include <DLL/Layer2.hpp>

#include <WNS/ldk/ShortcutFU.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>

namespace lte { namespace controlplane {

class FlowHandlerShortcut:
    public wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, FlowHandlerShortcut*>,
    public wns::ldk::CommandTypeSpecifier<>,
    public wns::ldk::HasReceptor<>,
    public wns::ldk::HasConnector<>,
    public wns::ldk::HasDeliverer<>,
    public lte::helper::HasModeName
{
public:
    FlowHandlerShortcut(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

    virtual
    ~FlowHandlerShortcut();

    virtual void
    onFUNCreated();

protected:
    wns::ldk::fun::FUN* fun_;

    dll::ILayer2* layer2_;

    struct Friends {
        Friends() {macg=NULL;};
        lte::macg::MACg* macg;
    } friends;
};

class FlowHandlerShortcutUT:
    public FlowHandlerShortcut,
    public wns::Cloneable<FlowHandlerShortcutUT>
{
public:
    FlowHandlerShortcutUT(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

    virtual
    ~FlowHandlerShortcutUT();

    /**
     * @brief Only receivers will be considered when sending data. Tell me, are you a receiver?
     */
    virtual bool
    isReceiver();

    /**
     * @brief If you are a receiver, what is your address?
     */
    virtual wns::service::dll::UnicastAddress
    getSourceAddress();

    /**
     * @brief To whom should I send this compound?
     */
    virtual wns::service::dll::UnicastAddress
    getDestinationAddress(const wns::ldk::CompoundPtr&);

   /**
    * @brief Shall I send this compound to every receiver?
    */
    virtual bool
    isBroadcast(const wns::ldk::CompoundPtr&);
};

class FlowHandlerShortcutBS:
    public FlowHandlerShortcut,
    public wns::Cloneable<FlowHandlerShortcutBS>
{
public:
    FlowHandlerShortcutBS(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

    virtual
    ~FlowHandlerShortcutBS();

    /**
     * @brief Only receivers will be considered when sending data. Tell me, are you a receiver?
     */
    virtual bool
    isReceiver();

    /**
     * @brief If you are a receiver, what is your address?
     */
    virtual wns::service::dll::UnicastAddress
    getSourceAddress();

    /**
     * @brief To whom should I send this compound?
     */
    virtual wns::service::dll::UnicastAddress
    getDestinationAddress(const wns::ldk::CompoundPtr&);

   /**
    * @brief Shall I send this compound to every receiver?
    */
    virtual bool
    isBroadcast(const wns::ldk::CompoundPtr&);
};

} // controlplane
} // lte

#endif // LTE_CONTROLPLANE_FLOWHANDLERSHORTCUT_HPP
