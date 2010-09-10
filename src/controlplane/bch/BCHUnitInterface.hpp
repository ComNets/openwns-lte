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

#ifndef LTE_CONTROLPLANE_BCH_BCHUNITINTERFACE_HPP
#define LTE_CONTROLPLANE_BCH_BCHUNITINTERFACE_HPP

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/multiplexer/OpcodeProvider.hpp>

#include <WNS/events/PeriodicTimeout.hpp>
#include <WNS/pyconfig/View.hpp>

/* deleted by chen */
// #include <LTE/controlplane/bch/BCHService.hpp>
// #include <LTE/macr/PhyCommand.hpp>
#include <LTE/helper/HasModeName.hpp>

#include <DLL/Layer2.hpp>
#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

#include <list>

/* deleted by chen */
// namespace dll { namespace services {namespace control {
//       class Association;
// 
//     }}}
namespace lte {
//   namespace macr {
//     class PhyUser;
//   }
  namespace controlplane {
//     namespace associationHandler {
//       class AssociationHandlerUT;
//     }

    namespace bch {

/* deleted by chen */
      /** @brief Command contributed by the BCH Functional Unit */
//       class BCHCommand
//     : public wns::ldk::Command
//       {
//       public:
// /* deleted by chen */
// //     BCHCommand()
// //     {
// //       peer.acknowledgement = false;
// //       magic.source = wns::service::dll::UnicastAddress();
// //     };
// 
//     struct {
//     } local;
//     struct {
//       bool acknowledgement;
//       std::list<wns::ldk::CompoundPtr> ackedUTs;
//     } peer;
//     struct {
//       wns::service::dll::UnicastAddress source;
//     } magic;
//       };

    class IBCHTimingTx
    {
    public:
        /** @brief called by event StartBCH */
        virtual void
        sendBCH(simTimeType duration) = 0;
    };

/* deleted by chen */
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
//       class BCHUnit :
//     public virtual wns::ldk::FunctionalUnit,
//     public wns::ldk::CommandTypeSpecifier<BCHCommand>,
//     public wns::ldk::HasReceptor<>,
//     public wns::ldk::HasConnector<>,
//     public wns::ldk::HasDeliverer<>,
//     public lte::helper::HasModeName
//       {
// /* deleted by chen */
// //       public:
// //     BCHUnit(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// //     virtual
// //     ~BCHUnit();
// 
// //     virtual void
// //     onFUNCreated();
// // 
// //     //
// //     // PDU size information
// //     //
// //     virtual void
// //     calculateSizes(const wns::ldk::CommandPool* commandPool, Bit& _pciSize, Bit& _pduSize) const;
// 
// /* inserted by chen */
//       public:
// 
//     virtual void
//     onFUNCreated() = 0;
// 
//     //
//     // PDU size information
//     //
//     virtual void
//     calculateSizes(const wns::ldk::CommandPool* commandPool, Bit& _pciSize, Bit& _pduSize) const = 0;
// 
// /* deleted by chen */
// //       protected:
// //     wns::logger::Logger logger;
// //     dll::ILayer2* layer2;
// //     struct {
// //       lte::macr::PhyUser* phyUser;
// //     } friends;
// // 
// //       private:
// //     int commandSize;
// //     std::string phyUserName;
//       };
// 
//       class BCHUnitRAP :
//     public BCHUnit,
//     public wns::Cloneable<BCHUnitRAP>,
//     public IBCHTimingTx
//       {
// /* deleted by chen */
// //       public:
// //     BCHUnitRAP(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// // 
// //     virtual
// //     ~BCHUnitRAP();
// 
// //     //
// //     // compound handler interface
// //     //
// //     virtual void
// //     doSendData(const wns::ldk::CompoundPtr& /* compound */);
// // 
// //     virtual void
// //     doOnData(const wns::ldk::CompoundPtr& /* compound */);
// // 
// //     //
// //     // For external Triggering
// //     //
// //     /** @brief called by event StartBCH */
// //     void sendBCH(simTimeType duration);
// 
// /* inserted by chen */
//       public:
// 
//     //
//     // compound handler interface
//     //
//     virtual void
//     doSendData(const wns::ldk::CompoundPtr& /* compound */) = 0;
// 
//     virtual void
//     doOnData(const wns::ldk::CompoundPtr& /* compound */) = 0;
// 
//     //
//     // For external Triggering
//     //
//     /** @brief called by event StartBCH */
//     virtual void
//     sendBCH(simTimeType duration) = 0;
// 
// /* deleted by chen */
// //       private:
// //     virtual bool
// //     doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;
// // 
// //     virtual void
// //     doWakeup();
// // 
// //     int preambleSubBand;
// //     /** @brief modulation&coding used to transmit BCH */
// //     wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;
// //     /** @brief power used to transmit BCH */
// //     wns::Power txPower;
// // 
// //     bool accepting;
// //     wns::ldk::CompoundPtr compound;
//       };
// 
//       class BCHUnitUT :
//     public BCHUnit,
//     public wns::Cloneable<BCHUnitUT>
//       {
// /* deleted by chen */
// //       public:
// //     BCHUnitUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// // 
// //     virtual
// //     ~BCHUnitUT();
// 
// //     //
// //     // compound handler interface
// //     //
// //     virtual void
// //     doSendData(const wns::ldk::CompoundPtr& /* compound */);
// // 
// //     virtual void
// //     doOnData(const wns::ldk::CompoundPtr& /* compound */);
// // 
// //     virtual void onFUNCreated();
// 
// /* inserted by chen */
//       public:
// 
//     //
//     // compound handler interface
//     //
//     virtual void
//     doSendData(const wns::ldk::CompoundPtr& /* compound */) = 0;
// 
//     virtual void
//     doOnData(const wns::ldk::CompoundPtr& /* compound */) = 0;
// 
//     virtual void
//     onFUNCreated() = 0;
// 
// /* deleted by chen */
// //       private:
// //     virtual bool
// //     doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;
// // 
// //     virtual void
// //     doWakeup();
// // 
// //     lte::controlplane::bch::BCHService* bchService;
// //     dll::services::control::Association* associationService;
// // 
// //     const wns::pyconfig::View config;
// // 
// //     wns::probe::bus::ContextCollector* sinrProbe;
// //     wns::probe::bus::ContextCollector* rxpProbe;
// //     wns::probe::bus::ContextCollector* interfProbe;
// 
//       };
// 
//       class NoBCH :
//     virtual public wns::ldk::FunctionalUnit,
//     public wns::ldk::CommandTypeSpecifier<>,
//     public wns::ldk::HasReceptor<>,
//     public wns::ldk::HasConnector<>,
//     public wns::ldk::HasDeliverer<>,
//     public wns::ldk::Forwarding<NoBCH>,
//     public wns::Cloneable<NoBCH>
//       {
//       public:
// /* deleted by chen */
// //     NoBCH(wns::ldk::fun::FUN* fun, const wns::pyconfig::View&) :
// //       wns::ldk::CommandTypeSpecifier<>(fun),
// //       wns::ldk::HasReceptor<>(),
// //       wns::ldk::HasConnector<>(),
// //       wns::ldk::HasDeliverer<>(),
// //       wns::ldk::Forwarding<NoBCH>(),
// //       wns::Cloneable<NoBCH>()
// //     {};
// 
//       };

    }}}

#endif // NOT defined LTE_CONTROLPLANE_BCH_BCHUNITINTERFACE_HPP
