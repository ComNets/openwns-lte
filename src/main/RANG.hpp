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

#ifndef LTE_MAIN_RANG_HPP
#define LTE_MAIN_RANG_HPP

#include <LTE/main/IRANG.hpp>
#include <LTE/main/Layer2.hpp>
#include <LTE/helper/TransactionID.hpp>

#include <DLL/RANG.hpp>
#include <DLL/Layer2.hpp>
#include <DLL/StationManager.hpp>

#include <WNS/service/dll/FlowID.hpp>
#include <WNS/ldk/fun/Main.hpp>
#include <WNS/module/Base.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/node/component/Component.hpp>
#include <WNS/events/PeriodicTimeout.hpp>
#include <WNS/container/Registry.hpp>
#include <WNS/container/Pool.hpp>

namespace lte {
    namespace upperconvergence{
	class ENBUpperConvergence;
    }
    namespace main {

	class RANG :
	    public dll::RANG,
	    public rang::IFlowManagement
	{
	public:
	    RANG(wns::node::Interface*, const wns::pyconfig::View&);
	    
	    virtual ~RANG(){};

	    /** @name wns::service::dll::DataTransmission service */
	    //@{
	    virtual void
	    sendData(
		     const wns::service::dll::UnicastAddress& _peer,
		     const wns::osi::PDUPtr& _data,
		     wns::service::dll::protocolNumber protocol,
		     wns::service::dll::FlowID _dllFlowID);

	    wns::service::dll::FlowID
	    onFlowRequest(lte::helper::TransactionID _transactionId,
			  lte::upperconvergence::ENBUpperConvergence* _bsUpperConvergence,
			  wns::service::dll::UnicastAddress utAddress);

	    void
	    onFlowRelease(wns::service::dll::FlowID flowID);

	    void
	    registerIRuleControl(wns::service::dll::IRuleControl*);

	    void
	    deleteFlow(wns::service::dll::FlowID flowID);

	private:
	    typedef wns::container::Registry<wns::service::dll::FlowID, wns::service::dll::UnicastAddress> FlowIDTable;

	    wns::pyconfig::View config;

	    wns::logger::Logger logger;
	    
	    wns::container::Pool<int> flowIDPool;

	    wns::service::dll::IRuleControl* ruleControl;

	    // saves FlowIDs to Basestation mapping
	    wns::container::Registry<wns::service::dll::FlowID, lte::upperconvergence::ENBUpperConvergence*> FlowIDToBS;

	    FlowIDTable flowIDForUT;
	};
  }
}

#endif // LTE_MAIN_RANG_HPP
