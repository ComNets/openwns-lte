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

#include <LTE/helper/Keys.hpp>

#include <LTE/rlc/RLCCommand.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/CommandReaderInterface.hpp>

using namespace lte::helper::key;

STATIC_FACTORY_REGISTER_WITH_CREATOR(FlowIDBuilder,
				     wns::ldk::KeyBuilder,
				     "lte.FlowID",
				     wns::ldk::FUNConfigCreator);


FlowID::FlowID(const FlowIDBuilder* factory,
	       const wns::ldk::CompoundPtr& compound)
{
  wns::ldk::CommandPool* commandPool = compound->getCommandPool();

  lte::rlc::RLCCommand* rlcCommand = factory->friends.rlcReader->readCommand<lte::rlc::RLCCommand>(commandPool);

  flowID = rlcCommand->peer.flowID;
}

FlowID::FlowID(wns::service::dll::FlowID _flowID)
{
  flowID = _flowID;
}

bool
FlowID::operator<(const wns::ldk::Key& _other) const
{
  assureType(&_other, const FlowID*);
  const FlowID* other = static_cast<const FlowID*>(&_other);
  return flowID < other->flowID;
}

std::string
FlowID::str() const
{
  std::stringstream ss;
  ss << "FlowID: " << flowID;
  return ss.str();
}

FlowIDBuilder::FlowIDBuilder(const wns::ldk::fun::FUN* _fun,
			     const wns::pyconfig::View& /* config */) :
  fun(_fun)
{
}

void
FlowIDBuilder::onFUNCreated()
{
  friends.rlcReader = fun->getProxy()->getCommandReader("rlc");
}

wns::ldk::ConstKeyPtr
FlowIDBuilder::operator () (const wns::ldk::CompoundPtr& compound, int /*direction*/) const
{
  return wns::ldk::ConstKeyPtr(new FlowID(this, compound));
} // operator()
