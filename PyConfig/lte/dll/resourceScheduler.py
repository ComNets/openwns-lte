# -*- coding: utf-8 -*-
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
import lte.dll.qos

import openwns.FUN
import openwns.Scheduler
import openwns.scheduler
import openwns.scheduler.metascheduler

import copy

class RegistryProxy(openwns.Scheduler.RegistryProxy, lte.modes.hasModeName.HasModeName):
    phyModeMapper = None
    logger = None
    queueSize = None # Queues read this value via colleagues.registry->getQueueSizeLimitPerConnection()
    powerCapabilitiesUT = None
    powerCapabilitiesBS = None
    qosClassMapping    = None
    numberOfPriorities = None
    numberOfQosClasses = None
    useCQI = None # True if CQI shall be used. CQI must be available.

    def __init__(self, mode, useCQI = False, parentLogger = None):
        self.setMode(mode)
        self.queueSize = 250000 # Bits
        """
        The default of 250000 Bit Queue Size per connection supports more than 330MBit/s on a single connection,
        assuming 8 frames per superframe and a superframe duration of 5.89ms
        For the RS-RX this must be more since here the queue reflects the state
        of all slave UT queues. So count numUTs times please
        """
        self.nameInRegistryProxyFactory = "lte.RegistryProxy"
        self.logger = openwns.logger.Logger("LTE", "RegProxy", True, parentLogger)
        self.powerCapabilitiesUT = mode.plm.phy.txPwrUT
        self.powerCapabilitiesBS = mode.plm.phy.txPwrBS
        self.qosClassMapping    = lte.dll.qos.QoSClasses()
        self.numberOfPriorities = self.qosClassMapping.getMaxPriority()+1 # [0..MaxPriority]
        self.numberOfQosClasses = self.qosClassMapping.getNumberOfQosClasses()

    def setPhyModeMapper(self, phyModeMapper):
        self.phyModeMapper = phyModeMapper

# (new [rs]) colleague of the scheduler to handle queue requests and delegate them to either a SimpleQueue (DL) or the RRHandler (UL master scheduler)
class QueueProxy(openwns.Scheduler.SimpleQueue):
    """ This is a proxy to simpleQueue or RRHandler. Determined in ResourceScheduler.cpp """
    nameInQueueFactory = None
    def __init__(self, parentLogger = None, sizeProbeName="QueueProxySize"):
        super(QueueProxy, self).__init__(parentLogger = parentLogger)
        self.logger = openwns.logger.Logger("WNS", "QueueProxy", True, parentLogger); # logger overwritten
        self.nameInQueueFactory = "lte.QueueProxy"
        self.sizeProbeName = sizeProbeName

class ResourceScheduler(openwns.FUN.FunctionalUnit, lte.modes.hasModeName.HasModeName):
    __plugin__ = None
    name = "ResourceScheduler"
    logger = None
    resourceUsageProbeName = None
    probeNameQueue = None
    probeNameQueueOverhead = None

    strategy = None
    registry = None
    grouper = None
    queue = None
    queueProxy = None
    harq = None

    maxBeams = None # MIMO
    numberOfTimeSlots = None # TDMA within TTI/frame
    freqChannels = None
    uplinkMaster = None # True only for RS-RX
    txSchedulerFUName = None
    classifierName = None
    slotDuration = None
    subCarriersPerSubChannel = None # rs: for correct PhyMode construction
    switchingPointOffset = None
    framesPerSuperFrame = None
    group = None
    symbolDuration = None

    writeMapOutput = False
    mapOutputFileName = None

    maxTxPower = None
    metaScheduler = None

    def setPhyModeMapper(self, phyModeMapper):
        self.registry.setPhyModeMapper(phyModeMapper)

    def setStrategy(self, strategy, plm):
        self.strategy = copy.deepcopy(strategy) # original object shares logger instance
        self.strategy.symbolDuration = plm.mac.fullSymbolDur
        self.strategy.setParentLogger(self.logger) # to be done after self.logger is set
        self.subCarriersPerSubChannel = plm.phy.phyResourceSize # rs: for correct PhyMode construction

    def __init__(self, mode, PLM, strategy, functionalUnitName, commandName,
                 uplinkMaster = False, group = None, parentLogger = None, txSchedulerFUName = None, maxTxPower = None,
                 **kw):
        super(ResourceScheduler,self).__init__(functionalUnitName, commandName)
        self.setMode(mode)
        self.registry = RegistryProxy(mode,parentLogger = self.logger)
        self.maxBeams = 1 # no SDMA for now
        self.numberOfTimeSlots = 1 # no TDMA for now
        self.freqChannels = PLM.mac.usedSubChannels
        self.switchingPointOffset = PLM.mac.switchingPointOffset
        self.group = group
        self.framesPerSuperFrame = PLM.mac.framesPerSuperFrame
        self.uplinkMaster = uplinkMaster
        self.symbolDuration = PLM.mac.symbolLength + PLM.mac.cpLength
        self.maxTxPower = maxTxPower
        self.resUsageProbeName = "lte.resourceUsage"
        self.ulTBSizeProbeName = "lte.uplinkTBSize"
        self.numRetransmissionsProbeName = "lte.numRetransmissions"


        if (self.uplinkMaster):
            self.slotDuration = PLM.mac.ulSymbols * PLM.mac.fullSymbolDur
            self.logger = openwns.logger.Logger("LTE", "RS-RX", True, parentLogger)
            self.probeNameQueue = "lte.schedulerRXQueueSize"
            self.probeNameQueueOverhead = "lte.schedulerRXSegmentOverhead"
            self.queueProxy = QueueProxy(parentLogger = self.logger, sizeProbeName = self.probeNameQueue)
            self.txSchedulerFUName = txSchedulerFUName
            self.harq = openwns.scheduler.HARQRetransmissionProxy(parentLogger = parentLogger)
            # uplinkMaster needs no HARQ because all packets are virtual
        else: # TX Schedulers (BS,UT)
            self.slotDuration = PLM.mac.dlSymbols * PLM.mac.fullSymbolDur
            # ^ this slotDuration is wrong for UL slave scheduler
            self.logger = openwns.logger.Logger("LTE", "RS-TX", True, parentLogger)
            self.resourceUsageProbeName = "lte.ResourceUsageTX"

            self.probeNameQueue = "lte.schedulerTXQueueSize"
            self.probeNameQueueOverhead = "lte.schedulerTXSegmentOverhead"
            # "um" == "unacknowledged mode"
            #funame = mode.modeName # "_um" doesnt work yet [dbn,rs]:
            #funame = mode.modeName + mode.separator + "um" # tdd100_BS_um
            _funame = mode.modeName + mode.separator + "um" # tdd100_BS_um
            _commandName = mode.modeBase + mode.separator + 'um' # tdd100_um

            # Segmentation/Concatenation according to 3GPP TS 36.322 V8.6.0 (5 bit SN)
            self.queue = openwns.Scheduler.SegmentingQueue(segmentHeaderFUName = _funame, 
                segmentHeaderCommandName = _commandName, parentLogger = self.logger, 
                sizeProbeName = self.probeNameQueue, overheadProbeName = self.probeNameQueueOverhead, 
                isDropping=True, delayProbeName = 'lte.schedulerQueue')

            # Only once per subframe, 8 bit RLC, 16 bit MAC, 24 bit CRC
            self.queue.fixedHeaderSize = 48
            # Only for RLC, MAC multiplexing not supported yet
            self.queue.extensionHeaderSize = 12
            self.queue.byteAlignHeader = False
            self.harq = openwns.scheduler.HARQ(parentLogger = parentLogger)
            # self.harq = None # should mean: don't use HARQ

        self.setStrategy(strategy, PLM)
        self.registry = RegistryProxy(mode,parentLogger = self.logger)
        self.metaScheduler = openwns.scheduler.metascheduler.NoMetaScheduler
        self.grouper = openwns.Scheduler.NoGrouper

class BS(ResourceScheduler):
    __plugin__ = 'lte.timing.ResourceScheduler.BS'

    def __init__(self, mode, fuName, commandName, uplinkMaster = False, group = None, parentLogger = None, txSchedulerFUName="", maxTxPower = None):
        if uplinkMaster:
            ResourceScheduler.__init__(self,
                                       mode,
                                       mode.plm,
                                       mode.scheduler.uplinkMaster.createStrategy(parentLogger),
                                       fuName,
                                       commandName,
                                       uplinkMaster,
                                       group,
                                       parentLogger,
                                       txSchedulerFUName,
                                       maxTxPower = maxTxPower)
        else:
            ResourceScheduler.__init__(self,
                                       mode,
                                       mode.plm,
                                       mode.scheduler.downlink.createStrategy(parentLogger),
                                       fuName,
                                       commandName,
                                       uplinkMaster,
                                       group,
                                       parentLogger,
                                       txSchedulerFUName,
                                       maxTxPower = maxTxPower)

class UT(ResourceScheduler):
    __plugin__ = 'lte.timing.ResourceScheduler.UT'
    def __init__(self, mode, fuName, commandName, group=None, parentLogger = None, maxTxPower = None):
            ResourceScheduler.__init__(self,
                                       mode,
                                       mode.plm,
                                       mode.scheduler.uplinkSlave.createStrategy(parentLogger),
                                       fuName,
                                       commandName,
                                       False,
                                       group,
                                       parentLogger,
                                       maxTxPower = maxTxPower)

class No(openwns.FUN.FunctionalUnit):
    __plugin__ = 'lte.timing.ResourceScheduler.None'
    def __init__(self, fuName, commandName):
        super(No,self).__init__(fuName, commandName)

    def setPhyModeMapper(self, dummy):
        pass

