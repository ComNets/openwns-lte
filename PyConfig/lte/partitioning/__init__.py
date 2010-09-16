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

import openwns.logger

class Partitioning:
    numberOfSubChannels = None
    logger = None
    groups = None

    class Band:
        fMin = None
        fMax = None

        def __init__(self, fMin, fMax):
            assert fMin<=fMax, "fMin must not be bigger than fMax"
            self.fMin=fMin
            self.fMax=fMax


        def overlaps(self, other):
            if other.fMin <= self.fMin <= other.fMax:
                return True
            elif other.fMin <= self.fMax <= other.fMax:
                return True
            elif other.fMin >= self.fMin and other.fMax <= self.fMax:
                return True
            elif self.fMin >= other.fMin and self.fMax <= other.fMax:
                return True
            else:
                return False

    class Group:
        number = None
        bands = None
        """Dedication may be one of 'Feeder', 'Universal' to denote what this band should be used for."""
        dedication = None

        def __init__(self, number, dedication):
            self.number = number
            assert dedication in ['Feeder', 'Universal' ], "Unknown dedication '%s'" % dedication
            self.dedication = dedication
            self.bands = []



    def __init__(self, numberOfSubChannels, parentLogger=None):
        self.numberOfSubChannels = numberOfSubChannels
        self.groups = {}
        self.logger = openwns.logger.Logger("LTE","Partition", True, parentLogger)

    def setParentLogger(self, parentLogger=None):
        myParentLogger = copy.deepcopy(parentLogger) # original object shares logger instance
        self.logger = openwns.logger.Logger("LTE","Partitioning", True, myParentLogger)

    def addGroup(self, number, dedication = 'Universal'):
        assert not self.groups.has_key(number), "Trying to add an already existing Group"
        self.groups[number] = Partitioning.Group(number, dedication)

    def addFreqRange(self, groupNumber, fMin, fMax):
        newRange = Partitioning.Band(fMin, fMax)
        assert self.groups.has_key(groupNumber), "Trying to add freqRange to unknown group"
        group = self.groups[groupNumber]
        for b in group.bands:
            if newRange.overlaps(b):
                raise Exception("The freq range %d - %d overlaps an already existing range" % (fMin, fMax))
        group.bands.append(newRange)

class Scheme:
    partitions = None

    def __init__(self):
        self.partitions = []

    def addPartition(self, part):
        assert isinstance(part, Partitioning), "Cannot append this object to list of partitions."
        self.partitions.append(part)

    def setParentLogger(self, parentLogger=None):
        for partition in self.partitions:
            logger = openwns.Logger.Logger("LTE","PartitionScheme", True, parentLogger)
            partition.setParentLogger(logger)

