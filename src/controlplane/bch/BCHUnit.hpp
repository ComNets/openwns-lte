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

#ifndef LTE_LTEBCHUNIT_HPP
#define LTE_LTEBCHUNIT_HPP

#include <LTE/controlplane/bch/BCHUnitInterface.hpp>
#include <LTE/controlplane/bch/BCHService.hpp>
#include <LTE/helper/HasModeName.hpp>
#include <LTE/macr/PhyCommand.hpp>
#include <LTE/macg/MACg.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/multiplexer/OpcodeProvider.hpp>

#include <WNS/events/PeriodicTimeout.hpp>
#include <WNS/pyconfig/View.hpp>

#include <DLL/Layer2.hpp>
#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

#include <list>

namespace dll { namespace services {namespace control {
      class Association;

    }}}


namespace lte {
  namespace timing {
    class ResourceScheduler;
  }
  namespace controlplane {
    namespace associationHandler {
      class AssociationHandlerUT;
    }

    namespace bch {

      /** @brief Command contributed by the BCH Functional Unit */
      class LTEBCHCommand
    : public wns::ldk::Command
      {
      public:
    LTEBCHCommand()
    {
      peer.acknowledgement = false;
      magic.source = wns::service::dll::UnicastAddress();
    };

    struct {
    } local;
    struct {
      bool acknowledgement;
      std::list<wns::ldk::CompoundPtr> ackedUTs;
    } peer;
    struct {
      wns::service::dll::UnicastAddress source;
    } magic;
      };

      /**
       * @brief Handles BCH compounds.
       *
       * In the outgoing path, this FU simply produces Compounds of a certain
       * configurable size.
       *
       * In the incoming path, this FU can store the received Broadcast compounds
       * of from multiple sources in a sorted way. The sorting criterion can be
       * defined, @see BCHCriterion .
       */
      class LTEBCHUnit :
    public virtual wns::ldk::FunctionalUnit,
    public wns::ldk::CommandTypeSpecifier<LTEBCHCommand>,
    public wns::ldk::HasReceptor<>,
    public wns::ldk::HasConnector<>,
    public wns::ldk::HasDeliverer<>,
    public lte::helper::HasModeName
      {
      public:
    LTEBCHUnit(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
    virtual
    ~LTEBCHUnit();

    virtual void
    onFUNCreated();

    //
    // PDU size information
    //
    virtual void
    calculateSizes(const wns::ldk::CommandPool* commandPool, Bit& _pciSize, Bit& _pduSize) const;

      protected:
    wns::logger::Logger logger;
    dll::ILayer2* layer2;
    struct {
      lte::timing::ResourceScheduler* scheduler;
      lte::macg::MACg* macg;
      lte::macr::PhyUser* phyUser;
    } friends;

	wns::ldk::CommandReaderInterface* rlcReader;
    std::string phyUserName;

      private:
    int commandSize;
      };

      class LTEBCHUnitRAP :
    public LTEBCHUnit,
    public wns::Cloneable<LTEBCHUnitRAP>,
    public IBCHTimingTx
      {
      public:
    LTEBCHUnitRAP(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

    virtual
    ~LTEBCHUnitRAP();

    virtual void
    onFUNCreated();

    //
    // compound handler interface
    //
    virtual void
    doSendData(const wns::ldk::CompoundPtr& /* compound */);

    virtual void
    doOnData(const wns::ldk::CompoundPtr& /* compound */);

    //
    // For external Triggering
    //
    /** @brief called by event StartBCH */
    virtual
    void sendBCH(simTimeType duration);

      protected:
    virtual bool
    doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;

    virtual void
    doWakeup();

    /** @brief power used to transmit BCH */
    wns::Power txPower;

    bool accepting;
    wns::ldk::CompoundPtr compound;
    std::string rlcName;
    std::string macgName;
    std::string flowManagerName;
      };

      class LTEBCHUnitUT :
    public LTEBCHUnit,
    public wns::Cloneable<LTEBCHUnitUT>
      {
      public:
    LTEBCHUnitUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

    virtual
    ~LTEBCHUnitUT();

    //
    // compound handler interface
    //
    virtual void
    doSendData(const wns::ldk::CompoundPtr& /* compound */);

    virtual void
    doOnData(const wns::ldk::CompoundPtr& /* compound */);

    virtual void onFUNCreated();

      private:
    virtual bool
    doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;

    virtual void
    doWakeup();

    lte::controlplane::bch::BCHService* bchService;
    dll::services::control::Association* associationService;

    const wns::pyconfig::View config;

    wns::probe::bus::ContextCollector* sinrProbe;
    wns::probe::bus::ContextCollector* rxpProbe;
    wns::probe::bus::ContextCollector* interfProbe;
    std::string schedulerName;
      };

      class NoBCH :
    virtual public wns::ldk::FunctionalUnit,
    public wns::ldk::CommandTypeSpecifier<>,
    public wns::ldk::HasReceptor<>,
    public wns::ldk::HasConnector<>,
    public wns::ldk::HasDeliverer<>,
    public wns::ldk::Forwarding<NoBCH>,
    public wns::Cloneable<NoBCH>
      {
      public:
    NoBCH(wns::ldk::fun::FUN* fun, const wns::pyconfig::View&) :
      wns::ldk::CommandTypeSpecifier<>(fun),
      wns::ldk::HasReceptor<>(),
      wns::ldk::HasConnector<>(),
      wns::ldk::HasDeliverer<>(),
      wns::ldk::Forwarding<NoBCH>(),
      wns::Cloneable<NoBCH>()
    {};
      };

    class LTEBCHUnitRAPNoSched :
        virtual public LTEBCHUnitRAP
    {
    public:
        LTEBCHUnitRAPNoSched(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

        virtual
        ~LTEBCHUnitRAPNoSched();

        virtual
        void sendBCH(simTimeType duration);

    private:
        /** @brief Modulation & Coding used to transmit BCH */
        wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;
        /** @brief Power used to transmit BCH */
        wns::Power txPower;
    }; 


    }}}

#endif // NOT defined LTE_LTEBCHUNIT_HPP
