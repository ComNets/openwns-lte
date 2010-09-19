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

import openwns
import openwns.logger
import openwns.SAR

class eNBRLC(openwns.FUN.FunctionalUnit, openwns.StaticFactoryClass):

    def __init__(self, parentLogger = None):
        openwns.StaticFactoryClass.__init__(self, 'lte.rlc.eNB')
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName = "rlc")

        self.logger = openwns.logger.Logger(moduleName = "LTE",
                                            name = "RLC",
                                            enabled = True,
                                            parent = parentLogger)

class UERLC(openwns.FUN.FunctionalUnit, openwns.StaticFactoryClass):

    def __init__(self, parentLogger = None):
        openwns.StaticFactoryClass.__init__(self, 'lte.rlc.UE')
        openwns.FUN.FunctionalUnit.__init__(self, functionalUnitName = "rlc")

        self.logger = openwns.logger.Logger(moduleName = "LTE",
                                            name = "RLC",
                                            enabled = True,
                                            parent = parentLogger)

class ReorderingWindow(openwns.SAR.ReorderingWindow):

    def __init__(self, snFieldLength, parentLogger = None):
        super(ReorderingWindow, self).__init__(snFieldLength, parentLogger)
        # self.tReordering can take values from 0ms to 200ms
        # According to 3GPP TS 36.331 V8.5.0 (2009-03)
        # However SRB1 and SRB2 default radio bearer configurations use 35ms
        # This is also described in TS 36.331
        self.tReordering = 0.035
        self.logger = openwns.logger.Logger('LTE', 'UM.Reordering', True, parentLogger)

class UnacknowledgedMode(openwns.SAR.SegAndConcat):

    """LTE UM as described in 3GPP TS 36.322 V8.5.0
    The size of and number PDUs passed to lower layers is calculated as follows:
    The totalsize to be segmented is the length of the SDU received from upper layers
    plus the sduLengthAddition (default 0). This total length is cut into pieces of
    segmentSize. The last segment can possible be smaller than the segment size.
    Each segment is then prepended by a header of length headerSize.
    """

    def __init__(self, segmentSize, headerSize, commandName, delayProbeName = None, parentLogger = None):
        super(UnacknowledgedMode, self).__init__(segmentSize, headerSize, commandName, delayProbeName, parentLogger)
        self.__plugin__ = "lte.rlc.UnacknowledgedMode"
        self.logger = openwns.logger.Logger('LTE', 'UM', True, parentLogger)
        self.sduLengthAddition = 0
        # long serial number option chosen for safety here. If too many segments
        # are on the way, the segments will not be reassembled if field length
        # is too short.
        # todo dbn: This should be set to the short option (5) for VoIP. In
        # general we need simulator parameter settings per QoS class
        self.reorderingWindow = ReorderingWindow(snFieldLength = 10, parentLogger = self.logger)
        self.isSegmenting = False
        self.segmentDropRatioProbeName = "lte.rlc.um.segmentDropRatio"
        self.reorderingWindowSizeProbeName = "lte.rlc.um.reorderingWindowSize"
        self.reassemblyBufferSizeProbeName = "lte.rlc.um.reassemblyBufferSize"
