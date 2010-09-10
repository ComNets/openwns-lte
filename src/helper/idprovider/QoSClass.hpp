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

#ifndef LTE_HELPER_IDPROVIDER_QOSCLASS_HPP
#define LTE_HELPER_IDPROVIDER_QOSCLASS_HPP

#include <WNS/probe/bus/CompoundContextProvider.hpp>
#include <WNS/ldk/fun/FUN.hpp>

namespace lte { namespace helper { namespace idprovider {

      class QoSClass :
    virtual public wns::probe::bus::CompoundContextProvider
      {
      public:
	QoSClass(wns::ldk::fun::FUN* fun);

	virtual
	~QoSClass();

	virtual const std::string&
	getKey() const
	{
	  return key;
	}
      private:
	virtual void
	doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const;

	wns::ldk::CommandReaderInterface* rlcCommandReader;
	const std::string key;
      };
    }}}

#endif // not defined LTE_HELPER_IDPROVIDER_QOSCLASS_HPP
