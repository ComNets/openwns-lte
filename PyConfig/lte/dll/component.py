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

import lte.dll.upperConvergence
import lte.dll.rlc
import lte.dll.macg
import lte.dll.phyUser
import lte.dll.dispatcher
import lte.dll.controlplane.association
import lte.dll.controlplane.flowmanager
import lte.partitioning.service
from lte.support.helper import connectFUs

import dll.Layer2
import dll.Services

import openwns.FUN
import openwns.Tools
import openwns.FlowSeparator

class eNBLayer2( dll.Layer2.Layer2 ):

    def __init__(self, node, name, modetypes, parentLogger=None):
        dll.Layer2.Layer2.__init__(self, node, name, parentLogger)
        self.nameInComponentFactory = "lte.Layer2"
        self.setStationID(node.nodeID)
        self.setStationType(node.getProperty("Type"))
        self.ring = node.getProperty("Ring")
        self.associations = []
        self.phyUsers = {}
        self.phyDataTransmission = {}
        self.phyNotification = {}

        self._setupControlServices()

        # Build the FUN
        self.fun = openwns.FUN.FUN(self.logger)

        upperConvergence = lte.dll.upperConvergence.eNB(self.logger)
        self.fun.add(upperConvergence)

        upperSynchronizer = openwns.Tools.Synchronizer(commandName='upperSynchronizer')
        self.fun.add(upperSynchronizer)

        upperFlowGate = openwns.FlowSeparator.FlowGate(fuName = 'upperFlowGate',
                                                       keyBuilder = lte.dll.controlplane.flowmanager.FlowID(),
                                                       parentLogger = self.logger)
        self.fun.add(upperFlowGate)

        arqFlowGate = openwns.FlowSeparator.FlowGate(fuName = 'arqFlowGate',
                                                     keyBuilder = lte.dll.controlplane.flowmanager.FlowID(),
                                                     parentLogger = self.logger)
        self.fun.add(arqFlowGate)

        macg = lte.dll.macg.BaseStation(fuName='macg', commandName='macg', parentLogger = self.logger)
        self.fun.add(macg)

        rlc = lte.dll.rlc.eNBRLC(self.logger)
        self.fun.add(rlc)

        # Each mode has its own PhyUser
        for mt in modetypes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(parentLogger = self.logger, default=False)
            
            self._setupControlServicesPerMode(aMode)

            self._setupManagementServicesPerMode(aMode)

            tf = aMode.createTaskFUN(self.fun, "BS")

            self.phyUsers[aMode.modeName] = aMode.phyUser

            self.fun.add(self.phyUsers[aMode.modeName])

            self.managementServices.append(tf.timer)

            connectFUs([
                    (tf.bottom, aMode.phyUser),
                    (macg, tf.top),
                    ])

        connectFUs([
                (upperConvergence, rlc),
                (rlc, upperSynchronizer),
                (upperSynchronizer, upperFlowGate),
                (upperFlowGate, arqFlowGate),
                (arqFlowGate, macg)
                ])

    def _setupControlServices(self):
        self.associationsProxy = lte.dll.controlplane.association.ENBAssociationsProxy(self.logger)
        self.controlServices.append(self.associationsProxy)

        self.flowManager = lte.dll.controlplane.flowmanager.FlowManagerBS(self.logger)
        self.controlServices.append(self.flowManager)

    def _setupControlServicesPerMode(self, mode):
        # Each mode provides association information service
        self.controlServices.append(dll.Services.Association(mode.modeName, mode.logger))
        
        p = lte.partitioning.service.PartitioningInfo(mode.defaultPartitioning,
                                                      mode.modeName,
                                                      mode.logger)
        mode.partInfo = p
        self.controlServices.append(p)

    def _setupManagementServicesPerMode(self, mode):
        i = dll.Services.InterferenceCache("INTERFERENCECACHE"+mode.modeName,
                                           alphaLocal = 0.2,
                                           alphaRemote= 0.05,
                                           parent = mode.logger)
        # If we have no value, we assume that we are located at the cell edge
        # and have an SINR of 1dB with interference set to background noise
        pathloss = float(mode.plm.phy.txPwrUT.nominalPerSubband.replace(" dBm","")) + 99.00
        i.notFoundStrategy.averageCarrier = "-99.0 dBm"
        i.notFoundStrategy.averageInterference = "-100.0 dBm"
        i.notFoundStrategy.deviationCarrier = "0.0 mW"
        i.notFoundStrategy.deviationInterference = "0.0 mW"
        i.notFoundStrategy.averagePathloss = "%f dB" % pathloss

        mode.interfCache = i
        self.managementServices.append(i)

    def setPhyDataTransmission(self, modeName, serviceName):
        """set the name of the PHY component for a certain mode"""
        self.phyDataTransmission[modeName] = serviceName

    def setPhyNotification(self, modeName, serviceName):
        self.phyNotification[modeName] = serviceName

class ueLayer2( dll.Layer2.Layer2 ):

    def __init__(self, node, name, modetypes, parentLogger=None):
        dll.Layer2.Layer2.__init__(self, node, name, parentLogger)
        self.nameInComponentFactory = "lte.Layer2"
        self.setStationID(node.nodeID)
        self.setStationType(node.getProperty("Type"))
        self.ring = node.getProperty("Ring")
        self.associations = []
        self.phyUsers = {}
        self.phyDataTransmission = {}
        self.phyNotification = {}

        self._setupControlServices()

        self.fun = openwns.FUN.FUN(self.logger)

        upperConvergence = lte.dll.upperConvergence.UE(self.logger)
        self.fun.add(upperConvergence)

        upperSynchronizer = openwns.Tools.Synchronizer(commandName='upperSynchronizer')
        self.fun.add(upperSynchronizer)

        upperFlowGate = openwns.FlowSeparator.FlowGate(fuName = 'upperFlowGate',
                                                       keyBuilder = lte.dll.controlplane.flowmanager.FlowID(),
                                                       parentLogger = self.logger)
        self.fun.add(upperFlowGate)

        arqFlowGate = openwns.FlowSeparator.FlowGate(fuName = 'arqFlowGate',
                                                     keyBuilder = lte.dll.controlplane.flowmanager.FlowID(),
                                                     parentLogger = self.logger)
        self.fun.add(arqFlowGate)

        macg = lte.dll.macg.UserTerminal(fuName='macg', commandName='macg', parentLogger = self.logger)
        self.fun.add(macg)

        rlc = lte.dll.rlc.UERLC(self.logger)
        self.fun.add(rlc)

        # Each mode has its own PhyUser
        for mt in modetypes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(parentLogger = self.logger, default=False)

            self._setupControlServicesPerMode(aMode)

            self._setupManagementServicesPerMode(aMode)

            tf = aMode.createTaskFUN(self.fun, "UT")

            self.phyUsers[aMode.modeName] = aMode.phyUser

            self.fun.add(self.phyUsers[aMode.modeName])

            self.managementServices.append(tf.timer)

            connectFUs([
                    (tf.bottom, aMode.phyUser),
                    (macg, tf.top),
                    ])

        connectFUs([
                (upperConvergence, rlc),
                (rlc, upperSynchronizer),
                (upperSynchronizer, upperFlowGate),
                (upperFlowGate, arqFlowGate),
                (arqFlowGate, macg),
                ])

    def _setupControlServices(self):
        self.associationsProxy = lte.dll.controlplane.association.UEAssociationsProxy(self.logger)
        self.controlServices.append(self.associationsProxy)

        self.flowManager = lte.dll.controlplane.flowmanager.FlowManagerUT(self.logger)
        self.controlServices.append(self.flowManager)


    def _setupControlServicesPerMode(self, mode):
        # Each mode provides association information service
        self.controlServices.append(dll.Services.Association(mode.modeName, mode.logger))

        p = lte.partitioning.service.PartitioningInfo(mode.defaultPartitioning,
                                                      mode.modeName,
                                                      mode.logger)
        mode.partInfo = p
        self.controlServices.append(p)

    def _setupManagementServicesPerMode(self, mode):
        i = dll.Services.InterferenceCache("INTERFERENCECACHE"+mode.modeName,
                                           alphaLocal = 0.2,
                                           alphaRemote= 0.05,
                                           parent = mode.logger)
        # If we have no value, we assume that we are located at the cell edge
        # and have an SINR of 1dB with interference set to background noise
        pathloss = float(mode.plm.phy.txPwrUT.nominalPerSubband.replace(" dBm","")) + 99.00
        i.notFoundStrategy.averageCarrier = "-99.0 dBm"
        i.notFoundStrategy.averageInterference = "-100.0 dBm"
        i.notFoundStrategy.deviationCarrier = "0.0 mW"
        i.notFoundStrategy.deviationInterference = "0.0 mW"
        i.notFoundStrategy.averagePathloss = "%f dB" % pathloss

        mode.interfCache = i
        self.managementServices.append(i)

    def setPhyDataTransmission(self, modeName, serviceName):
        """set the name of the PHY component for a certain mode"""
        self.phyDataTransmission[modeName] = serviceName

    def setPhyNotification(self, modeName, serviceName):
        self.phyNotification[modeName] = serviceName
