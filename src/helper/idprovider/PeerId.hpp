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

#ifndef LTE_HELPER_IDPROVIDER_PEERID_HPP
#define LTE_HELPER_IDPROVIDER_PEERID_HPP

#include <WNS/probe/bus/CompoundContextProvider.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <DLL/Layer2.hpp>

namespace lte { namespace helper { namespace idprovider {

      class PeerId :
    virtual public wns::probe::bus::CompoundContextProvider
      {
      public:
	PeerId(wns::ldk::fun::FUN* fun);

	virtual
	~PeerId();

	virtual const std::string&
	getKey() const
	{
	  return key;
	}
      private:
	virtual void
	doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const;

	/** @brief ILayer2 */
	dll::ILayer2* layer2;
	wns::ldk::CommandReaderInterface* rlcCommandReader;
	wns::ldk::CommandReaderInterface* macgCommandReader;
	const std::string key;
	int stationType;
      };
}}}

#endif // not defined LTE_HELPER_IDPROVIDER_PEERID_HPP
