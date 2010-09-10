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

#ifndef LTE_HELPER_QOSCLASSES_HPP
#define LTE_HELPER_QOSCLASSES_HPP

#include <string>
#include <WNS/Enum.hpp>
#include <WNS/service/qos/QoSClasses.hpp>

namespace lte {
    namespace helper {
        ENUM_BEGIN(QoSClasses);
        ENUM(UNDEFINED, 0);     // should not occur? Or map to worst priority?
        ENUM(PBCH, 1);          // Physical Broadcast Channel for BCH
        ENUM(PHICH, 2);         // Physical HARQ Indicator Channel for HARQ
        ENUM(PCCH, 3);          // Physical Control Channel for UL resource requests and CQI reporting
        ENUM(DCCH, 4);          // Dedicated Control Channel for RRC control signalling like admission control
        ENUM(CONVERSATIONAL, 5);// VoIP, video telephony
        ENUM(STREAMING, 6);     // video or audio streaming
        ENUM(INTERACTIVE, 7);   // WWW
        ENUM(BACKGROUND, 8);    // file transfer, e-mail
        ENUM_END();

        // lte::helper::QoSClasses::UNDEFINED() // use this to get a QoSClass

        // Usage:
        // lte::helper::QoSClasses::toString(qosClass);
        // lte::helper::QoSClasses::fromString("BACKGROUND")
        // lte::helper::QoSClasses::CONVERSATIONAL()

        // TODO: "UMTS Bearer Service Attributes"
        // http://www.umtsworld.com/technology/qos.htm

        /** @brief define stream operator for class QoSClass */
        /*       inline std::ostream&
        operator<< (std::ostream& s, const wns::service::qos::QoSClass& qosClass)
        {
            return s << lte::helper::QoSClasses::toString(qosClass);
            }*/
    }
}

#endif // LTE_HELPER_QOSCLASSES_HPP
