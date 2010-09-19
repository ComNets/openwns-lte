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

#ifndef LTE_RLC_UNACKNOWLEDGEDMODE_HPP
#define LTE_RLC_UNACKNOWLEDGEDMODE_HPP

#include <WNS/ldk/sar/SegAndConcat.hpp>
#include <DLL/Layer2.hpp>

namespace lte { namespace rlc {
    /**
     * @brief Implementation of LTE UM as described in 3GPP TS 36.322 V8.5.0
     * (2009-03) Section 5.1.2 ff
     */
    class UnacknowledgedMode:
        virtual public wns::ldk::sar::SegAndConcat
    {
    public:
        UnacknowledgedMode(wns::ldk::fun::FUN* fuNet, const wns::pyconfig::View& config);

        virtual
        ~UnacknowledgedMode();

        virtual void
        processIncoming(const wns::ldk::CompoundPtr& compound);

        virtual void
        processOutgoing(const wns::ldk::CompoundPtr&);
    private:
        dll::ILayer2* layer2;
    };    
}
}

#endif // LTE_RLC_UNACKNOWLEDGEDMODE_HPP
