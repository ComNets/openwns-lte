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
import lte.partitioning.fdd
import lte.phy.plm

import openwns.logger

class ModeCapabilities:
	duplexGroup = None

	def __init__(self, duplexGroup):
		self.duplexGroup = duplexGroup

class Mode(lte.modes.hasModeName.HasModeName):
	taskFUNModule = None
	# for mode selection
	priority = None
	# Thresholds
	thresholdCriterion = "SINR"
	upperThreshold = None
	lowerThreshold = None

	taskFUNs = None # taskFUNs[taskID]

	# top and bottom Node of Mode-specific-FUN (connected externally)
	top = None
	bottom = None

	# this modes' phy and mac parameters
	plm = None

	# channel properties
	channelModel = None

	# the logger, to be distributed to the mode's FUs
	logger = None

	# FUs commonly used by this Modes TaskFUNs
	bottomTaskDispatcher = None
	phyUser              = None

	# Mode-wide services
	partInfo = None

	interfCache = None

	powerControl = None

	# default Partitioning Name
	defaultPartitioning = None

	capabilities = None

	def __init__(self, name, PLM, parentLogger=None):
		self.taskFUNModule = None #lte.TaskFUNs
		self.fullConnects = []
		self.nodes = []
		self.modeName = name
                self.separator = "_"
                self.modeBase = name
		self.taskID = ""
		self.plm = PLM
		self.taskFUNs = {}
		self.logger = openwns.logger.Logger('LTE', name, True, parentLogger)
		self.rsNameSuffix = 'resourceScheduler'
		self.scorerSuffix  = self.rsNameSuffix+'TX'
		#self.phyUser = lte.dll.phyUser.PhyUser(self.modeName, PLM, self.modeName + self.separator + 'phyUser', self.modeBase + self.separator + self.rsNameSuffix +'TX', self.logger)
                self.phyUser = lte.dll.phyUser.PhyUser(self.modeName, PLM, self.modeName + self.separator + 'phyUser', self.logger)
		#self.taskDispatcher = lte.dll.TaskDispatcher(self.modeName, PLM,
                #                                             self.modeName + self.separator + 'TaskDispatcher', self.logger)
		self.mapModeNameToNumber(name)

	# Add TaskFUN Information
	def addTaskFUN(self,taskFUN):
		self.taskFUNs[taskFUN.taskID] = taskFUN

ltefdd10 = lte.phy.plm.getByName("ltefdd10") 
class LTEFDD10(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd10", ltefdd10, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd10", ltefdd10, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = 50, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = "Pathloss"
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.capabilities = ModeCapabilities(3)
