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
/* deleted by chen */
// #include <LTE/macr/Mode.hpp>

#include <vector>


namespace lte {

/* deleted by chen */
//   namespace controlplane { namespace flowmanagement {
//       class FlowManagerBS;
//     }}
/* inserted by chen */
    class lteDummy;

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

      std::vector<std::string>
      getTasks() const;

/* deleted by chen */
//       lte::controlplane::flowmanagement::FlowManagerBS*
//       getFlowManagerOfMasterBS() const;
/* inserted by chen */
      lte::lteDummy*
      getFlowManagerOfMasterBS() const;

    private:
      Layer2(const Layer2&);	//!< disallow copy constructor
      Layer2& operator=(const Layer2&); //!< disallow assignment

      virtual void
      doStartup();

      std::vector<std::string> tasks;

      /** @brief Will be used magic by the AssociationhandlerRN to get
       * the DCCH FlowID for forwarding associationReqs
       */

/* deleted by chen */
//       lte::controlplane::flowmanagement::FlowManagerBS* flowManagerOfMasterBS;
/* inserted by chen */
      lte::lteDummy* flowManagerOfMasterBS;

    };
  }}
#endif // NOT defined LTE_MAIN_LAYER2_HPP
