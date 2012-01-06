# -*- coding: utf-8 -*-
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
import lte.dll.rach
import lte.dll.measurements
import lte.modes.timing.timingConfig
import lte.llmapping.default
from lte.support.helper import connectFUs

import openwns.Scheduler
import openwns.ldk
import openwns.logger
import openwns.FUN
import openwns.Group
import openwns.FlowSeparator

class BS:

    def __init__(self, fun, mode, parentLogger = None):
        self.mode = mode
        self.taskID = ""

        self.logger = openwns.logger.Logger('LTE', self.mode.modeName, True, parentLogger)

        self.timer = lte.modes.timing.timingConfig.getTimingConfig('RAP', self.mode)

        bchBuffer = openwns.ldk.Buffer.Bounded(size = 1500,
                                               lossRatioProbeName = "lte.bchBufferLossRatio",
                                               sizeProbeName = "lte.bchBufferSize",
                                               functionalUnitName = mode.modeName + mode.separator + "bchBuffer",
                                               commandName = mode.modeBase + mode.separator + "bchBuffer")
        fun.add(bchBuffer)

#        bch = lte.dll.bch.RAP(mode, parentLogger = self.logger)
        bch = lte.dll.bch.RAPNoSched(mode, parentLogger = self.logger)
        fun.add(bch)

        rach = lte.dll.rach.ShortcutBS(mode)
        fun.add(rach)

        rachDispatcher = openwns.ldk.Multiplexer.Dispatcher(
            opcodeSize = 0,
            functionalUnitName = mode.modeName + mode.separator + 'rachDispatcher',
            commandName = mode.modeBase + mode.separator + 'rachDispatcher')
        fun.add(rachDispatcher)

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

        flowHandlerShortcut = lte.dll.controlplane.flowmanager.ShortcutBS(mode)
        fun.add(flowHandlerShortcut)

        controlPlaneDispatcher = openwns.ldk.Multiplexer.Dispatcher(
            opcodeSize = 0,
            functionalUnitName = mode.modeName + mode.separator + 'controlPlaneDispatcher',
            commandName = mode.modeBase + mode.separator + 'controlPlaneDispatcher')

        fun.add(controlPlaneDispatcher)

        queueProbe = openwns.Probe.Tick('lte.schedulerQueue')
        fun.add(queueProbe)

        lowerFlowSep = self._setupUnacknowledgedModePerFlow(fun, mode,
                                                            validFlowNeeded = True,
                                                            name="um",
                                                            commandName = "um",
                                                            separatorName="lowerFlowSeparator")
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
        mapHandler.txPower = mode.plm.phy.txPwrBS.nominalPerSubband
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

        phyMeasurement = self._setupPhyMeasurements(fun, mode)
        fun.add(phyMeasurement)

        connectFUs([
                (lowerFlowSep, lowerFlowGate),
                (lowerFlowGate, controlPlaneDispatcher),
                (controlPlaneDispatcher, queueProbe),
                (queueProbe, schedulerTX),

                (associationHandler, controlPlaneDispatcher),
                (associationHandler, bchBuffer),
                (associationHandler, rachDispatcher),

                (bchBuffer, bch),
#                (bch, controlPlaneDispatcher),
                (bch, dispatcher),

                (flowHandler, flowHandlerShortcut),

                (rrHandler, rrHandlerShortcut),
                (rrHandler, rachDispatcher),

                (rachDispatcher, rach),

                (schedulerTX, dispatcher),
                (schedulerRX, dispatcher),
                (mapHandler, dispatcher),
                (dispatcher, phyMeasurement),
                ])

        self.top = lowerFlowSep
        self.bottom = phyMeasurement

    def _setupTxScheduler(self, fun, mode):
        rsNameSuffix = 'resourceScheduler'
        txFUName = mode.modeName + mode.separator + rsNameSuffix +'TX'
        txTaskName = mode.modeBase + mode.separator + rsNameSuffix +'TX'

        group = 1

        phyModeMapping = lte.llmapping.default.LTEMapper.getInstance(mode)

        powerLimit = mode.plm.phy.txPwrBS.maxOverall
    
        resourceSchedulerTX = lte.dll.resourceScheduler.BS(mode, txFUName, txTaskName, False, group, self.logger, maxTxPower=powerLimit)
        resourceSchedulerTX.setPhyModeMapper(phyModeMapping)

        return resourceSchedulerTX

    def _setupRxScheduler(self, fun, mode):
        rsNameSuffix = 'resourceScheduler'
        txFUName = mode.modeName + mode.separator + rsNameSuffix +'TX'
        rxFUName = mode.modeName + mode.separator + rsNameSuffix +'RX'
        rxTaskName = mode.modeBase + mode.separator + rsNameSuffix +'RX'

        group = 1

        phyModeMapping = lte.llmapping.default.LTEMapper.getInstance(mode)

        resourceSchedulerRX = lte.dll.resourceScheduler.BS(mode,
                                                           rxFUName, rxTaskName,
                                                           uplinkMaster = True, group = group, parentLogger = self.logger,
                                                           txSchedulerFUName = txFUName,
                                                           maxTxPower = "0.0 dBm"
                                                           )
        resourceSchedulerRX.setPhyModeMapper(phyModeMapping)

        return resourceSchedulerRX

    def _setupUnacknowledgedModePerFlow(self, fun, mode, validFlowNeeded, name, commandName, separatorName):

        lowerSubFUN = openwns.FUN.FUN(self.logger)
        
        # Unacknowledged Mode FU
        _functionalUnitName = mode.modeName + mode.separator + name
        _commandName = mode.modeBase + mode.separator + commandName
        segmentSize = mode.plm.mac.dlSegmentSize
        um = lte.dll.rlc.UnacknowledgedMode(segmentSize = segmentSize - 1,
                                            headerSize = 1,
                                            commandName=_commandName,
                                            parentLogger = self.logger)
        um.sduLengthAddition = 16
        um = openwns.FUN.Node(_functionalUnitName, um, _commandName)
        
        lowerSubFUN.add(um)

        lowerGroup = openwns.Group.Group(lowerSubFUN, um.functionalUnitName, um.functionalUnitName)

        return self._flowSeparated(mode, separatorName, lowerGroup, validFlowNeeded)

    def _flowSeparated(self, mode, name, group, validFlowNeeded):

        config = openwns.FlowSeparator.Config( mode.modeName + mode.separator + name + 'Prototype', group)

        if validFlowNeeded:
            strategy = openwns.FlowSeparator.CreateOnValidFlow(config, fipName = 'FlowManagerBS')
        else:
            strategy = openwns.FlowSeparator.CreateOnFirstCompound(config)

        flowSeparator = openwns.FlowSeparator.FlowSeparator(
            lte.dll.controlplane.flowmanager.FlowID(),
            strategy,
            'lowerFlowSeparator',
            self.logger,
            functionalUnitName = mode.modeName + mode.separator + name,
            commandName = mode.modeBase + mode.separator + name)

        return flowSeparator

    def _setupPhyMeasurements(self, fun, mode):

        phyModeMapping = lte.llmapping.default.LTEMapper.getInstance(mode)
        phyMeasurement = lte.dll.measurements.PhyMeasurement(mode, phyModeMapping)
        return phyMeasurement

