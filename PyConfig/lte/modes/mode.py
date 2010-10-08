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
import lte.modes.taskfun.default
import lte.modes.scheduling.default

import lte.partitioning.fdd
import lte.phy.plm
import lte.dll.bch

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
	thresholdCriterion = lte.dll.bch.ThresholdCriterion("SINR")
	upperThreshold = None
	lowerThreshold = None
	triggersReAssociation = True

	taskFUNs = None # taskFUNs[taskID]

	# this modes' phy and mac parameters
	plm = None

	# scheduler settings
	scheduler = None

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
		self.taskFUNModule = lte.modes.taskfun.default
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
                self.phyUser = lte.dll.phyUser.PhyUser(self.modeName, PLM, self.modeName + self.separator + 'phyUser', self.modeBase + self.separator + self.rsNameSuffix +'TX', self.logger)
		#self.taskDispatcher = lte.dll.TaskDispatcher(self.modeName, PLM,
                #                                             self.modeName + self.separator + 'TaskDispatcher', self.logger)
		self.mapModeNameToNumber(name)

		self.scheduler = lte.modes.scheduling.default.SchedulerSettings()

	def createTaskFUN(self, fun, stationType, taskFUNModule = lte.modes.taskfun.default):
		if stationType == "BS":
			taskFUN = taskFUNModule.BS(fun = fun, mode = self, parentLogger = self.logger)
		if stationType == "UT":
			taskFUN = taskFUNModule.UT(fun = fun, mode = self, parentLogger = self.logger)

		self.taskFUNs[taskFUN.taskID] = taskFUN

		return taskFUN

ltefdd20 = lte.phy.plm.getByName("ltefdd20") 
class LTEFDD20(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd20", ltefdd20, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd20", ltefdd20, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = ltefdd20.numSubchannels, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = lte.dll.bch.ThresholdCriterion("Pathloss", "1.0 dB")
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.triggersReAssociation = False
        self.capabilities = ModeCapabilities(3)

ltefdd15 = lte.phy.plm.getByName("ltefdd15") 
class LTEFDD15(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd15", ltefdd15, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd15", ltefdd15, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = ltefdd15.numSubchannels, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = lte.dll.bch.ThresholdCriterion("Pathloss", "1.0 dB")
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.triggersReAssociation = False
        self.capabilities = ModeCapabilities(3)

ltefdd10 = lte.phy.plm.getByName("ltefdd10") 
class LTEFDD10(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd10", ltefdd10, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd10", ltefdd10, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = ltefdd10.numSubchannels, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = lte.dll.bch.ThresholdCriterion("Pathloss", "1.0 dB")
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.triggersReAssociation = False
        self.capabilities = ModeCapabilities(3)

ltefdd5 = lte.phy.plm.getByName("ltefdd5") 
class LTEFDD5(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd5", ltefdd5, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd5", ltefdd5, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = ltefdd5.numSubchannels, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = lte.dll.bch.ThresholdCriterion("Pathloss", "1.0 dB")
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.triggersReAssociation = False
        self.capabilities = ModeCapabilities(3)

ltefdd3 = lte.phy.plm.getByName("ltefdd3") 
class LTEFDD3(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd3", ltefdd10, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd3", ltefdd3, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = ltefdd3.numSubchannels, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = lte.dll.bch.ThresholdCriterion("Pathloss", "1.0 dB")
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.triggersReAssociation = False
        self.capabilities = ModeCapabilities(3)

ltefdd1p4 = lte.phy.plm.getByName("ltefdd1p4") 
class LTEFDD1p4(Mode):
    def __init__(self, parentLogger = None, default = True):
        Mode.__init__(self, "ltefdd10", ltefdd1p4, parentLogger)
        self.taskFUNModule = None # lte.TaskFUNs
        self.channelModel = "lte"
        if default==True:
            self.addTaskFUN( self.taskFUNModule.NoTask( "ltefdd1p4", ltefdd1p4, parentLogger ) )
        self.defaultPartitioning = lte.partitioning.fdd.LTEFDDReuse1(noChannels = ltefdd1p4.numSubchannels, noSubFrames = 10)
        self.priority = 5
        self.thresholdCriterion = lte.dll.bch.ThresholdCriterion("Pathloss", "1.0 dB")
        self.upperThreshold = -200.0 #[dBm]
        self.lowerThreshold = -250.0 #[dBm]
        self.triggersReAssociation = False
        self.capabilities = ModeCapabilities(3)
