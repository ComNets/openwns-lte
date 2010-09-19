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

#ifndef LTE_CONTROLPLANE_RRHANDLER
#define LTE_CONTROLPLANE_RRHANDLER

#include <LTE/timing/TimingScheduler.hpp>
#include <LTE/helper/HasModeName.hpp>
#include <LTE/helper/SwitchConnector.hpp>
#include <LTE/controlplane/MapHandlerInterface.hpp>

#include <DLL/services/control/Association.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/IConnectorReceptacle.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/Observer.hpp>
#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/scheduler/RegistryProxyInterface.hpp>
#include <WNS/service/dll/FlowID.hpp>

namespace lte {
    namespace timing {
        class ResourceSchedulerInterface;
        class ResourceSchedulerUT;
        class ResourceSchedulerBS;
        class RegistryProxy;
    }
    namespace macg {
        class MACg;
    }
    namespace rlc {
        class RLC;
    }

    namespace controlplane {
        namespace flowmanagement {
            class FlowManager;
        }
        namespace tests {
            class RRHandlerTests;
        }
        class RRHandlerUT;

        typedef std::map<wns::scheduler::UserID, double> ResourceShares;

        struct ResourceRequest {
            ResourceRequest() :
                user(NULL),
                allRequestedResources()
            {}
            wns::scheduler::UserID user;
            // QueueStatusContainer is a Registry, the key is the ConnectionID=FlowID
            wns::scheduler::QueueStatusContainer allRequestedResources;
        };

        class RequestStorageInterface
        {
            friend class lte::controlplane::tests::RRHandlerTests;
        public:
            virtual
            ~RequestStorageInterface(){};

            virtual void
            storeRequest(const wns::scheduler::UserID user, wns::scheduler::QueueStatusContainer& partialQueueStatusContainer) = 0;
            //virtual void
            //storeRequest(ResourceRequest&) = 0;

            virtual ResourceShares
            getResourceShares(const wns::scheduler::UserSet&) const = 0;

            virtual wns::scheduler::UserSet
            getActiveUsers() const = 0;

            virtual wns::scheduler::ConnectionSet
            getActiveConnections() const = 0;

            virtual wns::scheduler::ConnectionSet
            filterActiveConnections(wns::scheduler::ConnectionSet& inputConnectionSet) const = 0;

            virtual bool
            knowsFlow(wns::scheduler::ConnectionID flowId) const = 0;

            virtual void
            resetFlow(const wns::scheduler::ConnectionID flowId) = 0;

            virtual void
            resetUser(const wns::scheduler::UserID) = 0;

            virtual void
            reset(const wns::scheduler::UserSet&) = 0;

            virtual bool
            isEmpty() const = 0;

            virtual uint32_t
            numBitsForCid(wns::scheduler::ConnectionID cid) const = 0;
            virtual uint32_t
            numCompoundsForCid(wns::scheduler::ConnectionID cid) const = 0;
            virtual wns::scheduler::Bits
            decrementRequest(wns::scheduler::ConnectionID cid, wns::scheduler::Bits bits) = 0;

            virtual wns::scheduler::QueueStatusContainer
            getQueueStatus() const = 0;
            virtual std::string
            printQueueStatus() const = 0;
        }; // class RequestStorageInterface

        class RequestStorage :
                public virtual RequestStorageInterface
        {
            friend class lte::controlplane::tests::RRHandlerTests;
            //typedef std::list<ResourceRequest> RequestList;
            /** @brief list to store the requests received */
            //RequestList receivedRequests;
            /** @brief Holds QueueStatus for all cids. ResourceRequests contain such a container. */
            typedef wns::container::Registry<wns::scheduler::ConnectionID, wns::scheduler::UserID> ConnectionUserMapping;
            ConnectionUserMapping connectionUserMapping;
            /** @brief structure to store the requests received */
            wns::scheduler::QueueStatusContainer queueStatusContainer;
            /** @brief set of users which have requested ULresources */
            wns::scheduler::UserSet activeULUsers;
            /** @brief my Logger */
            wns::logger::Logger logger;

        public:
            RequestStorage(wns::logger::Logger& _logger);

            virtual
            ~RequestStorage();

            virtual void
            storeRequest(const wns::scheduler::UserID user, wns::scheduler::QueueStatusContainer& partialQueueStatusContainer);

            //virtual void
            //storeRequest(ResourceRequest&);

            virtual ResourceShares
            getResourceShares(const wns::scheduler::UserSet&) const;

            virtual wns::scheduler::UserSet
            getActiveUsers() const;

            virtual wns::scheduler::ConnectionSet
            getActiveConnections() const;

            virtual wns::scheduler::ConnectionSet
            filterActiveConnections(wns::scheduler::ConnectionSet& inputConnectionSet) const;

            virtual bool
            knowsFlow(wns::scheduler::ConnectionID flowId) const;

            virtual void
            resetFlow(const wns::scheduler::ConnectionID flowId);

            virtual void
            resetUser(const wns::scheduler::UserID);

            virtual void
            deleteUser(const wns::scheduler::UserID);

            virtual void
            reset(const wns::scheduler::UserSet&);

            virtual bool
            isEmpty() const;

            virtual uint32_t
            numBitsForCid(wns::scheduler::ConnectionID cid) const;
            virtual uint32_t
            numCompoundsForCid(wns::scheduler::ConnectionID cid) const;
            virtual wns::scheduler::Bits
            decrementRequest(wns::scheduler::ConnectionID cid, wns::scheduler::Bits bits);

            virtual wns::scheduler::QueueStatusContainer
            getQueueStatus() const;
            virtual std::string
            printQueueStatus() const;
        }; // class RequestStorage

        /** @brief Command for the RRHandler FU */
        class RRCommand :
                public wns::ldk::Command
        {
        public:
            RRCommand()
            {
                peer.request = ResourceRequest();
            };
            struct {
            } local;
            struct {
                ResourceRequest request;
            } peer;
            struct {
                wns::service::dll::UnicastAddress source;
            } magic;
        };

        /** @brief Functional Unit that handles the Resource Requests */
        class RRHandler :
                public virtual wns::ldk::FunctionalUnit,
                //public lte::controlplane::associationHandler::AssociationObserver,
                public wns::ldk::CommandTypeSpecifier<RRCommand>,
                public wns::ldk::HasReceptor<>,
                public wns::ldk::HasConnector<lte::helper::SwitchConnector>,
                public wns::ldk::HasDeliverer<>,
                public lte::helper::HasModeName
        {
            friend class lte::controlplane::tests::RRHandlerTests;
        public:
            /** @brief Constructor to be used by FUNConfigCreator */
            RRHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
            /** @brief Destructor */
            virtual ~RRHandler();

            /** @name CompoundHandlerInterface */
            //@{
            virtual void
            doSendData(const wns::ldk::CompoundPtr&);
            //@}

            //** @name AssociationObserverInterface */
            //@{
            //virtual void
            //onDisassociation(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
            //virtual void
            //onAssociationChanged(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
            //@}

            /** @brief Resolve Intra-FUN dependencies after the component
             * was created */
            virtual void
            onFUNCreated();

            virtual void
            setColleagues(wns::scheduler::RegistryProxyInterface* registry);

            /** @brief Add the size of my Command to the calculation */
            virtual void
            calculateSizes(const wns::ldk::CommandPool* commandPool,
                           Bit& commandPoolSize,
                           Bit& dataSize) const;

            /**@brief returns one for UTs, and #connected UTs in case of RNs */
            virtual int
            getTotalNumberOfUsers(const wns::scheduler::UserID user) const;

        protected:
            virtual void
            doWakeup();

            virtual bool
            doIsAccepting(const wns::ldk::CompoundPtr& /* compound */) const;

            /** @brief Contains pointers to the FUs the RRHandler has to
             *  collaborate with
             */
            struct Friends {
                Friends() {macg=NULL;rachDispatcher=NULL;cpDispatcher=NULL;flowManager=NULL;registry=NULL;};
                lte::macg::MACg* macg;
                wns::ldk::IConnectorReceptacle* rachDispatcher;
                wns::ldk::IConnectorReceptacle* cpDispatcher;
                lte::controlplane::flowmanagement::FlowManager* flowManager;
                wns::scheduler::RegistryProxyInterface* registry; // RegistryProxy of RS-RX (uplink master)
            } friends;

	    wns::ldk::CommandReaderInterface* rlcReader;
            /** @brief the switch connector is used to determine the
             * outgoing FU path*/
            lte::helper::SwitchConnector* connector;

            /** @brief Association service access is needed to correctly
             * address the packets */
            dll::services::control::Association* associationService;

            /** @brief assumed Size of command header */
            Bit commandSize;

            /** @brief Size of PCCH */
            Bit pcchSize;

            /** @brief my Logger */
            wns::logger::Logger logger;

            /** @brief ILayer2 */
            dll::ILayer2* layer2;

            wns::pyconfig::View pyco;

            bool usesShortcut;
        };

        class RRHandlerBS :
                public RRHandler,
                public wns::Cloneable<RRHandlerBS>,
                public dll::services::control::AssociationObserver,
                public wns::Observer<lte::timing::SuperFrameStartNotificationInterface>
        {
            /** @brief flag to make sure we call this only once. */
            bool firstWakeup;

            /**
             * @brief object encapsulating the storage of the requests and
             * the calculation of the resource shares requested by each user
             */
            RequestStorageInterface* rrStorage;
            RRHandlerUT* rnUplinkRRHandler;
            lte::timing::ResourceSchedulerBS* rsrx;
            /** @brief this is the default packet size assumed.
                Best choice is the number of bits that fit into a subchannel with the lowest PhyMode. */
            uint32_t ulSegmentSize;
        public:
            RRHandlerBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

            virtual
            ~RRHandlerBS();

            virtual void
            onFUNCreated();

            virtual int
            getDefaultBitsPerPDU();

            virtual wns::ldk::CompoundPtr
            createFakePDU(wns::service::dll::UnicastAddress destinationAddress, wns::scheduler::Bits bits, wns::service::dll::FlowID flowID);

            /** @brief Trigger method to generate fake reservations at the
             * beginning of the simulation */
            void
            fakeRelayRRs() const;

            virtual void
            doWakeup();

            /** @brief used by other classes to inform us when a super radio frame begins */
            virtual void
            onSuperFrameStart();

            virtual void
            doOnData(const wns::ldk::CompoundPtr&);

            virtual ResourceShares
            getResourceShares(const wns::scheduler::UserSet&) const;

            virtual wns::scheduler::UserSet
            getActiveUsers() const;

            virtual RequestStorageInterface*
            getRRStorage();

            virtual void
            onAssociated(wns::service::dll::UnicastAddress userAdr,
                         wns::service::dll::UnicastAddress dstAdr);

            virtual void
            onDisassociated(wns::service::dll::UnicastAddress userAdr,
                            wns::service::dll::UnicastAddress dstAdr);

        protected: // only used by unitTest:
            virtual void
            reset(const wns::scheduler::UserSet& users);

        };

        class RRHandlerUT :
                public RRHandler,
                public wns::Cloneable<RRHandlerUT>,
                public wns::Observer<lte::controlplane::ResourceGrantNotificationInterface>
        {
            /** @brief stores whether the last UL map received contained
                granted resources for this station, @see resourcesGranted,
                @see hasResourcesGranted */
            bool resourceGrantState;

            /** @brief pointer to the resource scheduler to ask about the
             * number of waiting PDUs */
            lte::timing::ResourceSchedulerUT* rstx;

            /** @brief Only used by RNs to forward the resource request of their own RUTs */
            wns::scheduler::QueueStatusContainer allRelayedRequestedResources;

            /** @brief Only used by RNs. They get the PCCH FlowID MAGIC */
            wns::service::dll::FlowID pcchFlowID;

        public:
            RRHandlerUT(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
                RRHandler(fun, config),
                wns::Cloneable<RRHandlerUT>(),
                resourceGrantState(false),
                rstx(NULL),
                allRelayedRequestedResources(),
                pcchFlowID(0)
            {}

            virtual
            ~RRHandlerUT();

            virtual void
            onFUNCreated();

            /** @brief Trigger method to generate an RR Compound */
            void
            createRRCompound();

            /** @brief call this to inform RRHandler whether resources have
             *  been granted or not. */
            virtual void
            resourcesGranted(bool state);

            bool
            hasResourcesGranted() const { return resourceGrantState; }

            virtual void
            doOnData(const wns::ldk::CompoundPtr&);
        };
    }}

#endif
