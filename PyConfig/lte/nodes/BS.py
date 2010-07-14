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

import scenarios.interfaces

import rise.Mobility

import openwns.node

class BS(scenarios.interfaces.INode, openwns.node.Node):
    
    def __init__(self, config, mobility):
        openwns.node.Node.__init__(self, "BS")

        self.properties = {}
        self.properties["Type"] = "BS"

        self.name += str(self.nodeID)

        self.phy = lte.phy.ofdma.BSOFDMAComponent(node = self, config = config)

        self.mobility = rise.Mobility.Component(node = self, 
                                                name = "BSMobility",
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
