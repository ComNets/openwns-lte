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
from openwns.evaluation import *
from lte.evaluation.generators import *

probeNamePrefix = 'lte.'

def installEvaluation(sim,
                      loggingStations = None,
                      eNBIdList = [],
                      ueIdList = [],
                      settlingTime = 0.0,
                      maxThroughputPerUE = 20.0e6,
                      byCellId = False):

    installModeIndependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime, maxThroughputPerUE, byCellId)
    installModeDependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime, byCellId)

def installModeIndependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime, maxThroughputPerUE, byCellId):

    if(byCellId):
        filterString = "MAC.CellId"
        filterNodes = eNBIdList
    else:
        filterString = "MAC.Id"
        filterNodes = loggingStations        

    for direction in [ 'incoming', 'outgoing', 'aggregated' ]:
        for what in ['compound', 'bit']:
            sourceName = probeNamePrefix + 'total.window.' + direction + '.' + what + 'Throughput'
            node = openwns.evaluation.createSourceNode(sim, sourceName)
            node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
            s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
            bs = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList))
            ut = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList))
            bs.getLeafs().appendChildren(SeparateOnlyBSs())
            uts=ut.getLeafs().appendChildren(SeparateOnlyUTs())
            bs.getLeafs().appendChildren(Separate(by = 'MAC.Id', forAll = eNBIdList, format='BS_MAC.Id%d'))
            bs.getLeafs().appendChildren(Moments(name = sourceName, description = 'Top %s %s throughput [Bit/s]' % (direction, what)))

            if what == "bit":
                uts.appendChildren(PDF(name = sourceName, minXValue = 0.0, maxXValue=maxThroughputPerUE, resolution=1000) )

    sourceName = probeNamePrefix + 'numUsers'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))

    node = node.appendChildren(PDF(name = sourceName,
                      description = 'Associated UTs',
                      minXValue = 0,
                      maxXValue = len(ueIdList),
                      resolution = len(ueIdList)))

    node = node.appendChildren(Separate(by = 'MAC.Id', forAll = eNBIdList, format='BS_MAC.Id%d'))                      
    node.getLeafs().appendChildren(PDF(name = sourceName,
                      description = 'Associated UTs',
                      minXValue = 0,
                      maxXValue = len(ueIdList),
                      resolution = len(ueIdList)))                      


    sourceName = probeNamePrefix + 'schedulerQueue.delay'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s=node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName,
                                description = 'Downlink Queue Delay',
                                minXValue = 0.0,
                                maxXValue = 0.1,
                                resolution = 200))
    uplink.appendChildren(PDF(name = sourceName,
                              description = 'Uplink Queue Delay',
                              minXValue = 0.0,
                              maxXValue = 0.1,
                              resolution = 200))
    node.getLeafs().appendChildren(Plot2D(xDataKey = 'MAC.Id',
                                            minX = 0,
                                            maxX = max(loggingStations) + 1,
                                            resolution = max(loggingStations) + 1,
                                            statEvals = ['mean','deviation','max']))

    sourceName = probeNamePrefix + 'totalTxDelay'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s=node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName,
                                description = 'Downlink Tx Delay',
                                minXValue = 0.0,
                                maxXValue = 0.1,
                                resolution = 200))
    uplink.appendChildren(PDF(name = sourceName,
                              description = 'Uplink Tx Delay',
                              minXValue = 0.0,
                              maxXValue = 0.1,
                              resolution = 200))
    node.getLeafs().appendChildren(Plot2D(xDataKey = 'MAC.PeerId',
                                            minX = 0,
                                            maxX = max(loggingStations) + 1,
                                            resolution = max(loggingStations) + 1,
                                            statEvals = ['mean','deviation','max']))

def installModeDependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime, byCellId):

    if(byCellId):
        filterString = "MAC.CellId"
        filterNodes = eNBIdList
    else:
        filterString = "MAC.Id"
        filterNodes = loggingStations

    sourceName = probeNamePrefix + 'SINR'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    # center cell DL
    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    #dl.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell DL SINR distribution [dB]'))
    dl.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell DL SINR distribution [dB]',
                                     minXValue = -50.0,
                                     maxXValue = 100.0,
                                     resolution = 150))
    # center cell UL: only UEs in center cell assiciated to any center cell BS
    ul = node.appendChildren(Accept(by='MAC.Id', ifIn = eNBIdList, suffix='UL_CenterCell'))
    # ul.getLeafs().appendChildren(Logger())
    ul.getLeafs().appendChildren(Accept(by='Peer.NodeID', ifIn = ueIdList))
    #ul.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell UL SINR distribution [dB]'))
    ul.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell UL SINR distribution [dB]',
                                     minXValue = -50.0,
                                     maxXValue = 100.0,
                                     resolution = 150))

    sourceName = probeNamePrefix + 'MAP_SINR'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    # center cell DL
    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    dl.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell DL SINR distribution [dB]',
                                     minXValue = -50.0,
                                     maxXValue = 100.0,
                                     resolution = 150))

    sourceName = probeNamePrefix + 'Carrier'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    # center cell DL
    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    #dl.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell DL SINR distribution [dB]'))
    dl.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell DL SINR distribution [dB]',
                                     minXValue = -200.0,
                                     maxXValue = 200.0,
                                     resolution = 400))
    # center cell UL: only UEs in center cell assiciated to any center cell BS
    ul = node.appendChildren(Accept(by='MAC.Id', ifIn = eNBIdList, suffix='UL_CenterCell'))
    # ul.getLeafs().appendChildren(Logger())
    ul.getLeafs().appendChildren(Accept(by='Peer.NodeID', ifIn = ueIdList))
    #ul.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell UL SINR distribution [dB]'))
    ul.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell UL SINR distribution [dB]',
                                     minXValue = -200.0,
                                     maxXValue = 200.0,
                                     resolution = 400))

    sourceName = probeNamePrefix + 'Interference'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    # center cell DL
    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    #dl.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell DL SINR distribution [dB]'))
    dl.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell DL SINR distribution [dB]',
                                     minXValue = -200.0,
                                     maxXValue = 200.0,
                                     resolution = 400))
    # center cell UL: only UEs in center cell assiciated to any center cell BS
    ul = node.appendChildren(Accept(by='MAC.Id', ifIn = eNBIdList, suffix='UL_CenterCell'))
    # ul.getLeafs().appendChildren(Logger())
    ul.getLeafs().appendChildren(Accept(by='Peer.NodeID', ifIn = ueIdList))
    #ul.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell UL SINR distribution [dB]'))
    ul.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell UL SINR distribution [dB]',
                                     minXValue = -200.0,
                                     maxXValue = 200.0,
                                     resolution = 400))

    sourceName = probeNamePrefix + 'TxPower'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s=node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName,
                                description = 'Downlink Tx Power distribution',
                                minXValue = -10.0,
                                maxXValue = 40.0,
                                resolution = 100))
    uplink.appendChildren(PDF(name = sourceName,
                              description = 'Uplink Tx Power distribution',
                              minXValue = -110.0,
                              maxXValue = 22.0,
                              resolution = 100))

    sourceName = probeNamePrefix + 'IoT'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = 0.0, maxXValue=70.0, resolution=100, description = 'Interference over Thermal (IoT) [dB]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = 0.0, maxXValue=70.0, resolution=100, description = 'Interference over Thermal (IoT) [dB]'))

    sourceName = probeNamePrefix + 'SINRestError'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = -100.0, maxXValue=100.0, resolution=200, description = 'SINR Estimation Error [dB]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = -100.0, maxXValue=100.0, resolution=200, description = 'SINR Estimation Error [dB]'))

    sourceName = probeNamePrefix + 'IestError'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = -100.0, maxXValue=100.0, resolution=200, description = 'Interference Estimation Error [dBm]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = -100.0, maxXValue=100.0, resolution=200, description = 'Interference Estimation Error [dBm]'))

    sourceName = probeNamePrefix + 'SestError'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = -100.0, maxXValue=100.0, resolution=200, description = 'Carrier Estimation Error [dBm]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = -100.0, maxXValue=100.0, resolution=200, description = 'Carrier Estimation Error [dBm]'))

    sourceName = probeNamePrefix + 'SINRest'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = -50.0, maxXValue=100.0, resolution=150, description = 'SINR Estimation [dB]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = -50.0, maxXValue=100.0, resolution=150, description = 'SINR Estimation Error [dB]'))

    sourceName = probeNamePrefix + 'effSINRest'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = -50.0, maxXValue=100.0, resolution=150, description = 'SINR Estimation [dB]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = -50.0, maxXValue=100.0, resolution=150, description = 'SINR Estimation Error [dB]'))

    sourceName = probeNamePrefix + 'PhyMode'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = filterString, ifIn = filterNodes))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    ul = node.appendChildren(Accept(by='MAC.Id', ifIn = eNBIdList, suffix='UL_CenterCell'))
    ul.appendChildren(PDF(name = sourceName,
                          description = 'UL PhyMode',
                          minXValue = 0.0,
                          maxXValue = 20.0,
                          resolution = 20.0))

    dl.appendChildren(PDF(name = sourceName,
                          description = 'DL PhyMode',
                          minXValue = 0.0,
                          maxXValue = 20.0,
                          resolution = 20.0))

    sourceName = probeNamePrefix + 'resourceUsage'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node = node.appendChildren(Accept(by='MAC.Id', ifIn = loggingStations, suffix="CenterCell"))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))

    dl = node.appendChildren(Accept(by = 'SchedulerSpot', ifIn = [1], suffix='DL'))
    ulm = node.appendChildren(Accept(by = 'SchedulerSpot', ifIn = [2], suffix='ULMaster'))
    uls = node.appendChildren(Accept(by = 'SchedulerSpot', ifIn = [3], suffix='ULSlave'))
    node.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Resource Usage',
                                     minXValue = 0.0,
                                     maxXValue = 1.0,
                                     resolution = 100))


    sourceName = probeNamePrefix + 'uplinkTBSize'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node = node.appendChildren(Accept(by='MAC.Id', ifIn = loggingStations, suffix="CenterCell"))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))

    node.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Uplink TB Size',
                                     minXValue = 0.0,
                                     maxXValue = 100.0,
                                     resolution = 100))
    node.getLeafs().appendChildren(Plot2D(xDataKey = 'MAC.Id',
                                            minX = 0,
                                            maxX = max(loggingStations) + 1,
                                            resolution = max(loggingStations) + 1,
                                            statEvals = ['mean','deviation','max']))


    sourceName = 'scheduler.harq.effSINR'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node = node.appendChildren(Accept(by='nodeID', ifIn = loggingStations, suffix="CenterCell"))
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))

    dl = node.appendChildren(Accept(by='nodeID', ifIn = ueIdList, suffix='UL_CenterCell'))
    ul = node.appendChildren(Accept(by='nodeID', ifIn = eNBIdList, suffix='DL_CenterCell'))
    node.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Effective SINR',
                                     minXValue = -50,
                                     maxXValue = 100,
                                     resolution = 150))
    
    dl.appendChildren(Accept(by = 'decoded', ifIn = [1], suffix='Decoded'))
    dl.appendChildren(Accept(by = 'decoded', ifIn = [0], suffix='Undecoded'))
    ul.appendChildren(Accept(by = 'decoded', ifIn = [1], suffix='Decoded'))
    ul.appendChildren(Accept(by = 'decoded', ifIn = [0], suffix='Undecoded'))

    node.getLeafs().appendChildren(PDF(name = sourceName,
                                 description = 'Effective SINR',
                                 minXValue = -50,
                                 maxXValue = 100,
                                 resolution = 150))

    sourceName = probeNamePrefix + "numRetransmissions"
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))

    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    ul = node.appendChildren(Accept(by='MAC.Id', ifIn = eNBIdList, suffix='UL_CenterCell'))
    node.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'HARQ Retransmissions',
                                     minXValue = 0,
                                     maxXValue = 20,
                                     resolution = 20))
    node.getLeafs().appendChildren(Plot2D(xDataKey = 'MAC.PeerId',
                                            minX = 0,
                                            maxX = max(loggingStations) + 1,
                                            resolution = max(loggingStations) + 1,
                                            statEvals = ['mean','deviation','max']))


