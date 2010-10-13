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

import lte.phy.ofdma
import lte.dll.component
import lte.modes

import scenarios.interfaces

import rise.Mobility

import openwns.node
import openwns.logger

class BS(scenarios.interfaces.INode, openwns.node.Node):
    
    def __init__(self, config, mobility):
        openwns.node.Node.__init__(self, "eNB")

        self.logger = openwns.logger.Logger("LTE", "eNB%d" % self.nodeID, True)

        self.properties = {}
        self.properties["Type"] = "eNB"
        self.properties["Ring"] = 1
        self.phys = {}

        self.name += str(self.nodeID)

        self.dll = lte.dll.component.eNBLayer2(node = self, name = "eNB", modetypes = config.modes, parentLogger = self.logger)

        self.mobility = rise.Mobility.Component(node = self, 
                                                name = "eNBMobility",
                                                mobility = mobility)

        # Each mode has its own OFDMA subsystem
        for mt in config.modes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(parentLogger = self.logger, default=False)

            self.phys[aMode.modeName] = lte.phy.ofdma.BSOFDMAComponent(node = self, mode = aMode)

            self.dll.setPhyDataTransmission(aMode.modeName, self.phys[aMode.modeName].dataTransmission)
            self.dll.setPhyNotification(aMode.modeName, self.phys[aMode.modeName].notification)

    def setPosition(self, position):
        self.mobility.mobility.setCoords(position)      

    def getPosition(self):
        return self.mobility.mobility.getCoords()

    def setAntenna(self, antenna):
        assert len(self.phys.keys()) == 1, "Only one antenna per BS!"

        for key in self.phys.keys():
            self.phys[key].ofdmaStation.antennas = [antenna]

    def setChannelModel(self, channelModel):
        for phy in self.phys.values():
            phy.ofdmaStation.receiver[0].propagation.configure(channelModel)
            phy.ofdmaStation.transmitter[0].propagation.configure(channelModel)

    def getProperty(self, propertyName):
        return self.properties[propertyName]

    def setProperty(self, propertyName, propertyValue):
        self.properties[propertyName] = propertyValue
