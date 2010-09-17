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

#ifndef LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLER_HPP
#define LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLER_HPP

#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>
#include <LTE/helper/HasModeName.hpp>
#include <LTE/helper/SwitchConnector.hpp>

#include <DLL/services/control/Association.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/Enum.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/Subject.hpp>
#include <WNS/SmartPtr.hpp>


namespace lte { namespace controlplane{ namespace associationHandler {

      ENUM_BEGIN(CompoundType);
      ENUM(association_req,       0);
      ENUM(association_ack,       1);
      ENUM(disassociation_req,    2);
      ENUM(disassociation_ack,    3);
      ENUM_END();

      class AssociationHandler;

      /** @brief Command for the AssociationHandler FU */
      class AssociationCommand :
	public wns::ldk::Command
      {
      public:
	/** @brief Corresponding with the CompoundType ENUM */
	typedef int CompoundType;

	AssociationCommand() {
	};

	struct Local {
	} local;

	struct Peer {
	  CompoundType myCompoundType;
	  wns::service::dll::UnicastAddress src;
	  wns::service::dll::UnicastAddress dst;
	  wns::service::dll::UnicastAddress user;
	  lte::controlplane::flowmanagement::IFlowSwitching::ControlPlaneFlowIDs controlPlaneFlowIDs;

	  // in disassociation_req
	  wns::service::dll::UnicastAddress targetRAP;

	  // in disassociation_ack
	  bool preserve;

	  // used for mode switch of association_ack by RN
	  std::string mode;
	  // duplexgroup: TDD = 0, fd = 3,hd = 1/2
	  int duplexGroup;
	} peer;

	struct Magic {
	  // user is not allowed to know the bs address
	  // which but must be used now for the e2e flow control
	  wns::service::dll::UnicastAddress bs;
	  // can be used to do assures for correct reception
	  const AssociationHandler* sender;
	  // can be used to do assures for correct reception
	  //SmartPtr<OnDeleteWarner> warner;
	} magic;
      };

      class AssociationHandler :
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier<AssociationCommand>,
	public wns::ldk::HasReceptor<>,
	public wns::ldk::HasConnector<lte::helper::SwitchConnector>,
	public wns::ldk::HasDeliverer<>,
	public lte::helper::HasModeName,
	public dll::services::control::AssociationInfo
      {
      public:
	/** @brief Constructor to be used by FUNConfigCreator */
	AssociationHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

	/** @brief Destructor */
	virtual ~AssociationHandler();

	virtual void
	onFUNCreated();

	bool
	doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;

	void
	doSendData(const wns::ldk::CompoundPtr& /* compound */);

	void
	doWakeup();

	virtual void
	disassociationOnTimeout(const wns::service::dll::UnicastAddress /*adr*/, const std::string /*mode*/){};

	void
	calculateSizes(const wns::ldk::CommandPool* commandPool,
		       Bit& commandPoolSize,
		       Bit& dataSize) const;

	virtual int getMyDuplexGroup(int frameNr, bool isDL) = 0;
	virtual int getPeerDuplexGroup(wns::service::dll::UnicastAddress user) = 0;
      private:
	wns::ldk::fun::FUN* fun;

      protected:

	void
	notifyOnAssociated(wns::service::dll::UnicastAddress,
			   wns::service::dll::UnicastAddress );

	void
	notifyOnDisassociated(wns::service::dll::UnicastAddress,
			      wns::service::dll::UnicastAddress );

	struct {
	  wns::ldk::IConnectorReceptacle* bchBuffer;
	  wns::ldk::IConnectorReceptacle* rachDispatcher;
	  wns::ldk::IConnectorReceptacle* cpDispatcher;
	  lte::controlplane::flowmanagement::IFlowSwitching* flowManager;
	} friends;

	wns::ldk::CommandReaderInterface* macgReader;
	wns::ldk::CommandReaderInterface* rlcReader;

	dll::ILayer2* layer2;

	dll::services::control::Association* associationService;

	lte::helper::SwitchConnector* connector;

	/** @brief assumed Size of command header */
	Bit commandSize;

	/** @brief lists of flowseparators in the mode*/
	std::list<std::string> flowSeparatorNames;
	std::list<wns::ldk::FlowSeparator*> flowSeparators;

	/** @brief my Logger */
	wns::logger::Logger logger;
      };
    }
  }
}
#endif // LTE_CONTROLPLANE_ASSOCIATIONHANDLER_ASSOCIATIONHANDLER_HPP
