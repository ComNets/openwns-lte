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

#include <LTE/controlplane/bch/BCHService.hpp>
#include <LTE/controlplane/associationHandler/AssociationHandlerUT.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/StationManager.hpp>

#define A2N(a) (((a).getInteger()>0) ? layer2->getStationManager()->getStationByMAC(a)->getName() : "DLL<0")

using namespace lte::controlplane;
using namespace lte::controlplane::bch;

STATIC_FACTORY_REGISTER_WITH_CREATOR(lte::controlplane::bch::BCHService,
				     wns::ldk::ControlServiceInterface,
				     "lte.controlplane.BCHService",
				     wns::ldk::CSRConfigCreator);

BCHService::BCHService(wns::ldk::ControlServiceRegistry* csr,
					   const wns::pyconfig::View& config) :
	wns::ldk::ControlService(csr),
	lte::helper::HasModeName(config),

	bchStorage(),
	criterion(config.get<std::string>("Criterion")),

	upperThreshold(config.get<double>("upperThreshold")),
	lowerThreshold(config.get<double>("lowerThreshold")),
	layer2(NULL),
	associationHandler(NULL),
	logger(config.get("logger")),
	timeout(config.get<simTimeType>("timeout")),
	triggersAssociation(config.get<bool>("triggersAssociation")),
	triggersReAssociation(config.get<bool>("triggersReAssociation"))
{
	bchStorage.reset();
    startPeriodicTimeout(timeout, 0);
}

lte::controlplane::bch::Best
BCHService::getBest() const
{
	lte::controlplane::bch::BCHRecordPtr best;

	double observedValue = 0;

	if (criterion == "SINR"){
		best = bchStorage.getBest<compare::BestSINR>();
		if (best != BCHRecordPtr())
			observedValue = best->sinr.get_dB();
	}
	else if (criterion == "RxPower"){
		best = bchStorage.getBest<compare::BestRXPWR>();
		if (best != BCHRecordPtr())
			observedValue = best->rxpower.get_dBm();
	}
	else if (criterion == "Distance"){
		best = bchStorage.getBest<compare::BestDIST>();
		if (best != BCHRecordPtr())
			observedValue = (-1) * best->distance;
	}
	else if (criterion == "Pathloss"){
		best = bchStorage.getBest<compare::BestPathloss>();
		if (best != BCHRecordPtr())
			observedValue = (-1) * best->pathloss.get_dB();
	}
	else
	{
		throw wns::Exception("Unsupported BCH Storage criterion");
		best = BCHRecordPtr();
	}

	Best retVal;
	retVal.entry = best;
	retVal.value = observedValue;
	return retVal;
}


void
BCHService::periodically()
{
	if (!layer2){
		layer2 = dynamic_cast<dll::ILayer2*>(getCSR()->getLayer());
	}
	assure(layer2!=NULL,"invalid layer2");
	if (!associationHandler){
		associationHandler = dynamic_cast<dll::ILayer2*>(getCSR()->getLayer())
		  ->getFUN()
		  ->findFriend<lte::controlplane::associationHandler::AssociationHandlerUT*>(mode+separator+ "associationHandler");
	}
	assure(associationHandler!=NULL,"invalid associationHandler");

	lte::controlplane::bch::Best bestEntry = getBest();
	lte::controlplane::bch::BCHRecordPtr best = bestEntry.entry;
	double observedValue = bestEntry.value;

	if (best != BCHRecordPtr())
	{
		MESSAGE_BEGIN(NORMAL, logger, m, "Best BCH currently received from ");
		m << A2N(best->source)
		  << " with: \n"
		  << "SINR: " << best->sinr.get_dB() << "\n"
		  << "PL: " << best->pathloss.get_dB() << "\n"
		  << "RxPwr: " << best->rxpower.get_dBm() << "\n"
		  << "Dist.: " << best->distance << "\n"
		  << "Active Criterion is '"<< criterion << "'\n";
		MESSAGE_END();

		if (triggersAssociation)
		{
			// report to associationHandler
			if (observedValue >= upperThreshold)
			{
				MESSAGE_SINGLE(NORMAL, logger, "BCHService detected " << criterion << " value above threshold (" << upperThreshold << ")");
				associationHandler->bestRAP(best->source);
			}
			else if (observedValue <= lowerThreshold)
			{
				if (bchStorage.getBCHKeys().size() != 1)
				{
					MESSAGE_SINGLE(NORMAL, logger, "BCHService detected " << criterion
							   << " value below threshold (actual=" << observedValue <<
							   ", threshold="<< lowerThreshold << ")");
					associationHandler->belowThreshold(best->source);
				}
				else
				{
					// if only one BS, do not do disassociation
					MESSAGE_SINGLE(NORMAL, logger, "BCHService detected " << criterion
								   << " value below threshold (actual=" << observedValue <<
								   ", threshold="<< lowerThreshold << ")");
					MESSAGE_SINGLE(NORMAL, logger, "Only one AP ("<<A2N(best->source)<<") is detected. Disassociation isn't needed");
					associationHandler->bestRAP(best->source);
				}
			}
			else
				MESSAGE_SINGLE(NORMAL, logger, "BCHService not triggering any action.");

			if (triggersReAssociation == false) // do this only once
			{
				MESSAGE_SINGLE(NORMAL, logger, "Stopping the triggering of further Re-Associations");
				triggersAssociation = false;
			}
		}
	}
	else
	{
		MESSAGE_BEGIN(NORMAL, logger, m, "No BCH measured yet.");
		MESSAGE_END();
	}
}

void
BCHService::storeMeasurement(const wns::service::dll::UnicastAddress& source,
                             const wns::service::phy::power::PowerMeasurementPtr&  phyMeasurementPtr, double distance, int subband)
{
	if (!layer2){
		layer2 = dynamic_cast<dll::ILayer2*>(getCSR()->getLayer());
	}
	assure(layer2!=NULL,"invalid layer2");
	BCHRecordPtr _bchRecord = BCHRecordPtr(
		new BCHRecord(source,
			      phyMeasurementPtr->getSINR(),
			      phyMeasurementPtr->getPathLoss(),
			      phyMeasurementPtr->getRxPower(),
			      distance,
                  subband));

	// store data
	bchStorage.store(source,_bchRecord);

	MESSAGE_BEGIN(NORMAL, logger, m, "measured and stored BCH from " );
	m << A2N(source) << " with:\n"
	  << "SINR: " << _bchRecord->sinr.get_dB() << "\n"
	  << "PL: "   << _bchRecord->pathloss.get_dB() << "\n"
	  << "RxPwr: " << _bchRecord->rxpower.get_dBm() << "\n"
	  << "SubBand: " << _bchRecord->subBand << "\n"
	  << "Dist.: " << _bchRecord->distance << "\n";
	MESSAGE_END();
}

void
BCHService::storeMeasurement(const wns::service::dll::UnicastAddress& source,
			     const lte::macr::PhyCommand& phyCommand)
{
	if (!layer2){
		layer2 = dynamic_cast<dll::ILayer2*>(getCSR()->getLayer());
	}
	assure(layer2!=NULL,"invalid layer2");
	wns::service::phy::power::PowerMeasurementPtr phyMeasurementPtr = phyCommand.local.rxPowerMeasurementPtr;
    this->storeMeasurement(source, phyMeasurementPtr, phyCommand.local.distance, phyCommand.local.subBand);
}

uint32_t
BCHService::getSubBand(const wns::service::dll::UnicastAddress& source) const
{
	BCHRecordPtr entry = bchStorage.get(source);

	assure(entry != BCHRecordPtr(), "You may not retrieve the subBand for an address that's unknown to you(" << source << ")" );

	return entry->subBand;
}



