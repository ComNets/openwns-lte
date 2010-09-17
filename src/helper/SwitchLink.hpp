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

#ifndef LTE_HELPER_SWITCHLINK_HPP
#define LTE_HELPER_SWITCHLINK_HPP

#include <WNS/ldk/Link.hpp>

#include <WNS/Assure.hpp>

#include <list>

namespace lte { namespace helper {

        namespace tests {
            class SwitchLinkTest;
        }

        template <typename RECEPTACLETYPE>
        class SwitchLink :
        virtual public wns::ldk::Link<RECEPTACLETYPE>
        {
            friend class tests::SwitchLinkTest;
        public:
            SwitchLink()
                : recs(),
                  active(NULL)
            {}

            virtual
            ~SwitchLink()
            {
                recs.clear();
                active = NULL;
            }

            /**
             * @name Link interface
             */
            //{@
            virtual void
            add(RECEPTACLETYPE* it)
            {
                assureNotNull(it);
                recs.push_back(it);
            }

            virtual long unsigned int
            size() const
            {
                return recs.size();
            }

            virtual void
            clear()
            {
                recs.clear();
            }

            virtual const typename wns::ldk::Link<RECEPTACLETYPE>::ExchangeContainer
            get() const
            {
                typename wns::ldk::Link<RECEPTACLETYPE>::ExchangeContainer container;

                for (typename std::list<RECEPTACLETYPE*>::const_iterator iter = recs.begin();
                     iter != recs.end();
                     ++iter)
                    container.push_back(*iter);

                return container;
            }

            virtual void
            set(const typename wns::ldk::Link<RECEPTACLETYPE>::ExchangeContainer& src)
            {
                // setting a new set of links deletes the activation. It needs to be re-set
                active = NULL;

                if (!src.empty())
                {
                    recs.clear();
                    for (typename wns::ldk::Link<RECEPTACLETYPE>::ExchangeContainer::const_iterator iter = src.begin();
                         iter != src.end();
                         ++iter)
                    {
                        assureNotNull(*iter);
                        recs.push_back(*iter);
                    }
                }
            }
            //@}

            void
            activate(const typename wns::ldk::Link<RECEPTACLETYPE>::ExchangeContainer& src)
            {
                assure(src.size()<2, "SwitchLink can not activate more than one link at a time.");

                if (src.size()>0){
                    assure(find(recs.begin(), recs.end(), src[0]) != recs.end(),
                           "Trying to activate unknown link.");

                    activate(src[0]);
                }
            }

            void
            activate(const RECEPTACLETYPE* src)
            {
                assureNotNull(src);

                assure(find(recs.begin(), recs.end(), src) != recs.end(),
                       "Trying to activate unknown link.");

                active = *(find(recs.begin(), recs.end(), src));
            }

            typename std::list<RECEPTACLETYPE*>
            getFUs() const
            {
                return recs;
            }

        protected:
            std::list<RECEPTACLETYPE*> recs;

            RECEPTACLETYPE* active;
        };
    } // helper
} // lte

#endif // LTE_HELPER_SWITCHLINK_HPP


