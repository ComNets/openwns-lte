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

#include <LTE/controlplane/AssociationsProxy.hpp>
#include <LTE/rlc/UE.hpp>
#include <LTE/macg/MACg.hpp>

#include <LTE/controlplane/associationHandler/IAssociationHandler.hpp>
#include <LTE/helper/Keys.hpp>

#include <DLL/RANG.hpp>
#include <DLL/StationManager.hpp>
#include <WNS/simulator/Time.hpp>

#define A2N(a) layer2->getStationManager()->getStationByMAC(a)->getName()

using namespace lte::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(AssociationsProxyBS,
				     wns::ldk::ControlServiceInterface,
				     "lte.controlplane.ENBAssociationsProxy",
				     wns::ldk::CSRConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(AssociationsProxyUT,
				     wns::ldk::ControlServiceInterface,
				     "lte.controlplane.UEAssociationsProxy",
				     wns::ldk::CSRConfigCreator);

AssociationsProxy::AssociationsProxy(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
  wns::ldk::ControlService(csr),
  layer2(NULL),
  rlcReader(NULL),
  myModes(),
  logger(config.get("logger")),
  activeUsers()
{
  for (int i = 0; i < config.len("modeNames"); ++i) {
    std::string modeName = config.get<std::string>("modeNames", i);
    myModes.push_back(modeName);
  }
}

AssociationsProxy::~AssociationsProxy()
{
  layer2 = NULL;
}

void
AssociationsProxy::onCSRCreated()
{
	layer2 = dynamic_cast<dll::ILayer2*>(getCSR()->getLayer());
	rlcReader = layer2->getFUN()->getCommandReader("rlc");
}

void
AssociationsProxy::periodically()
{
  // Walk through the modes
  for(std::list<std::string>::iterator iter = myModes.begin(); iter != myModes.end(); ++iter)
    {
      ModeName mode = *iter;

      // Count connected users in this mode
      uint32_t userCounter = 0;
      for (UserInfoLookup::const_iterator user = activeUsers.begin();
	   user != activeUsers.end();
	   ++user)
	{
	  ModeInfo modeInfo = user->second;
	  if (modeInfo.mode == mode) ++userCounter;
	}
    }
}


///////////////////////////
//                       //
//  AssociationsProxyBS  //
//                       //
///////////////////////////

AssociationsProxyBS::AssociationsProxyBS(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
  AssociationsProxy(csr, config),
  flowManagerENB(NULL),
  myUpperConvergence(NULL),
  upperSynchronizer(NULL),
  myRECRAPs(),
  preservedUsers()
{
}

AssociationsProxyBS::~AssociationsProxyBS()
{}

void
AssociationsProxyBS::onCSRCreated()
{
  AssociationsProxy::onCSRCreated();

  wns::ldk::ControlServiceInterface* csi = layer2->getControlService<wns::ldk::ControlServiceInterface>("FlowManagerBS");
  flowManagerENB = dynamic_cast<lte::controlplane::flowmanagement::IFlowManagerENB*>(csi);
  assureNotNull(flowManagerENB);

  // Add my own address to the list of RAPs in my REC
  this->addRAPofREC(layer2->getDLLAddress());

  myUpperConvergence = layer2->getFUN()->findFriend<dll::APUpperConvergence*>("upperConvergence");

  upperSynchronizer = layer2->getFUN()->findFriend<wns::ldk::tools::Synchronizer*>("upperSynchronizer");

  /**
   * @todo dbn: lterelease: Enable periodic timeout when timing is used
   */
  //startPeriodicTimeout(0.5);
}

lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs
AssociationsProxyBS::associatedPerMode(wns::service::dll::UnicastAddress userAdr,
				       wns::service::dll::UnicastAddress rapAdr,
				       ModeName mode)
{
  assure(!hasAssociationTo(userAdr), "user has already associated!");

  // store user information
  ModeInfo userModeInfo;
  userModeInfo.mode = mode;
  userModeInfo.rapAdr = rapAdr;
  activeUsers.insert(std::pair<wns::service::dll::UnicastAddress, ModeInfo>(userAdr, userModeInfo));

  assure(hasAssociatedPerMode(userAdr, mode), "Store user information not correctly!");

  MESSAGE_BEGIN(NORMAL, logger, m, "Added user=");
  m << A2N(userAdr) << " for mode: " << userModeInfo.mode << " (RAP=" << A2N(userModeInfo.rapAdr) << ").";
  MESSAGE_END();

  if (hasPreserved(userAdr))
    {
      // flow gates keep close for the flow until Flow_Rebuild completed

      // remove user from preservedUsers
      std::set<wns::service::dll::UnicastAddress>::iterator foundPreserve = preservedUsers.find(userAdr);
      preservedUsers.erase(foundPreserve);
      assure(!hasPreserved(userAdr), "remove preserved user not correctly!");
    }

  // update downlink route in RANG
  assure(!(myUpperConvergence->getRANG()->knowsAddress(userAdr)), "User is already known in RANG!");
  myUpperConvergence->getRANG()->updateAPLookUp(userAdr, myUpperConvergence);

  MESSAGE_BEGIN(NORMAL, logger, m, "Added user=");
  m << A2N(userAdr) << " to RANG.";
  MESSAGE_END();
  lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs controlPlaneFlowIDs;
  if (rapAdr == layer2->getDLLAddress()) { // BS-to-UT singlehop
    controlPlaneFlowIDs = flowManagerENB->getControlPlaneFlowIDs(userAdr);
  } else { // multihop
    controlPlaneFlowIDs = flowManagerENB->getControlPlaneFlowIDs(rapAdr);
  }
  return controlPlaneFlowIDs;
}

void
AssociationsProxyBS::disassociatedPerMode(wns::service::dll::UnicastAddress userAdr,
					  wns::service::dll::UnicastAddress targetAdr,
					  ModeName mode)
{
  assure(hasAssociatedPerMode(userAdr, mode), "Unknown user!");

  // update downlink route in RANG
  assure(myUpperConvergence->getRANG()->knowsAddress(userAdr), "User is unknown in RANG!");
  myUpperConvergence->getRANG()->removeAddress(userAdr, myUpperConvergence);
  assure(!(myUpperConvergence->getRANG()->knowsAddress(userAdr)), "remove user in RANG not successfully!");

  UserInfoLookup::iterator foundUser = activeUsers.find(userAdr);

  activeUsers.erase(foundUser);

  MESSAGE_BEGIN(NORMAL, logger, m, "Removed user=");
  m << A2N(userAdr) << " for mode: " << mode << ".";
  MESSAGE_END();

  assure(!hasAssociationTo(userAdr), "Remove user information not correctly!");

  if (inMyREC(targetAdr))
    {
      flowManagerENB->onDisassociationReq(userAdr, mode, /*preserved=*/inMyREC(targetAdr));
    }
  else
    {
      // the targetAdr is either invalid (timeout or plain disassociation) or in another REC
      // and user is also not preserved, so we can "forget" this user
      flowManagerENB->onDisassociationReq(userAdr, mode, /*preserved=*/inMyREC(targetAdr));
    }
}

bool
AssociationsProxyBS::hasAssociationTo(const wns::service::dll::UnicastAddress& dstAdr) const
{
  UserInfoLookup::const_iterator foundUser = activeUsers.find(dstAdr);

  return (foundUser != activeUsers.end());
}

bool
AssociationsProxyBS::hasAssociatedPerMode(wns::service::dll::UnicastAddress userAdr, ModeName mode) const
{
  UserInfoLookup::const_iterator foundUser = activeUsers.find(userAdr);

  return ((foundUser != activeUsers.end()) && ((*foundUser).second.mode == mode));
}

wns::service::dll::UnicastAddress
AssociationsProxyBS::getRAPforUserPerMode(wns::service::dll::UnicastAddress userAdr, ModeName mode) const
{
  UserInfoLookup::const_iterator foundUser = activeUsers.find(userAdr);
  assure(foundUser != activeUsers.end(), "getRAPforUserPerMode: User "<<A2N(userAdr)<<" not found.");
  assure((*foundUser).second.mode == mode, "User hasn't associated per this mode but getRAP called from this mode!");
  assure((*foundUser).second.rapAdr.isValid(), "invalid RAP of user!");
  return (*foundUser).second.rapAdr;
}

bool
AssociationsProxyBS::inMyREC(wns::service::dll::UnicastAddress adr) const
{
  std::set<wns::service::dll::UnicastAddress>::const_iterator found = myRECRAPs.find(adr);
  return (found != myRECRAPs.end());
}

void
AssociationsProxyBS::addRAPofREC(wns::service::dll::UnicastAddress rap)
{
  // Avoid adding the same RAP twice
  if (!inMyREC(rap))
    myRECRAPs.insert(rap);
}

bool
AssociationsProxyBS::hasPreserved(wns::service::dll::UnicastAddress userAdr) const
{
  std::set<wns::service::dll::UnicastAddress>::const_iterator found = preservedUsers.find(userAdr);
  return (found != preservedUsers.end());
}

/////////////////////////
//                     //
// AssociationsProxyUT //
//                     //
/////////////////////////

AssociationsProxyUT::AssociationsProxyUT(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config) :
  AssociationsProxy(csr, config),
  rlc(NULL),
  upperSynchronizer(NULL),
  macg(NULL),
  activeAssociation(),
  detectedModes(),
  interMode(),
  busy(false),
  plainDisassociation(false),
  preserve(false),
  associationStartTime(0.0),
  disAssociationStartTime(0.0),
  modePriorityLookup(),
  associationDurationProbe(NULL),
  initialaccessDurationProbe(NULL),
  initialaccessFirstTime(true),
  handoverDurationProbe(NULL)
{
  wns::pyconfig::View modePriorityView = config.getView("modePriority");
  unsigned int numModes = modePriorityView.get<int>("__len__()");
  assure(numModes == myModes.size(), "Mismatch of mode priority!");
  int priorityForNone = 0;
  for(unsigned int i=0; i < numModes; i++)
    {
      ModeName modeName = modePriorityView.get<ModeName>("keys()", i);
      int priority = modePriorityView.get<int>("values()", i);
      modePriorityLookup[modeName] = priority;
      priorityForNone = priority + 1 ;
    }
  modePriorityLookup["none"] = priorityForNone;
}

AssociationsProxyUT::~AssociationsProxyUT()
{
  if (associationDurationProbe != NULL) delete associationDurationProbe; associationDurationProbe = NULL;
  if (initialaccessDurationProbe != NULL) delete initialaccessDurationProbe; initialaccessDurationProbe = NULL;
  if (handoverDurationProbe != NULL) delete handoverDurationProbe; handoverDurationProbe = NULL;
}

void
AssociationsProxyUT::onCSRCreated()
{
  AssociationsProxy::onCSRCreated();

  flowManagerUE = layer2->getControlService<lte::controlplane::flowmanagement::IFlowManagerUE>("FlowManagerUT");

  rlc = layer2->getFUN()->findFriend<lte::rlc::UERLC*>("rlc");

  upperSynchronizer = layer2->getFUN()->findFriend<wns::ldk::tools::Synchronizer*>("upperSynchronizer");

  // Only interested in the receptacle aspect of the macg FU to send a wakeup
  macg = layer2->getFUN()->findFriend<lte::macg::MACg*>("macg");

  for(std::list<std::string>::iterator iter = myModes.begin(); iter != myModes.end(); ++iter)
    {
      ModeName mode = *iter;
      detectedModes[mode] = wns::service::dll::UnicastAddress();
    }

  // set activeAssociation as none
  activeAssociation.mode = "none";

  // set interMode as none
  interMode.mode = "none";

  associationDurationProbe = new wns::probe::bus::ContextCollector(
								   wns::probe::bus::ContextProviderCollection(getCSR()->getLayer()->getContextProviderCollection()), "lte.AssociationDuration");

  initialaccessDurationProbe = new wns::probe::bus::ContextCollector(
								     wns::probe::bus::ContextProviderCollection(getCSR()->getLayer()->getContextProviderCollection()), "lte.InitialAccessDuration");

  handoverDurationProbe = new wns::probe::bus::ContextCollector(
								wns::probe::bus::ContextProviderCollection(getCSR()->getLayer()->getContextProviderCollection()), "lte.HandoverDuration");
}

// triggered by AssociationHandler after reception of AssociationAck from RAP:
void
AssociationsProxyUT::associatedPerMode(wns::service::dll::UnicastAddress rapAdr,
				       wns::service::dll::UnicastAddress bsAdr,
				       ModeName mode,
				       lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs controlPlaneFlowIDs)
{
  assure(!hasAssociationTo(bsAdr),"UT has already association to the BS!");
  assure(!hasAssociation(), "UT has already association!");
  flowManagerUE->setControlPlaneFlowIDs(rapAdr, controlPlaneFlowIDs);

  activeAssociation.mode = mode;
  activeAssociation.rap = rapAdr;
  activeAssociation.bs = bsAdr;

  MESSAGE_BEGIN(NORMAL, logger, m, "Stored RAP=");
  m << A2N(activeAssociation.rap) << " for mode=" << activeAssociation.mode
    << " (BS=" << A2N(activeAssociation.bs) << ").";
  MESSAGE_END();

  assure(hasAssociationTo(bsAdr), "store association info not correctly!");

  // check this association is preserved
  if(preserve)
    {
      // reset preserve
      preserve = false;
    }
  flowManagerUE->onAssociatedPerMode(rapAdr, preserve);

  // check this association is intra or inter mode handover
  if (interMode.mode != "none")
    {
      assure(interMode.rapAdr.isValid(), "Wrong information stored for interMode!");
      // reset interMode
      interMode.mode = "none";
      // reset interMode handover destination
      interMode.rapAdr = wns::service::dll::UnicastAddress();
    }
  else
    {
      // Intra-Mode-Handover completed
      assure(!(interMode.rapAdr.isValid()), "Wrong information stored for interMode!");
      // set destination to RLC, so that RLC ready to accept outgoing compounds
      rlc->setDestination(bsAdr);
      macg->wakeup();
    }

  // write the association duration Probe
  writeProbes(true);

  if (disAssociationStartTime == 0.0)
    {
      // free again for new signaling, because it was an initial access
      // no need to wait for flow establishment
      busy = false;
      MESSAGE_SINGLE(NORMAL, logger, "Not busy any more.");
    }
} // associatedPerMode

void
AssociationsProxyUT::disassociatedPerMode(wns::service::dll::UnicastAddress rapAdr, ModeName mode, bool preserved)
{
  assure(activeAssociation.rap == rapAdr, "UT has no association!");

  wns::service::dll::UnicastAddress bsAdr = activeAssociation.bs;

  activeAssociation = AssociationInfo();
  activeAssociation.mode = "none";

  MESSAGE_BEGIN(NORMAL, logger, m, "Removed RAP=");
  m << A2N(rapAdr) << " for mode=" << mode << "(BS=" << A2N(bsAdr) << ").";
  MESSAGE_END();

  assure(!hasAssociationTo(bsAdr),"Reset association info not correctly!");

  if (plainDisassociation)
    {
      MESSAGE_SINGLE(NORMAL, logger, "Not busy any more.");
      busy = false;
      plainDisassociation = false;

      //maybe wrong place to call, but for now enough for purposes:
      flowManagerUE->onPlainDisassociation(mode);

      // check if other mode already detected
      ModeInfo otherDetected = getBestDetected();
      if (otherDetected.rapAdr.isValid())
	{
	  busy = true;

	  MESSAGE_BEGIN(NORMAL, logger, m, "Associating mode=") ;
	  m << mode << " (RAP=" << A2N(rapAdr) << ").";
	  MESSAGE_END();

	  layer2->getFUN()->findFriend<lte::controlplane::associationHandler::IAssociationHandlerTriggers*>(mode+"_associationHandler")
	    ->associateTo(rapAdr);

	  associationStartTime = wns::simulator::getEventScheduler()->getTime();
	  MESSAGE_SINGLE(NORMAL, logger, "disassociatedPerMode(): plainDisassociation, AssociationStartTime="<< associationStartTime);
	}
      else // no other detected modes, PlainDisAssociation
	{
	  //disable AssociationDuration Probe Measurement
	  associationStartTime = 0.0;
	  MESSAGE_SINGLE(NORMAL, logger, "disassociatedPerMode(): plainDisassociation, AssociationStartTime="<< associationStartTime);

	  return;
	}
    }
  else //it is a HANDOVER
    {
      // check preserve
      if(preserved)
	{
	  preserve = true;
	}
      else // no preserve
	{
	  // reset the destination to RLC, so that RLC not accept outgoing compounds
	  rlc->setDestination(wns::service::dll::UnicastAddress());
	}
      associationStartTime = wns::simulator::getEventScheduler()->getTime();
      MESSAGE_SINGLE(NORMAL, logger, "disassociatedPerMode(): HANDOVER, AssociationStartTime="<< associationStartTime);
      flowManagerUE->onDisassociatedPerMode(bsAdr, mode, preserved);
    }

  // check if this disassociation is for inter or intra mode handover or
  // only the plain disassociation because of too low SINR
  if (interMode.rapAdr.isValid())
    {
      // Inter-Mode-Handover to complete
      assure(interMode.mode != "none", "Mode name not found for inter mode handover!");
      layer2->getFUN()->findFriend<lte::controlplane::associationHandler::IAssociationHandlerTriggers*>
	(interMode.mode+"_associationHandler")->associateTo(interMode.rapAdr);
    }
  //else is Intra-Mode-Handover,
  //the associationHandler continues with the association phase
  //signaling will be completed by associatedPerMode(rap, bs, mode)
} // disassociatedPerMode

bool
AssociationsProxyUT::hasAssociationTo(const wns::service::dll::UnicastAddress& destination) const
{
  return (activeAssociation.bs == destination || activeAssociation.rap == destination);
}

wns::service::dll::UnicastAddress
AssociationsProxyUT::getBSforMode(ModeName mode) const
{
  if (activeAssociation.mode == mode)
    return activeAssociation.bs;
  else
    return wns::service::dll::UnicastAddress();
}

void
AssociationsProxyUT::disassociationOnTimeout(wns::service::dll::UnicastAddress dst, ModeName mode)
{
  wns::service::dll::UnicastAddress bs = getBSforMode(mode);
  assure(bs.isValid(), "invalid base station!");
  bool preserved = false;
  disassociatedPerMode(dst, mode, preserved);

  dll::ILayer2* destination = layer2->getStationManager()->getStationByMAC(dst);

  // associated to a BS
  if (bs == dst)
    destination->getFUN()->findFriend<lte::controlplane::associationHandler::IAssociationHandler*>(mode+"_associationHandler")
      ->disassociationOnTimeout(layer2->getDLLAddress(), mode);

  // associated to a RN
  else
    destination->getFUN()->findFriend<lte::controlplane::associationHandler::IAssociationHandler*>(mode+"_BS_associationHandler")
      ->disassociationOnTimeout(layer2->getDLLAddress(), mode);
}

bool
AssociationsProxyUT::isBusy() const
{
  return busy;
}

void
AssociationsProxyUT::modeDetected(ModeName mode, wns::service::dll::UnicastAddress rapAdr)
{
  if (!(detectedModes[mode] == rapAdr))
    {
      MESSAGE_BEGIN(NORMAL, logger, m, "Detected mode=");
      m << mode << " (RAP=" << A2N(rapAdr) << ").";
      MESSAGE_END();

      // update detected mode(overwrite the RAP address)
      detectedModes[mode] = rapAdr;
    }// else the rap is already detected! nothing to do.

  if (!busy)
    {
      // has no association -> associate the detected mode
      if (!hasAssociation())
	{
	  busy = true;

	  MESSAGE_BEGIN(NORMAL, logger, m, "Associating mode=") ;
	  m << mode << " (RAP=" << A2N(rapAdr) << ").";
	  MESSAGE_END();

	  layer2->getFUN()->findFriend<lte::controlplane::associationHandler::IAssociationHandlerTriggers*>(mode+"_associationHandler")
	    ->associateTo(rapAdr);

	  associationStartTime = wns::simulator::getEventScheduler()->getTime();
	  MESSAGE_SINGLE(NORMAL, logger, "modeDetected(): AssociationStartTime="<< associationStartTime);
	}

      else
	{
	  if (activeAssociation.mode == mode) // Intra-Mode-HO decision
	    {
	      if (activeAssociation.rap == rapAdr)
		{
		  // UT is already associated with this RAP. Nothing to do
		  MESSAGE_SINGLE(NORMAL, logger, "modeDetected(): Already associated to " << A2N(rapAdr) << " per mode=" << mode);
		  return;
		}
	      else // Intra-Mode-HO
		{
		  MESSAGE_SINGLE(NORMAL, logger, "modeDetected(): Intra-Mode(Inter- or Intra-rec)");
		  // only the best RAP per this detected mode will be
		  // reported -> the new RAP is better than the old RAP
		  assure(interMode.mode == "none", "Wrong information stored for interMode!");
		  busy = true;

		  MESSAGE_BEGIN(NORMAL, logger, m, "modeDetected(): Intra-Mode Handover for mode=");
		  m << mode << ", target=" << A2N(rapAdr) << ").";
		  MESSAGE_END();

		  //FlowManager has to release existing Flows to the RAP going to disassociate from
		  flowManagerUE->disassociating(mode);
		  //start time for HO interruption time
		  //probe:
		  disAssociationStartTime = wns::simulator::getEventScheduler()->getTime();
		  MESSAGE_SINGLE(NORMAL, logger, "disAssociationStartTime set (modeDetected): "<< disAssociationStartTime);
		  layer2->getFUN()->findFriend<lte::controlplane::associationHandler::IAssociationHandlerTriggers*>
		    (mode+"_associationHandler")->switchAssociationTo(rapAdr);
		}
	    }
	  else // Inter-Mode-HO
	    {
	      if (modePriorityLookup[mode] < modePriorityLookup[activeAssociation.mode]) // Decision for Inter-Mode-HO
		{
		  // store the next association, disassociation first
		  interMode.mode = mode;
		  interMode.rapAdr = rapAdr;

		  busy = true;

		  MESSAGE_BEGIN(NORMAL, logger, m, "modeDetected(): Inter-Mode Handover for mode=");
		  m << mode << ", target=" << A2N(rapAdr) << ").";
		  MESSAGE_END();

		  //FlowManager has to release mode-dependent existing Flows to the RAP going to disassociate from
		  flowManagerUE->disassociating(activeAssociation.mode);

		  //start time for HO interruption time
		  //probe:
		  disAssociationStartTime = wns::simulator::getEventScheduler()->getTime();
		  MESSAGE_SINGLE(NORMAL, logger, "modeDetected(): disAssociationStartTime="<< disAssociationStartTime);

		  layer2->getFUN()
		    ->findFriend<lte::controlplane::associationHandler::IAssociationHandlerTriggers*>
		    (activeAssociation.mode + "_associationHandler")->disassociate(rapAdr);
		}
	    }
	}
    }
}

void
AssociationsProxyUT::disassociationNeeded(ModeName mode, wns::service::dll::UnicastAddress /*rapAdr*/)
{
  // called from the associationHandler,
  // which was informed from BCHService that the SINR is now too low,
  // -> delete this detected Mode!
  if (detectedModes[mode].isValid())
    {
      // reset the RAP address of the mode in detectedModes
      detectedModes[mode] = wns::service::dll::UnicastAddress();
    } //else do nothing

  if ((activeAssociation.mode == mode) && (!busy))
    {
      // get the best of the detectedModes
      // Intra-Mode-HO impossible!
      // the best is either another mode or none
      interMode = getBestDetected();

      // set busy because of signaling
      busy = true;

      MESSAGE_BEGIN(NORMAL, logger, m, "disassociationNeeded(): Disassociating mode=");
      m << mode << " (RAP=" << A2N(activeAssociation.rap) << ") because of threshold violation.";
      MESSAGE_END();

      // get the associationHandler of the active association
      lte::controlplane::associationHandler::IAssociationHandlerTriggers* ah = layer2->getFUN()
	->findFriend<lte::controlplane::associationHandler::IAssociationHandlerTriggers*>(mode+"_associationHandler");

      if (interMode.rapAdr.isValid()) // other detected mode -> Inter-Mode-HO
	{
	  assure(!(interMode.mode == mode), "disassociation because of threshold violation doesn't lead to Intra-Mode-HO!");
	  //setting disAssociationStartTime for HO interruption time probe:
	  disAssociationStartTime = wns::simulator::getEventScheduler()->getTime();
	  MESSAGE_SINGLE(NORMAL, logger, "disassociationNeeded(): disAssociationStartTime="<< disAssociationStartTime);
	}
      else // no other modes detected -> plain disassociation
	{
	  assure(interMode.mode == "none", "Wrong mode information stored in interMode!");
	  // set plainDisassociation so that associationsProxy resets busy after the disassociation
	  plainDisassociation = true;
	  MESSAGE_SINGLE(NORMAL, logger, "disassociationNeeded()plainDisassociation="<< plainDisassociation);
	  // do not evaluate handoverduration because it is PlainDisassociation
	  disAssociationStartTime = 0.0;
	  MESSAGE_SINGLE(NORMAL, logger, "disassociationNeeded(): disAssociationStartTime="<< disAssociationStartTime);
	}
      //FlowManager has to release mode-dependent existing Flows to the RAP going to disassociate from
      flowManagerUE->disassociating(activeAssociation.mode);
      ah->disassociate(interMode.rapAdr);
    }
} // disassociationNeeded

bool
AssociationsProxyUT::hasAssociation() const
{
  return (activeAssociation.mode != "none");
}

ModeInfo
AssociationsProxyUT::getBestDetected() const
{
  ModeInfo modeInfo;
  modeInfo.mode = "none";
  int currentPriority = 0;

  // result
  ModeInfo bestModeInfo;
  bestModeInfo.mode = "none";
  int bestPriority = 0;

  for(std::list<ModeName>::const_iterator iter = myModes.begin(); iter != myModes.end(); ++iter)
    {
      // get the RAP address of the mode in detectedModes
      ModeName modeName = (*iter);
      std::map<ModeName, wns::service::dll::UnicastAddress>::const_iterator foundDetected = detectedModes.find(modeName);
      wns::service::dll::UnicastAddress foundRAPAdr = (*foundDetected).second;
      if (!(foundRAPAdr.isValid())) // forget the undetected modes
	continue;
      std::map<ModeName, int>::const_iterator foundCurrentMode = modePriorityLookup.find(modeName);
      currentPriority = (*foundCurrentMode).second;

      // the first update
      if(!(bestModeInfo.rapAdr.isValid()))
	{
	  bestPriority = currentPriority;
	  bestModeInfo.mode = modeName;
	  bestModeInfo.rapAdr = foundRAPAdr;
	}
      else
	{
	  // update the result if the current is better
	  if(currentPriority < bestPriority)
	    {
	      bestPriority = currentPriority;
	      bestModeInfo.mode = modeName;
	      bestModeInfo.rapAdr = foundRAPAdr;
	    }
	}
    }

  return bestModeInfo;
} // getBestDetected

void
AssociationsProxyUT::writeProbes(const bool alreadyAfterAssociation)
{
  wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();
  if (alreadyAfterAssociation)
    {
      // here the duration of the association procedure itself is measured
      assure(associationStartTime > 0.0, "associationStartTime is 0 after association!");
      associationDurationProbe->put(now - associationStartTime);
      MESSAGE_SINGLE(VERBOSE, logger, "AssociationStartTime: Writeprobes AssociationDuration written: "<<now - associationStartTime);
    }
  else
    {
      //InitialAccess after PlainDisassociation:
      if (disAssociationStartTime == 0.0)
	{
	  //assure(associationStartTime > 0.0, "No InitialAccess without Association.");
	  if(associationStartTime > 0.0)
	    {
	      if(initialaccessFirstTime == true)
		{
		  //don't consider the first time. because association and starting an
		  //application (building first flow) are independent
		  initialaccessFirstTime = false;
		}
	      else
		{
		  // this is written after the association AND the first
		  // flow is established, i.e. user plane data can be sent
		  initialaccessDurationProbe->put(now - associationStartTime);
		  // reset in order to measure only after the first flow
		  // is established
		  MESSAGE_SINGLE(VERBOSE, logger, "AssociationStartTime set 0.0. Writeprobes InitialAccess written: "<<now - associationStartTime);
		}
	      associationStartTime = 0.0;
	    }
	  else
	    MESSAGE_SINGLE(VERBOSE, logger, "AssociationStartTime: 0.0, disAssociationStartTime: 0.0. No InitialAccess to measure! (May be further more Flow(s).");
	}
      // Handover:
      else
	{
	  // this is written if and only if there was a
	  // disassociation before the association AND again after
	  // the first flow is established, i.e. user plane data
	  // can be sent. This is a handover
	  handoverDurationProbe->put(now - disAssociationStartTime);
	  MESSAGE_SINGLE(VERBOSE, logger, "disAssociationStartTime set 0.0. Writeprobes HO-duration written: "<<now - disAssociationStartTime);

	  // reset in order to measure only after the first flow
	  // is established
	  disAssociationStartTime = 0.0;
	  associationStartTime = 0.0;
	}
    }
} // writeProbes

void
AssociationsProxyUT::flowBuilt()
{
  // false means that measurement is not already after association
  // procedure, but after the first flow is established
  writeProbes(false);
  busy = false;
  MESSAGE_SINGLE(NORMAL, logger, "flowBuilt(): Not busy any more.");
}
