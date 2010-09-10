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

#ifndef LTE_LTEDUMMY_HPP
#define LTE_LTEDUMMY_HPP

#include <LTE/controlplane/bch/BCHUnitInterface.hpp>
/* @chen duplicated from src/controlplane/bch/BCHUnitInterface.hpp */
// #include <LTE/controlplane/bch/LTEBCHUnitInterface.hpp>
#include <LTE/timing/ResourceSchedulerInterface.hpp>
#include <LTE/controlplane/MapHandlerInterface.hpp>
#include <LTE/macr/RACHInterface.hpp>

#include <LTE/macr/CQIMeasurementInterface.hpp>
#include <LTE/controlplane/AssociationsProxyInterface.hpp>
#include <LTE/controlplane/flowmanagement/FlowManagerInterface.hpp>
#include <LTE/macg/MACgInterface.hpp>
#include <LTE/rlc/RLCInterface.hpp>

namespace lte
{
	class lteDummy :
/* included in LTE/controlplane/bch/BCHUnitInterface.hpp */
        public lte::controlplane::bch::IBCHTimingTx,
//         public lte::controlplane::bch::LTEBCHUnitRAP,

/* included in LTE/timing/ResourceSchedulerInterface.hpp */
        public lte::timing::MasterScheduler,
        public lte::timing::SlaveScheduler,
/* @chen already inherited by lte::timing::MasterScheduler or lte::timing::SlaveScheduler */
//         public lte::timing::SchedulerIncoming,
        public lte::timing::SchedulerFUInterface,

/* included in LTE/controlplane/MapHandlerInterface.hpp */
        public lte::controlplane::IMapHandlerTiming,

/* included in LTE/macr/RACHInterface.hpp */
        public lte::macr::IRachTimingTx,
        public lte::macr::IRachTimingRx,

/* included in LTE/macr/CQIMeasurementInterface.hpp */
        public lte::macr::CQIMeasurement,

/* included in LTE/controlplane/AssociationsProxyInterface.hpp */
        public lte::controlplane::AssociationsProxyBS,

/* included in LTE/controlplane/flowmanagement/FlowManagerInterface.hpp */
        public lte::controlplane::flowmanagement::FlowManagerBS,

/* included in LTE/macg/MACgInterface.hpp */
        public lte::macg::MACgCommand,

/* included in LTE/rlc/RLCInterface.hpp */
        public lte::rlc::RLCCommand
	{
	public:
/* inherited from lte::controlplane::bch::IBCHTimingTx */
    virtual void
    sendBCH(simTimeType duration);

/* inherited from lte::timing::MasterScheduler
    lte::timing::SlaveScheduler
    lte::timing::SchedulerIncoming
    lte::timing::SchedulerFUInterface */

    virtual void
    startCollection(int frameNr);

    virtual void
    finishCollection(int frameNr, simTimeType _startTime);

    virtual void
    deliverReceived();

    virtual void
    resetHARQScheduledPeerRetransmissions();

    virtual void
    onNodeCreated();

/* inherited from lte::controlplane::IMapHandlerTiming */
    virtual void
    setCurrentPhase();

    virtual void
    startMapTx(simTimeType duration, std::vector<int> dlFrameNumbers, std::vector<int> ulFrameNumbers);

    virtual void
    startMapRx();

    virtual void
    resetResources(int frameNr);

/* inherited from lte::macr::IRachTimingTx
    lte::macr::IRachTimingRx */
    virtual void
    startTx(simTimeType duration);

    virtual void
    startRx(simTimeType duration);

/* inherited from lte::macr::CQIMeasurement */
    virtual void
    setMeasurementService(wns::service::Service* phy);

/* inherited from lte::controlplane::flowmanagement::FlowManagerBS */

/* inherited from lte::controlplane::AssociationsProxyBS */
    virtual void
    addRAPofREC(wns::service::dll::UnicastAddress rap);

/* inherited from lte::macg::MACgCommand */

/* inherited from lte::rlc::RLCCommand */

	};
}

#endif // NOT defined LTE_LTEDUMMY_HPP

