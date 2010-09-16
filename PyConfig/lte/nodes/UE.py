import lte.phy.ofdma

import rise.Mobility

import scenarios.interfaces

import openwns.node

class UE(scenarios.interfaces.INode, openwns.node.Node):
    
    def __init__(self, config, mobility):
        openwns.node.Node.__init__(self, "UE")

        self.logger = openwns.logger.Logger("LTE", "UE%d" % self.nodeID, True)

        self.name += str(self.nodeID)

        self.properties = {}
        self.properties["Type"] = "UE"
        self.properties["Ring"] = 1
        self.phys = {}

        self.dll = lte.dll.component.ueLayer2(node = self, name = "UE", modetypes = config.modes, parentLogger = self.logger)

        self.mobility = rise.Mobility.Component(node = self, 
                                                name = "UEMobility",
                                                mobility = mobility)


        # Each mode has its own OFDMA subsystem
        for mt in config.modes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(parentLogger = self.logger, default=False)

            self.phys[aMode.modeName] = lte.phy.ofdma.UEOFDMAComponent(node = self, mode = aMode)

            self.dll.setPhyDataTransmission(aMode.modeName, self.phys[aMode.modeName].dataTransmission)
            self.dll.setPhyNotification(aMode.modeName, self.phys[aMode.modeName].notification)

    def setPosition(self, position):
        self.mobility.mobility.setCoords(position)      

    def getPosition(self):
        return self.mobility.mobility.getCoords()

    def setAntenna(self, antenna):
        self.antenna = antenna

    def setChannelModel(self, channelModel):
        for phy in self.phys.values():
            phy.ofdmaStation.receiver[0].propagation.configure(channelModel)
            phy.ofdmaStation.transmitter[0].propagation.configure(channelModel)
    
    def getProperty(self, propertyName):
        return self.properties[propertyName]

    def setProperty(self, propertyName, propertyValue):
        self.properties[propertyName] = propertyValue
