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

#ifndef LTE_HELPER_HASMODENAME_HPP
#define LTE_HELPER_HASMODENAME_HPP

#include <WNS/pyconfig/View.hpp>
#include <WNS/Assure.hpp>

namespace lte
{

namespace helper
{

class HasModeName
{

public:
    HasModeName( const wns::pyconfig::View& config ) :
            mode( config.get<std::string>( "modeName" ) ),
            modeBase( config.get<std::string>( "modeBase" ) ),
            separator( config.get<std::string>( "separator" ) ),
            taskID( config.get<std::string>( "taskID" ) ),
            rsNameSuffix( config.get<std::string>( "rsNameSuffix" ) )
    {
        assure( modeBase == getTaskStrippedModeName(), "Mode Base Name mismatch!" );
    };

    std::string getMode()
    {
        return mode;
    };

    std::string getRSName()
    {
        return mode+separator+rsNameSuffix;
    };

    std::string getTaskSuffix()
    {
        return taskID;
    };

    std::string getTimerName()
    {
        return modeBase+separator+"Timer";
    };

    std::string getTaskTimerName( std::string task )
    {
        return modeBase+separator+task+separator+"Timer";
    };

protected:
    std::string mode;
    std::string modeBase;
    std::string separator;
    std::string taskID;
    std::string rsNameSuffix;

private:
    std::string getTaskStrippedModeName()
    {
        if ( taskID!="" )
        {
            /** Assuming that the taskID is appended to the actual
             * modename with the separator */
            assure( mode.find( separator+taskID ) != std::string::npos, "Can't strip Task from modeName" );
            std::string retVal = mode;
            retVal.erase( retVal.find( separator+taskID ), std::string::npos );
            return retVal;
        }
        else
            return mode;
    };

};

} // helper
} // lte

#endif // NOT defined LTE_HELPER_HASMODENAME_HPP


