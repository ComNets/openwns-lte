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

#ifndef LTE_CONTROLPLANE_ASSOCIATIONSPROXY_HPP
#define LTE_CONTROLPLANE_ASSOCIATIONSPROXY_HPP

#include <LTE/helper/Keys.hpp>
#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>

#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/ldk/buffer/Dropping.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/logger/Logger.hpp>
#include <DLL/Layer2.hpp>
#include <DLL/UpperConvergence.hpp>

namespace lte { namespace macg { class MACg;}}
namespace lte { namespace rlc { class UERLC;}}

namespace lte { namespace controlplane {

    typedef std::string ModeName;

    struct ModeInfo
    {
      ModeName mode;
      wns::service::dll::UnicastAddress rapAdr;
    };

    typedef std::map<wns::service::dll::UnicastAddress, ModeInfo> UserInfoLookup; // for RAP

    struct AssociationInfo {
      ModeName mode;
      wns::service::dll::UnicastAddress rap;
      wns::service::dll::UnicastAddress bs;
    }; // for UT

    class AssociationsProxy :
    public wns::ldk::ControlService,
    public wns::events::PeriodicTimeout
    {
    public:
      AssociationsProxy(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
  
      virtual ~AssociationsProxy();

      virtual void
      onCSRCreated();

      virtual void
      periodically();

    protected:
      dll::ILayer2* layer2;
      wns::ldk::CommandReaderInterface* rlcReader;
      std::list<ModeName> myModes;
      wns::logger::Logger logger;
      UserInfoLookup activeUsers;
    };

    class AssociationsProxyBS :
    public AssociationsProxy
    {
    public:
      AssociationsProxyBS(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);

      virtual ~AssociationsProxyBS();

      virtual void
      onCSRCreated();

      /** @brief registers that 'userAdr' has associated itself to
       * 'rapAdr' per 'mode'*/
      virtual lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs
      associatedPerMode(wns::service::dll::UnicastAddress userAdr,
			wns::service::dll::UnicastAddress rapAdr,
			ModeName mode);

      /** @brief registers that 'userAdr' has disassociated
       * itself from whatever RAP where 'mode' is the mode in
       * which the AssociationHandler received the disassociation_req */
      virtual void
      disassociatedPerMode(wns::service::dll::UnicastAddress userAdr,
			   wns::service::dll::UnicastAddress targetRAP,
			   ModeName mode);

      bool
      hasAssociatedPerMode(wns::service::dll::UnicastAddress userAdr,
			   ModeName mode) const;

      wns::service::dll::UnicastAddress
      getRAPforUserPerMode(wns::service::dll::UnicastAddress userAdr, ModeName mode) const;

      void
      addRAPofREC(wns::service::dll::UnicastAddress rap);

      bool
      inMyREC(wns::service::dll::UnicastAddress adr) const ;

    private:

      bool
      hasPreserved(wns::service::dll::UnicastAddress userAdr) const;

      virtual bool
      hasAssociationTo(const wns::service::dll::UnicastAddress& dstAdr) const ;

      lte::controlplane::flowmanagement::IFlowManagerENB* flowManagerENB;
      dll::APUpperConvergence* myUpperConvergence;
      wns::ldk::tools::Synchronizer* upperSynchronizer;
      std::set<wns::service::dll::UnicastAddress> myRECRAPs;
      std::set<wns::service::dll::UnicastAddress> preservedUsers;
    };

    class AssociationsProxyUT :
    public AssociationsProxy
    {
    public:
      AssociationsProxyUT(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);

      virtual ~AssociationsProxyUT();

      virtual void
      onCSRCreated();

      // Also used by AssociationsProxyUTusingMIH
      virtual void
      associatedPerMode(wns::service::dll::UnicastAddress rapAdr,
			wns::service::dll::UnicastAddress bsAdr,
			ModeName mode,
			lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs controlPlaneFlowIDs);

      // Also used by AssociationsProxyUTusingMIH
      virtual void
      disassociatedPerMode(wns::service::dll::UnicastAddress rapAdr,
			   ModeName mode, bool preserve);

      wns::service::dll::UnicastAddress
      getBSforMode(ModeName mode) const;

      void
      disassociationOnTimeout(wns::service::dll::UnicastAddress dst, ModeName mode);

      // check statement
      bool
      isBusy() const;

      // mode detection report from association handler
      void
      modeDetected(ModeName mode, wns::service::dll::UnicastAddress rapAdr);

      // inform from associationHandler that the SINR is below
      // the threshold and this mode should be disassociated
      void
      disassociationNeeded(ModeName mode, wns::service::dll::UnicastAddress rapAdr);

      void
      flowBuilt();

      virtual void
      periodically(){}

      ModeInfo
      getBestDetected() const;

      bool
      hasAssociation() const;

    protected:
      virtual bool
      hasAssociationTo(const wns::service::dll::UnicastAddress& dstAdr) const ;

      void
      writeProbes(const bool alreadyAfterAssociation);

      lte::rlc::UERLC* rlc;
      wns::ldk::tools::Synchronizer* upperSynchronizer;
      wns::ldk::Receptor* macg;
      AssociationInfo activeAssociation;
      std::map<ModeName, wns::service::dll::UnicastAddress> detectedModes;
      ModeInfo interMode;
      bool busy;
      bool plainDisassociation;
      bool preserve;
      wns::simulator::Time associationStartTime;
      wns::simulator::Time disAssociationStartTime;
    private:
      std::map<ModeName, int> modePriorityLookup;
      wns::probe::bus::ContextCollector* associationDurationProbe;
      wns::probe::bus::ContextCollector* initialaccessDurationProbe;
      bool initialaccessFirstTime;
      wns::probe::bus::ContextCollector* handoverDurationProbe;
      lte::controlplane::flowmanagement::IFlowManagerUE* flowManagerUE;
    };

  } // controlplane
} // lte

#endif // NOT defined LTE_CONTROLPLANE_ASSOCIATIONSPROXY_HPP
