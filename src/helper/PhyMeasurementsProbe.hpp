/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#ifndef LTE_HELPER_PHYMEASUREMENTPROBE_HPP
#define LTE_HELPER_PHYMEASUREMENTPROBE_HPP

#include <LTE/helper/HasModeName.hpp>

#include <WNS/probe/bus/ContextCollector.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/CommandReaderInterface.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>

#include <WNS/pyconfig/View.hpp>

namespace lte { namespace helper {

	class PhyMeasurementProbe :
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier<>,
	public wns::ldk::HasReceptor<>,
	public wns::ldk::HasConnector<>,
	public wns::ldk::HasDeliverer<>,
	public wns::Cloneable<PhyMeasurementProbe>,
	public wns::ldk::Processor<PhyMeasurementProbe>,
	public lte::helper::HasModeName
	{
	    /** @brief pointer to the phy command reader */
	    wns::ldk::CommandReaderInterface* phyCommandReader;
	    /** @brief mapping of ordered phyModes and SINR ranges */
	    wns::service::phy::phymode::PhyModeMapperInterface* phyModeMapper;
	    
	    wns::probe::bus::ContextCollector* sinrProbe;
	    wns::probe::bus::ContextCollector* sinrEstProbe;
	    wns::probe::bus::ContextCollector* sinrEstErrProbe;
	    wns::probe::bus::ContextCollector* rxPwrProbe;
	    wns::probe::bus::ContextCollector* txPwrProbe;
	    wns::probe::bus::ContextCollector* interfProbe;
	    wns::probe::bus::ContextCollector* iotProbe;
	    wns::probe::bus::ContextCollector* modulationProbe;
	    wns::probe::bus::ContextCollector* phyModeProbe;
	    wns::probe::bus::ContextCollector* pathlossProbe;
	    
	    wns::logger::Logger logger;

	public:

	    PhyMeasurementProbe(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	    virtual ~PhyMeasurementProbe();
	    
	    virtual void
	    onFUNCreated();

	    virtual void
	    processIncoming(const wns::ldk::CompoundPtr& compound);

	    virtual void
	    processOutgoing(const wns::ldk::CompoundPtr&){}
	    
	};


}}

#endif // LTE_HELPER_PHYMEASUREMENTPROBE_HPP
