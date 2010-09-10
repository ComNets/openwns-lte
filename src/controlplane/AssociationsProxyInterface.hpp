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

#ifndef LTE_CONTROLPLANE_ASSOCIATIONSPROXYINTERFACE_HPP
#define LTE_CONTROLPLANE_ASSOCIATIONSPROXYINTERFACE_HPP

/* deleted by chen */
// #include <LTE/controlplane/AssociationControlInterface.hpp>
// #include <LTE/controlplane/LinkObserverInterface.hpp>
// #include <LTE/controlplane/flowmanagement/FlowManagerInterface.hpp>

/* deleted by chen */
// #include <LTE/rlc/RLC.hpp>
// #include <LTE/rlc/RLCUT.hpp>
// #include <LTE/helper/Keys.hpp>

#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/ldk/buffer/Dropping.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/logger/Logger.hpp>
#include <DLL/Layer2.hpp>
#include <DLL/UpperConvergence.hpp>

namespace lte {
/* deleted by chen */
//   namespace macg {
//     class MACg;
//   }
  namespace controlplane {
/* deleted by chen */
//     typedef std::string ModeName;
// 
//     struct ModeInfo
//     {
//       ModeName mode;
//       wns::service::dll::UnicastAddress rapAdr;
//     };
// 
//     typedef std::map<wns::service::dll::UnicastAddress, ModeInfo> UserInfoLookup; // for RAP
// 
//     struct AssociationInfo {
//       ModeName mode;
//       wns::service::dll::UnicastAddress rap;
//       wns::service::dll::UnicastAddress bs;
//     }; // for UT

/* deleted by chen */
//     class AssociationsProxy :
//       public wns::ldk::ControlService,
//       public wns::events::PeriodicTimeout
/* inserted by chen */
    class AssociationsProxy

    {
/* deleted by chen */
//     public:
//       AssociationsProxy(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
//       virtual ~AssociationsProxy();

//       virtual void
//       onCSRCreated();
// 
//       virtual void
//       periodically();

/* inserted by chen */
    public:
/* deleted by chen */
//       virtual void
//       onCSRCreated() = 0;
// 
//       virtual void
//       periodically() = 0;

/* deleted by chen */
//     protected:
//       dll::ILayer2* layer2;
//       wns::ldk::CommandReaderInterface* rlcReader;
//       std::list<ModeName> myModes;
//       wns::logger::Logger logger;
//       UserInfoLookup activeUsers;
    };

    class AssociationsProxyBS :
      public AssociationsProxy
    {
/* deleted by chen */
//     public:
//       AssociationsProxyBS(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
//       virtual ~AssociationsProxyBS();

//       virtual void
//       onCSRCreated();
// 
//       /** @brief registers that 'userAdr' has associated itself to
//        * 'rapAdr' per 'mode'*/
//       virtual lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs
//       associatedPerMode(wns::service::dll::UnicastAddress userAdr,
// 			wns::service::dll::UnicastAddress rapAdr,
// 			ModeName mode);
// 
//       /** @brief registers that 'userAdr' has disassociated
//        * itself from whatever RAP where 'mode' is the mode in
//        * which the AssociationHandler received the disassociation_req */
//       virtual void
//       disassociatedPerMode(wns::service::dll::UnicastAddress userAdr,
// 			   wns::service::dll::UnicastAddress targetRAP,
// 			   ModeName mode);
// 
// 
//       bool
//       hasAssociatedPerMode(wns::service::dll::UnicastAddress userAdr,
// 			   ModeName mode) const;
// 
//       wns::service::dll::UnicastAddress
//       getRAPforUserPerMode(wns::service::dll::UnicastAddress userAdr, ModeName mode) const;
// 
//       void
//       addRAPofREC(wns::service::dll::UnicastAddress rap);
// 
//       bool
//       inMyREC(wns::service::dll::UnicastAddress adr) const ;

/* inserted by chen */
    public:

/* deleted by chen */
//       virtual void
//       onCSRCreated() = 0;
// 
//       /** @brief registers that 'userAdr' has associated itself to
//        * 'rapAdr' per 'mode'*/
//       virtual lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs
//       associatedPerMode(wns::service::dll::UnicastAddress userAdr,
//                         wns::service::dll::UnicastAddress rapAdr,
//                         ModeName mode) = 0;
// 
//       /** @brief registers that 'userAdr' has disassociated
//                * itself from whatever RAP where 'mode' is the mode in
//        * which the AssociationHandler received the disassociation_req */
//       virtual void
//       disassociatedPerMode(wns::service::dll::UnicastAddress userAdr,
//                            wns::service::dll::UnicastAddress targetRAP,
//                            ModeName mode) = 0;
// 
// 
//       virtual bool
//       hasAssociatedPerMode(wns::service::dll::UnicastAddress userAdr,
//                            ModeName mode) const = 0;
// 
//       virtual wns::service::dll::UnicastAddress
//       getRAPforUserPerMode(wns::service::dll::UnicastAddress userAdr, ModeName mode) const = 0;

      virtual void
      addRAPofREC(wns::service::dll::UnicastAddress rap) = 0;

/* deleted by chen */
//       virtual bool
//       inMyREC(wns::service::dll::UnicastAddress adr) const = 0;

/* deleted by chen */
//     private:
// 
//       bool
//       hasPreserved(wns::service::dll::UnicastAddress userAdr) const;
// 
//       virtual bool
//       hasAssociationTo(const wns::service::dll::UnicastAddress& dstAdr) const ;
// 
//       lte::controlplane::flowmanagement::FlowManagerBS* flowManagerBS;
//       dll::APUpperConvergence* myUpperConvergence;
//       wns::ldk::tools::Synchronizer* upperSynchronizer;
//       std::set<wns::service::dll::UnicastAddress> myRECRAPs;
//       std::set<wns::service::dll::UnicastAddress> preservedUsers;
    };

/* deleted by chen */
//     class AssociationsProxyRN :
//       public AssociationsProxy
//     {
// /* deleted by chen */
// //     public:
// //       AssociationsProxyRN(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// //       virtual ~AssociationsProxyRN();
// 
// //       virtual void
// //       onCSRCreated();
// // 
// //       virtual lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs
// //       associatedPerMode(wns::service::dll::UnicastAddress userAdr,
// // 			wns::service::dll::UnicastAddress rapAdr,
// // 			ModeName mode,
// // 			lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDsHop1);
// // 
// //       virtual void
// //       disassociatingPerMode(wns::service::dll::UnicastAddress rapAdr,
// // 			    ModeName mode, std::string task);
// // 
// //       virtual void
// //       disassociatedPerMode(wns::service::dll::UnicastAddress userAdr,
// // 			   ModeName mode, std::string task);
// // 
// //       ModeName
// //       getModeBetweenRNandBS() const;
// 
// /* inserted by chen */
//     public:
// 
//       virtual void
//       onCSRCreated() = 0;
// 
//       virtual lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs
//       associatedPerMode(wns::service::dll::UnicastAddress userAdr,
//                         wns::service::dll::UnicastAddress rapAdr,
//                         ModeName mode,
//                         lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDsHop1) = 0;
// 
//       virtual void
//       disassociatingPerMode(wns::service::dll::UnicastAddress rapAdr,
//                             ModeName mode, std::string task) = 0;
// 
//       virtual void
//       disassociatedPerMode(wns::service::dll::UnicastAddress userAdr,
//                            ModeName mode, std::string task)= 0 ;
// 
//       virtual ModeName
//       getModeBetweenRNandBS() const = 0;
// 
// /* deleted by chen */
// //     private:
// //       virtual bool
// //       hasAssociationTo(const wns::service::dll::UnicastAddress& dstAdr) const ;
// // 
// //       virtual void
// //       deleteDataFlows(wns::service::dll::UnicastAddress userAdr);
// // 
// //       std::vector<wns::ldk::buffer::Dropping*> relayInjectBuffers;
// //       ModeName modeBetweenRNandBS;
// // 
// //       bool
// //       hasAssociatedPerMode(wns::service::dll::UnicastAddress userAdr, ModeName mode) const;
// // 
// //       lte::controlplane::flowmanagement::FlowManagerRN* flowManagerRN;
//     };
// 
//     class AssociationsProxyUT :
//       public lte::controlplane::LinkInfo,
//       public AssociationsProxy
//     {
// /* deleted by chen */
// //     public:
// //       AssociationsProxyUT(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// //       virtual ~AssociationsProxyUT();
// 
// //       virtual void
// //       onCSRCreated();
// // 
// //       // Also used by AssociationsProxyUTusingMIH
// //       virtual void
// //       associatedPerMode(wns::service::dll::UnicastAddress rapAdr,
// // 			wns::service::dll::UnicastAddress bsAdr,
// // 			ModeName mode,
// // 			lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs);
// // 
// //       // Also used by AssociationsProxyUTusingMIH
// //       virtual void
// //       disassociatedPerMode(wns::service::dll::UnicastAddress rapAdr,
// // 			   ModeName mode, bool preserve);
// // 
// //       wns::service::dll::UnicastAddress
// //       getBSforMode(ModeName mode) const;
// // 
// //       void
// //       disassociationOnTimeout(wns::service::dll::UnicastAddress dst, ModeName mode);
// // 
// //       // check statement
// //       bool
// //       isBusy() const;
// // 
// //       // mode detection report from association handler
// //       void
// //       modeDetected(ModeName mode, wns::service::dll::UnicastAddress rapAdr);
// // 
// //       // inform from associationHandler that the SINR is below
// //       // the threshold and this mode should be disassociated
// //       void
// //       disassociationNeeded(ModeName mode, wns::service::dll::UnicastAddress rapAdr);
// // 
// //       void
// //       flowBuilt();
// // 
// //       virtual void
// //       periodically(){}
// // 
// //       ModeInfo
// //       getBestDetected() const;
// // 
// //       bool
// //       hasAssociation() const;
// 
// /* inserted by chen */
//     public:
// 
//       virtual void
//       onCSRCreated() = 0;
// 
//       // Also used by AssociationsProxyUTusingMIH
//       virtual void
//       associatedPerMode(wns::service::dll::UnicastAddress rapAdr,
//                         wns::service::dll::UnicastAddress bsAdr,
//                         ModeName mode,
//                         lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowIDs) = 0;
// 
//       // Also used by AssociationsProxyUTusingMIH
//       virtual void
//       disassociatedPerMode(wns::service::dll::UnicastAddress rapAdr,
//                            ModeName mode, bool preserve) = 0;
// 
//       virtual wns::service::dll::UnicastAddress
//       getBSforMode(ModeName mode) const = 0;
// 
//       virtual void
//       disassociationOnTimeout(wns::service::dll::UnicastAddress dst, ModeName mode) = 0;
// 
//       // check statement
//       virtual bool
//       isBusy() const = 0;
// 
//       // mode detection report from association handler
//       virtual void
//       modeDetected(ModeName mode, wns::service::dll::UnicastAddress rapAdr) = 0;
// 
//       // inform from associationHandler that the SINR is below
//       // the threshold and this mode should be disassociated
//       virtual void
//       disassociationNeeded(ModeName mode, wns::service::dll::UnicastAddress rapAdr) = 0;
// 
//       virtual void
//       flowBuilt() = 0;
// 
//       virtual void
//       periodically() = 0;
// 
//       virtual ModeInfo
//       getBestDetected() const = 0;
// 
//       virtual bool
//       hasAssociation() const = 0;
// 
// /* deleted by chen */
// //     protected:
// //       virtual bool
// //       hasAssociationTo(const wns::service::dll::UnicastAddress& dstAdr) const ;
// // 
// //       void
// //       writeProbes(const bool alreadyAfterAssociation);
// // 
// //       lte::rlc::RLCUT* rlc;
// //       wns::ldk::tools::Synchronizer* upperSynchronizer;
// //       lte::macg::MACg* macg;
// //       AssociationInfo activeAssociation;
// //       std::map<ModeName, wns::service::dll::UnicastAddress> detectedModes;
// //       ModeInfo interMode;
// //       bool busy;
// //       bool plainDisassociation;
// //       bool preserve;
// //       wns::simulator::Time associationStartTime;
// //       wns::simulator::Time disAssociationStartTime;
// //     private:
// //       std::map<ModeName, int> modePriorityLookup;
// //       wns::probe::bus::ContextCollector* associationDurationProbe;
// //       wns::probe::bus::ContextCollector* initialaccessDurationProbe;
// //       bool initialaccessFirstTime;
// //       wns::probe::bus::ContextCollector* handoverDurationProbe;
// //       lte::controlplane::flowmanagement::FlowManagerUT* flowManagerUT;
//     };
// 
//     class AssociationsProxyUTusingMIH :
//       virtual public lte::controlplane::AssociationControlInterface,
//       public AssociationsProxyUT
//     {
// /* deleted by chen */
// //     public:
// //       AssociationsProxyUTusingMIH(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// //       virtual ~AssociationsProxyUTusingMIH();
// // 
// //       /** @name
// //        * lte::controlplane::AssociationControlInterface
// //        */
// //       //@{
// //       virtual void
// //       connect(const wns::service::dll::UnicastAddress& dst, const ModeName& mode);
// // 
// //       virtual void
// //       disconnect(const wns::service::dll::UnicastAddress& dst, const ModeName& mode);
// // 
// //       virtual bool
// //       isConnectedPerModeAndDestination(const wns::service::dll::UnicastAddress& dst, const ModeName& mode);
// // 
// //       virtual bool
// //       isDisconnectedPerModeAndDestination(const wns::service::dll::UnicastAddress& dst, const ModeName& mode);
// //       //@}
// // 
// //       virtual void
// //       associatedPerMode(wns::service::dll::UnicastAddress rapAdr,
// // 			wns::service::dll::UnicastAddress bsAdr,
// // 			ModeName mode,
// // 			      lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowID);
// // 
// //       virtual void
// //       disassociatedPerMode(wns::service::dll::UnicastAddress rapAdr,
// // 			   ModeName mode, bool preserve);
// 
// /* inserted by chen */
//     public:
// 
//       /** @name
//        * lte::controlplane::AssociationControlInterface
//        */
//       //@{
//       virtual void
//       connect(const wns::service::dll::UnicastAddress& dst, const ModeName& mode) = 0;
// 
//       virtual void
//       disconnect(const wns::service::dll::UnicastAddress& dst, const ModeName& mode) = 0;
// 
//       virtual bool
//       isConnectedPerModeAndDestination(const wns::service::dll::UnicastAddress& dst, const ModeName& mode) = 0;
// 
//       virtual bool
//       isDisconnectedPerModeAndDestination(const wns::service::dll::UnicastAddress& dst, const ModeName& mode) = 0;
//       //@}
// 
//       virtual void
//       associatedPerMode(wns::service::dll::UnicastAddress rapAdr,
//                         wns::service::dll::UnicastAddress bsAdr,
//                         ModeName mode,
//                         lte::controlplane::flowmanagement::FlowManager::ControlPlaneFlowIDs controlPlaneFlowID) = 0;
// 
//       virtual void
//       disassociatedPerMode(wns::service::dll::UnicastAddress rapAdr,
//                            ModeName mode, bool preserve) = 0;
//     };
  }
}
#endif // NOT defined LTE_CONTROLPLANE_ASSOCIATIONSPROXYINTERFACE_HPP
