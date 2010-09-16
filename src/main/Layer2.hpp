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

#ifndef LTE_MAIN_LAYER2_HPP
#define LTE_MAIN_LAYER2_HPP

#include <DLL/Layer2.hpp>

#include <vector>


namespace lte {


  namespace controlplane { namespace flowmanagement {
       class FlowManagerBS;
    }}

  namespace main {

    /** @brief DLL for (WINNER Protocol Stack) */
    class Layer2 :
      public dll::Layer2
    {
    public:
      Layer2(wns::node::Interface*, const wns::pyconfig::View&);
      virtual ~Layer2();

      /** @name wns::node::component::Component Interface */
      //@{
      /** @brief ComponentInterface: called when this node is ready (all FU
       * objects initialized). Sets e.g. ResourceSchedulerInterface */
      virtual void onNodeCreated();
      /** @brief ComponentInterface: called when all nodes are ready (all
       * global FU objects initialized). builds associations -> routes ->
       * destinations */
      virtual void onWorldCreated();

      /** @brief Called before EventScheduler is really stopped */
      virtual void onShutdown();
      //@}

    private:
      Layer2(const Layer2&);	//!< disallow copy constructor
      Layer2& operator=(const Layer2&); //!< disallow assignment

      virtual void
      doStartup();
    };
  }}
#endif // NOT defined LTE_MAIN_LAYER2_HPP
