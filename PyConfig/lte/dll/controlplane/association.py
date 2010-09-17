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
import openwns.FUN

class ENBAssociationsProxy(dll.Services.Service):

    def __init__(self, parentLogger = None):
        self.nameInServiceFactory = 'lte.controlplane.ENBAssociationsProxy'
        self.serviceName = 'AssociationsProxy'
        self.modeNames = []
        self.logger = openwns.logger.Logger("LTE", "AssociationsProxy", True, parentLogger)

class UEAssociationsProxy(dll.Services.Service):

    def __init__(self, parentLogger = None):
        self.nameInServiceFactory = 'lte.controlplane.UEAssociationsProxy'
        self.serviceName = 'AssociationsProxy'
        self.modeNames = []
        self.modePriority = {}
        self.logger = openwns.logger.Logger("LTE", "AssociationsProxy", True, parentLogger)

class AssociationHandler(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):

    def __init__(self, modeName, functionalUnitName, commandName, parentLogger):
        super(AssociationHandler, self).__init__(functionalUnitName, commandName)
        self.flowSeparators = []
        self.modeName = modeName
        self.commandSize = 12
        self.logger = openwns.logger.Logger('LTE','AssociationHandler',True, parentLogger)

    def addFlowSeparator(self, flowSeparatorName):
        self.flowSeparators.append(flowSeparatorName)

class AssociationHandlerUT(AssociationHandler, openwns.StaticFactoryClass):
    
    def __init__(self, modeName, mode, functionalUnitName, commandName, parentLogger):
        AssociationHandler.__init__(self, modeName, functionalUnitName, commandName, parentLogger)
        openwns.StaticFactoryClass.__init__(self, 'lte.controlplane.AssociationHandler.UserTerminal')

        self.timeout = 1.0

        self.capabilities = mode.capabilities

class AssociationHandlerBS(AssociationHandler, openwns.StaticFactoryClass):

    def __init__(self, modeName, mode, functionalUnitName, commandName, parentLogger):
        AssociationHandler.__init__(self, modeName, functionalUnitName, commandName, parentLogger)
        openwns.StaticFactoryClass.__init__(self, 'lte.controlplane.AssociationHandler.BaseStation')
        self.capabilities = mode.capabilities
