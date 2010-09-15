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

        self.name += str(self.nodeID)

        plm = lte.phy.plm.getByName(config.plmName)

        self.phy = lte.phy.ofdma.BSOFDMAComponent(node = self, plm = plm)

        self.dll = lte.dll.component.eNBLayer2(node = self, name = "LTE", plm = plm, parentLogger = self.logger)
        self.dll.setPhyDataTransmission(self.phy.dataTransmission)
        self.dll.setPhyNotification(self.phy.notification)

        self.mobility = rise.Mobility.Component(node = self, 
                                                name = "eNBMobility",
                                                mobility = mobility)

    def setPosition(self, position):
        self.mobility.mobility.setCoords(position)      

    def getPosition(self):
        return self.mobility.mobility.getCoords()

    def setAntenna(self, antenna):
        self.antenna = antenna

    def setChannelModel(self, channelModel):
        self.channelModel = channelModel

    def getProperty(self, propertyName):
        return self.properties[propertyName]


    def setProperty(self, propertyName, propertyValue):
        self.properties[propertyName] = propertyValue
