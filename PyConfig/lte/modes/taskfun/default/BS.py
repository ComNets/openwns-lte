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

import lte.dll.controlplane.association
import lte.dll.controlplane.flowmanager

from lte.support.helper import connectFUs
import openwns.FlowSeparator
import openwns.ldk
import openwns.logger

class BS:

    def __init__(self, fun, mode, parentLogger = None):
        self.mode = mode
        self.taskID = ""

        self.logger = openwns.logger.Logger('LTE', self.mode.modeName, True, parentLogger)

        associationHandler = lte.dll.controlplane.association.AssociationHandlerBS(
            self.mode.modeName, self.mode,
            self.mode.modeName + self.mode.separator + 'associationHandler',
            self.mode.modeBase + self.mode.separator + 'associationHandler',
            self.logger)
        associationHandler.taskID = self.taskID
        fun.add(associationHandler)  

        flowHandler = lte.dll.controlplane.flowmanager.FlowHandlerBS(
            self.mode.modeName,
            self.mode.modeName + self.mode.separator + 'FlowHandler',
            self.mode.modeBase + self.mode.separator + 'FlowHandlerCommand',
            self.logger)
        flowHandler.taskID = self.taskID
        fun.add(flowHandler)

        controlPlaneDispatcher = openwns.ldk.Multiplexer.Dispatcher(
            opcodeSize = 0,
            functionalUnitName = mode.modeName + mode.separator + 'controlPlaneDispatcher',
            commandName = mode.modeBase + mode.separator + 'controlPlaneDispatcher')

        fun.add(controlPlaneDispatcher)

        lowerFlowGate = openwns.FlowSeparator.FlowGate(fuName = self.mode.modeName + self.mode.separator + 'lowerFlowGate',
                                                       keyBuilder = lte.dll.controlplane.flowmanager.FlowID(),
                                                       parentLogger = self.logger)
        fun.add(lowerFlowGate)

        connectFUs([
                (flowHandler, controlPlaneDispatcher),
                (associationHandler, controlPlaneDispatcher),
                ])
