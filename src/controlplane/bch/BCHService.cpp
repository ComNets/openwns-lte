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

#include <WNS/distribution/DiscreteUniform.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/StationManager.hpp>

#include <boost/lambda/lambda.hpp>

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
	criterion(config.get("criterion")),

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

	if (criterion.name == "SINR"){
		best = bchStorage.getBest<compare::BestSINR>();
		if (best != BCHRecordPtr())
        {
            if(criterion.ratioMargin > wns::Ratio::from_dB(0.0))
            {
                wns::Ratio lowerBoundpl = best->sinr - criterion.ratioMargin;
                wns::Ratio upperBoundpl = best->sinr;

                using namespace boost::lambda;
                boost::function<bool (wns::Ratio, wns::Ratio)> cmpr = _1 <= _2;
                boost::function<wns::Ratio (BCHRecord*)> gettersinr = (_1 ->* &BCHRecord::sinr);

                std::vector<BCHRecordPtr> r;
                r = bchStorage.getBestInRange(lowerBoundpl, upperBoundpl, gettersinr, cmpr);

                wns::distribution::DiscreteUniform rng(0, r.size() - 1);
                best = r[rng()];                
            }
			observedValue = best->sinr.get_dB();
        }
        
	}
	else if (criterion.name == "RxPower"){
		best = bchStorage.getBest<compare::BestRXPWR>();
		if (best != BCHRecordPtr())
        {
            if(criterion.powerMargin > wns::Power::from_dBm(0.0))
            {
                wns::Power lowerBoundpl = best->rxpower - criterion.powerMargin;
                wns::Power upperBoundpl = best->rxpower;

                using namespace boost::lambda;
                boost::function<bool (wns::Power, wns::Power)> cmpp = _1 <= _2;
                boost::function<wns::Power (BCHRecord*)> getterrxp = (_1 ->* &BCHRecord::rxpower);

                std::vector<BCHRecordPtr> r;
                r = bchStorage.getBestInRange(lowerBoundpl, upperBoundpl, getterrxp, cmpp);

                wns::distribution::DiscreteUniform rng(0, r.size() - 1);
                best = r[rng()];                
            }
			observedValue = best->rxpower.get_dBm();
        }
	}
	else if (criterion.name == "Distance"){
		best = bchStorage.getBest<compare::BestDIST>();
		if (best != BCHRecordPtr())
        {
            if(criterion.distanceMargin > 0.0)
            {
                double lowerBoundpl = best->distance;
                double upperBoundpl = best->distance + criterion.distanceMargin;

                using namespace boost::lambda;
                boost::function<bool (double, double)> cmpd = _1 <= _2;
                boost::function<double (BCHRecord*)> getterdist = (_1 ->* &BCHRecord::distance);

                std::vector<BCHRecordPtr> r;
                r = bchStorage.getBestInRange(lowerBoundpl, upperBoundpl, getterdist, cmpd);

                wns::distribution::DiscreteUniform rng(0, r.size() - 1);
                best = r[rng()];                
            }
			observedValue = (-1) * best->distance;
        }
	}
	else if (criterion.name == "Pathloss"){
		best = bchStorage.getBest<compare::BestPathloss>();
		if (best != BCHRecordPtr())
        {
            if(criterion.ratioMargin > wns::Ratio::from_dB(0.0))
            {
                wns::Ratio lowerBoundpl = best->pathloss;
                wns::Ratio upperBoundpl = best->pathloss + criterion.ratioMargin;

                using namespace boost::lambda;
                boost::function<bool (wns::Ratio, wns::Ratio)> cmpr = _1 <= _2;
                boost::function<wns::Ratio (BCHRecord*)> getterpl = (_1 ->* &BCHRecord::pathloss);

                std::vector<BCHRecordPtr> r;
                r = bchStorage.getBestInRange(lowerBoundpl, upperBoundpl, getterpl, cmpr);

                wns::distribution::DiscreteUniform rng(0, r.size() - 1);
                best = r[rng()];                
            }
   			observedValue = (-1) * best->pathloss.get_dB();
        }
	}
    else if (criterion.name == "MAC_ID")
    {
        best = bchStorage.get(criterion.address);
        observedValue = upperThreshold;
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
    if(!triggersAssociation)
    {
	    MESSAGE_SINGLE(NORMAL, logger, "BCHService does not trigger actions (any more).");
        return;
    }

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
		  << "Active Criterion is '"<< criterion.name << "'\n";
		MESSAGE_END();

		if (triggersAssociation)
		{
			// report to associationHandler
			if (observedValue >= upperThreshold)
			{
				MESSAGE_SINGLE(NORMAL, logger, "BCHService detected " << criterion.name << " value above threshold (" << upperThreshold << ")");
				associationHandler->bestRAP(best->source);
			}
			else if (observedValue <= lowerThreshold)
			{
				if (bchStorage.getBCHKeys().size() != 1)
				{
					MESSAGE_SINGLE(NORMAL, logger, "BCHService detected " << criterion.name
							   << " value below threshold (actual=" << observedValue <<
							   ", threshold="<< lowerThreshold << ")");
					associationHandler->belowThreshold(best->source);
				}
				else
				{
					// if only one BS, do not do disassociation
					MESSAGE_SINGLE(NORMAL, logger, "BCHService detected " << criterion.name
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



