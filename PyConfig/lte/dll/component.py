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
import lte.dll.phyUser
from lte.support.helper import connectFUs

import dll.Layer2
import openwns.FUN

class eNBLayer2( dll.Layer2.Layer2 ):

    def __init__(self, node, name, plm, parentLogger=None):
        dll.Layer2.Layer2.__init__(self, node, name, parentLogger)
        self.nameInComponentFactory = "lte.Layer2"
        self.setStationID(node.nodeID)
        self.setStationType(node.getProperty("Type"))
        self.ring = node.getProperty("Ring")
        self.associations = []

        self.fun = openwns.FUN.FUN(self.logger)

        upperConvergence = lte.dll.upperConvergence.eNB(self.logger)
        self.fun.add(upperConvergence)

        rlc = lte.dll.rlc.eNBRLC(self.logger)
        self.fun.add(rlc)

        phy = lte.dll.phyUser.PhyUser(plm = plm, parentLogger = self.logger)
        self.fun.add(phy)

        connectFUs([
                (upperConvergence, rlc),
                (rlc, phy)
                ])

class ueLayer2( dll.Layer2.Layer2 ):

    def __init__(self, node, name, plm, parentLogger=None):
        dll.Layer2.Layer2.__init__(self, node, name, parentLogger)
        self.nameInComponentFactory = "lte.Layer2"
        self.setStationID(node.nodeID)
        self.setStationType(node.getProperty("Type"))
        self.ring = node.getProperty("Ring")
        self.associations = []

        self.fun = openwns.FUN.FUN(self.logger)

        upperConvergence = lte.dll.upperConvergence.UE(self.logger)
        self.fun.add(upperConvergence)

        rlc = lte.dll.rlc.UERLC(self.logger)
        self.fun.add(rlc)

        phy = lte.dll.phyUser.PhyUser(plm = plm, parentLogger = self.logger)
        self.fun.add(phy)

        connectFUs([
                (upperConvergence, rlc),
                (rlc, phy)
                ])
