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
                      settlingTime = 0.0):

    installModeIndependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime)
    installModeDependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime)

def installModeIndependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime):
    for direction in [ 'incoming', 'outgoing', 'aggregated' ]:
        for what in [ 'bit', 'compound' ]:
            sourceName = probeNamePrefix + 'total.window.' + direction + '.' + what + 'Throughput'
            node = openwns.evaluation.createSourceNode(sim, sourceName)
            s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
            bs = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList))
            ut = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList))
            bs.getLeafs().appendChildren(SeparateOnlyBSs())
            uts=ut.getLeafs().appendChildren(SeparateOnlyUTs())
            bs.getLeafs().appendChildren(Separate(by = 'MAC.Id', forAll = eNBIdList, format='BS_MAC.Id%d'))
            ut.getLeafs().appendChildren(Separate(by = 'MAC.Id', forAll = ueIdList, format='UT_MAC.Id%d'))
	    bs.getLeafs().appendChildren(Moments(name = sourceName, description = 'Top %s %s throughput [Bit/s]' % (direction, what)))
            ut.getLeafs().appendChildren(Moments(name = sourceName, description = 'Top %s %s throughput [Bit/s]' % (direction, what)))

            uts.appendChildren(PDF(name = sourceName + "ALLUT", minXValue = 0.0, maxXValue=10e6, resolution=100) )

def installModeDependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime):

    sourceName = probeNamePrefix + 'SINR'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    # center cell DL
    dl = node.appendChildren(Accept(by='MAC.Id', ifIn = ueIdList, suffix='DL_CenterCell'))
    #dl.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell DL SINR distribution [dB]'))
    dl.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell DL SINR distribution [dB]',
                                     minXValue = -20.0,
                                     maxXValue = 30.0,
                                     resolution = 100))
    # center cell UL: only UEs in center cell assiciated to any center cell BS
    ul = node.appendChildren(Accept(by='MAC.Id', ifIn = eNBIdList, suffix='UL_CenterCell'))
    # ul.getLeafs().appendChildren(Logger())
    ul.getLeafs().appendChildren(Accept(by='Peer.NodeID', ifIn = ueIdList))
    #ul.getLeafs().appendChildren(Moments(name=sourceName, description='Center Cell UL SINR distribution [dB]'))
    ul.getLeafs().appendChildren(PDF(name = sourceName,
                                     description = 'Center Cell UL SINR distribution [dB]',
                                     minXValue = -20.0,
                                     maxXValue = 30.0,
                                     resolution = 100))

    sourceName = probeNamePrefix + 'TxPower'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
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
    node.appendChildren(Accept(by = 'MAC.Id', ifIn = loggingStations))
    s = node.getLeafs().appendChildren(SettlingTimeGuard(settlingTime=settlingTime))
    downlink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = ueIdList, suffix="DL_CenterCell"))
    uplink = s.appendChildren(Accept(by = 'MAC.Id', ifIn = eNBIdList, suffix="UL_CenterCell"))
    downlink.appendChildren(PDF(name = sourceName, minXValue = 0.0, maxXValue=70.0, resolution=100, description = 'Interference over Thermal (IoT) [dB]'))
    uplink.appendChildren(PDF(name = sourceName, minXValue = 0.0, maxXValue=70.0, resolution=100, description = 'Interference over Thermal (IoT) [dB]'))

    sourceName = probeNamePrefix + 'PhyMode'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(Accept(by = 'MAC.Id', ifIn = loggingStations))
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

