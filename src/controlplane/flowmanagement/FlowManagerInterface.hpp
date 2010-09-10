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

#ifndef LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWMANAGERINTERFACE_HPP
#define LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWMANAGERINTERFACE_HPP

#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WNS/ldk/FlowGate.hpp>
#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/service/tl/FlowID.hpp>
#include <WNS/distribution/Distribution.hpp>
#include <WNS/container/Pool.hpp>

#include <DLL/StationManager.hpp>

#include <LTE/helper/TransactionID.hpp>

#include <map>

namespace lte {
/* deleted by chen */
//   namespace upperconvergence {
//     class UTUpperConvergence;
//     class BSUpperConvergence;
//   }
  namespace controlplane {
//     class AssociationsProxyUT;
//     class AssociationsProxyRN;
//     class AssociationsProxyBS;

    namespace flowmanagement {

/* deleted by chen */
//       namespace flowhandler{
// 	class FlowHandlerBS;
// 	class FlowHandlerUT;
// 	class FlowHandlerRN;
//       }

/* deleted by chen */
//       class FlowManager :
// 	public wns::ldk::ControlService,
// 	virtual public wns::ldk::flowseparator::FlowInfoProvider
/* inserted by chen */
      class FlowManager
      {
/* deleted by chen */
//       public:
// 	typedef std::string ModeName;
// 	typedef std::list<wns::service::dll::FlowID> FlowIdList;
// 	typedef std::map<wns::service::qos::QoSClass,
// 			 wns::service::dll::FlowID> ControlPlaneFlowIDs;
// 	typedef wns::container::Registry<wns::service::dll::FlowID,
// 					 wns::service::dll::FlowID> SwitchingTable;
// 	typedef wns::container::Registry<wns::service::dll::FlowID,
// 					 wns::service::dll::UnicastAddress> FlowIDTable;
// 	typedef wns::container::Registry<wns::service::dll::FlowID,
// 					 wns::service::qos::QoSClass> FlowIDToQoSTable;
// 	typedef wns::container::Registry<wns::service::dll::UnicastAddress,
// 					 ControlPlaneFlowIDs> ControlPlaneFlowIdTable;
// 
// /* deleted by chen */
// // 	FlowManager(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// // 
// // 	virtual
// //     ~FlowManager();
// 
// // 	virtual wns::service::qos::QoSClass
// // 	getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// // 	virtual wns::service::qos::QoSClass
// // 	getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// // 
// // 	virtual	ControlPlaneFlowIDs
// // 	getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress) = 0;
// // 	virtual void
// // 	setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs) = 0;
// // 	virtual std::string
// // 	getFlowTable() const;
// // 
// // 	virtual std::string
// // 	printControlPlaneFlowIDs(ControlPlaneFlowIDs flowIDs) const;
// // 
// // 	virtual bool
// // 	isControlPlaneFlowID(wns::service::dll::FlowID flowID,
// // 			     ControlPlaneFlowIDs flowIDs) const;
// // 
// // 	virtual bool
// // 	isControlPlaneFlowID(wns::service::dll::UnicastAddress peerAddress, wns::service::dll::FlowID flowID) const;
// // 
// // 	// call FM to build ILayer2 Flow for TL-FlowID
// // 	virtual void
// // 	buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass){};
// // 
// // 	virtual void
// // 	flowBuilt(lte::helper::TransactionID /*transactionID*/){};
// // 
// //     wns::service::dll::FlowID
// //     getBCHFlowID();
// // 
// // 	// get the FlowIDs from the switching table
// // 	wns::service::dll::FlowID
// // 	getFlowIDin(wns::service::dll::FlowID flowIDout);
// // 
// // 	wns::service::dll::FlowID
// // 	getFlowIDout(wns::service::dll::FlowID flowIDin);
// // 
// // 	bool
// // 	hasFlowIDout(wns::service::dll::FlowID flowIDout);
// // 
// // 	bool
// // 	isAwaitingAck(wns::service::dll::FlowID _flowID);
// // 
// // 	void
// // 	deleteAllUpperFlows(wns::service::dll::UnicastAddress bsAdr);
// // 
// // 	void
// // 	closeUpperFlows(wns::service::dll::UnicastAddress userAdr);
// // 
// // 	void
// // 	closeUpperFlow(wns::service::dll::FlowID flowID);
// // 
// // 	void
// // 	deleteAllFlowSeparators(wns::service::dll::UnicastAddress utAddress);
// // 
// // 	void
// // 	openUpperFlow(wns::service::dll::FlowID flowID);
// // 
// // 	void
// // 	deleteFlowSeparator(wns::service::dll::FlowID flowID);
// // 
// // 	/** @brief called by FlowHandler */
// // 	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// // 	/** @brief called by FlowHandler */
// // 	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// // 
// // 	wns::container::Registry<lte::helper::TransactionID,
// // 				 wns::service::tl::FlowID> TransactionIDToTlFlowID;
// // 
// // 	wns::container::Registry<lte::helper::TransactionID,
// // 				 wns::service::dll::FlowID> TransactionIDToOldFlowID;
// // 
// // 	wns::container::Registry<wns::service::tl::FlowID,
// // 				 wns::service::dll::FlowID> TlFlowIDToDllFlowID;
// // 
// // 	wns::container::Registry<lte::helper::TransactionID,
// // 				 wns::service::qos::QoSClass> TransactionIDToQoSClass;
// // 
// // 	wns::container::Registry<wns::service::dll::FlowID,
// // 				 wns::service::tl::FlowID> DllFlowIDToTlFlowID;
// // 
// // 	//maps outgoing transactionID to the incoming one (uplink direction)
// // 	wns::container::Registry<lte::helper::TransactionID,
// // 				 lte::helper::TransactionID> TransactionIdOutToIn;
// // 
// // 	//maps outgoing transactionID to FlowHandler which has requested the flowID
// // 	wns::container::Registry<lte::helper::TransactionID,
// // 				 lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS*> TransactionIdOutToFlowHandler;
// // 
// // 	ControlPlaneFlowIdTable controlPlaneFlowIdsForPeer;
// // 
// // 	// Switching table: maps incoming FlowID to outgoing FlowID
// // 	SwitchingTable FlowIDInToFlowIDOut;
// // 	FlowIDTable FlowIDToUT;
// // 
// // 	/**@brief from flowInfoProvider */
// // 	virtual bool
// // 	isValidFlow(const wns::ldk::ConstKeyPtr& key) const;
// // 
// // 	virtual bool
// // 	isValidFlowId(wns::service::dll::FlowID flowID) const;
// // 
// // 	/**@brief return the number of registered flows */
// // 	virtual int
// // 	countFlows() const;
// // 
// // 	/**@brief return the number of registered flows to/from a user */
// // 	virtual int
// // 	countFlows(wns::service::dll::UnicastAddress utAddress) const;
// // 
// // 	void
// // 	insertFlowIDToUT(wns::service::dll::FlowID flowID, wns::service::dll::UnicastAddress utAdress);
// // 
// // 	virtual void
// // 	deleteFlowsForUT(wns::service::dll::UnicastAddress utAddress);
// // 
// // 	virtual wns::service::dll::UnicastAddress
// // 	getUTForFlow(wns::service::dll::FlowID flowID);
// // 
// // 	virtual FlowIdList
// // 	getFlowsForUT(wns::service::dll::UnicastAddress utAddress);
// // 
// // 	virtual void
// // 	deleteSwitchingTableForUT(wns::service::dll::UnicastAddress utAddress);
// // 
// // 	bool
// // 	hasPreserved(wns::service::dll::UnicastAddress userAdr) const;
// // 
// // 	bool
// // 	hasPreserved(wns::service::dll::FlowID flowID) const;
// // 
// // 	void
// // 	registerPreservedUser(wns::service::dll::UnicastAddress userAdr);
// // 
// // 	void
// // 	registerPreservedFlowID(wns::service::dll::FlowID flowID);
// // 
// // 	void
// // 	deletePreservedUser(wns::service::dll::UnicastAddress userAdr);
// 
// /* inserted by chen */
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual ControlPlaneFlowIDs
//     getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress) = 0;
// 
//     virtual void
//     setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs) = 0;
// 
//     virtual std::string
//     getFlowTable() const = 0;
// 
//     virtual std::string
//     printControlPlaneFlowIDs(ControlPlaneFlowIDs flowIDs) const = 0;
// 
//     virtual bool
//     isControlPlaneFlowID(wns::service::dll::FlowID flowID,
//                          ControlPlaneFlowIDs flowIDs) const = 0;
// 
//     virtual bool
//     isControlPlaneFlowID(wns::service::dll::UnicastAddress peerAddress, wns::service::dll::FlowID flowID) const = 0;
// 
//     // call FM to build ILayer2 Flow for TL-FlowID
//     virtual void
//     buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass) = 0;
// 
//     virtual void
//     flowBuilt(lte::helper::TransactionID /*transactionID*/) = 0;
// 
//     virtual wns::service::dll::FlowID
//     getBCHFlowID() = 0;
// 
//     // get the FlowIDs from the switching table
//     virtual wns::service::dll::FlowID
//     getFlowIDin(wns::service::dll::FlowID flowIDout) = 0;
// 
//     virtual wns::service::dll::FlowID
//     getFlowIDout(wns::service::dll::FlowID flowIDin) = 0;
// 
//     virtual bool
//     hasFlowIDout(wns::service::dll::FlowID flowIDout) = 0;
// 
//     virtual bool
//     isAwaitingAck(wns::service::dll::FlowID _flowID) = 0;
// 
//     virtual void
//     deleteAllUpperFlows(wns::service::dll::UnicastAddress bsAdr) = 0;
// 
//     virtual void
//     closeUpperFlows(wns::service::dll::UnicastAddress userAdr) = 0;
// 
//     virtual void
//     closeUpperFlow(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     deleteAllFlowSeparators(wns::service::dll::UnicastAddress utAddress) = 0;
// 
//     virtual void
//     openUpperFlow(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     deleteFlowSeparator(wns::service::dll::FlowID flowID) = 0;
// 
//     /** @brief called by FlowHandler */
//     virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// 
//     /** @brief called by FlowHandler */
//     virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// 
//     wns::container::Registry<lte::helper::TransactionID,
//     wns::service::tl::FlowID> TransactionIDToTlFlowID;
// 
//     wns::container::Registry<lte::helper::TransactionID,
//     wns::service::dll::FlowID> TransactionIDToOldFlowID;
// 
//     wns::container::Registry<wns::service::tl::FlowID,
//     wns::service::dll::FlowID> TlFlowIDToDllFlowID;
// 
//     wns::container::Registry<lte::helper::TransactionID,
//     wns::service::qos::QoSClass> TransactionIDToQoSClass;
// 
//     wns::container::Registry<wns::service::dll::FlowID,
//     wns::service::tl::FlowID> DllFlowIDToTlFlowID;
// 
//     //maps outgoing transactionID to the incoming one (uplink direction)
//     wns::container::Registry<lte::helper::TransactionID,
//     lte::helper::TransactionID> TransactionIdOutToIn;
// 
//     //maps outgoing transactionID to FlowHandler which has requested the flowID
//     wns::container::Registry<lte::helper::TransactionID,
//     lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS*> TransactionIdOutToFlowHandler;
// 
//     ControlPlaneFlowIdTable controlPlaneFlowIdsForPeer;
// 
//     // Switching table: maps incoming FlowID to outgoing FlowID
//     SwitchingTable FlowIDInToFlowIDOut;
//     FlowIDTable FlowIDToUT;
// 
//     /**@brief from flowInfoProvider */
//     virtual bool
//     isValidFlow(const wns::ldk::ConstKeyPtr& key) const = 0;
// 
//     virtual bool
//     isValidFlowId(wns::service::dll::FlowID flowID) const = 0;
// 
//     /**@brief return the number of registered flows */
//     virtual int
//     countFlows() const = 0;
// 
//     /**@brief return the number of registered flows to/from a user */
//     virtual int
//     countFlows(wns::service::dll::UnicastAddress utAddress) const = 0;
// 
//     virtual void
//     insertFlowIDToUT(wns::service::dll::FlowID flowID, wns::service::dll::UnicastAddress utAdress) = 0;
// 
//     virtual void
//     deleteFlowsForUT(wns::service::dll::UnicastAddress utAddress) = 0;
// 
//     virtual wns::service::dll::UnicastAddress
//     getUTForFlow(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual FlowIdList
//     getFlowsForUT(wns::service::dll::UnicastAddress utAddress) = 0;
// 
//     virtual void
//     deleteSwitchingTableForUT(wns::service::dll::UnicastAddress utAddress) = 0;
// 
//     virtual bool
//     hasPreserved(wns::service::dll::UnicastAddress userAdr) const = 0;
// 
//     virtual bool
//     hasPreserved(wns::service::dll::FlowID flowID) const = 0;
// 
//     virtual void
//     registerPreservedUser(wns::service::dll::UnicastAddress userAdr) = 0;
// 
//     virtual void
//     registerPreservedFlowID(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     deletePreservedUser(wns::service::dll::UnicastAddress userAdr) = 0;
// 
// /* deleted by chen */
// //       protected:
// // 	lte::helper::TransactionID drawNewTransactionID() const;
// // 
// // 	dll::ILayer2* layer2;
// // 	bool plainDisassociation;
// // 	wns::ldk::CommandReaderInterface* rlcReader;
// // 	wns::distribution::Distribution* transactionIdDistribution;
// // 	wns::logger::Logger logger;
// // 	wns::container::Pool<int> flowIDPool;
// //     wns::container::Pool<int> broadcastFlowIDPool;
// // 	std::string separator;
// // 	std::list<ModeName> myModes;
// // 	std::list<std::string> flowSeparatorNames;
// // 	std::list<wns::ldk::FlowSeparator*> flowSeparators;
// // 	wns::ldk::tools::Synchronizer* upperSynchronizer;
// // 	wns::ldk::FlowGate* upperFlowGate;
// // 	wns::ldk::FlowGate* arqFlowGate;
// // 	std::set<wns::service::dll::UnicastAddress> preservedUsers;
// // 	std::set<wns::service::dll::FlowID> preservedFlowIDs;
// //     wns::service::dll::FlowID bchFlowID_;
// //       private:
// // 	wns::service::dll::FlowID dllflowID;
      };

      class FlowManagerBS :
	public FlowManager
      {
/* deleted by chen */
//       public:
// 	FlowManagerBS(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// 
// 	virtual
// 	~FlowManagerBS();

// 	virtual wns::service::qos::QoSClass
// 	getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const;
// 	virtual wns::service::qos::QoSClass
// 	getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const;
// 	virtual void
// 	setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs);
// 	virtual std::string
// 	getFlowTable() const;
// 
// 	virtual void
// 	buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass);
// 
// 	virtual void
// 	flowBuilt(wns::service::dll::FlowID flowID);
// 
// 	virtual	ControlPlaneFlowIDs
// 	getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress);
// 
// 	void
// 	forwardFlowRequest(lte::helper::TransactionID _transactionID,
// 			   lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* _flowHandlerBS,
// 			   wns::service::dll::UnicastAddress utAddress,
// 			   wns::service::dll::FlowID oldFlowID,
// 			   wns::service::qos::QoSClass qosClass);
// 
// 	void
// 	registerFlowID(lte::helper::TransactionID _transactionId,
// 		       wns::service::dll::FlowID _flowIDout,
// 		       wns::service::dll::UnicastAddress utAddress);
// 
// 	void
// 	releaseFlow(wns::service::dll::FlowID flowID);
// 
// 	virtual void
// 	onCSRCreated();
// 
// 	void
// 	onFlowReleaseAck(wns::service::dll::FlowID flowIDout);
// 
// 
// 	void
// 	deleteFlowsInRang(wns::service::dll::UnicastAddress userAdr);
// 
// 	void
// 	deleteLowerFlow(wns::service::dll::FlowID flowID);
// 
// 	void
// 	deleteAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode);
// 
// 	void
// 	onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, bool preserved);
// 
// 	void
// 	preserveFlows(wns::service::dll::UnicastAddress userAdr);
// 
// 	/** @brief AssociationObserver interface */
// 	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
// 	/** @brief AssociationObserver interface */
// 	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);

/* deleted by chen */
/* inserted by chen */
//       public:
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual void
//     setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs) = 0;
// 
//     virtual std::string
//     getFlowTable() const = 0;
// 
//     virtual void
//     buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass) = 0;
// 
//     virtual void
//     flowBuilt(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual ControlPlaneFlowIDs
//     getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress) = 0;
// 
//     virtual void
//     forwardFlowRequest(lte::helper::TransactionID _transactionID,
//                        lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* _flowHandlerBS,
//                        wns::service::dll::UnicastAddress utAddress,
//                        wns::service::dll::FlowID oldFlowID,
//                        wns::service::qos::QoSClass qosClass) = 0;
// 
//     virtual void
//     registerFlowID(lte::helper::TransactionID _transactionId,
//                    wns::service::dll::FlowID _flowIDout,
//                    wns::service::dll::UnicastAddress utAddress) = 0;
// 
//     virtual void
//     releaseFlow(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     onCSRCreated() = 0;
// 
//     virtual void
//     onFlowReleaseAck(wns::service::dll::FlowID flowIDout) = 0;
// 
//     virtual void
//     deleteFlowsInRang(wns::service::dll::UnicastAddress userAdr) = 0;
// 
//     virtual void
//     deleteLowerFlow(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     deleteAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode) = 0;
// 
//     virtual void
//     onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, bool preserved) = 0;
// 
//     virtual void
//     preserveFlows(wns::service::dll::UnicastAddress userAdr) = 0;
// 
//     /** @brief AssociationObserver interface */
//     virtual void
//     onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// 
//     /** @brief AssociationObserver interface */
//     virtual void
//     onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;

/* deleted by chen */
//       private:
// 	lte::upperconvergence::BSUpperConvergence* bsUpperConvergence;
// 	lte::controlplane::AssociationsProxyBS* aProxyBS;
// 
// 	wns::container::Registry<wns::service::dll::FlowID,
// 				 wns::service::qos::QoSClass> DllFlowIDToQoSClass;
      };

/* deleted by chen */
//       class FlowManagerUT :
// 	public FlowManager
//       {
//       public:
// 	FlowManagerUT(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// 
// 	virtual
// 	~FlowManagerUT();

// 	virtual wns::service::qos::QoSClass
// 	getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const;
// 	virtual wns::service::qos::QoSClass
// 	getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const;
// 	virtual void
// 	setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs);
// 	virtual std::string
// 	getFlowTable() const;
// 
// 	virtual void
// 	buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass);
// 
// 	virtual void
// 	flowBuilt(lte::helper::TransactionID _transactionID,  wns::service::dll::FlowID _dllFlowID);
// 
// 	virtual	ControlPlaneFlowIDs
// 	getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress);
// 
// 	void
// 	releaseFlow(wns::service::tl::FlowID flowID);
// 
// 	void
// 	flowReleased(wns::service::dll::FlowID flowID);
// 
// 	void
// 	getAssociations();
// 
// 	std::string
// 	selectMode();
// 
// 	virtual void
// 	onCSRCreated();
// 
// 	void
// 	onDisassociatedPerMode(wns::service::dll::UnicastAddress bsAdr, ModeName mode, bool preserved);
// 
// 	void
// 	onAssociatedPerMode(wns::service::dll::UnicastAddress rapAdr, bool preserved);
// 
// 	void
// 	disassociating(ModeName mode);
// 
// 	void
// 	onPlainDisassociation(ModeName mode);
// 
// 	void
// 	reBuildFlows(wns::service::dll::UnicastAddress bsAdr, bool preserved);
// 
// 	void
// 	deleteLowerFlow(wns::service::dll::FlowID flowID);
// 
// 	void
// 	deleteAllLowerFlows(ModeName mode);
// 
// 	void
// 	closeLowerFlows(ModeName mode);
// 
// 	/** @brief AssociationObserver interface */
// 	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
// 	/** @brief AssociationObserver interface */
// 	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);

/* deleted by chen */
/* inserted by chen */
//       public:
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual void
//     setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs) = 0;
// 
//     virtual std::string
//     getFlowTable() const = 0;
// 
//     virtual void
//     buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass) = 0;
// 
//     virtual void
//     flowBuilt(lte::helper::TransactionID _transactionID,  wns::service::dll::FlowID _dllFlowID) = 0;
// 
//     virtual ControlPlaneFlowIDs
//     getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress) = 0;
// 
//     virtual void
//     releaseFlow(wns::service::tl::FlowID flowID) = 0;
// 
//     virtual void
//     flowReleased(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     getAssociations() = 0;
// 
//     virtual std::string
//     selectMode() = 0;
// 
//     virtual void
//     onCSRCreated() = 0;
// 
//     virtual void
//     onDisassociatedPerMode(wns::service::dll::UnicastAddress bsAdr, ModeName mode, bool preserved) = 0;
// 
//     virtual void
//     onAssociatedPerMode(wns::service::dll::UnicastAddress rapAdr, bool preserved) = 0;
// 
//     virtual void
//     disassociating(ModeName mode) = 0;
// 
//     virtual void
//     onPlainDisassociation(ModeName mode) = 0;
// 
//     virtual void
//     reBuildFlows(wns::service::dll::UnicastAddress bsAdr, bool preserved) = 0;
// 
//     virtual void
//     deleteLowerFlow(wns::service::dll::FlowID flowID) = 0;
// 
//     virtual void
//     deleteAllLowerFlows(ModeName mode) = 0;
// 
//     virtual void
//     closeLowerFlows(ModeName mode) = 0;
// 
//     /** @brief AssociationObserver interface */
//     virtual void
//     onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// 
//     /** @brief AssociationObserver interface */
//     virtual void
//     onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;

/* deleted by chen */
//       private:
// 	void
// 	insertDroppedFlowIDs();
// 
// 
// 	FlowIDTable DroppedFlowIDs;
// 	wns::service::dll::FlowID l2FlowID;
// 	std::string selectedMode;
// 	lte::controlplane::AssociationsProxyUT* aProxyUT;
// 	lte::upperconvergence::UTUpperConvergence* utUpperConvergence;
// 	wns::container::Registry<wns::service::tl::FlowID,
// 				 lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*> TlFlowIDToFlowHandler;
// 	FlowIDToQoSTable DllFlowIDToQoSClass;
// 
//       };

/* deleted by chen */
//       class FlowManagerRN :
// 	public FlowManager
//       {
//       public:
// 	FlowManagerRN(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
// 
// 	virtual
//     ~FlowManagerRN();

// 	virtual wns::service::qos::QoSClass
// 	getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const;
// 	virtual wns::service::qos::QoSClass
// 	getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const;
// 
// 	virtual bool
// 	isValidFlow(const wns::ldk::ConstKeyPtr& key) const;
// 
// 	bool
// 	isValidFlowIdHop1(wns::service::dll::FlowID flowID) const;
// 
// 	virtual void
// 	setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs);
// 	virtual std::string
// 	getFlowTable() const;
// 
// 	void
// 	getAssociations();
// 
// 	virtual void
// 	onCSRCreated();
// 
// 	void
// 	onFlowRequest(lte::helper::TransactionID transActionIdIn,
// 		      lte::controlplane::flowmanagement::flowhandler::FlowHandlerRN* _flowHandlerRNBStask,
// 		      wns::service::dll::UnicastAddress utAddress,
// 		      wns::service::dll::FlowID oldFlowID,
// 		      wns::service::qos::QoSClass qosClass);
// 
// 	void
// 	onFlowConfirm(lte::helper::TransactionID transActionIdIn,
// 		      wns::service::dll::FlowID flowID,
// 		      wns::service::dll::UnicastAddress utAddress);
// 
// 	void
// 	onFlowAck(wns::service::dll::FlowID flowIDin,
// 		  wns::service::dll::UnicastAddress utAdress);
// 
// 	void
// 	onFlowReleaseReq(wns::service::dll::FlowID flowIDin);
// 
// 	void
// 	onFlowReleaseAck(wns::service::dll::FlowID flowIDout);
// 
// 	void
// 	onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task);
// 
// 	void
// 	closeAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task);
// 
// 	void
// 	onDisassociationAck(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task);
// 
// 	void
// 	deleteAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task);
// 
// 	virtual	ControlPlaneFlowIDs
// 	getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress);
// 
// 	/** @brief AssociationObserver interface */
// 	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
// 	/** @brief AssociationObserver interface */
// 	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);

/* deleted by chen */
/* inserted by chen */
//       public:
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual wns::service::qos::QoSClass
//     getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const = 0;
// 
//     virtual bool
//     isValidFlow(const wns::ldk::ConstKeyPtr& key) const = 0;
// 
//     virtual bool
//     isValidFlowIdHop1(wns::service::dll::FlowID flowID) const = 0;
// 
//     virtual void
//     setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs) = 0;
// 
//     virtual std::string
//     getFlowTable() const = 0;
// 
//     virtual void
//     getAssociations() = 0;
// 
//     virtual void
//     onCSRCreated() = 0;
// 
//     virtual void
//     onFlowRequest(lte::helper::TransactionID transActionIdIn,
//                   lte::controlplane::flowmanagement::flowhandler::FlowHandlerRN* _flowHandlerRNBStask,
//                   wns::service::dll::UnicastAddress utAddress,
//                   wns::service::dll::FlowID oldFlowID,
//                   wns::service::qos::QoSClass qosClass) = 0;
// 
//     virtual void
//     onFlowConfirm(lte::helper::TransactionID transActionIdIn,
//                   wns::service::dll::FlowID flowID,
//                   wns::service::dll::UnicastAddress utAddress) = 0;
// 
//     virtual void
//     onFlowAck(wns::service::dll::FlowID flowIDin,
//               wns::service::dll::UnicastAddress utAdress) = 0;
// 
//     virtual void
//     onFlowReleaseReq(wns::service::dll::FlowID flowIDin) = 0;
// 
//     virtual void
//     onFlowReleaseAck(wns::service::dll::FlowID flowIDout) = 0;
// 
//     virtual void
//     onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task) = 0;
// 
//     virtual void
//     closeAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task) = 0;
// 
//     virtual void
//     onDisassociationAck(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task) = 0;
// 
//     virtual void
//     deleteAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode, std::string task) = 0;
// 
//     virtual ControlPlaneFlowIDs
//     getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress) = 0;
// 
//     /** @brief AssociationObserver interface */
//     virtual void
//     onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
// 
//     /** @brief AssociationObserver interface */
//     virtual void
//     onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;

/* deleted by chen */
//       private:
// 	lte::controlplane::AssociationsProxyRN* associationsProxy;
// 	std::string uplinkModeName;
// 	bool flowIdSwitchingEnabled;
// 	wns::container::Registry<
// 	  lte::helper::TransactionID,
// 	  lte::controlplane::flowmanagement::flowhandler::FlowHandlerRN*>TransactionIdOutToFlowHandler;
// 
// 	wns::container::Registry<
// 	  wns::service::dll::FlowID,
// 	  lte::controlplane::flowmanagement::flowhandler::FlowHandlerRN*> FlowIDInToFlowHandler;
// 
// 	FlowIDToQoSTable DllFlowIDToQoSClassHop1;
// 	FlowIDToQoSTable DllFlowIDToQoSClassHop2;
//       };
    }
  }
}

#endif // NOT defined LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWMANAGERINTERFACE_HPP
