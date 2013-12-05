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

#ifndef LTE_CONTROLPLANE_FLOWMANAGER_HPP
#define LTE_CONTROLPLANE_FLOWMANAGER_HPP

#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>
#include <LTE/helper/TransactionID.hpp>

#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WNS/ldk/FlowGate.hpp>
#include <WNS/ldk/tools/Synchronizer.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/service/tl/FlowID.hpp>
#include <WNS/distribution/Distribution.hpp>
#include <WNS/container/Pool.hpp>

#include <DLL/StationManager.hpp>

namespace lte { namespace upperconvergence { class UEUpperConvergence; class ENBUpperConvergence; }}
namespace lte { namespace controlplane { class AssociationsProxyUT; class AssociationsProxyBS; }}
namespace lte { namespace controlplane { namespace flowmanagement { namespace flowhandler{ class FlowHandlerBS; class FlowHandlerUT; }}}}

namespace lte { namespace controlplane { namespace flowmanagement {
      class FlowManager :
    virtual public wns::ldk::ControlService,
	virtual public IFlowSwitching,
	virtual public wns::ldk::flowseparator::FlowInfoProvider
      {
      public:
	typedef std::list<wns::service::dll::FlowID> FlowIdList;

	typedef wns::container::Registry<wns::service::dll::FlowID,
					 wns::service::dll::FlowID> SwitchingTable;
	typedef wns::container::Registry<wns::service::dll::FlowID,
					 wns::service::dll::UnicastAddress> FlowIDTable;
	typedef wns::container::Registry<wns::service::dll::FlowID,
					 wns::service::qos::QoSClass> FlowIDToQoSTable;
	typedef wns::container::Registry<wns::service::dll::UnicastAddress,
					 ControlPlaneFlowIDs> ControlPlaneFlowIdTable;

	FlowManager(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);
	virtual ~FlowManager();

	virtual std::string
	getFlowTable() const;

	virtual std::string
	printControlPlaneFlowIDs(ControlPlaneFlowIDs flowIDs) const;

	virtual bool
	isControlPlaneFlowID(wns::service::dll::FlowID flowID,
			     ControlPlaneFlowIDs flowIDs) const;

	virtual bool
	isControlPlaneFlowID(wns::service::dll::UnicastAddress peerAddress, wns::service::dll::FlowID flowID) const;

	virtual void
	buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass){};

	virtual void
	flowBuilt(lte::helper::TransactionID) {};

	wns::service::dll::FlowID
	getBCHFlowID();

	virtual wns::service::dll::FlowID
	getFlowIDin(wns::service::dll::FlowID flowIDout);

	virtual wns::service::dll::FlowID
	getFlowIDout(wns::service::dll::FlowID flowIDin);

	bool
	hasFlowIDout(wns::service::dll::FlowID flowIDout);

	bool
	isAwaitingAck(wns::service::dll::FlowID _flowID);

	void
	deleteAllUpperFlows(wns::service::dll::UnicastAddress bsAdr);

	void
	closeUpperFlows(wns::service::dll::UnicastAddress userAdr);

	void
	closeUpperFlow(wns::service::dll::FlowID flowID);

	void
	deleteAllFlowSeparators(wns::service::dll::UnicastAddress utAddress);

	void
	openUpperFlow(wns::service::dll::FlowID flowID);

	void
	deleteFlowSeparator(wns::service::dll::FlowID flowID);

	/** @brief called by FlowHandler */
	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;
	/** @brief called by FlowHandler */
	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr) = 0;

	wns::container::Registry<lte::helper::TransactionID,
				 wns::service::tl::FlowID> TransactionIDToTlFlowID;

	wns::container::Registry<lte::helper::TransactionID,
				 wns::service::dll::FlowID> TransactionIDToOldFlowID;

	wns::container::Registry<wns::service::tl::FlowID,
				 wns::service::dll::FlowID> TlFlowIDToDllFlowID;

	wns::container::Registry<lte::helper::TransactionID,
				 wns::service::qos::QoSClass> TransactionIDToQoSClass;

	wns::container::Registry<wns::service::dll::FlowID,
				 wns::service::tl::FlowID> DllFlowIDToTlFlowID;

	//maps outgoing transactionID to the incoming one (uplink direction)
	wns::container::Registry<lte::helper::TransactionID,
				 lte::helper::TransactionID> TransactionIdOutToIn;

	//maps outgoing transactionID to FlowHandler which has requested the flowID
	wns::container::Registry<lte::helper::TransactionID,
				 lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS*> TransactionIdOutToFlowHandler;

	ControlPlaneFlowIdTable controlPlaneFlowIdsForPeer;

	// Switching table: maps incoming FlowID to outgoing FlowID
	SwitchingTable FlowIDInToFlowIDOut;
	FlowIDTable FlowIDToUT;

	/**@brief from flowInfoProvider */
	virtual bool
	isValidFlow(const wns::ldk::ConstKeyPtr& key) const;

	virtual bool
	isValidFlowId(wns::service::dll::FlowID flowID) const;

	/**@brief return the number of registered flows */
	virtual int
	countFlows() const;

	/**@brief return the number of registered flows to/from a user */
	virtual int
	countFlows(wns::service::dll::UnicastAddress utAddress) const;

	void
	insertFlowIDToUT(wns::service::dll::FlowID flowID, wns::service::dll::UnicastAddress utAdress);

	virtual void
	deleteFlowsForUT(wns::service::dll::UnicastAddress utAddress);

	virtual wns::service::dll::UnicastAddress
	getUTForFlow(wns::service::dll::FlowID flowID);

	virtual FlowIdList
	getFlowsForUT(wns::service::dll::UnicastAddress utAddress);

	virtual void
	deleteSwitchingTableForUT(wns::service::dll::UnicastAddress utAddress);

	bool
	hasPreserved(wns::service::dll::UnicastAddress userAdr) const;

	bool
	hasPreserved(wns::service::dll::FlowID flowID) const;

	void
	registerPreservedUser(wns::service::dll::UnicastAddress userAdr);

	void
	registerPreservedFlowID(wns::service::dll::FlowID flowID);

	void
	deletePreservedUser(wns::service::dll::UnicastAddress userAdr);

      protected:
	lte::helper::TransactionID drawNewTransactionID() const;

	dll::ILayer2* layer2;
	bool plainDisassociation;
	wns::ldk::CommandReaderInterface* rlcReader;
	wns::distribution::Distribution* transactionIdDistribution;
	wns::logger::Logger logger;
	wns::container::Pool<int> flowIDPool;
	wns::container::Pool<int> broadcastFlowIDPool;
	std::string separator;
	std::list<IFlowSwitching::ModeName> myModes;
	std::list<std::string> flowSeparatorNames;
	std::list<wns::ldk::FlowSeparator*> flowSeparators;
	wns::ldk::tools::Synchronizer* upperSynchronizer;
	wns::ldk::FlowGate* upperFlowGate;
	wns::ldk::FlowGate* arqFlowGate;
	std::set<wns::service::dll::UnicastAddress> preservedUsers;
	std::set<wns::service::dll::FlowID> preservedFlowIDs;
    wns::service::dll::FlowID bchFlowID_;
      private:
	wns::service::dll::FlowID dllflowID;
      };

      class FlowManagerBS :
	public FlowManager,
	public IFlowManagerENB
      {
      public:
	FlowManagerBS(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);

	virtual
	~FlowManagerBS();

	virtual wns::service::qos::QoSClass
	getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const;
	virtual wns::service::qos::QoSClass
	getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const;
	virtual void
	setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs);
	virtual std::string
	getFlowTable() const;

	virtual void
	buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass);

	virtual void
	flowBuilt(wns::service::dll::FlowID flowID);

	virtual	ControlPlaneFlowIDs
	getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress);

	void
	forwardFlowRequest(lte::helper::TransactionID _transactionID,
			   lte::controlplane::flowmanagement::flowhandler::FlowHandlerBS* _flowHandlerBS,
			   wns::service::dll::UnicastAddress utAddress,
			   wns::service::dll::FlowID oldFlowID,
			   wns::service::qos::QoSClass qosClass);

	virtual void
	onCSRCreated();

	void
	releaseFlow(wns::service::dll::FlowID flowID);

	void
	deleteFlowsInRang(wns::service::dll::UnicastAddress userAdr);

	void
	deleteLowerFlow(wns::service::dll::FlowID flowID);

	void
	deleteAllLowerFlows(wns::service::dll::UnicastAddress userAdr, ModeName mode);

	void
	onDisassociationReq(wns::service::dll::UnicastAddress userAdr, ModeName mode, bool preserved);

	void
	preserveFlows(wns::service::dll::UnicastAddress userAdr);

	/** @brief AssociationObserver interface */
	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
	/** @brief AssociationObserver interface */
	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
      private:
	lte::upperconvergence::ENBUpperConvergence* bsUpperConvergence;
	lte::controlplane::AssociationsProxyBS* aProxyBS;

	wns::container::Registry<wns::service::dll::FlowID,
				 wns::service::qos::QoSClass> DllFlowIDToQoSClass;
      };

      class FlowManagerUT :
	public FlowManager,
	public IFlowManagerUE
      {
      public:
	FlowManagerUT(wns::ldk::ControlServiceRegistry* csr, const wns::pyconfig::View& config);

	virtual
	~FlowManagerUT();

	virtual wns::service::qos::QoSClass
	getQoSClassForBSFlowID(wns::service::dll::FlowID dllFlowID) const;
	virtual wns::service::qos::QoSClass
	getQoSClassForUTFlowID(wns::service::dll::FlowID dllFlowID) const;
	virtual void
	setControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress, ControlPlaneFlowIDs flowIDs);
	virtual std::string
	getFlowTable() const;

	virtual void
	buildFlow(wns::service::tl::FlowID flowID, wns::service::qos::QoSClass qosClass);

	virtual void
	flowBuilt(lte::helper::TransactionID _transactionID,  wns::service::dll::FlowID _dllFlowID);

	virtual	ControlPlaneFlowIDs
	getControlPlaneFlowIDs(wns::service::dll::UnicastAddress peerAddress);

	void
	releaseFlow(wns::service::tl::FlowID flowID);

	void
	flowReleased(wns::service::dll::FlowID flowID);

	void
	getAssociations();

	std::string
	selectMode();

	virtual void
	onCSRCreated();

	void
	onDisassociatedPerMode(wns::service::dll::UnicastAddress bsAdr, ModeName mode, bool preserved);

	void
	onAssociatedPerMode(wns::service::dll::UnicastAddress rapAdr, bool preserved);

	void
	disassociating(ModeName mode);

	void
	onPlainDisassociation(ModeName mode);

	void
	reBuildFlows(wns::service::dll::UnicastAddress bsAdr, bool preserved);

	void
	deleteLowerFlow(wns::service::dll::FlowID flowID);

	void
	deleteAllLowerFlows(ModeName mode);

	void
	closeLowerFlows(ModeName mode);

	/** @brief AssociationObserver interface */
	virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
	/** @brief AssociationObserver interface */
	virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
      private:
	void
	insertDroppedFlowIDs();


	FlowIDTable DroppedFlowIDs;
	wns::service::dll::FlowID l2FlowID;
	std::string selectedMode;
	lte::controlplane::AssociationsProxyUT* aProxyUT;
	lte::upperconvergence::UEUpperConvergence* utUpperConvergence;
	wns::container::Registry<wns::service::tl::FlowID,
				 lte::controlplane::flowmanagement::flowhandler::FlowHandlerUT*> TlFlowIDToFlowHandler;
	FlowIDToQoSTable DllFlowIDToQoSClass;
      };
    }
  }
}

#endif // LTE_CONTROLPLANE_FLOWMANAGER_HPP
