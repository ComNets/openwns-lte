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

import constanze.node

import tcp.TCP

import ip.Component
import ip.Address
import ip.AddressResolver

import lte.phy.ofdma

import rise.Mobility

import scenarios.interfaces

import openwns.node

class UE(openwns.node.Node, scenarios.interfaces.INode):
    
    def __init__(self, config, mobility):
        openwns.node.Node.__init__(self, "UE")

        self.logger = openwns.logger.Logger("LTE", "UE%d" % self.nodeID, True)

        self.name += str(self.nodeID)

        self.setProperty("Type", "UE")
        self.setProperty("Ring", 1)
        self.phys = {}

        self.dll = lte.dll.component.ueLayer2(node = self, name = "UE", modetypes = config.modes, parentLogger = self.logger)

        self._setupIP()
        self._setupTCP(config.useTCP)
        self._setupLoad()

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

    def _setupIP(self):
        assert self.nodeID < 65000, "Only 65000 Terminals supported currently :-) !"
        domainName = "UT" + str(self.nodeID) + ".lte.wns.org"
        ipAddress = ip.Address.ipv4Address("192.168.0.1") + self.nodeID
        self.nl = ip.Component.IPv4Component(self, domainName + ".ip", domainName, useDllFlowIDRule = False)
        self.nl.addDLL(_name = "lte",
                       # Where to get IP Adresses
                       _addressResolver = ip.AddressResolver.VirtualDHCPResolver("LTERAN"),
                       # Name of ARP zone
                       _arpZone = "LTERAN",
                       # We cannot deliver locally to other UTs without going to the gateway
                       _pointToPoint = True,
                       _dllDataTransmission = self.dll.dataTransmission,
                       _dllNotification = self.dll.notification)

        self.nl.addRoute("0.0.0.0", "0.0.0.0", "192.168.254.254", "lte")

    def _setupTCP(self, useTCP):
        if useTCP == True:
            self.tl = tcp.TCP.TCPComponent(self, "tcp", self.nl.dataTransmission, self.nl.notification)
        else:
            self.tl = tcp.TCP.UDPComponent(self, "udp", self.nl.dataTransmission, self.nl.notification)

        self.tl.addFlowHandling(
            _dllNotification = self.dll.notification,
            _flowEstablishmentAndRelease = self.dll.flowEstablishmentAndRelease)

    def _setupLoad(self):
        domainName = "UT" + str(self.nodeID) + ".lte.wns.org"
        self.load = constanze.node.ConstanzeComponent(self, domainName + ".constanze", self.logger)

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
    
    def addTraffic(self, binding, load):
        self.load.addTraffic(binding, load)

class AppUE(UE):

    def __init__(self, config, mobility):
        UE.__init__(self, config, mobility)
        self.components.remove(self.load)

        domainName = "UT" + str(self.nodeID) + ".lte.wns.org"
        
        import applications
        import applications.component
        self.load = applications.component.Client(self, domainName + ".applications", self.logger)

