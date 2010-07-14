import lte.phy.ofdma

import rise.Mobility

import scenarios.interfaces

import openwns.node

class UE(scenarios.interfaces.INode, openwns.node.Node):
    
    def __init__(self, config, mobility):
        openwns.node.Node.__init__(self, "UE")

        self.name += str(self.nodeID)

        self.properties = {}
        self.properties["Type"] = "UE"

        self.phy = lte.phy.ofdma.UEOFDMAComponent(node = self, config = config)

        self.mobility = rise.Mobility.Component(node = self, 
                                                name = "UEMobility",
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
