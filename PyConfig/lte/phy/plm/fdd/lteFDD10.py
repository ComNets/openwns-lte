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
import default

class LTEFDD1p4(default.LTE):

    def __init__(self):
        
        self.numSubchannels = 6

        default.LTE.__init__(self)

class LTEFDD3(default.LTE):

    def __init__(self):
        
        self.numSubchannels = 15

        default.LTE.__init__(self)

class LTEFDD5(default.LTE):

    def __init__(self):
        
        self.numSubchannels = 25

        default.LTE.__init__(self)

class LTEFDD10(default.LTE):

    def __init__(self):
        
        self.numSubchannels = 50

        default.LTE.__init__(self)

class LTEFDD15(default.LTE):

    def __init__(self):
        
        self.numSubchannels = 75

        default.LTE.__init__(self)

class LTEFDD20(default.LTE):

    def __init__(self):
        
        self.numSubchannels = 100

        default.LTE.__init__(self)

lte.phy.plm.PLMBroker.PLMs["ltefdd20"] = LTEFDD20()
lte.phy.plm.PLMBroker.PLMs["ltefdd15"] = LTEFDD15()
lte.phy.plm.PLMBroker.PLMs["ltefdd10"] = LTEFDD10()
lte.phy.plm.PLMBroker.PLMs["ltefdd5"] = LTEFDD5()
lte.phy.plm.PLMBroker.PLMs["ltefdd3"] = LTEFDD3()
lte.phy.plm.PLMBroker.PLMs["ltefdd1p4"] = LTEFDD1p4()
