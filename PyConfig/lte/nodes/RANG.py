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

import openwns.node
import scenarios.interfaces
import dll.Layer2

import constanze.node
import tcp.TCP
import ip.Component
import ip.AddressResolver

class Tunnel( dll.Layer2.Layer2 ):
    dataTransmission = None
    notification = None
    flowIDUnbindDelay = 60.0 # was 60.0
    dllDataTransmissions = None
    dllNotifications = None

    def __init__(self, node, parentLogger = None):
        dll.Layer2.Layer2.__init__(self, node, "RANG", parentLogger = parentLogger)
        self.dataTransmission = "RANG.dllDataTransmission"
        self.notification = "RANG.dllNotification"
        self.nameInComponentFactory = "lte.RANG"
        self.dllDataTransmissions = []
        self.dllNotifications = []
        self.logger.enabled=True

    def addBS(self, bs):
        self.dllDataTransmissions.append(openwns.node.FQSN(bs, bs.dll.dataTransmission))
        self.dllNotifications.append(openwns.node.FQSN(bs, bs.dll.notification))

class RANG(openwns.node.Node, scenarios.interfaces.INode):
    tunnel = None
    nl  = None
    load = None
    tl = None

    def __init__(self, useTCP=True):
        super(RANG, self).__init__("RANG")

        self.setProperty("Type", "RANG")

        # create Tunnel to reach the BSs
        self.tunnel = Tunnel(self, self.logger)
        address = 256*255 - 1
        self.tunnel.setStationID(address)

        # create Network Layer
        domainName = "RANG.lte.wns.org"
        
        self.nl = ip.Component.IPv4Component(self, domainName + ".ip",domainName, useDllFlowIDRule = True)
        self.nl.addDLL(_name = "tun",
                       # Where to get my IP Address
                       _addressResolver = ip.AddressResolver.FixedAddressResolver("192.168.254.254", "255.255.0.0"),
                       # ARP zone
                       _arpZone = "LTERAN",
                       # We can deliver locally
                       _pointToPoint = False,
                       # DLL service names
                       _dllDataTransmission = self.tunnel.dataTransmission,
                       _dllNotification = self.tunnel.notification)

        if useTCP == True:
            self.tl = tcp.TCP.TCPComponent(self, "tcp", self.nl.dataTransmission, self.nl.notification)
        else:
            self.tl = tcp.TCP.UDPComponent(self, "udp", self.nl.dataTransmission, self.nl.notification)

        # create Loadgen
        self.load = constanze.node.ConstanzeComponent(self, domainName + ".constanze", self.logger)

    def setPosition(self, position):
        pass

    def getPosition(self):
        return None

    def setAntenna(self, antenna):
        pass

    def setChannelModel(self, channelModel):
        pass

class AppRANG(RANG):

    def __init__(self, useTCP = True):
        super(AppRANG, self).__init__(useTCP)
        self.components.remove(self.load)

        domainName = "RANG.lte.wns.org"

        import applications
        import applications.component
        self.load = applications.component.Server(self, domainName + ".applications", self.logger)

