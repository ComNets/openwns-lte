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
import lte.dll.bch
import lte.modes.timing.timingConfig
import lte.llmapping.default
from lte.support.helper import connectFUs

import openwns.Scheduler
import openwns.ldk
import openwns.logger
import openwns.FUN
import openwns.Group
import openwns.FlowSeparator

class UT:

    def __init__(self, fun, mode, parentLogger = None):
        self.mode = mode
        self.taskID = ""

        self.logger = openwns.logger.Logger('LTE', self.mode.modeName, True, parentLogger)

        self.timer = lte.modes.timing.timingConfig.getTimingConfig('UT', self.mode)

        bch = lte.dll.bch.UT(mode, parentLogger = self.logger)
        fun.add(bch)

        associationHandler = lte.dll.controlplane.association.AssociationHandlerUT(
            self.mode.modeName, self.mode,
            self.mode.modeName + self.mode.separator + 'associationHandler',
            self.mode.modeBase + self.mode.separator + 'associationHandler',
            self.logger)
        associationHandler.taskID = self.taskID
        fun.add(associationHandler)

        flowHandler = lte.dll.controlplane.flowmanager.FlowHandlerUT(
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

        lowerFlowSep = self._setupLowerFlowSeparator(fun, mode)
        fun.add(lowerFlowSep)

        lowerFlowGate = openwns.FlowSeparator.FlowGate(fuName = self.mode.modeName + self.mode.separator + 'lowerFlowGate',
                                                       keyBuilder = lte.dll.controlplane.flowmanager.FlowID(),
                                                       parentLogger = self.logger)
        fun.add(lowerFlowGate)

        schedulerTX = self._setupTxScheduler(fun, mode)
        fun.add(schedulerTX)

        schedulerRX = self._setupRxScheduler(fun, mode)
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
        
        rrHandler = lte.dll.rrHandler.UT(mode)
        rrHandler.cpDispatcherName = "rrhandlershortcut"
        rrHandler.usesShortcut = True
        fun.add(rrHandler)

        rrHandlerShortcut = lte.dll.rrHandler.ShortcutUT(mode)
        fun.add(rrHandlerShortcut)

        connectFUs([
                (lowerFlowSep, lowerFlowGate),
                (lowerFlowGate, controlPlaneDispatcher),
                (controlPlaneDispatcher, schedulerTX),

                (associationHandler, controlPlaneDispatcher),

                (bch, controlPlaneDispatcher),
                (flowHandler, controlPlaneDispatcher),

                (rrHandler, rrHandlerShortcut),

                (schedulerTX, dispatcher),
                (schedulerRX, dispatcher),
                (mapHandler, dispatcher),
                ])

        self.top = lowerFlowSep
        self.bottom = dispatcher

    def _setupTxScheduler(self, fun, mode):
        rsNameSuffix = 'resourceScheduler'
        txFUName = mode.modeName + mode.separator + rsNameSuffix +'TX'
        txTaskName = mode.modeBase + mode.separator + rsNameSuffix +'TX'

        group = 1

        phyModeMapping = lte.llmapping.default.LTEMapper(mode)

        resourceSchedulerTX = lte.dll.resourceScheduler.UT(mode, txFUName, txTaskName, group, self.logger)
        resourceSchedulerTX.setPhyModeMapper(phyModeMapping)

        return resourceSchedulerTX

    def _setupRxScheduler(self, fun, mode):
        rsNameSuffix = 'resourceScheduler'
        rxFUName = mode.modeName + mode.separator + rsNameSuffix +'RX'
        rxTaskName = mode.modeBase + mode.separator + rsNameSuffix +'RX'

        resourceSchedulerRX = lte.dll.resourceScheduler.No(rxFUName, rxTaskName)

        return resourceSchedulerRX

    def _setupLowerFlowSeparator(self, fun, mode):

        lowerSubFUN = openwns.FUN.FUN(self.logger)
        
        # Unacknowledged Mode FU
        _functionalUnitName = mode.modeName + mode.separator + 'um'
        _commandName = mode.modeBase + mode.separator + 'um'
        segmentSize = mode.plm.mac.dlSegmentSize
        um = lte.dll.rlc.UnacknowledgedMode(segmentSize = segmentSize - 1,
                                            headerSize = 1,
                                            commandName=_commandName,
                                            parentLogger = self.logger)
        um.sduLengthAddition = 16
        um = openwns.FUN.Node(_functionalUnitName, um, _commandName)
        
        lowerSubFUN.add(um)

        lowerGroup = openwns.Group.Group(lowerSubFUN, um.functionalUnitName, um.functionalUnitName)

        return self._flowSeparated(mode, "lowerFlowSeparator", lowerGroup)

    def _flowSeparated(self, mode, name, group):
        config = openwns.FlowSeparator.Config( mode.modeName + mode.separator + name + 'Prototype', group)

        flowSeparator = openwns.FlowSeparator.FlowSeparator(
            lte.dll.controlplane.flowmanager.FlowID(),
            openwns.FlowSeparator.CreateOnValidFlow(config, fipName = 'FlowManagerBS'),
            'lowerFlowSeparator',
            self.logger,
            functionalUnitName = mode.modeName + mode.separator + name,
            commandName = mode.modeBase + mode.separator + name)

        return flowSeparator
