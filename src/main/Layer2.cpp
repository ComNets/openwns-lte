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

#include <DLL/StationManager.hpp>

#include <LTE/main/Layer2.hpp>
#include <LTE/timing/TimingScheduler.hpp>
#include <LTE/helper/idprovider/HopCount.hpp>
#include <LTE/helper/idprovider/Distance.hpp>
#include <LTE/helper/idprovider/QoSClass.hpp>
#include <LTE/helper/idprovider/PeerId.hpp>
#include <LTE/macr/PhyUser.hpp>
/* deleted by chen */
// #include <LTE/timing/ResourceSchedulerInterface.hpp>

/* deleted by chen */
// #include <LTE/helper/Exception.hpp>
// #include <LTE/macr/CQIMeasurement.hpp>
// #include <LTE/controlplane/AssociationsProxy.hpp>
// #include <LTE/controlplane/flowmanagement/FlowManager.hpp>
/* inserted by chen */
#include <LTE/macr/Mode.hpp>
// #include <LTE/macr/CQIMeasurementInterface.hpp>
// #include <LTE/controlplane/AssociationsProxyInterface.hpp>
// #include <LTE/controlplane/flowmanagement/FlowManagerInterface.hpp>
#include <LTE/lteDummy.hpp>

#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/ldk/utils.hpp>
#include <WNS/pyconfig/Parser.hpp>
/* inserted by chen */
#include <WNS/service/phy/ofdma/Measurements.hpp>

#include <DLL/services/control/Association.hpp>

#include <sstream>

using namespace lte;
using namespace lte::main;

STATIC_FACTORY_REGISTER_WITH_CREATOR(Layer2,
				     wns::node::component::Interface,
				     "lte.Layer2",
				     wns::node::component::ConfigCreator);

Layer2::Layer2(wns::node::Interface* _node, const wns::pyconfig::View& _config) :
  dll::Layer2(_node, _config, NULL),
  tasks(),
  flowManagerOfMasterBS(NULL)
{
  // read tasks
  if (!config.isNone("tasks"))
    {
      for (int rr=0; rr<config.len("tasks"); ++rr){
	std::string taskName =
	  config.get<std::string>("tasks",rr);
	tasks.push_back(taskName);
      }
    }
} // Layer2

void
Layer2::doStartup()
{
  dll::Layer2::doStartup();
  // Trigger all FUs in the FUN to perform their dependency resolution with
  // other FUs and related tasks
  fun->onFUNCreated();

  // register HopCount, QoSClass and PeerId IDProvider
  getNode()->getContextProviderCollection().addProvider(lte::helper::idprovider::HopCount(fun));
  getNode()->getContextProviderCollection().addProvider(lte::helper::idprovider::QoSClass(fun));
  getNode()->getContextProviderCollection().addProvider(lte::helper::idprovider::PeerId(fun));

  MESSAGE_BEGIN(NORMAL, logger, m, "Done Creating Layer2 component in ");
  m << this->getNode()->getName()
    << ". MAC-Address is: "
    << address
    << ". Tasks are: [";
  for (unsigned int rr=0; rr<tasks.size(); ++rr)
    m  << tasks.at(rr) << ", ";
  m << "]";
  MESSAGE_END();
}

Layer2::~Layer2()
{} // ~Layer2

void
Layer2::onNodeCreated()
{
  MESSAGE_SINGLE(NORMAL, logger, "Layer2::onNodeCreated()");

  // register IDProvider for per-compound distance Calculation
  getNode()->getContextProviderCollection().
    addProvider(lte::helper::idprovider::Distance(fun,getStationManager()));

  // Initialize management and control services
  getMSR()->onMSRCreated();
  getCSR()->onCSRCreated();

  // Interface to the PHY layer below.
  // we need phyNotification for constructing the upstack connection (from phy
  // to l2)
  // We have mode specific service names configured in the PyConfig.
  // for each service there is a dict. The dict key is the mode and there must
  // be one dict entry per mode.

  // First obtain the dicts with the PHY service names.
  wns::pyconfig::View phyDataTransView(config, "phyDataTransmission");
  wns::pyconfig::View phyNotifyView(config, "phyNotification");
  wns::pyconfig::View phyMeasurementsView(config, "phyMeasurements");
  // Check whether the dicts are complete (i.e., contain equal number of modes.)
  int numModes  = phyDataTransView.get<int>("__len__()");
  int numModes2 = phyNotifyView.get<int>("__len__()");
  int numModes3 = phyMeasurementsView.get<int>("__len__()");
  assure(numModes == numModes2, "unequal size of phyDataTransmission and phyNotification service dicts");
  assure(numModes == numModes3, "unequal size of phyDataTransmission and phyMeasurements service dicts");

  for(int i=0; i<numModes; i++) // forall modes
    {
      // obtain mode name from keys() list in the dict
      std::string modeName = phyDataTransView.get<std::string>("keys()", i);
      // Safety check: make sure that the ordering [i] is the same for both Dicts
      assure( modeName == phyNotifyView.get<std::string>("keys()", i),
	      "mode name mismatch between phyDataTransmission and phyNotification service dicts" );
      assure( modeName == phyMeasurementsView.get<std::string>("keys()", i),
	      "mode name mismatch between phyDataTransmission and phyMeasurements service dicts" );

      // obtain PHY service names from values() list in the dicts
      std::string transServiceName       = phyDataTransView.get<std::string>("values()", i);
      std::string notifyServiceName      = phyNotifyView.get<std::string>("values()", i);
      std::string measurementServiceName = phyMeasurementsView.get<std::string>("values()", i);

      MESSAGE_SINGLE(NORMAL, logger, "onNodeCreated(): setting physical layer for mode " << modeName);

      // obtain pointer to mode-specific PhyUser in the FUN
      lte::macr::PhyUser* phyUser =
	getFUN()->findFriend<lte::macr::PhyUser*>(modeName+"_phyUser");
      // identified by Python member phyUser.functionalUnitName

      // Set services in PhyUser to establish the connection of my FUN with lower layer
      // obtain and set DataTransmission Service
      wns::service::phy::ofdma::DataTransmission* phyDataTx =
	getService<wns::service::phy::ofdma::DataTransmission*>( transServiceName );
      phyUser->setDataTransmissionService( phyDataTx );
      // obtain and set Notification Service
      wns::service::phy::ofdma::Notification* phyNotification =
	getService<wns::service::phy::ofdma::Notification*>( notifyServiceName );
      phyUser->setNotificationService( phyNotification );

      // Inform PhyUser about our MAC Address
      phyUser->setMACAddress(address);
      // Inform PhyUser about our Mobility Service
      phyUser->setMobility( getNode()->getService<wns::PositionableInterface*>("mobility") );

      // obtain and set Measurements Service
      wns::service::phy::ofdma::Measurements* phyMeasurements =
	getService<wns::service::phy::ofdma::Measurements*>( measurementServiceName );

/* deleted by chen */
//       lte::macr::CQIMeasurement* fuCQIMeasurement = NULL;
/* inserted by chen */
      lte::lteDummy* fuCQIMeasurement = NULL;

      // connect lower layer Measurements service to FU CQIMeasurement [rs]
      if (getFUN()->knowsFunctionalUnit(modeName+"_CQIMeasurement")) { // BS or UT

/* deleted by chen */
// 	fuCQIMeasurement = getFUN()->findFriend<lte::macr::CQIMeasurement*>(modeName+"_CQIMeasurement");
/* inserted by chen */
    fuCQIMeasurement = getFUN()->findFriend<lte::lteDummy*>(modeName+"_CQIMeasurement");

	fuCQIMeasurement->setMeasurementService( phyMeasurements );
      } else { // in a RN both tasks (BS+UT) have their own CQIMeasurement
	if (getFUN()->knowsFunctionalUnit(modeName+"_BS_CQIMeasurement")) { // 'ltefdd20_BS_CQIMeasurement'

/* deleted by chen */
// 	  fuCQIMeasurement = getFUN()->findFriend<lte::macr::CQIMeasurement*>(modeName+"_BS_CQIMeasurement");
/* inserted by chen */
      fuCQIMeasurement = getFUN()->findFriend<lte::lteDummy*>(modeName+"_BS_CQIMeasurement");

	  fuCQIMeasurement->setMeasurementService( phyMeasurements );
	}
	if (getFUN()->knowsFunctionalUnit(modeName+"_UT_CQIMeasurement")) { // 'ltefdd20_UT_CQIMeasurement'

/* deleted by chen */
// 	  fuCQIMeasurement = getFUN()->findFriend<lte::macr::CQIMeasurement*>(modeName+"_UT_CQIMeasurement");
/* inserted by chen */
      fuCQIMeasurement = getFUN()->findFriend<lte::lteDummy*>(modeName+"_UT_CQIMeasurement");

	  fuCQIMeasurement->setMeasurementService( phyMeasurements );
	}
      }
      if (fuCQIMeasurement == NULL) {
	MESSAGE_SINGLE(NORMAL, logger, "CQIMeasurement FU not found");
      }

      // dependency resolution in TX and RX ResourceScheduler
      if (tasks.empty())
	{

/* deleted by chen */
// 	  lte::timing::SchedulerFUInterface* rstx =
// 	    getFUN()->findFriend<lte::timing::SchedulerFUInterface*>(modeName+"_resourceSchedulerTX");
/* inserted by chen */
      lte::lteDummy* rstx =
        getFUN()->findFriend<lte::lteDummy*>(modeName+"_resourceSchedulerTX");

	  rstx->onNodeCreated();

/* deleted by chen */
// 	  lte::timing::SchedulerFUInterface* rsrx =
// 	    getFUN()->findFriend<lte::timing::SchedulerFUInterface*>(modeName+"_resourceSchedulerRX");
/* inserted by chen */
      lte::lteDummy* rsrx =
        getFUN()->findFriend<lte::lteDummy*>(modeName+"_resourceSchedulerRX");

	  rsrx->onNodeCreated();
	}
      else
	{
	  for (unsigned int rr=0; rr<tasks.size(); ++rr){ // forall tasks in a relay (BS,UT)
	    std::string taskName = tasks.at(rr);

/* deleted by chen */
// 	    lte::timing::SchedulerFUInterface* rstx =
// 	      getFUN()->findFriend<lte::timing::SchedulerFUInterface*>(modeName+"_"+taskName+"_resourceSchedulerTX");
/* inserted by chen */
        lte::lteDummy* rstx =
          getFUN()->findFriend<lte::lteDummy*>(modeName+"_"+taskName+"_resourceSchedulerTX");

	    rstx->onNodeCreated();

/* deleted by chen */
// 	    lte::timing::SchedulerFUInterface* rsrx =
// 	      getFUN()->findFriend<lte::timing::SchedulerFUInterface*>(modeName+"_"+taskName+"_resourceSchedulerRX");
/* inserted by chen */
        lte::lteDummy* rsrx =
          getFUN()->findFriend<lte::lteDummy*>(modeName+"_"+taskName+"_resourceSchedulerRX");

	    rsrx->onNodeCreated();
	  }
	}
    }
}

void
Layer2::onWorldCreated()
{
	// initial associations, if present
	if(config.knows("associations"))
	{
		for(int i = 0; i < config.len("associations"); ++i) { // forall associations
			// get route of "associations" (containing source=this,destination,mode)
			wns::pyconfig::View routeConfig = wns::pyconfig::View(config, "associations", i);
			// source
			assure(dynamic_cast<lte::main::Layer2*>(
					   stationManager->getStationByID(
						   routeConfig.get<Layer2::StationIDType>("source.stationID")))==this,
				   "Tried to handle somebody else's association!");
			// destination
			ILayer2::StationIDType destinationID =
				routeConfig.get<Layer2::StationIDType>("destination.stationID");
			dll::ILayer2* destination = stationManager->getStationByID(destinationID);
			// get mode string
			lte::macr::Mode mode = routeConfig.get<lte::macr::Mode>("mode");
			// association from this to the master
			getControlService<dll::services::control::Association>("ASSOCIATION"+mode)
				->associate(this, destination);

	if (this->getStationType() == wns::service::dll::StationTypes::FRS())
	  {

/* deleted by chen */
// 	    destination->getControlService<lte::controlplane::AssociationsProxyBS>("AssociationsProxy")
// 	      ->addRAPofREC(this->getDLLAddress());
/* inserted by chen */
        destination->getControlService<lte::lteDummy>("AssociationsProxy")
          ->addRAPofREC(this->getDLLAddress());

	    // RNs will get the DCCH FlowID magic later via this flowManagerMasterBS

/* deleted by chen */
// 	    flowManagerOfMasterBS =
// 	      destination->getControlService<lte::controlplane::flowmanagement::FlowManagerBS>("FlowManagerBS");
/* inserted by chen */
        flowManagerOfMasterBS =
          destination->getControlService<lte::lteDummy>("FlowManagerBS");

	  }// this is RN
	else
	  {
	    assure(false,"static associations for UT,BS not allowed anymore");
	  }
      }// forall associations
    } // if
  // init TimingScheduler services
  /** @todo pab: 2006-12-05 This Initialization is very ugly. A cleverer
   * scheme for dependency resolution is under discussion with msg */
  for (int ii = 0; ii<config.len("managementServices"); ++ii){
    wns::pyconfig::View managementServiceView = config.get("managementServices",ii);
    std::string serviceName = managementServiceView.get<std::string>("serviceName");
    lte::timing::TimingScheduler* ts = NULL;
    wns::ldk::ManagementServiceInterface* ms = NULL;
    ms = this->getManagementService<wns::ldk::ManagementServiceInterface>(serviceName);
    ts = dynamic_cast<lte::timing::TimingScheduler*>(ms);
    if (ts)
      ts->onWorldCreated();
  }

}


void Layer2::onShutdown()
{}

std::vector<std::string>
Layer2::getTasks() const
{
  return tasks;
}

/* deleted by chen */
// lte::controlplane::flowmanagement::FlowManagerBS*
// Layer2::getFlowManagerOfMasterBS() const
/* inserted by chen */
lte::lteDummy*
Layer2::getFlowManagerOfMasterBS() const

{
  assure(this->getStationType() == wns::service::dll::StationTypes::FRS(), "Only RNs are allowed to ask Layer2 for the FlowManagerOfMasterBS!!!");
  assure(flowManagerOfMasterBS != NULL, "FlowManagerOfMasterBS not set!");
  return flowManagerOfMasterBS;
}
