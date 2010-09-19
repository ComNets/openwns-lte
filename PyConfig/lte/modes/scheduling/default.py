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

import lte.dll.bch

import openwns.Scheduler
import openwns.scheduler.DSAStrategy
import openwns.scheduler.APCStrategy

class DownlinkSchedulerSetting:
    
    def __init__(self):
        useHARQ = True
        self.subStrategiesTXDL =(
            lte.dll.bch.BCHSchedulerStrategy(), # for priority 0
            openwns.Scheduler.HARQRetransmission(), # for priority 1
            openwns.Scheduler.RoundRobin(), # for priority 2
            openwns.Scheduler.RoundRobin(), # for priority 3
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 4
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 5
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 6
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ) # for priority 7
            )

        self.dsastrategyDL = openwns.scheduler.DSAStrategy.LinearFFirst(oneUserOnOneSubChannel = True, useRandomChannel = True)

        self.dsafbstrategyDL = openwns.scheduler.DSAStrategy.LinearFFirst(oneUserOnOneSubChannel = True, useRandomChannel = True)

        self.apcstrategy = openwns.scheduler.APCStrategy.UseNominalTxPower()


    def createStrategy(self, parentLogger = None):
        strategy = openwns.Scheduler.StaticPriority(parentLogger=parentLogger,
                                                    txMode = True,
                                                    subStrategies = self.subStrategiesTXDL,
                                                    dsastrategy=self.dsastrategyDL,
                                                    dsafbstrategy=self.dsafbstrategyDL,
                                                    apcstrategy=self.apcstrategy)
        return strategy

class UplinkMasterSchedulerSetting:
    
    def __init__(self):
        useHARQ = True
        self.subStrategiesRXUL =(
            openwns.Scheduler.Disabled(), # for priority 0
            openwns.Scheduler.HARQUplinkRetransmission(), # for priority 1
            openwns.Scheduler.RoundRobin(), # for priority 2
            openwns.Scheduler.RoundRobin(), # for priority 3
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 4
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 5
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 6
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ) # for priority 7
            )

        self.dsastrategyULMaster = openwns.scheduler.DSAStrategy.LinearFFirst(oneUserOnOneSubChannel = True, useRandomChannel = True)

        self.dsafbstrategyULMaster = openwns.scheduler.DSAStrategy.LinearFFirst(oneUserOnOneSubChannel = True, useRandomChannel = True)

        self.apcstrategy = openwns.scheduler.APCStrategy.UseNominalTxPower()


    def createStrategy(self, parentLogger = None):
        strategy = openwns.Scheduler.StaticPriority(parentLogger=parentLogger,
                                                    txMode = False,
                                                    subStrategies = self.subStrategiesRXUL,
                                                    dsastrategy=self.dsastrategyULMaster,
                                                    dsafbstrategy=self.dsafbstrategyULMaster,
                                                    apcstrategy=self.apcstrategy)
        return strategy

class UplinkSlaveSchedulerSetting:
    
    def __init__(self):
        useHARQ = True
        self.subStrategiesTXUL = (
            openwns.Scheduler.Disabled(), # for priority 0
            openwns.Scheduler.HARQUplinkSlaveRetransmission(), # for priority 1
            openwns.Scheduler.RoundRobin(), # for priority 2
            openwns.Scheduler.RoundRobin(), # for priority 3
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 4
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 5
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ), # for priority 6
            openwns.Scheduler.RoundRobin(useHARQ=useHARQ) # for priority 7
            )

        self.dsastrategyULSlave  = openwns.scheduler.DSAStrategy.DSASlave(oneUserOnOneSubChannel = True)
        self.dsafbstrategyULSlave  = openwns.scheduler.DSAStrategy.DSASlave(oneUserOnOneSubChannel = True)
        self.apcstrategy  = openwns.scheduler.APCStrategy.UseNominalTxPower()

    def createStrategy(self, parentLogger = None):
        strategy = openwns.Scheduler.StaticPriority(parentLogger=parentLogger,
                                                    txMode = True,
                                                    subStrategies = self.subStrategiesTXUL,
                                                    dsastrategy=self.dsastrategyULSlave,
                                                    dsafbstrategy=self.dsafbstrategyULSlave,
                                                    apcstrategy=self.apcstrategy,
                                                    powerControlSlave=True)
        return strategy

class SchedulerSettings:

    def __init__(self):
        self.downlink = DownlinkSchedulerSetting()
        self.uplinkMaster = UplinkMasterSchedulerSetting()
        self.uplinkSlave =  UplinkSlaveSchedulerSetting()

