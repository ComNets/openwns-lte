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

from lte.modes.mode import Mode

import openwns.logger
import openwns.FUN

class Best(openwns.StaticFactoryClass):

    def __init__(self):
        openwns.StaticFactoryClass.__init__(self, 'lte.macg.strategy.Best')
        name = 'Best'

class MACg(openwns.FUN.FunctionalUnit):
    scorerNameSuffixes = None
    separator = None
    modes = None
    strategy = None
    logger = None

    task = None

    def __init__(self, functionalUnitName, commandName, task):
        super(MACg,self).__init__(functionalUnitName, commandName)
        self.scorerNameSuffixes = {}
        self.separator = Mode.separator
        self.modes = []
        self.task = task

    def addMode(self, mode):
        self.modes.append(mode)

class UserTerminal(MACg, openwns.StaticFactoryClass):

    def __init__(self, fuName, commandName, parentLogger):
        MACg.__init__(self, fuName, commandName, "")
        openwns.StaticFactoryClass.__init__(self, 'lte.macg.UserTerminal')

        self.strategy = Best()
        self.logger = openwns.logger.Logger('LTE','MACgUT',True, parentLogger)

class BaseStation(MACg, openwns.StaticFactoryClass):

    def __init__(self, fuName, commandName,parentLogger):
        MACg.__init__(self, fuName, commandName, "")
        openwns.StaticFactoryClass.__init__(self, 'lte.macg.BaseStation')

        self.strategy = Best()
        self.logger = openwns.logger.Logger('LTE','MACgBS',True, parentLogger)
