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

#ifndef LTE_MACR_PHYUSER_HPP
#define LTE_MACR_PHYUSER_HPP

#include <LTE/macr/PhyCommand.hpp>

#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Receptor.hpp>
#include <WNS/ldk/Connector.hpp>

#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/service/phy/ofdma/Notification.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/service/phy/power/PowerMeasurement.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/dll/Address.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/Positionable.hpp>
#include <WNS/Observer.hpp>

namespace dll {
    class ILayer2;
    namespace services {
        namespace management {
            class InterferenceCache;
        }
    }
}

namespace dll {
    class StationManager;
    namespace services { namespace control {
            class Association;
        }}}

namespace wns {
    namespace service { namespace phy { namespace phymode {
                class SNR2MIInterface;
            }}}
}
namespace lte{
    namespace macr {

        class IRxTxSettable
        {
        public:
            /** @brief Identifier for the current state of the PhyUser - TDD should
             * switch between Tx and Rx, FDD can be in state 'BothRxTx' */
            /** @todo rethink concept for FDD */
            typedef enum { Invalid, Tx, Rx, BothRxTx } StateRxTx;

            /** @brief for external setting of transmission/reception state by
             * Events from lte::timing::events */
            /** @todo setTransceiverDirection could be a better name */
            virtual void
            setStateRxTx(StateRxTx _state) = 0;
        };

        /** @brief FU sitting on the lowest end of Layer2, contact to PHY, e.g. OFDMAPhy */
        class PhyUser :
                virtual public wns::ldk::FunctionalUnit,
                public wns::ldk::CommandTypeSpecifier< PhyCommand >,
                public wns::ldk::HasReceptor<>,
                public wns::ldk::HasConnector<>,
                public wns::ldk::HasDeliverer<>,
                public wns::Cloneable<PhyUser>,
                virtual public wns::service::phy::ofdma::Handler,
                public IRxTxSettable
        {
            /** @brief Event triggering the start of a PHY transmission */
            class StartTxEvent
            {
                wns::ldk::CompoundPtr compound;
                int subBand;
                PhyUser* phyUser;
            public:
                StartTxEvent(const wns::ldk::CompoundPtr& _compound,
                             PhyUser* _phyUser) :
                    compound(_compound),
                    phyUser(_phyUser)
                {};

                virtual ~StartTxEvent(){
                    compound = wns::ldk::CompoundPtr();
                }

                virtual void
                operator()()
                {
                    phyUser->startTransmission(compound);
                };
            };

            /** @brief Event triggering the end of a PHY transmission */
            class StopTxEvent
            {
                wns::ldk::CompoundPtr compound;
                int subBand;
                PhyUser* phyUser;
            public:
                StopTxEvent(const wns::ldk::CompoundPtr& _compound,
                            int _subBand,
                            PhyUser* _phyUser) :
                    compound(_compound),
                    subBand(_subBand),
                    phyUser(_phyUser)
                {};

                virtual
                ~StopTxEvent(){
                    compound = wns::ldk::CompoundPtr();
                }

                virtual void
                operator()()
                {
                    phyUser->stopTransmission(compound, subBand);
                };
            };

        public:

            PhyUser(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyConfigView);
            virtual ~PhyUser();


            /** @name FU Interface */
            //@{
            virtual void
            onFUNCreated();
            //@}

            /** @name CompoundHandlerInterface */
            //@{
            virtual bool
            doIsAccepting(const wns::ldk::CompoundPtr& compound) const;
            virtual void
            doSendData(const wns::ldk::CompoundPtr& sdu);
            virtual void
            doOnData(const wns::ldk::CompoundPtr& compound);
            virtual void
            doWakeup();
            //@}

            /** @name wns::service::phy::ofdma::Handler:  Interface to lower layer
             * (OFDMAPhy) */
            //@{
            virtual void
            onData(wns::osi::PDUPtr pdu, wns::service::phy::power::PowerMeasurementPtr rxPowerMeasurement);
            //@}

            /** @name Trigger methods used by Events */
            //@{
            void
            startTransmission(const wns::ldk::CompoundPtr& compound);
            void
            stopTransmission(wns::osi::PDUPtr pdu, int subBand);
            //@}

            /** @brief method used for sanity checking, returns TRUE if PHY is not
             * transmitting and not receiving anything */
            bool
            checkIdle();

            /** @brief for external setting of transmission/reception state by
             * Events from lte::timing::events */
            /** @todo setTransceiverDirection could be a better name */
            void
            setStateRxTx(StateRxTx _state);

            /** @brief method used for debug output */
            std::string
            getStateRxTx() const;

            /** @brief method for setting Rx/Tx frequencies to RAP task, e.g. in an FDD relay */
            virtual void
            rapTuning();

            /** @brief method for setting Rx/Tx frequencies to ut task, e.g. in an FDD relay */
            virtual void
            utTuning();

            /** @brief registration of the service (upstack data interface) to PHY */
            virtual void
            setNotificationService(wns::service::Service* phy);

            /** @brief get service handle (upstack data interface) to PHY */
            virtual wns::service::phy::ofdma::Notification*
            getNotificationService() const;

            /** @brief registration of the service (downstack data interface) to PHY */
            virtual void
            setDataTransmissionService(wns::service::Service* phy);

            /** @brief get service (downstack data interfaces) to PHY */
            virtual wns::service::phy::ofdma::DataTransmission*
            getDataTransmissionService() const;

            /** @brief MAC address has to be set externally @see lte::Layer2::Layer2::onNodeCreated() */
            virtual void
            setMACAddress(const wns::service::dll::UnicastAddress& address);

            void
            setMobility(wns::PositionableInterface* _mobility);

            /** @brief true if FDD is possible */
            bool
            isFddCapable() const;

            void
            setReceiveAntennaPattern(wns::node::Interface* destination, wns::service::phy::ofdma::PatternPtr pattern);

            /** @brief delete RxAntenna Patterns */
            void
            deleteReceiveAntennaPatterns();
        private:

            /** @brief my DLL */
            dll::ILayer2* layer2;
            /** @brief the currently active State (typedef enum { Tx, Rx, BothRxTx } StateRxTx;) */
            StateRxTx stateRxTx;
            wns::logger::Logger logger;

            /** @brief list of subchannels on which we currently transmit  */
            std::list< std::pair<wns::osi::PDUPtr, int> > activeSubBands;

            /** @brief fddCapable, if UL and DL center frequencies differ */
            bool fddCapable;

            /** @brief for TDD: a small amount of time between switching Rx/Tx */
            simTimeType safetyFraction;

            /** @brief we generate our own events to notify us about StartTxEvent,StartRxEvent */
            wns::events::scheduler::Interface* es;

            wns::service::dll::UnicastAddress address;

            /** @brief Pointer to InterferenceCache */
            dll::services::management::InterferenceCache* iCache;

            /** @brief service from the PHY layer (downstack data) */
            wns::service::phy::ofdma::BFInterface* bfTransmission;

            /** @brief service from the PHY layer (downstack data) */
            wns::service::phy::ofdma::NonBFInterface* transmission;

            /** @brief service from the PHY layer (upstack data) */
            wns::service::phy::ofdma::Notification* notificationService;

            /** @brief pointer to mobility component */
            wns::PositionableInterface* mobility;

            /** @brief pointer to the station Manager info service */
            dll::StationManager* stationManager;

            wns::simulator::Time measurementDelay_;
        };
    } }
#endif // NOT defined LTE_MACR_PHYUSER_HPP


