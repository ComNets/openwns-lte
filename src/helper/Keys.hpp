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

#ifndef LTE_HELPER_KEYS_HPP
#define LTE_HELPER_KEYS_HPP

#include <WNS/service/dll/FlowID.hpp>
#include <WNS/ldk/Key.hpp>

namespace lte { namespace helper { namespace key {

class FlowIDBuilder;

class FlowID :
    public wns::ldk::Key
{
public:
  FlowID(const FlowIDBuilder* factory, const wns::ldk::CompoundPtr& compound);

  FlowID(wns::service::dll::FlowID _flowID);
  
  std::string str() const;
  
  bool operator<(const wns::ldk::Key& other) const;
  
  wns::service::dll::FlowID flowID;
};

class FlowIDBuilder :
    public wns::ldk::KeyBuilder
{
public:
  FlowIDBuilder(const wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
  
  virtual void onFUNCreated();
  
  virtual wns::ldk::ConstKeyPtr operator () (const wns::ldk::CompoundPtr& compound, int /*direction*/) const;

  const wns::ldk::fun::FUN* fun;

  struct Friends {
    wns::ldk::CommandReaderInterface* rlcReader;
  } friends;
};

} // key
} // helper
} // lte

#endif // LTE_HELPER_KEYS_HPP
