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

#ifndef LTE_CONTROLPLANE_MAPHANDLERINTERFACE_HPP
#define LTE_CONTROLPLANE_MAPHANDLERINTERFACE_HPP

#include <WNS/scheduler/SchedulingMap.hpp>

namespace lte {

    namespace controlplane {
 
         /** @brief Interface to notify Observers that require knowledge about
          * whether we have been granted resources or not */
         class ResourceGrantNotificationInterface
         {
         public:
             virtual
             ~ResourceGrantNotificationInterface(){}
 
             /** @brief Inform Observer whether the received map contained
              * allocations for us.
              *
              * When the parameter is false, it means that the mapHandler has
              * received a map but no resources were assigned to this terminal, true
              * means that a map was received which DID contain resource allocation info
              * for this terminal
              */
             virtual void
             resourcesGranted(bool state) = 0;
         };

        class IMapHandlerTiming
        {
        public:
            virtual void
            startMapTx(simTimeType duration, std::vector<int> dlFrameNumbers, std::vector<int> ulFrameNumbers) = 0;

            virtual void
            startMapRx() = 0;

            virtual void
            resetResources(int frameNr) = 0;

            virtual void
            setCurrentPhase() = 0;
        };

	class IMapHandlerRS
	{
	public:
	    /** @brief UT asks for allowed resources (old method?). timing/ut/Events.cpp */
	    virtual wns::scheduler::SchedulingMapPtr
	    getMasterMapForSlaveScheduling(int frameNr) = 0;

	    virtual void
	    saveDLMap(int frameNr, wns::scheduler::SchedulingMapPtr schedulingMap) = 0;
	    
	    /** @brief put scheduled resources from RS and save into SuperFrame map */
	    virtual void
	    saveULMap(int frameNr, wns::scheduler::SchedulingMapPtr schedulingMap) = 0;
	};
    }
} // namespace lte::controlplane


#endif // NOT defined LTE_CONTROLPLANE_MAPHANDLERINTERFACE_HPP


