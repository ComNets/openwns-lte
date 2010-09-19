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

#ifndef LTE_MACR_SCORER_HPP
#define LTE_MACR_SCORER_HPP

#include <LTE/macr/ScorerInterface.hpp>

#include <DLL/services/control/Association.hpp>
#include <WNS/service/dll/Address.hpp>

namespace lte { namespace macr {

/** @brief Implements ScorerInterface to support the MACg scheduling (mode selection) */
class Scorer :
	virtual public ScorerInterface
{
public:
    Scorer(const std::string& _modeName, const wns::pyconfig::View& config);

    virtual ~Scorer();

    /** @brief return a route (including) the score for this compound to support
     * the MACg scheduling process */
    virtual lte::helper::Route
    score(const wns::ldk::CompoundPtr& compound);

    void
    setRoute(const wns::service::dll::UnicastAddress& source,
	     const wns::service::dll::UnicastAddress& receivedFrom);

    void
    deleteRoute(const wns::service::dll::UnicastAddress& source);
    
    bool
    hasRoute(const wns::service::dll::UnicastAddress& source);

    wns::service::dll::UnicastAddress
    getRoute(const wns::service::dll::UnicastAddress& source);

    void
    setAssociationService(dll::services::control::Association* service);

    void
    setRLC(wns::ldk::CommandReaderInterface* _rlcReader);

private:
    /** @brief the SeenTable is a dataStructure to support Layer2-Routing in
     * the Multihop case. It relates the DLL address of a packet's original
     * sender to the DLL address of the sender we actually received it from.
     */
    typedef std::map<wns::service::dll::UnicastAddress,
		     wns::service::dll::UnicastAddress> SeenTable;

    /** @brief map of source DLL Addresses to lte::main::Layer2
     * pointers, used for "Routing"
     */
    SeenTable seen;
    dll::services::control::Association* associationService;
    std::string modeName;
    wns::logger::Logger logger;
    wns::ldk::CommandReaderInterface* rlcReader;
};

}}

#endif // NOT defined LTE_MACR_SCORER_HPP


