###############################################################################
# This file is part of openWNS (open Wireless Network Simulator)
# _____________________________________________________________________________
#
# Copyright (C) 2004-2007
# Chair of Communication Networks (ComNets)
# Kopernikusstr. 5, D-52074 Aachen, Germany
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

from openwns.pyconfig import attrsetter
import openwns.qos

#begin example "lte.qos.example"
undefinedQosClass      = openwns.qos.QoSClass("UNDEFINED",0,priority=6)
pbchQosClass           = openwns.qos.QoSClass("PBCH",1,priority=0)
phichQosClass          = openwns.qos.QoSClass("PHICH",2,priority=1) # used by HARQ
pcchQosClass           = openwns.qos.QoSClass("PCCH",3,priority=2)
dcchQosClass           = openwns.qos.QoSClass("DCCH",4,priority=3)
conversationalQosClass = openwns.qos.QoSClass("CONVERSATIONAL",5,priority=4)
streamingQosClass      = openwns.qos.QoSClass("STREAMING",6,priority=5)
interactiveQosClass    = openwns.qos.QoSClass("INTERACTIVE",7,priority=6)
backgroundQosClass     = openwns.qos.QoSClass("BACKGROUND",8,priority=7)
#end example

class QoSClasses(openwns.qos.QoSClasses):

    def __init__(self, **kw):
        openwns.qos.QoSClasses.__init__(self)
        self.mapEntries = []
        self.mapEntries.append(undefinedQosClass)
        self.mapEntries.append(pbchQosClass)
        self.mapEntries.append(phichQosClass)
        self.mapEntries.append(pcchQosClass)
        self.mapEntries.append(dcchQosClass)
        self.mapEntries.append(conversationalQosClass)
        self.mapEntries.append(streamingQosClass)
        self.mapEntries.append(interactiveQosClass)
        self.mapEntries.append(backgroundQosClass)
        attrsetter(self, kw)
