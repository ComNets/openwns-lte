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

#include <LTE/macr/PhyCommand.hpp>
#include <LTE/helper/PhyMeasurementsProbe.hpp>

#include <DLL/Layer2.hpp>

#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>
#include <WNS/StaticFactory.hpp>


#include <boost/bind.hpp>


using namespace lte::helper;
STATIC_FACTORY_REGISTER_WITH_CREATOR(PhyMeasurementProbe,
				     wns::ldk::FunctionalUnit,
				     "lte.helper.PhyMeasurement",
				     wns::ldk::FUNConfigCreator);


PhyMeasurementProbe::PhyMeasurementProbe(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::CommandTypeSpecifier<>(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasDeliverer<>(),
    wns::Cloneable<PhyMeasurementProbe>(),
    wns::ldk::Processor<PhyMeasurementProbe>(),
    lte::helper::HasModeName(config),
    phyCommandReader(NULL),
    phyModeMapper(wns::service::phy::phymode::PhyModeMapperInterface::getPhyModeMapper(config.getView("phyModeMapper"))),
    sinrProbe(NULL),
    sinrEstProbe(NULL),
    sinrEstErrProbe(NULL),
    rxPwrProbe(NULL),
    interfProbe(NULL),
    modulationProbe(NULL),
    phyModeProbe(NULL),
    pathlossProbe(NULL),
    logger(config.get("logger"))
{

    dll::ILayer2* layer2 = fun->getLayer<dll::ILayer2*>();
    wns::node::Interface* node = layer2->getNode();
    wns::probe::bus::ContextProviderCollection localIDs(&node->getContextProviderCollection());

    // read the localIDs from the config
    for (int ii = 0; ii<config.len("localIDs.keys()"); ++ii)
	{
	    std::string key = config.get<std::string>("localIDs.keys()",ii);
	    uint32_t value  = config.get<uint32_t>("localIDs.values()",ii);
	    localIDs.addProvider(wns::probe::bus::contextprovider::Constant(key, value));
	    MESSAGE_SINGLE(VERBOSE, logger, "Using Local IDName '"<<key<<"' with value: "<<value);
	}

    sinrProbe        = new wns::probe::bus::ContextCollector(localIDs, "lte.SINR");
    sinrEstProbe = new wns::probe::bus::ContextCollector(localIDs, "lte.SINRest");
    sinrEstErrProbe = new wns::probe::bus::ContextCollector(localIDs, "lte.SINRestError");
    rxPwrProbe      = new wns::probe::bus::ContextCollector(localIDs, "lte.RxPower");
    txPwrProbe      = new wns::probe::bus::ContextCollector(localIDs, "lte.TxPower");
    interfProbe     = new wns::probe::bus::ContextCollector(localIDs, "lte.Interference");
    iotProbe        = new wns::probe::bus::ContextCollector(localIDs, "lte.IoT");
    modulationProbe = new wns::probe::bus::ContextCollector(localIDs, "lte.Modulation");
    phyModeProbe    = new wns::probe::bus::ContextCollector(localIDs, "lte.PhyMode");
    pathlossProbe   = new wns::probe::bus::ContextCollector(localIDs, "lte.Pathloss");
}


PhyMeasurementProbe::~PhyMeasurementProbe()
{
    delete sinrProbe; sinrProbe = NULL;
    delete sinrEstProbe; sinrEstProbe = NULL;
    delete sinrEstErrProbe; sinrEstErrProbe = NULL;
    delete rxPwrProbe; rxPwrProbe = NULL;
    delete interfProbe; interfProbe = NULL;
    delete iotProbe; iotProbe = NULL;
    delete modulationProbe; modulationProbe = NULL;
    delete phyModeProbe; phyModeProbe = NULL;
    phyCommandReader = NULL;
}

void
PhyMeasurementProbe::onFUNCreated()
{
    phyCommandReader = getFUN()->getCommandReader(modeBase+separator+"phyUser");
    assure(phyModeMapper!=NULL,"cannot get phyModeMapper");
}

void
PhyMeasurementProbe::processIncoming(const wns::ldk::CompoundPtr& compound)
{
    lte::macr::PhyCommand* phyCommand =
	phyCommandReader->readCommand<lte::macr::PhyCommand>(compound->getCommandPool());

    if (phyCommand->magic.destination == NULL) { // broadcast
	// no probing for broadcast compounds, it would be nicer though to
	// realize this through a ConnectionIDProvider ...
	return;
    }

    // TODO: why should the default be 0dB?
    // It may be misinterpreted if written to the probes.
    wns::Ratio sinrEstimation = wns::Ratio::from_dB(0.0);
    if (phyCommand->magic.estimatedSINR.I.get_mW() != 0.0) {
	sinrEstimation = phyCommand->magic.estimatedSINR.C / phyCommand->magic.estimatedSINR.I;
    }

    double sinr = phyCommand->local.rxPowerMeasurementPtr->getSINR().get_dB();
    double estimationError = sinrEstimation.get_dB() - sinr;

    sinrProbe->put(compound, sinr, boost::make_tuple("Peer.NodeID", phyCommand->magic.source->getNodeID()));
    sinrEstProbe->put(compound, sinrEstimation.get_dB());
    sinrEstErrProbe->put(compound, estimationError);
    rxPwrProbe->put(compound, phyCommand->local.rxPowerMeasurementPtr->getRxPower().get_dBm());
    txPwrProbe->put(compound, phyCommand->magic.txp.get_dBm());
    interfProbe->put(compound, phyCommand->local.rxPowerMeasurementPtr->getInterferencePower().get_dBm());
    iotProbe->put(compound, phyCommand->local.rxPowerMeasurementPtr->getIoT().get_dB());
    pathlossProbe->put(compound, phyCommand->magic.txp.get_dBm() - phyCommand->local.rxPowerMeasurementPtr->getRxPower().get_dBm());
    wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = phyCommand->local.phyModePtr;
    int phyModeIndex = phyModeMapper->getIndexForPhyMode(*phyModePtr); // O(n)

    modulationProbe->put(compound, phyModePtr->getModulation());
    phyModeProbe->put(compound, phyModeIndex);
}

