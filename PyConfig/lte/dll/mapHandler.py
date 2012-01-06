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

from lte.modes.hasModeName import HasModeName

import openwns.logger
import openwns.FUN

class MapHandler(openwns.FUN.FunctionalUnit, HasModeName):
    __plugin__ = 'lte.controlplane.MapHandler'
    name = None
    logger = None
    txPower = "0 mW"
    phyMode = None
    freqChannels = None
    framesPerSuperFrame = None
    pduSize = None
    lowestMapChannel = None  # OFDMA subChannels to use for MAP DL broadcast
    highestMapChannel = None # ^ Each station should have a non-overlapping set
    writeMapOutput = False
    mapOutputFileNameDL = None
    mapOutputFileNameUL = None

    def __init__(self, mode):
        super(MapHandler,self).__init__(mode.modeName + mode.separator + 'mapHandler',
                                        mode.modeBase + mode.separator + 'mapHandler')
        self.setMode(mode)
        self.phyMode = mode.plm.mac.mapHandlerPhyMode
        self.freqChannels = mode.plm.mac.usedSubChannels
        self.lowestMapChannel = 1
        self.highestMapChannel = 1
        # calculate the pduSize from the duration of the Phase and the other PHY parameters,
        # taking into account modulation&coding with BPSK 1/2 => factor 1/2
        self.pduSize = int(mode.plm.mac.mapLength/(mode.plm.mac.symbolLength + mode.plm.mac.cpLength) * mode.plm.phy.phyResourceSize * (1.0/2.0))
        self.logger = openwns.logger.Logger("LTE", "MapH", True, mode.logger)
        self.framesPerSuperFrame = mode.plm.mac.framesPerSuperFrame
