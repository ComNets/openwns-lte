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

import lte.modes.hasModeName
import openwns.FUN
import openwns.logger
import openwns.distribution

class RACH(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):
    __plugin__ = None
    logger = None
    phyMode = None

    def __init__(self, mode, parentLogger = None):
        super(RACH,self).__init__(functionalUnitName = mode.modeName+self.separator+"rach",
                                  commandName = mode.modeBase+self.separator+"rach")
        self.setMode(mode)
        self.phyMode = mode.plm.mac.rachPhyMode

class UT(RACH):
    txPower = "0 mW"

    def __init__(self, mode):
        RACH.__init__(self, mode)
        self.__plugin__ = "lte.macr.RACH.UT"
        self.txPower = mode.plm.phy.txPwrUT.nominalPerSubband
        self.logger = openwns.logger.Logger("LTE", "RACH.UT", True, mode.logger)

class BS(RACH):
    def __init__(self, mode):
        RACH.__init__(self, mode)
        self.__plugin__ = "lte.macr.RACH.BS"
        self.logger = openwns.logger.Logger("LTE", "RACH.BS", True, mode.logger)

class ShortcutUT(openwns.FUN.FunctionalUnit):

    def __init__(self, mode):
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName=mode.modeName+mode.separator+"rach", commandName = mode.modeBase+mode.separator+"rach")
        self.__plugin__ = "lte.macr.RACH.ShortcutUT"

        self.delay = openwns.distribution.Fixed(0.010)
        self.offset = 0.0
        self.logger = openwns.logger.Logger("LTE", "RACH.BS", True, mode.logger)

class ShortcutBS(openwns.FUN.FunctionalUnit):

    def __init__(self, mode):
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName=mode.modeName+mode.separator+"rach", commandName = mode.modeBase+mode.separator+"rach")
        self.__plugin__ = "lte.macr.RACH.ShortcutBS"

        self.delay = openwns.distribution.Fixed(0.010)
        self.offset = 0.0
        self.logger = openwns.logger.Logger("LTE", "RACH.BS", True, mode.logger)
