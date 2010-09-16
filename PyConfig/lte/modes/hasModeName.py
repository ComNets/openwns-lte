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

# global mapping of mode name to unique integer (used as context in probes "MAC.mode")
mappingOfModeNamesToNumbers = {}

class HasModeName:
	# modename attribute
	__modeName = None
	# suffix describing the task of a specific taskFUN
	__taskID = ""
	localIDs = None # list
	modeNumber = None

	def __modeNameGetter(self):
		return self.__modeName

	def __modeNameSetter(self, value):
		self.__modeName = value
		self.modeBase = value.rstrip(self.taskID).rstrip(self.separator)

	def __taskIDGetter(self):
		return self.__taskID

	def __taskIDSetter(self, value):
		self.__taskID = value
		self.modeBase = self.__modeName.rstrip(value).rstrip(self.separator)

	# ModeName
	modeName = property(__modeNameGetter, __modeNameSetter)
	# modename with taskID stripped
	modeBase = None
	# Modename Separator
	separator = "_"
	# suffix by which the resource scheduler can be found in the FUN
	rsNameSuffix = "resourceScheduler"
	# suffix by which the scorer can be found in the FUN
	scorerSuffix = rsNameSuffix
	# suffix describing the task of a specific taskFUN
	taskID = property(__taskIDGetter, __taskIDSetter)

	def mapModeNameToNumber(self, name):
		if (name in mappingOfModeNamesToNumbers):
			self.modeNumber = mappingOfModeNamesToNumbers[name]
		else: # we need a new number for this mode
			newNumber = len(mappingOfModeNamesToNumbers)
			mappingOfModeNamesToNumbers[name] = newNumber
			self.modeNumber = newNumber

	def setMode(self, mode):
		self.localIDs = {}
		self.modeName = mode.modeName
		self.modeBase = mode.modeBase
		self.separator = mode.separator
		self.rsNameSuffix = mode.rsNameSuffix
		self.scorerSuffix = mode.scorerSuffix
		self.taskID = mode.taskID # 'UT'/'BS' for RNs, '' for BSs and UTs
		# modeNumber is only used for Probe context/sorting
		self.mapModeNameToNumber(self.modeBase)
		if (self.taskID == ""):
			self.localIDs = {}
		elif (self.taskID == "BS"):
			self.localIDs = { 'MAC.task' : 0 }
		elif (self.taskID == "UT"):
			self.localIDs = { 'MAC.task' : 1 }
		else:
			assert False
		self.localIDs['MAC.mode'] = self.modeNumber
