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

#include <LTE/helper/idprovider/PeerId.hpp>

#include <LTE/rlc/RLCCommand.hpp>
#include <LTE/macg/MACgCommand.hpp>

#include <WNS/service/Service.hpp>
#include <WNS/ldk/CommandReaderInterface.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/isClass.hpp>

using namespace lte::helper::idprovider;

PeerId::PeerId(wns::ldk::fun::FUN* fun) :
  layer2(fun->getLayer<dll::ILayer2*>()),
  rlcCommandReader(fun->getCommandReader("rlc")),
  macgCommandReader(fun->getCommandReader("macg")),
  key("MAC.PeerId"),
  stationType(layer2->getStationType())
{
}

PeerId::~PeerId()
{
}

void
PeerId::doVisit(wns::probe::bus::IContext& c, const wns::ldk::CompoundPtr& compound) const
{
  assure(compound, "Received NULL CompoundPtr");

  int peerIdInt = 0;
  wns::service::dll::UnicastAddress myAddress = layer2->getDLLAddress();
  wns::service::dll::UnicastAddress sourceAddress;
  wns::service::dll::UnicastAddress destinationAddress;
  wns::service::dll::UnicastAddress peerAddress;

  if ( rlcCommandReader->commandIsActivated(compound->getCommandPool()) ) {

    rlc::RLCCommand* rlcCommand =
      rlcCommandReader->readCommand<rlc::RLCCommand>(compound->getCommandPool());

    sourceAddress = rlcCommand->peer.source;
    destinationAddress = rlcCommand->peer.destination;

    if (myAddress == destinationAddress)
      {
	// it is the incoming path, i.e. the source is the peer
	peerAddress = sourceAddress;
      }
    else if (myAddress == sourceAddress)
      {
	// it is the outgoing path, i.e. the destination is the peer
	peerAddress = destinationAddress;
      }
  }
  // no RLC E2E peer address available, then ask the MACg next-hop address
  if ( peerAddress == wns::service::dll::UnicastAddress() && macgCommandReader->commandIsActivated(compound->getCommandPool())) {

    macg::MACgCommand* macgCommand =
      macgCommandReader->readCommand<macg::MACgCommand>(compound->getCommandPool());


    sourceAddress = macgCommand->peer.source;
    destinationAddress = macgCommand->peer.dest;

    if (myAddress == destinationAddress)
      {
	// it is the incoming path, i.e. the source is the peer
	peerAddress = sourceAddress;
      }
    else if (myAddress == sourceAddress)
      {
	// it is the outgoing path, i.e. the destination is the peer
	peerAddress = destinationAddress;
      }
  }
  // only if finally a peerAddress could be determined, write the probe context
  // e.g. for BCH there is no peer address, since it is broadcast
  if (peerAddress != wns::service::dll::UnicastAddress()) {
    peerIdInt = peerAddress.getInteger();
    c.insertInt( this->key, peerIdInt);
  }
}
