###############################################################################
# This file is part of openWNS (open Wireless Network Simulator)
# _____________________________________________________________________________
#
# Copyright (C) 2004-2007
# Chair of Communication Networks (ComNets)
# Kopernikusstr. 16, D-52074 Aachen, Germany
# phone: ++49-241-80-27910,
# fax: ++49-241-80-22242
# email: info@openwns.org
# www: http://www.openwns.org
# _____________________________________________________________________________
#
# openWNS is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License version 2 as published by the
# Free Software Foundation;
#
# openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

import lte.dll.timingScheduler

def getTimingConfig(stationType, mode):
    """
    stationType : RAP, UT
    """

    if mode.plm.mac.duplex == "FDD":
        return getFDDTimingConfig(stationType, mode)

def getFDDTimingConfig(stationType, mode):
    timer = lte.dll.timingScheduler.TimingScheduler(mode)
    # Setup the phases of the Timing Scheduler
    for i in xrange(mode.plm.mac.framesPerSuperFrame):
        timer.addStationTaskPhase(lte.dll.timingScheduler.StationTaskPhase(stationType, mode.plm.mac.frameLength))
        timer.phases.append( lte.dll.timingScheduler.Map(mode, i) )
        timer.phases.append( lte.dll.timingScheduler.Data(mode, i) )

    return timer
