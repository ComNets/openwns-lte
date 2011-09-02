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

from rise.PhyMode import PhyMode, PhyModeMapper
from openwns.interval import Interval

LTELowestPhyMode = PhyMode("QPSK-lte_m_2_tbs_1384") # this is the PhyMode for MAP/BCH/RACH etc
class LTEMapper(PhyModeMapper):
    ltemapper = None
    count = 0

    @staticmethod
    def getInstance(mode, tolerablePER=0.1):
        if(LTEMapper.ltemapper == None):
            LTEMapper.ltemapper = LTEMapper(mode, tolerablePER)
        assert LTEMapper.ltemapper.subCarriersPerSubChannel == mode.plm.phy.phyResourceSize, "Mode mismatch"
        assert LTEMapper.ltemapper.tolerablePER == tolerablePER, "Tolerable PER mismatch"
        return LTEMapper.ltemapper

    def __init__(self, mode, tolerablePER=0.1):
        LTEMapper.count = LTEMapper.count + 1
        if(LTEMapper.count > 1):
            print "More than one PhyModeMapper instance, is this intended?"
        self.tolerablePER = tolerablePER
        symbolDuration = mode.plm.mac.symbolLength + mode.plm.mac.cpLength
        subCarriersPerSubChannel = mode.plm.phy.phyResourceSize
        PhyModeMapper.__init__(self, symbolDuration, subCarriersPerSubChannel)

        if (tolerablePER == 0.1):
            # targetPER = 0.1
            self.setMinimumSINR(-6.0934)
            self.addPhyMode(Interval(-200,-4.06716,"(]"),PhyMode("QPSK-lte_m_2_tbs_1384"))
            self.addPhyMode(Interval(-4.06716,-1.96295,"(]"),PhyMode("QPSK-lte_m_2_tbs_2216"))
            self.addPhyMode(Interval(-1.96295,-0.164423,"(]"),PhyMode("QPSK-lte_m_2_tbs_3624"))
            self.addPhyMode(Interval(-0.164423,1.92114,"(]"),PhyMode("QPSK-lte_m_2_tbs_5160"))
            self.addPhyMode(Interval(1.92114,3.82291,"(]"),PhyMode("QPSK-lte_m_2_tbs_6968"))
            self.addPhyMode(Interval(3.82291,5.80874,"(]"),PhyMode("QAM16-lte_m_4_tbs_8760"))
            self.addPhyMode(Interval(5.80874,8.46731,"(]"),PhyMode("QAM16-lte_m_4_tbs_11448"))
            self.addPhyMode(Interval(8.46731,9.91998,"(]"),PhyMode("QAM16-lte_m_4_tbs_15264"))
            self.addPhyMode(Interval(9.91998,12.5036,"(]"),PhyMode("QAM64-lte_m_6_tbs_16416"))
            self.addPhyMode(Interval(12.5036,14.756,"(]"),PhyMode("QAM64-lte_m_6_tbs_21384"))
            self.addPhyMode(Interval(14.756,16.0828,"(]"),PhyMode("QAM64-lte_m_6_tbs_25456"))
            self.addPhyMode(Interval(16.0828,17.8246,"(]"),PhyMode("QAM64-lte_m_6_tbs_28336"))
            self.addPhyMode(Interval(17.8246,200,"(]"),PhyMode("QAM64-lte_m_6_tbs_31704"))
            return
        if (tolerablePER == 0.01):
            self.setMinimumSINR(-5.87536)
            self.addPhyMode(Interval(-200,-3.87562,"(]"),PhyMode("QPSK-lte_m_2_tbs_1384"))
            self.addPhyMode(Interval(-3.87562,-1.80653,"(]"),PhyMode("QPSK-lte_m_2_tbs_2216"))
            self.addPhyMode(Interval(-1.80653,0.0121014,"(]"),PhyMode("QPSK-lte_m_2_tbs_3624"))
            self.addPhyMode(Interval(0.0121014,2.10849,"(]"),PhyMode("QPSK-lte_m_2_tbs_5160"))
            self.addPhyMode(Interval(2.10849,3.95881,"(]"),PhyMode("QPSK-lte_m_2_tbs_6968"))
            self.addPhyMode(Interval(3.95881,5.94965,"(]"),PhyMode("QAM16-lte_m_4_tbs_8760"))
            self.addPhyMode(Interval(5.94965,8.6444,"(]"),PhyMode("QAM16-lte_m_4_tbs_11448"))
            self.addPhyMode(Interval(8.6444,10.0589,"(]"),PhyMode("QAM16-lte_m_4_tbs_15264"))
            self.addPhyMode(Interval(10.0589,12.6809,"(]"),PhyMode("QAM64-lte_m_6_tbs_16416"))
            self.addPhyMode(Interval(12.6809,14.9322,"(]"),PhyMode("QAM64-lte_m_6_tbs_21384"))
            self.addPhyMode(Interval(14.9322,16.2451,"(]"),PhyMode("QAM64-lte_m_6_tbs_25456"))
            self.addPhyMode(Interval(16.2451,17.9849,"(]"),PhyMode("QAM64-lte_m_6_tbs_28336"))
            self.addPhyMode(Interval(17.9849,200,"(]"),PhyMode("QAM64-lte_m_6_tbs_31704"))
            return
        if (tolerablePER == 0.001):
            self.setMinimumSINR(-5.69962)
            self.addPhyMode(Interval(-200,-3.77288,"(]"),PhyMode("QPSK-lte_m_2_tbs_1384"))
            self.addPhyMode(Interval(-3.77288,-1.73754,"(]"),PhyMode("QPSK-lte_m_2_tbs_2216"))
            self.addPhyMode(Interval(-1.73754,0.11596,"(]"),PhyMode("QPSK-lte_m_2_tbs_3624"))
            self.addPhyMode(Interval(0.11596,2.22579,"(]"),PhyMode("QPSK-lte_m_2_tbs_5160"))
            self.addPhyMode(Interval(2.22579,4.07278,"(]"),PhyMode("QPSK-lte_m_2_tbs_6968"))
            self.addPhyMode(Interval(4.07278,6.09003,"(]"),PhyMode("QAM16-lte_m_4_tbs_8760"))
            self.addPhyMode(Interval(6.09003,8.78537,"(]"),PhyMode("QAM16-lte_m_4_tbs_11448"))
            self.addPhyMode(Interval(8.78537,10.1496,"(]"),PhyMode("QAM16-lte_m_4_tbs_15264"))
            self.addPhyMode(Interval(10.1496,12.8918,"(]"),PhyMode("QAM64-lte_m_6_tbs_16416"))
            self.addPhyMode(Interval(12.8918,15.1701,"(]"),PhyMode("QAM64-lte_m_6_tbs_21384"))
            self.addPhyMode(Interval(15.1701,16.3705,"(]"),PhyMode("QAM64-lte_m_6_tbs_25456"))
            self.addPhyMode(Interval(16.3705,18.1644,"(]"),PhyMode("QAM64-lte_m_6_tbs_28336"))
            self.addPhyMode(Interval(18.1644,200,"(]"),PhyMode("QAM64-lte_m_6_tbs_31704"))
            return
        assert False, "Unsupported tolerablePER setting. Choose from [0.1, 0.01, 0.001]"
