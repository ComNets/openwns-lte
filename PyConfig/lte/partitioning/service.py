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

import dll.Services
import openwns.logger

class PartitioningInfo(dll.Services.Service):
   scheme = None
   logger = None

   def __init__(self, scheme, name='', parentLogger = None):
      self.nameInServiceFactory = "lte.timing.partitioning.PartitioningInfo"
      self.serviceName = 'PARTITIONINGINFO'+name
      self.scheme = scheme
      self.scheme.setParentLogger(parentLogger);
      self.logger = openwns.logger.Logger("WinProSt", "PartInfo", True, parentLogger)
