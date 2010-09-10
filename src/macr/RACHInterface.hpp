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

#ifndef LTE_MACR_RACHINTERFACE_HPP
#define LTE_MACR_RACHINTERFACE_HPP

#include <LTE/helper/HasModeName.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/PowerRatio.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/ldk/ShortcutFU.hpp>
#include <WNS/ldk/fu/Plain.hpp>
#include <WNS/service/dll/Address.hpp>

/* deleted by chen */
// namespace dll { namespace services { namespace control {
// 	class Association;
// }}}

namespace lte {
/* deleted by chen */
//     namespace controlplane {
//     	class RRHandlerUT;
// 	    namespace bch {
// 		    class BCHService;
// 	    }
//     }
    namespace macr {
// 	class PhyUser;

    class IRachTimingTx
    {
    public:
       /** @brief Trigger method to start RACH Tx phase */
        virtual void
        startTx(simTimeType duration) = 0;

/* deleted by chen */
        /** @brief Trigger method to stop RACH Tx phase */
//         virtual void
//         stopTx() = 0;
    };

    class IRachTimingRx
    {
    public:
       /** @brief Trigger method to start RACH Rx phase */
        virtual void
        startRx(simTimeType duration) = 0;

/* deleted by chen */
        /** @brief Trigger method to stop RACH Rx phase */
//         virtual void
//         stopRx() = 0;
    };

/* deleted by chen */
	/** @brief Functional Unit that sends Compounds to the PhyUser during
	 * the RACH phase */
// 	class RACH :
// 		public virtual wns::ldk::FunctionalUnit,
// 		public wns::ldk::CommandTypeSpecifier<>,
// 		public wns::ldk::HasReceptor<>,
// 		public wns::ldk::HasConnector<>,
// 		public wns::ldk::HasDeliverer<>,
// 		public helper::HasModeName
// 	{
// /* deleted by chen */
// // 	public:
// // 		/** @brief Constructor to be used by FUNConfigCreator */
// // 		RACH(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// // 		/** @brief Destructor */
// // 		virtual ~RACH();
// // 
// // 		/** @brief Resolve Intra-FUN dependencies after the component
// // 		 * was created */
// // 
// // 		virtual void
// // 		onFUNCreated();
// 
// /* inserted by chen */
//     public:
// 
//         /** @brief Resolve Intra-FUN dependencies after the component
//          * was created */
// 
//         virtual void
//         onFUNCreated() = 0;
// 
// /* deleted by chen */
// // 	protected:
// // 		/** @brief Contains pointers to the FUs on which the RACH has a
// // 		 *  dependency
// // 		 */
// // 		struct {
// // 			PhyUser* phyUser;
// // 		} friends;
// // 
// // 		/** @brief Association service access is needed to correctly
// // 		 * address the packets */
// // 		dll::services::control::Association* associationService;
// // 
// // 		/** @brief modulation&coding used to transmit RACH */
// // 		//wns::SmartPtr<wns::service::phy::phymode::PhyModeInterface> phyMode;
// // 		wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr;
// // 
// // 		/** @brief my Logger */
// // 		wns::logger::Logger logger;
// 
// 	};
// 
// 	class RACHUT :
// 		public RACH,
//         public IRachTimingTx,
// 		public wns::Cloneable<RACHUT>
// 	{
// /* deleted by chen */
// 		/** @brief Event triggering the end of a RACH phase */
// // 		class StopEvent
// // 		{
// // 			RACHUT* rach;
// // 		public:
// // 			StopEvent(RACHUT* _rach) :
// // 				rach(_rach)
// // 			{
// // 			}
// // 
// // 			virtual
// // 			~StopEvent()
// // 			{
// // 			}
// // 
// // 			virtual void
// // 			operator()()
// // 			{
// // 				rach->stopTx();
// // 			}
// // 		};
// // 
// // 		/** @name CompoundHandlerInterface */
// // 		//@{
// // 		virtual bool
// // 		doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;
// // 
// // 		virtual void
// // 		doWakeup();
// // 		//@}
// // 
// // 		/** @brief */
// // 		lte::controlplane::RRHandlerUT* rrh;
// // 
// // 		lte::controlplane::bch::BCHService* bchService;
// // 
// // 		int subBandCounter;
// // 
// // 		bool accepting;
// // 		bool inWakeup;
// // 		simTimeType stopTime;
// // 		//lte::helper::PhyCapacity phyCapacity;
// // 
// // 		wns::Power txPower;
// 
// /* deleted by chen */
// // 	public:
// // 		RACHUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
// // 			RACH(fun, config),
// // 			wns::Cloneable<RACHUT>(),
// // 			rrh(NULL),
// // 			bchService(NULL),
// // 			subBandCounter(0),
// // 			accepting(false),
// // 			inWakeup(false),
// // 			stopTime(0.0),
// // 			//phyCapacity(config.getView("plm")),
// // 			txPower(config.get<wns::Power>("txPower"))
// // 		{}
// // 
// // 		virtual
// // 		~RACHUT(){}
// // 
// // 		/** @brief Resolve Intra-FUN dependencies after the component
// // 		 * was created */
// // 		virtual void
// // 		onFUNCreated();
// // 
// // 		/** @name CompoundHandlerInterface */
// // 		//@{
// // 		virtual void
// // 		doSendData(const wns::ldk::CompoundPtr&);
// // 		virtual void
// // 		doOnData(const wns::ldk::CompoundPtr&);
// // 		//@}
// // 
// // 
// // 		/** @brief Trigger method to start RACH Tx phase */
// // 		void
// // 		startTx(simTimeType duration);
// // 
// // 		/** @brief Trigger method to stop RACH Tx phase */
// // 		void
// // 		stopTx();
// 
// /* inserted by chen */
//     public:
// 
//         /** @brief Resolve Intra-FUN dependencies after the component
//          * was created */
//         virtual void
//         onFUNCreated() = 0;
// 
//         /** @name CompoundHandlerInterface */
//         //@{
//         virtual void
//         doSendData(const wns::ldk::CompoundPtr&) = 0;
// 
//         virtual void
//         doOnData(const wns::ldk::CompoundPtr&) = 0;
//         //@}
// 
//         /** @brief Trigger method to start RACH Tx phase */
//         virtual void
//         startTx(simTimeType duration) = 0;
// 
//         /** @brief Trigger method to stop RACH Tx phase */
//         virtual void
//         stopTx() = 0;
// 	};
// 
// 	class RACHBS :
// 		public RACH,
//         public IRachTimingRx,
// 		public wns::Cloneable<RACHBS>
// 	{
// /* deleted by chen */
// 		/** @name CompoundHandlerInterface */
// // 		//@{
// // 		virtual bool
// // 		doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;
// // 
// // 		virtual void
// // 		doWakeup();
// // 		//@}
// 
// /* deleted by chen */
// // 	public:
// // 		RACHBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
// // 			RACH(fun, config),
// // 			wns::Cloneable<RACHBS>()
// // 		{}
// // 
// // 		virtual
// // 		~RACHBS(){}
// // 
// // 		/** @brief Resolve Intra-FUN dependencies after the component
// // 		 * was created */
// // // 		virtual void
// // // 		onFUNCreated();
// // 
// // 		/** @name CompoundHandlerInterface */
// // 		//@{
// // 		virtual void
// // 		doSendData(const wns::ldk::CompoundPtr&);
// // 		virtual void
// // 		doOnData(const wns::ldk::CompoundPtr&);
// // 		//@}
// // 
// // 
// // 		/** @brief Trigger method to start RACH Rx phase */
// // 		void
// // 		startRx(simTimeType duration);
// // 
// //         /** @brief Trigger method to start RACH Rx phase */
// //         void
// //         stopRx();
// 
// /* inserted by chen */
//     public:
// 
//         /** @brief Resolve Intra-FUN dependencies after the component
//          * was created */
// //      virtual void
// //      onFUNCreated();
// 
//         /** @name CompoundHandlerInterface */
//         //@{
//         virtual void
//         doSendData(const wns::ldk::CompoundPtr&) = 0;
// 
//         virtual void
//         doOnData(const wns::ldk::CompoundPtr&) = 0;
//         //@}
// 
//         /** @brief Trigger method to start RACH Rx phase */
//         virtual void
//         startRx(simTimeType duration) = 0;
// 
//         /** @brief Trigger method to start RACH Rx phase */
//         virtual void
//         stopRx() = 0;
// 	};
// 
//     class RACHShortcut:
//         public wns::ldk::ShortcutFU<wns::service::dll::UnicastAddress, RACHShortcut*>,
//         public wns::ldk::CommandTypeSpecifier<>,
//         public wns::ldk::HasReceptor<>,
//         public wns::ldk::HasConnector<>,
//         public wns::ldk::HasDeliverer<>
//     {
// /* deleted by chen */
// //     public:
// //         RACHShortcut(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// // 
// //         virtual ~RACHShortcut();
// // 
// //         virtual wns::service::dll::UnicastAddress
// //         getSourceAddress();
// // 
// //         virtual wns::service::dll::UnicastAddress
// //         getDestinationAddress(const wns::ldk::CompoundPtr&);
// // 
// //         virtual bool
// //         isBroadcast(const wns::ldk::CompoundPtr&);
// 
// /* inserted by chen */
//     public:
// 
//         virtual wns::service::dll::UnicastAddress
//         getSourceAddress() = 0;
// 
//         virtual wns::service::dll::UnicastAddress
//         getDestinationAddress(const wns::ldk::CompoundPtr&) = 0;
// 
//         virtual bool
//         isBroadcast(const wns::ldk::CompoundPtr&) = 0;
//     };
// 
//     class RACHShortcutBS:
//         public RACHShortcut,
//         public wns::Cloneable<RACHShortcutBS>
//     {
// /* deleted by chen */
// //     public:
// //         RACHShortcutBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// 
// //         virtual ~RACHShortcutBS() {}
// 
// //         virtual bool
// //         isReceiver();
// 
// /* inserted by chen */
//     public:
// 
//         virtual bool
//         isReceiver() = 0;
//     };
// 
//     class RACHShortcutUT:
//         public RACHShortcut,
//         public wns::Cloneable<RACHShortcutUT>
//     {
// /* deleted by chen */
// //     public:
// //         RACHShortcutUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// 
// //         virtual ~RACHShortcutUT() {}
// 
// //         virtual bool
// //         isReceiver();
// 
// /* inserted by chen */
//     public:
// 
//         virtual bool
//         isReceiver() = 0;
//     };

    } // macr
} // lte

#endif // LTE_MACR_RACHINTERFACE_HPP


