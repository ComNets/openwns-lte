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

#ifndef LTE_TIMING_REGISTRYPROXY_HPP
#define LTE_TIMING_REGISTRYPROXY_HPP

#include <WNS/scheduler/SchedulingMap.hpp>
#include <WNS/scheduler/RegistryProxyInterface.hpp>
#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>

#include <DLL/StationManager.hpp>
#include <DLL/services/control/Association.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>

#include <LTE/helper/HasModeName.hpp>
#include <string>

namespace wns { namespace scheduler { namespace harq { class HARQInterface;}}}
namespace dll { class Layer2;}
namespace dll { namespace services { namespace control { class Association; }}}
namespace dll { namespace services { namespace management { class InterferenceCache; }}}
namespace lte { namespace controlplane { namespace associationHandler { class AssociationHandler; }}}

namespace lte { namespace controlplane { class RRHandlerBS; class RRHandlerUT; }}
namespace lte { namespace controlplane { namespace flowmanagement { class FlowManager; }}}

namespace lte { namespace timing {

        class TimingScheduler;

        /**
         * @brief this is one of the "colleagues" of the ResourceScheduler
         * used to interface to the generic scheduler in libwns.
         */
        class RegistryProxy :
            virtual public wns::scheduler::RegistryProxyInterface,
            public lte::helper::HasModeName,
            public dll::services::control::AssociationObserver
        {
        public:
            typedef std::map<wns::scheduler::UserID, wns::scheduler::ConnectionVector> UserIdToConnections;
            typedef wns::container::Registry<wns::scheduler::ConnectionID, wns::scheduler::UserID> CidToUserId;

            RegistryProxy(wns::ldk::fun::FUN* _fun, const wns::pyconfig::View& config);
            ~RegistryProxy();

            void
            setFUN(const wns::ldk::fun::FUN* _fun);

            // obsolete. We need more classifiers than one. RLC+MACg
            void setFriends(const wns::ldk::CommandTypeSpecifierInterface* _classifier);

            /**@brief determines UserID of next hop */
            wns::scheduler::UserID
            getUserForCID(wns::scheduler::ConnectionID cid);

            /**@brief determines Address of next hop */
            wns::service::dll::UnicastAddress
            getPeerAddressForCID(wns::scheduler::ConnectionID cid);

            wns::scheduler::ConnectionVector
            getConnectionsForUser(const wns::scheduler::UserID user/*nextHop!*/);

            wns::scheduler::ConnectionID
            getCIDforPDU(const wns::ldk::CompoundPtr& compound);

            std::string
            getNameForUser(const wns::scheduler::UserID user);

            wns::service::phy::phymode::PhyModeMapperInterface*
            getPhyModeMapper() const;

            wns::service::phy::phymode::PhyModeInterfacePtr
            getBestPhyMode(const wns::Ratio&);

            /**@brief get UserID of this node/station */
            wns::scheduler::UserID
            getMyUserID();

            wns::scheduler::ChannelQualityOnOneSubChannel
            estimateTxSINRAt(const wns::scheduler::UserID user, int slot = 0);

            wns::scheduler::ChannelQualityOnOneSubChannel
            estimateRxSINROf(const wns::scheduler::UserID user, int slot = 0);

            wns::scheduler::Bits
            getQueueSizeLimitPerConnection();

            /** @brief returns StationType of argument station-ID */
            wns::service::dll::StationType

            getStationType(const wns::scheduler::UserID user);

            /** @brief reduce the set 'user' to those that can currently be reached. */
            wns::scheduler::UserSet
            filterReachable(wns::scheduler::UserSet users);

            /** @brief reduce the set 'user' to those that can can be reached at the given frameNr. */
            wns::scheduler::UserSet
            filterReachable(wns::scheduler::UserSet users, const int frameNr);

            /** @brief reduce the set of connections to those that can currently be reached. */
            //wns::scheduler::ConnectionSet
            //filterReachable(wns::scheduler::ConnectionSet connections);

            /** @brief reduce the set of connections to those that can be reached at the given frameNr */
            wns::scheduler::ConnectionSet
            filterReachable(wns::scheduler::ConnectionSet connections, const int frameNr, bool useHARQ);


	    /**
	     *@todo dbn: DEPRECATED: calcULResources is only used in old PCRR scheduling strategy. To be removed
	     */
            virtual wns::scheduler::PowerMap
            calcULResources(const wns::scheduler::UserSet&, unsigned long int) const;

            virtual wns::scheduler::UserSet
            getActiveULUsers() const;

            /**@brief returns one for UTs, and #connected UTs in case of RNs */
            virtual int
            getTotalNumberOfUsers(const wns::scheduler::UserID user);

            /**@brief returns total number of flows/cids handled by this scheduler */
            virtual int
            getTotalNumberOfFlows() const;

            /**@brief returns total number of flows/cids to a certain (end-end) user */
            virtual int
            getNumberOfFlowsForUser(const wns::scheduler::UserID user) const;

            void setRelaysOnly();
            void setAllStations();

            /** @brief get the ChannelsQualities (CQI) on all the subbands of the user.
                Eventually for a future frameNr (prediction). */
            wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
            getChannelQualities4UserOnUplink(wns::scheduler::UserID user, int frameNr);
            wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
            getChannelQualities4UserOnDownlink(wns::scheduler::UserID user, int frameNr);

            /** @brief retrieve info about a user's power capabilities */
            virtual wns::scheduler::PowerCapabilities
            getPowerCapabilities(const wns::scheduler::UserID user) const;

            /** @brief retrieve info about a user's power capabilities */
            virtual wns::scheduler::PowerCapabilities
            getPowerCapabilities() const;

            void
            setFrameNumberToBeScheduled(int frameNr);

            int
            getFrameNumberToBeScheduled() const { return frameNumberToBeScheduled; }

            bool
            hasResourcesGranted() const;

            /** @brief only true for RS-Tx in BS or RN-BS */
            void
            setDL(bool _isForDL);

            /** @brief only true for RS-Tx in BS or RN-BS */
            bool
            getDL() const;

            bool
            getCQIAvailable() const;

            void
            setAssociationHandler(lte::controlplane::associationHandler::AssociationHandler* ah);

            void
            setHARQ(wns::scheduler::harq::HARQInterface* harq);

            bool
            duplexGroupIsReachableAt(const wns::scheduler::UserID user, const int frameNr) const;

            /** @brief gets the number of QoS classes (for QoS Scheduling) **/
            int
            getNumberOfQoSClasses();
            /** @brief gets the number of priorities (for QoS Scheduling) **/
            virtual int
            getNumberOfPriorities();

            /** @brief get all CIds for the Priority Class (for QoS Scheduling) **/
            virtual wns::scheduler::ConnectionList&
            getCIDListForPriority(int priority);

            virtual wns::scheduler::ConnectionSet
            getConnectionsForPriority(int priority);

            /** @brief get all nodes in the simulation.
                Can be used to get the neighborhood (neighbor BSs). **/
            virtual dll::NodeList
            getNodeList();

            int
            virtual getPriorityForConnection(wns::scheduler::ConnectionID cid);

            /** @brief deregisterCID (important e.g. for Handover) */
            void
            deregisterCID(wns::scheduler::ConnectionID cid, wns::scheduler::UserID user/*nextHop!*/);
        private:
            /** @brief Determine whether the given user is listening at the given frame */
            bool
            isReachableAt(const wns::scheduler::UserID user, const int frameNr, bool useHARQ) const;

            /** @brief registerCID given next hop userID (not end-to-end) */
            void
            registerCID(wns::scheduler::ConnectionID cid, wns::scheduler::UserID user/*nextHop!*/);

            /** @brief deregisterUser (important e.g. for Handover) */
            void
            deregisterUser(const wns::scheduler::UserID user/*nextHop!*/);

            /** @brief AssociationObserver interface */
            void
            onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
            void
            onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);

            /** @brief QoS->Priority mapping */
            int
            mapQoSClassToPriority(wns::service::qos::QoSClass qosClass);

            /** @brief the Functional Unit Network I'm located in */
            wns::ldk::fun::FUN* fun;
            /** @brief the layer2 I'm located in */
            dll::Layer2* layer2;
            //dll::StationManager* stationManager;
            /** @brief my Logger */
            wns::logger::Logger logger;

            /** @brief the pointer to the association service */
            dll::services::control::Association* associationService;

            /** @brief the pointer to the interference cache service */
            dll::services::management::InterferenceCache* iCache;

            /** @brief neighbour FUs in my FUN. We need to exchange infos with them */
            struct Friends {
                Friends() {rlcCommandReader=NULL;macgCommandReader=NULL;harq=NULL;timer=NULL;associationHandler=NULL;rrHandler=NULL; rrHandlerUT=NULL; flowManager=NULL;};
                wns::ldk::CommandReaderInterface* rlcCommandReader;
                wns::ldk::CommandReaderInterface* macgCommandReader;
                wns::scheduler::harq::HARQInterface* harq;
                lte::timing::TimingScheduler* timer;
                lte::controlplane::associationHandler::AssociationHandler* associationHandler;
                lte::controlplane::RRHandlerBS* rrHandler;
                lte::controlplane::RRHandlerUT* rrHandlerUT;
                lte::controlplane::flowmanagement::FlowManager* flowManager;
            } friends;

            /** @brief mapping of ordered phyModes and SINR ranges
             * the only reason for being here is to provide it to the libwns scheduler strategies */
            wns::service::phy::phymode::PhyModeMapperInterface* phyModeMapper;

            wns::scheduler::ChannelQualitiesOnAllSubBandsPtr fakeChannelQualities;

            uint32_t queueSize;

            bool relaysOnly;

            wns::scheduler::PowerCapabilities powerUT;
            wns::scheduler::PowerCapabilities powerBS;

            bool eirpLimited;

            std::string apcstrategyName;
            std::string dsastrategyName;
            std::string dsafbstrategyName;

            /** @brief at what time do we need to calculate "reachable" */
            int frameNumberToBeScheduled;
            simTimeType frameNumberToBeScheduled_setTime; // for asserts
            /** @brief DuplexGroups must match for DL; For UL they are toggled */
            bool isForDL;
            /** @brief for RelayNodes: in which task are we */
            wns::scheduler::TaskBSorUTType myTask;

            int numberOfPriorities;
            int numberOfQosClasses;
            std::vector<int> qosToPriorityMapping;

            CidToUserId cidToUserId; // RN|UT
            wns::container::Registry<wns::scheduler::ConnectionID, int> cidToPrio; // New[rs]

            /** @brief Container to map userids to connections */
            UserIdToConnections userIdToConnections;

            /** @brief container for list of connections within priority */
            std::vector<wns::scheduler::ConnectionList> connectionsForPriority;
        };

    }
} // namespace lte::timing
#endif // LTE_TIMING_REGISTRYPROXYLTE_HPP
