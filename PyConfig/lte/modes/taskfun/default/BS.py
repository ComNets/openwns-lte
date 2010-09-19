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
import lte.dll.resourceScheduler
import lte.dll.mapHandler
import lte.dll.rrHandler
import lte.modes.timing.timingConfig
import lte.llmapping.default
from lte.support.helper import connectFUs

import openwns.FlowSeparator
import openwns.Scheduler
import openwns.ldk
import openwns.logger

class BS:

    def __init__(self, fun, mode, parentLogger = None):
        self.mode = mode
        self.taskID = ""

        self.logger = openwns.logger.Logger('LTE', self.mode.modeName, True, parentLogger)

        self.timer = lte.modes.timing.timingConfig.getTimingConfig('RAP', self.mode)

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

        schedulerTX = self._setupTxScheduler(fun, mode, parentLogger)
        fun.add(schedulerTX)

        schedulerRX = self._setupRxScheduler(fun, mode, parentLogger)
        fun.add(schedulerRX)

        mapHandler = lte.dll.mapHandler.MapHandler(mode)
        fun.add(mapHandler)

        # opcodesize set to 0 for correct PDU size calculation
        dispatcher = lte.dll.dispatcher.NamedDispatcher(
            opcodeSize = 0,
            modeName = mode.modeName,
            functionalUnitName = mode.modeName + mode.separator + 'dispatcher',
            commandName = mode.modeBase + mode.separator + 'dispatcher',
            parentLogger = self.logger)
        dispatcher.logger.enabled = False
        fun.add(dispatcher)
        
        rrHandler = lte.dll.rrHandler.BS(mode)
        rrHandler.cpDispatcherName = "rrhandlershortcut"
        rrHandler.usesShortcut = True
        fun.add(rrHandler)

        rrHandlerShortcut = lte.dll.rrHandler.ShortcutBS(mode)
        fun.add(rrHandlerShortcut)

        connectFUs([
                (lowerFlowGate, controlPlaneDispatcher),
                (controlPlaneDispatcher, schedulerTX),

                (flowHandler, controlPlaneDispatcher),
                (associationHandler, controlPlaneDispatcher),

                (rrHandler, rrHandlerShortcut),

                (schedulerTX, dispatcher),
                (schedulerRX, dispatcher),
                (mapHandler, dispatcher),
                ])

        self.top = lowerFlowGate
        self.bottom = dispatcher

    def _setupTxScheduler(self, fun, mode, parentLogger):
        rsNameSuffix = 'resourceScheduler'
        txFUName = mode.modeName + mode.separator + rsNameSuffix +'TX'
        txTaskName = mode.modeBase + mode.separator + rsNameSuffix +'TX'

        strategyTX = openwns.Scheduler.ExhaustiveRR(parentLogger=self.logger)

        group = 1

        phyModeMapping = lte.llmapping.default.LTEMapper(mode)

        resourceSchedulerTX = lte.dll.resourceScheduler.BS(mode, mode.plm, strategyTX, txFUName, txTaskName, False, group, self.logger)
        resourceSchedulerTX.setPhyModeMapper(phyModeMapping)

        return resourceSchedulerTX

    def _setupRxScheduler(self, fun, mode, parentLogger):
        rsNameSuffix = 'resourceScheduler'
        txFUName = mode.modeName + mode.separator + rsNameSuffix +'TX'
        rxFUName = mode.modeName + mode.separator + rsNameSuffix +'RX'
        rxTaskName = mode.modeBase + mode.separator + rsNameSuffix +'RX'

        strategyRX = openwns.Scheduler.RoundRobinUL(parentLogger=self.logger,
                                                    blockDuration = mode.plm.mac.symbolsPerBlock*mode.plm.mac.fullSymbolDur)

        group = 1

        phyModeMapping = lte.llmapping.default.LTEMapper(mode)

        resourceSchedulerRX = lte.dll.resourceScheduler.BS(mode, mode.plm, strategyRX,
                                                           rxFUName, rxTaskName,
                                                           uplinkMaster = True, group = group, parentLogger = self.logger,
                                                           txSchedulerFUName = txFUName
                                                           )
        resourceSchedulerRX.setPhyModeMapper(phyModeMapping)

        return resourceSchedulerRX
