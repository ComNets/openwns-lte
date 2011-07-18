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

import openwns.FUN
import lte.modes.hasModeName
import openwns.distribution

class Handler(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):
    __plugin__= None
    commandSize = None
    pcchSize = None
    logger = None
    cpDispatcherName = None
    usesShortcut = None

    def __init__(self, mode):
        super(Handler,self).__init__(functionalUnitName = mode.modeName + mode.separator + 'RRHandler',
                                     commandName = mode.modeBase + mode.separator + 'RRHandler')
        self.setMode(mode)
        self.commandSize = 48 # Bit
        self.usesShortcut = False
        self.pcchSize = self.commandSize
        self.cpDispatcherName = "controlPlaneDispatcher";

class UT(Handler):

    def __init__(self, mode):
        Handler.__init__(self, mode)
        self.__plugin__= "lte.controlplane.RRHandler.UT"
        self.updatesOnly = False
        self.logger = openwns.logger.Logger("LTE", "RRHandler.UT", True, mode.logger)

class BS(Handler):
    ulSegmentSize = None
    def __init__(self, mode):
        self.__plugin__= "lte.controlplane.RRHandler.BS"
        Handler.__init__(self, mode)
        self.logger = openwns.logger.Logger("LTE", "RRHandler.BS", True, mode.logger)
        self.ulSegmentSize = mode.plm.mac.ulSegmentSize

class ShortcutUT(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):

    __plugin__ = None
    delay = None
    offset = None
    logger = None

    def __init__(self, mode):
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName=mode.modeName+mode.separator+"rrhandlershortcut", commandName = mode.modeBase+mode.separator+"rrhandlershortcut")
        self.setMode(mode)
        self.__plugin__ = "lte.controlplane.RRHandler.ShortcutUT"

        self.delay = openwns.distribution.Fixed(0.001)
        self.offset = 0.0
        self.logger = openwns.logger.Logger("LTE", "RRHandlerShortcut.UT", True, mode.logger)

class ShortcutBS(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):

    __plugin__ = None
    delay = None
    offset = None
    logger = None

    def __init__(self, mode):
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName=mode.modeName+mode.separator+"rrhandlershortcut", commandName = mode.modeBase+mode.separator+"rrhandlershortcut")
        self.setMode(mode)
        self.__plugin__ = "lte.controlplane.RRHandler.ShortcutBS"

        self.delay = openwns.distribution.Fixed(0.001)
        self.offset = 0.0
        self.logger = openwns.logger.Logger("LTE", "RRHandlerShortcut.BS", True, mode.logger)
