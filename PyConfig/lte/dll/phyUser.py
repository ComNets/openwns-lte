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
import openwns

class PhyUser(openwns.FUN.FunctionalUnit,
              openwns.StaticFactoryClass,
              lte.modes.hasModeName.HasModeName):

    def __init__(self, modeName, plm, functionalUnitName = "phyUser", schedulingCommandReaderName = "Unknown", parentLogger=None):
        openwns.StaticFactoryClass.__init__(self, 'lte.macr.PhyUser')
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName = functionalUnitName)

        self.modeName = modeName

        self.plm = plm

        self.measurementDelay = 6.0e-03

        self.logger = openwns.logger.Logger('LTE','PhyUser', True, parentLogger)

        self.schedulingCommandReaderName = schedulingCommandReaderName
