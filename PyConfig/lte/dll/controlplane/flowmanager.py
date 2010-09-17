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

import lte.modes.hasModeName

import dll.Services

import openwns.logger
import openwns.distribution
import openwns.FUN

class FlowID(openwns.StaticFactoryClass):
    
    def __init__(self):
        openwns.StaticFactoryClass.__init__(self, "lte.FlowID")

class FlowManagerBS(dll.Services.Service):
    def __init__(self, parentLogger = None):
        self.flowSeparatorNames = []
        self.modeNames = []
        self.serviceName = "FlowManagerBS"
        self.nameInServiceFactory = 'lte.controlplane.flowmanagement.FlowManagerBS'
        self.logger = openwns.logger.Logger("LTE", self.serviceName, True, parentLogger)
        self.transactionIdDistribution = openwns.distribution.Uniform(10000.0, 0.0)
        self.flowIDUnbindDelay = 60.0 
    
class FlowManagerUT(dll.Services.Service):
    def __init__(self, parentLogger = None):
        self.flowSeparatorNames = []
        self.modeNames = []
        self.serviceName = "FlowManagerUT"
        self.nameInServiceFactory = 'lte.controlplane.flowmanagement.FlowManagerUT'
        self.logger = openwns.logger.Logger("LTE", self.serviceName, True, parentLogger)
        self.transactionIdDistribution = openwns.distribution.Uniform(10000.0, 0.0)
        self.flowIDUnbindDelay = 60.0 

class FlowHandler(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):
    flowSeparators = None
    logger = None
    commandSize = None

    def __init__(self, modeName, functionalUnitName, commandName, parentLogger):
        super(FlowHandler, self).__init__(functionalUnitName, commandName)
        self.flowSeparators = []
        self.modeName = modeName
        self.commandSize = 10 # Bit (arbitrarily chosen like any other command size in the wns)
        self.logger = openwns.logger.Logger('LTE','FlowHandler',True, parentLogger)

    def addFlowSeparator(self, flowSeparatorName):
        self.flowSeparators.append(flowSeparatorName)

class FlowHandlerUT(FlowHandler):
    __plugin__ = 'lte.controlplane.flowmanagement.flowhandler.FlowHandlerUT'
    timeout = 0
    commandname = None

    def __init__(self, modeName, functionalUnitName, commandName, parentLogger):
        super(FlowHandlerUT,self).__init__(modeName, functionalUnitName, commandName, parentLogger)
        self.timeout = 1 #0.012 # sec
        self.commandname = commandName

class FlowHandlerBS(FlowHandler):
    __plugin__ = 'lte.controlplane.flowmanagement.flowhandler.FlowHandlerBS'

    def __init__(self, modeName, functionalUnitName, commandName, parentLogger):
        super(FlowHandlerBS,self).__init__(modeName, functionalUnitName, commandName, parentLogger)

