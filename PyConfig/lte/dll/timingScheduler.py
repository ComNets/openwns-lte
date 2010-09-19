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

from openwns.logger import Logger
from lte.modes.hasModeName import HasModeName
import openwns.FUN
import dll.Services

class StationTaskPhase(HasModeName):
    """ base class for different phases of a frame """
    duration = None # [s]
    startTime = None # always relative to start of superframe
    task = None # possible are 'RAP' or 'UT' or 'IDLE'
    def __init__(self, _task, _duration, **kw):
        self.task = _task
        self.duration = _duration

# The following event types are handled by TimingScheduler
# and correspond to certain events defined in Eventx.cpp
# The frame structures are defined in Stations.py
class TimingEvent(HasModeName):
    """ base class for different phases of a frame """
    duration = None
    time = None # always relative to start of superframe
    stationType = ""
    logger = None

    def __init__(self, mode):
        self.logger = Logger('LTE','TimingEvent',True, mode.logger)
        self.setMode(mode)

class RACH(TimingEvent):
    __plugin__ = "lte.timing.RACH"

    def __init__(self, mode):
        TimingEvent.__init__(self, mode)
        self.time = 0.0
        self.duration = mode.plm.mac.RACHLength

class BCH(TimingEvent):
    __plugin__ = "lte.timing.BCH"
    def __init__(self, mode):
        TimingEvent.__init__(self, mode)
        self.time = mode.plm.mac.RACHLength + mode.plm.mac.preambleGuard
        self.duration = mode.plm.mac.preambleLength

class Map(TimingEvent):
    __plugin__ = "lte.timing.Map"
    frameNr = None
    framesPerSuperFrame = None
    useMapResourcesInUL = None

    def __init__(self, mode, frameNr):
        TimingEvent.__init__(self,mode)
        self.frameNr = frameNr
        self.framesPerSuperFrame = mode.plm.mac.framesPerSuperFrame
        self.time = mode.plm.mac.RACHLength + mode.plm.mac.preambleGuard + mode.plm.mac.preambleLength + frameNr*mode.plm.mac.frameLength
        self.duration = mode.plm.mac.mapLength
        # minor shift to avoid timing problems in FDD mode
        self.duration -= mode.plm.mac.safetyFraction
        self.time += mode.plm.mac.safetyFraction
        self.useMapResourcesInUL = mode.plm.mac.useMapResourcesInUL

class Data(TimingEvent):
    __plugin__ = "lte.timing.Data"
    frameNr = None
    framesPerSuperFrame = None
    useMapResourcesInUL = None

    def __init__(self, mode, frameNr):
        TimingEvent.__init__(self, mode)
        self.frameNr = frameNr
        self.framesPerSuperFrame = mode.plm.mac.framesPerSuperFrame
        ### startTime
        self.time = mode.plm.mac.RACHLength + mode.plm.mac.preambleGuard + mode.plm.mac.preambleLength + frameNr*mode.plm.mac.frameLength + mode.plm.mac.mapLength
        ### total duration:
        self.duration = (mode.plm.mac.symbolsPerFrame - mode.plm.mac.symbolsPerMap) * mode.plm.mac.fullSymbolDur + mode.plm.mac.duplexGuardInterval
        self.useMapResourcesInUL = mode.plm.mac.useMapResourcesInUL

class Timer(openwns.FUN.FunctionalUnit, HasModeName):
    __plugin__ = 'lte.timing.Timer'
    name = None
    timingScheduler = None

    def __init__(self, sched, commandName):
        super(Timer,self).__init__(commandName)
	self.timingScheduler = sched

class TimingScheduler(dll.Services.Service, HasModeName):
    name = 'TimingScheduler'
    logger = None
    mac = None
    """phases = list of TimingEvents, which can be the above:
BCH, RACH, Map, Data
phases are appended in Stations.py"""
    phases = None
    peers = None
    stationTaskPhases = None
    duplex = None
    frameLength = None
    startOfFirstFrame = None # ...within the superframe

    def __init__(self, mode):
        self.setMode(mode)
        self.serviceName = mode.modeName + mode.separator + 'Timer'
        self.nameInServiceFactory = 'lte.timing.TimingScheduler'
        self.phases = []
        self.peers = [] # only needed as long as we make static associations
        self.stationTaskPhases = []
        self.mac = mode.plm.mac
        self.logger = Logger('LTE','TimingSched',True, mode.logger)
        self.frameLength = self.mac.frameLength
        self.startOfFirstFrame = self.mac.preambleGuard + self.mac.RACHLength + self.mac.preambleLength + 1E-6 #+1E-6 to be in the first frame...

    def addPeer(self, DLLAddress):
        self.peers.append(DLLAddress)

    def addStationTaskPhase(self, _stationTaskPhase):
        if (self.stationTaskPhases.__len__() == 0): # empty
            _stationTaskPhase.startTime = 0.0
        else:
            _stationTaskPhase.startTime = self.stationTaskPhases[-1].startTime + self.stationTaskPhases[-1].duration
        self.stationTaskPhases.append(_stationTaskPhase)
        #print "addStationTaskPhase: S=", _stationTaskPhase.startTime, ", D=", _stationTaskPhase.duration
