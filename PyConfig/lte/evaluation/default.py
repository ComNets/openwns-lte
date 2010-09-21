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
            node.appendChildren(Logger())

def installModeDependentDefaultEvaluation(sim, loggingStations, eNBIdList, ueIdList, settlingTime):
    pass
