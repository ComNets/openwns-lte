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

#ifndef LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLER_FLOWHANDLER_HPP
#define LTE_CONTROLPLANE_FLOWMANAGEMENT_FLOWHANDLER_FLOWHANDLER_HPP

#include <LTE/helper/HasModeName.hpp>
#include <LTE/helper/TransactionID.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/services/control/Association.hpp>
#include <DLL/StationManager.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/FlowGate.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/service/dll/FlowID.hpp>
#include <WNS/service/qos/QoSClasses.hpp>
#include <WNS/Enum.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/Subject.hpp>

namespace lte {

    namespace macg {
        class MACg;
    }
    namespace rlc {
        class RLC;
    }

    namespace controlplane{

        namespace flowmanagement {
            class FlowManager;

            namespace flowhandler {

                ENUM_BEGIN(CompoundType);
                ENUM(flow_req,       0);
                ENUM(flow_confirm,   1);
                ENUM(flow_ack,       2);
                ENUM(flow_rel,       3);
                ENUM(flow_rel_ack,   4);
                ENUM_END();

                /** @brief Command for the FlowHandler FU */
                class FlowHandlerCommand :
                    public wns::ldk::Command
                {
                public:
                    /** @brief Corresponding with the CompoundType ENUM */
                    typedef int CompoundType;

                    FlowHandlerCommand() {
                    };

                    struct {
                    } local;

                    struct {
                        CompoundType myCompoundType;
                        wns::service::dll::FlowID flowID;
                        wns::service::dll::UnicastAddress user;
                        lte::helper::TransactionID transactionId;
                        bool preserved;
                        wns::service::dll::FlowID oldFlowID;
                        wns::service::qos::QoSClass qosClass;
                    } peer;

                    struct {
                    } magic;
                };

                class FlowHandler :
                    virtual public wns::ldk::FunctionalUnit,
                    public wns::ldk::CommandTypeSpecifier<FlowHandlerCommand>,
                    public wns::ldk::HasConnector<>,
                    public wns::ldk::HasReceptor<>,
                    public wns::ldk::HasDeliverer<>,
                    public lte::helper::HasModeName,
                    public dll::services::control::AssociationObserver

                {
                public:
                    /** @brief Constructor to be used by FUNConfigCreator */
                    FlowHandler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

                    /** @brief Destructor */
                    virtual ~FlowHandler();

                    virtual void
                    onFUNCreated();

                    virtual bool
                    doIsAccepting(const wns::ldk::CompoundPtr& compound ) const;

                    void
                    doSendData(const wns::ldk::CompoundPtr& /* compound */);

                    void
                    doWakeup();

                    void
                    calculateSizes(const wns::ldk::CommandPool* commandPool,
                                   Bit& commandPoolSize,
                                   Bit& dataSize) const;

                    virtual void
                    createFlow(wns::service::dll::FlowID){}

                    virtual void
                    destroyFlow(wns::service::dll::FlowID);

                    void
                    closeFlow(wns::service::dll::FlowID flowID);

                    /** @brief AssociationObserver interface */
                    virtual void onAssociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);
                    /** @brief AssociationObserver interface */
                    virtual void onDisassociated(wns::service::dll::UnicastAddress userAdr, wns::service::dll::UnicastAddress dstAdr);

                protected:
                    wns::ldk::fun::FUN* fun;
                    dll::ILayer2* layer2;

                    lte::macg::MACg* macg;
		    wns::ldk::CommandReaderInterface* rlcReader;
                    lte::controlplane::flowmanagement::FlowManager* flowManager;
                    dll::services::control::Association* associationService;
                    wns::logger::Logger logger;
                    Bit commandSize;
                    wns::ldk::FlowGate* lowerFlowGate;
                    std::list<std::string> flowSeparatorNames;
                    std::list<wns::ldk::FlowSeparator*> flowSeparators;
                };
            }
        }
    }
}
#endif // LTE_CONTROLPLANE_FLOWMANAGEMENT__FLOWHANDLER_FLOWHANDLER_HPP
