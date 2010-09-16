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
#include <LTE/timing/ResourceSchedulerInterface.hpp>

#include <LTE/macr/Mode.hpp>
#include <LTE/macr/CQIMeasurementInterface.hpp>
#include <LTE/controlplane/AssociationsProxyInterface.hpp>

#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/ldk/utils.hpp>
#include <WNS/pyconfig/Parser.hpp>

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
  dll::Layer2(_node, _config, NULL)
{
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
    << address;
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



  // First obtain the dicts with the PHY service names.
  wns::pyconfig::View phyDataTransView(config, "phyDataTransmission");
  wns::pyconfig::View phyNotifyView(config, "phyNotification");
  wns::pyconfig::View phyMeasurementsView(config, "phyMeasurements");

  // Check whether the dicts are complete (i.e., contain equal number of modes.)
  int numModes  = phyDataTransView.get<int>("__len__()");
  int numModes2 = phyNotifyView.get<int>("__len__()");

  assure(numModes == numModes2, "unequal size of phyDataTransmission and phyNotification service dicts");

  for(int i=0; i<numModes; i++) // forall modes
    {
      // obtain mode name from keys() list in the dict
      std::string modeName = phyDataTransView.get<std::string>("keys()", i);

      // Safety check: make sure that the ordering [i] is the same for both Dicts
      assure( modeName == phyNotifyView.get<std::string>("keys()", i),
	      "mode name mismatch between phyDataTransmission and phyNotification service dicts" );

      // obtain PHY service names from values() list in the dicts
      std::string transServiceName       = phyDataTransView.get<std::string>("values()", i);
      std::string notifyServiceName      = phyNotifyView.get<std::string>("values()", i);

      MESSAGE_SINGLE(NORMAL, logger, "onNodeCreated(): setting physical layer for mode " << modeName);

      // obtain pointer to mode-specific PhyUser in the FUN
      lte::macr::PhyUser* phyUser =
	getFUN()->findFriend<lte::macr::PhyUser*>(modeName+"_phyUser");

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

      /**
       * @todo dbn: lterelease: Enable dependency resolution for resource scheduler when available
       */
      // dependency resolution in TX and RX ResourceScheduler
      /*if (tasks.empty())
	{
	  winprost::timing::SchedulerFUInterface* rstx =
	    getFUN()->findFriend<winprost::timing::SchedulerFUInterface*>(modeName+"_resourceSchedulerTX");
	  rstx->onNodeCreated();
	  winprost::timing::SchedulerFUInterface* rsrx =
	    getFUN()->findFriend<winprost::timing::SchedulerFUInterface*>(modeName+"_resourceSchedulerRX");
	  rsrx->onNodeCreated();
	}
      else
	{
	  for (unsigned int rr=0; rr<tasks.size(); ++rr){ // forall tasks in a relay (BS,UT)
	    std::string taskName = tasks.at(rr);

	    winprost::timing::SchedulerFUInterface* rstx =
	      getFUN()->findFriend<winprost::timing::SchedulerFUInterface*>(modeName+"_"+taskName+"_resourceSchedulerTX");
	    rstx->onNodeCreated();
	    winprost::timing::SchedulerFUInterface* rsrx =
	      getFUN()->findFriend<winprost::timing::SchedulerFUInterface*>(modeName+"_"+taskName+"_resourceSchedulerRX");
	    rsrx->onNodeCreated();
	  }
	  }*/
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
	    destination->getControlService<lte::controlplane::AssociationsProxyBS>("AssociationsProxy")
 	      ->addRAPofREC(this->getDLLAddress());
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
