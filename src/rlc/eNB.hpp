/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#ifndef LTE_RLC_ENB_HPP
#define LTE_RLC_ENB_HPP

#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/controlplane/flowmanagement/IFlowManager.hpp>

#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/logger/Logger.hpp>

namespace lte { namespace controlplane { namespace flowmanager { class IFlowSwitching;}}}

namespace lte { namespace rlc {

class ENBRLC:
    public wns::ldk::Processor<ENBRLC>,
    public wns::ldk::HasReceptor<>,
    public wns::ldk::HasConnector<>,
    public wns::ldk::HasDeliverer<>,
    public wns::ldk::CommandTypeSpecifier<RLCCommand>,
    public wns::Cloneable<ENBRLC>
{
public:

  ENBRLC(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

  ~ENBRLC();
  
  virtual void processOutgoing(const wns::ldk::CompoundPtr& compound);
  
  virtual void processIncoming(const wns::ldk::CompoundPtr& compound);

  virtual wns::ldk::CommandPool*
  createReply(const wns::ldk::CommandPool* original) const;

  virtual void
  onFUNCreated();

private:
  struct {
    lte::controlplane::flowmanager::IFlowSwitching* flowswitching;
  } friends;

  wns::ldk::CommandReaderInterface* upperConvergenceReader;
  wns::logger::Logger logger;
};

} // rlc
} // lte

#endif // LTE_RLC_ENB_HPP
