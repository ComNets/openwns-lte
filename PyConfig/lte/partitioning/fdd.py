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

import lte.partitioning

class LTEFDDReuse1(lte.partitioning.Scheme):
    """
    Partitioning for LTE FDD given the number of subchannels and subframes
    Intended for scenarios without relaying
    """

    def __init__(self, noChannels, noSubFrames):
        lte.partitioning.Scheme.__init__(self)

        odd = lte.partitioning.Partitioning(noChannels)
        even = lte.partitioning.Partitioning(noChannels)

        odd.addGroup(number = 1, dedication = 'Universal')
        even.addGroup(number = 1, dedication = 'Universal')

        odd.addFreqRange(groupNumber = 1, fMin = 0, fMax = noChannels-1) # all subchannels for BS
        even.addFreqRange(groupNumber = 1, fMin = 0, fMax = noChannels-1) # all subchannels for BS

        # For the uplink master scheduler, to reserve resources for the PUCCH
        odd.addGroup(number = 2, dedication = 'Universal')
        even.addGroup(number = 2, dedication = 'Universal')

        odd.addFreqRange(groupNumber = 2, fMin = 4, fMax = noChannels-1) # 4 SCh for PUCCH
        even.addFreqRange(groupNumber = 2, fMin = 4, fMax = noChannels-1) # 4 SCh for PUCCH

        # add 20 (10*2) partitions to the static scheme
        for i in xrange(noSubFrames):
            self.addPartition(odd)
            self.addPartition(even)
