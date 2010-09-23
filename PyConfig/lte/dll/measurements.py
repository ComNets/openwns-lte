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

class PhyMeasurement(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):
    __plugin__ = "lte.helper.PhyMeasurement"

    phyModeMapper = None
    logger = None

    def __init__(self, mode, phyModeMapper, parentLogger = None):
        self.setMode(mode)
        fuName = self.modeName+self.separator+'PhyMeasurement'
        super(PhyMeasurement, self).__init__(functionalUnitName = fuName)
        self.logger = openwns.logger.Logger(moduleName = "LTE",
                                            name = "PhyMeasurement",
                                            enabled = True,
                                            parent = parentLogger)
        self.phyModeMapper = phyModeMapper
