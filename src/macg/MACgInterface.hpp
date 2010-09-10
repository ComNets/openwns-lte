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

#ifndef LTE_MACG_MACGINTERFACE_HPP
#define LTE_MACG_MACGINTERFACE_HPP

/* deleted by chen */
// #include <LTE/main/Layer2.hpp>
// #include <LTE/macr/ScorerInterface.hpp>
/* inserted by chen */
#include <LTE/helper/Route.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/Forwarding.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/service/dll/Address.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/Enum.hpp>

#include <set>

namespace lte {

/* deleted by chen */
/* inserted by chen */
//     namespace macr {
//         class ScorerInterface;
//     }

	namespace macg {

/* deleted by chen */
// 	namespace modeselection {
// 		class Strategy;
// 	}

/* deleted by chen */
// 	class MACg;
// 	class Best;

/* deleted by chen */
// 	typedef std::vector<lte::macr::ScorerInterface*> ScorerContainer;
// 	typedef std::vector<wns::ldk::IConnectorReceptacle*> LayerContainer;

/* deleted by chen */
// 	class MACgCommand :
// 		public wns::ldk::Command
/* inserted by chen */
    class MACgCommand

	{
	public:
/* deleted by chen */
// 		MACgCommand()
// 		{
// 			local.modeID = -1;
// 			local.modeName = "";
// 			peer.source = wns::service::dll::UnicastAddress();
// 			peer.dest   = wns::service::dll::UnicastAddress();
// 			magic.hopCount = 1;
// 		}

		struct {
			int modeID;
			std::string modeName;
		} local;
		struct {
			wns::service::dll::UnicastAddress source;
			wns::service::dll::UnicastAddress dest; // next hop address
		} peer;
		struct {
			unsigned int hopCount;
		} magic;
	}; // MACgCommand

/* deleted by chen */
// 	class MACgScheduler :
// 		public wns::ldk::Connector
// 	{
// /* deleted by chen */
// // 	public:
// // 		/** @brief Default Constructor */
// // 		MACgScheduler() :
// // 			macg(NULL),
// // 			logger(NULL){};
// // 
// // 		virtual ~MACgScheduler();
// // 
// // 		virtual unsigned long int size() const;
// // 
// // 		virtual lte::helper::Route getRoute(const wns::ldk::CompoundPtr& compound);
// // 
// // 		void addDestination(lte::macr::ScorerInterface* scorer);
// // 
// // 		virtual void setStrategy(lte::macg::modeselection::Strategy* strategy);
// // 
// // 		// Link Interface
// // 		virtual void add(wns::ldk::IConnectorReceptacle*);
// // 		virtual void clear(){assure(false, "clear not supported.");};
// //             virtual const wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer get() const
// //             {
// //                 assure(false, "Exchange not supported.");
// //                 return wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer();
// //             }
// // 
// //             virtual void set(const wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer&)
// //             {
// //                 assure(false, "set not supported.");
// //             }
// // 
// // 		/** @brief late Logger instantiation.
// // 		 * Due to the need for a default Constructor,
// // 		 * we can not instantiate the Logger immediately  */
// // 		void setLogger(const wns::pyconfig::View& config);
// 
// /* inserted by chen */
//     public:
//         /** @brief Default Constructor */
// 
//         virtual unsigned long int
//         size() const = 0;
// 
//         virtual lte::helper::Route
//         getRoute(const wns::ldk::CompoundPtr& compound) = 0;
// 
//         virtual void
//         addDestination(lte::macr::ScorerInterface* scorer) = 0;
// 
//         virtual void
//         setStrategy(lte::macg::modeselection::Strategy* strategy) = 0;
// 
//         // Link Interface
//         virtual void
//         add(wns::ldk::IConnectorReceptacle*) = 0;
// 
//         virtual void
//         clear() = 0;
// 
//         virtual const
//         wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer get() const = 0;
// 
//         virtual void
//         set(const wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer&) = 0;
// 
//         /** @brief late Logger instantiation.
//          * Due to the need for a default Constructor,
//          * we can not instantiate the Logger immediately  */
//         virtual void
//         setLogger(const wns::pyconfig::View& config) = 0;
// 
// /* deleted by chen */
// // 	protected:
// // 		// The FUs which are conneted with the connector set of MACg
// // 		LayerContainer layers;
// // 		// To manage the (several) Scorers in the different modes
// // 		ScorerContainer scorers;
// // 
// // 		// coupling the MACg with the MACgScheduler(below)
// // 		void setMACg(MACg* _macg);
// // 		MACg* macg;
// // 		lte::macg::modeselection::Strategy* strategy;
// // 
// // 		// Logger Instance
// // 		wns::logger::Logger* logger;
// // 	private:
// 
// 	}; // MACgScheduler
// 
// 	class MACg :
// 		virtual public wns::ldk::FunctionalUnit,
// 		public wns::ldk::CommandTypeSpecifier<MACgCommand>
// 	{
// /* deleted by chen */
// // 	public:
// // 		MACg(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
// // 		virtual ~MACg() {}
// // 
// // 		virtual void onFUNCreated();
// // 		virtual wns::ldk::CommandPool*	createReply(const wns::ldk::CommandPool* original) const;
// 
// /* inserted by chen */
//     public:
// 
//         virtual void
//         onFUNCreated() = 0;
// 
//         virtual wns::ldk::CommandPool*
//         createReply(const wns::ldk::CommandPool* original) const = 0;
// 
// /* deleted by chen */
// // 	protected:
// // 		wns::ldk::fun::FUN* fun;
// // 
// // 	private:
// // 		wns::pyconfig::View config;
// // 		wns::logger::Logger logger;
// 	}; // MACg

}}

#endif // NOT defined LTE_MACG_MACGINTERFACE_HPP


