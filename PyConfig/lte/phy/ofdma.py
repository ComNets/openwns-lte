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

import lte.phy.plm

import ofdmaphy.Station as ofdmaSTA
import ofdmaphy.Receiver as ofdmaRCV
import ofdmaphy.Transmitter as ofdmaTRM

import rise.scenario.Propagation

import openwns.logger

class BSReceiver(ofdmaRCV.OFDMAReceiver):
    def __init__(self, prop, propch, parentLogger = None):
        ofdmaRCV.OFDMAReceiver.__init__(self,
                                        propagation = prop,
                                        propagationCharacteristicName = propch,
                                        receiverNoiseFigure = "5 dB",
                                        parentLogger = parentLogger)
        self.doMeasurementUpdates = False

class BSTransmitter(ofdmaTRM.OFDMATransmitter):
    def __init__(self, prop, propch, parentLogger = None):
        ofdmaTRM.OFDMATransmitter.__init__(self,
                                           propagation = prop,
                                           propagationCharacteristicName = propch,
                                           parentLogger = parentLogger)

class UEReceiver(ofdmaRCV.OFDMAReceiver):
    def __init__(self, prop, propch, parentLogger = None):
        ofdmaRCV.OFDMAReceiver.__init__(self,
                                        propagation = prop,
                                        propagationCharacteristicName = propch,
                                        receiverNoiseFigure = "7 dB",
                                        parentLogger = parentLogger)
        self.doMeasurementUpdates = False

class UETransmitter(ofdmaTRM.OFDMATransmitter):
    def __init__(self, prop, propch, parentLogger = None):
        ofdmaTRM.OFDMATransmitter.__init__(self,
                                           propagation = prop,
                                           propagationCharacteristicName = propch,
                                           parentLogger = parentLogger)

class BSOFDMAComponent(ofdmaSTA.OFDMAComponent):

    def __init__(self, node, mode):
        bsReceiver = BSReceiver(prop = rise.scenario.Propagation.PropagationSingleton.getInstance(),
                                propch = mode.plm.phy.eNBTransceiverType,
                                parentLogger = node.logger)
        
        bsTransmitter = BSTransmitter(prop = rise.scenario.Propagation.PropagationSingleton.getInstance(),
                                      propch = mode.plm.phy.eNBTransceiverType,
                                      parentLogger = node.logger,)

        phyStation = ofdmaSTA.OFDMAStation([bsReceiver], [bsTransmitter], node.logger, eirpLimited = True)
        phyStation.beamformingAntenna = None

        phyStation.txFrequency = mode.plm.phy.dlCenterFreq * 1E-6
        phyStation.rxFrequency = mode.plm.phy.ulCenterFreq * 1E-6
        phyStation.txPower = mode.plm.phy.txPwrBS.nominalPerSubband
        # overall Power
        phyStation.totalPower = mode.plm.phy.txPwrBS.maxOverall
        phyStation.numberOfSubCarrier = mode.plm.numSubchannels
        phyStation.bandwidth = mode.plm.bandwidth
        #phyStation.antennas = [rise.Antenna.Isotropic([0,0,1.5])]
        ofdmaSTA.OFDMAComponent.__init__(self, node, mode.modeName, phyStation, node.logger)

class UEOFDMAComponent(ofdmaSTA.OFDMAComponent):

    def __init__(self, node, mode):
        ueReceiver = UEReceiver(prop = rise.scenario.Propagation.PropagationSingleton.getInstance(),
                                propch = mode.plm.phy.utTransceiverType,
                                parentLogger = node.logger)
        
        ueTransmitter = UETransmitter(prop = rise.scenario.Propagation.PropagationSingleton.getInstance(),
                                      propch = mode.plm.phy.utTransceiverType,
                                      parentLogger = node.logger)

        phyStation = ofdmaSTA.OFDMAStation([ueReceiver], [ueTransmitter], node.logger, eirpLimited = True)
        phyStation.beamformingAntenna = None

        phyStation.txFrequency = mode.plm.phy.ulCenterFreq * 1E-6
        phyStation.rxFrequency = mode.plm.phy.dlCenterFreq * 1E-6
        phyStation.txPower = mode.plm.phy.txPwrUT.nominalPerSubband
        # overall Power
        phyStation.totalPower = mode.plm.phy.txPwrUT.maxOverall
        phyStation.numberOfSubCarrier = mode.plm.numSubchannels
        phyStation.bandwidth = mode.plm.bandwidth
        #phyStation.antennas = [rise.Antenna.Isotropic([0,0,1.5])]
        ofdmaSTA.OFDMAComponent.__init__(self, node, mode.modeName, phyStation, node.logger)
