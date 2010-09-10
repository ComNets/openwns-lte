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

#ifndef LTE_HELPER_IDPROVIDER_HOPCOUNT_HPP
#define LTE_HELPER_IDPROVIDER_HOPCOUNT_HPP

#include <WNS/probe/bus/CompoundContextProvider.hpp>
#include <WNS/ldk/CommandReaderInterface.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/Compound.hpp>

/* deleted by chen */
// #include <LTE/macg/MACg.hpp>
/* inserted by chen */
// #include <LTE/macg/MACgInterface.hpp>
#include <LTE/lteDummy.hpp>

#include <WNS/isClass.hpp>

namespace lte {
    namespace helper {
        namespace idprovider {

            /** @brief Provides Connection-based hopcount calculation */
            class HopCount :
        virtual public wns::probe::bus::CompoundContextProvider
            {
                /** @brief to access the PhyCommand */
                wns::ldk::CommandReaderInterface* macgCommandReader;
                const std::string key;

            public:
                HopCount(wns::ldk::fun::FUN* fun):
                    macgCommandReader(fun->getCommandReader("macg")),
                    key("MAC.HopCount")
                {}

                virtual
                ~HopCount(){}

                virtual const std::string&
                getKey() const
                {
                    return this->key;
                }
            private:

                virtual void
                doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const
                {
                    assure(compound, "Received NULL CompoundPtr");

                    if (macgCommandReader->commandIsActivated(compound->getCommandPool()) == true)
                    {
                        // and if our command is activated, we add the hopCount to the Context

/* deleted by chen */
//                         int hopCount = macgCommandReader->readCommand<lte::macg::MACgCommand>(compound->getCommandPool())->magic.hopCount;
/* inserted by chen */
                        int hopCount = macgCommandReader->readCommand<lte::lteDummy>(compound->getCommandPool())->MACgCommand::magic.hopCount;

                        assure(hopCount >= 1, "number of hops must be >=1, but it is " << hopCount);
                        assure(hopCount <= 2, "number of hops must be <=2, but it is " << hopCount);
                        c.insertInt(this->key, hopCount);
                    }
                }
            };
        }
    }
}

#endif // not defined LTE_HELPER_IDPROVIDER_HOPCOUNT_HPP
