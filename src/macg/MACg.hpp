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

#ifndef LTE_MACG_MACG_HPP
#define LTE_MACG_MACG_HPP

#include <LTE/macg/MACgCommand.hpp>
#include <LTE/main/Layer2.hpp>
#include <LTE/macr/ScorerInterface.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/Forwarding.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/logger/Logger.hpp>

namespace lte { namespace macg { namespace modeselection { class Strategy; }}}

namespace lte { namespace macg {

	class MACg;
	class Best;

	typedef std::vector<lte::macr::ScorerInterface*> ScorerContainer;
	typedef std::vector<wns::ldk::IConnectorReceptacle*> LayerContainer;

	class MACgScheduler :
	public wns::ldk::Connector
	{
	public:

	    MACgScheduler() :
		macg(NULL),
		logger(NULL){};

	    virtual ~MACgScheduler();

	    virtual unsigned long int
	    size() const;

	    virtual lte::helper::Route
	    getRoute(const wns::ldk::CompoundPtr& compound);

	    void
	    addDestination(lte::macr::ScorerInterface* scorer);

	    virtual
	    void setStrategy(lte::macg::modeselection::Strategy* strategy);
	    
	    // Link Interface
	    virtual void
	    add(wns::ldk::IConnectorReceptacle*);

	    virtual void
	    clear(){assure(false, "clear not supported.");};

            virtual const wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer
	    get() const
            {
                assure(false, "Exchange not supported.");
                return wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer();
            }

            virtual void
	    set(const wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer&)
            {
                assure(false, "set not supported.");
            }

	    /** @brief late Logger instantiation.
	     * Due to the need for a default Constructor,
	     * we can not instantiate the Logger immediately  */
	    void setLogger(const wns::pyconfig::View& config);

	protected:
	    // coupling the MACg with the MACgScheduler(below)
	    void
	    setMACg(MACg* _macg);

	    // The FUs which are conneted with the connector set of MACg
	    LayerContainer layers;

	    // To manage the (several) Scorers in the different modes
	    ScorerContainer scorers;

	    MACg* macg;
	    lte::macg::modeselection::Strategy* strategy;

	    // Logger Instance
	    wns::logger::Logger* logger;
	}; // MACgScheduler

	class MACg :
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier<MACgCommand>
	{
	public:
	    MACg(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	    
	    virtual ~MACg() {}

	    virtual void
	    onFUNCreated();
	    
	    virtual wns::ldk::CommandPool*
	    createReply(const wns::ldk::CommandPool* original) const;

	protected:
	    wns::ldk::fun::FUN* fun;

	private:
	    wns::pyconfig::View config;
	    wns::logger::Logger logger;
	}; // MACg
	
    }
}

#endif // LTE_MACG_MACG_HPP


