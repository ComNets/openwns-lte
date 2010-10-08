import openwns.logger
import openwns.FUN
from lte.modes.hasModeName import HasModeName
import dll.Services

class ThresholdCriterion(object):
    name = None
    margin = None

    def __init__(self, name, margin = None):
        self.name = name
        self.margin = margin
        if self.margin == None:
            if self.name == "SINR" or self.name == "Pathloss":
                self.margin = "0 dB"
            elif self.name == "RxPower":
                self.margin = "0 dBm"
            else:
                self.margin = 0.0


class BCHService(dll.Services.Service, HasModeName):
    # Criterion for which terminal decides for best BS/RN in reach: ???

    # can be one of [ "SINR", "RxPower", "Distance", "Pathloss"]
    criterion  = ThresholdCriterion("RxPower") 
    logger = None
    timeout = None
    upperThreshold = None
    lowerThreshold = None
    mihCapable = None
    triggersAssociation = True
    triggersReAssociation = True

    def __init__(self, mode):
        self.modeName = mode.modeName
        self.separator = mode.separator
        self.nameInServiceFactory = 'lte.controlplane.BCHService'
        self.serviceName = 'BCHSERVICE' + self.separator + self.modeName
        self.timeout = 0.01 # seconds
        self.criterion = mode.thresholdCriterion
        self.upperThreshold = mode.upperThreshold
        self.lowerThreshold = mode.lowerThreshold
        self.triggersReAssociation = mode.triggersReAssociation
        self.logger = openwns.logger.Logger("LTE",
                                            "BCHService"+self.separator+self.modeName,
                                            True,
                                            mode.logger)

class BCHSchedulerStrategy:
    __plugin__ = "lte.BCH.BCHSchedulerStrategy"
    def __init__(self, parentLogger = None):
        self.logger = openwns.logger.Logger("LTE", "BCHSchedulerStrategy", True, parentLogger)
        self.availableInSubframes = [0]
        self.numberOfSubchannels = 6

    def setParentLogger(self,parentLogger = None):
        self.logger = openwns.logger.Logger("LTE", "BCHSchedulerStrategy", True, parentLogger)

# BCH Unit configuration
class BCHUnit(openwns.FUN.FunctionalUnit, HasModeName):
    """ BCH Unit configuration """
    """size of the BCH command to be generated"""
    commandSize    = None
    logger         = None

    def __init__(self, mode, parentLogger = None):
        super(BCHUnit,self).__init__(functionalUnitName = mode.modeName + mode.separator + 'bch',
                                     commandName = mode.modeBase + mode.separator + 'bch')
        self.setMode(mode)
        self.logger = openwns.logger.Logger("LTE","BCHUnit", True, parentLogger)
        """The BCH command may piggyback an additional AssociationHandler Signal"""
        self.commandSize = 1

class RAP(BCHUnit):
    __plugin__ = 'lte.controlplane.BCHUnit.RAP'
    """has to be set externally"""
    txPower = None
    """Can this BS handle MIH"""
    rlcName        = None
    macgName = None
    flowManagerName = None

    def __init__(self, mode, parentLogger = None):
        BCHUnit.__init__(self, mode, parentLogger)
        self.txPower = mode.plm.phy.txPwrBS.nominalPerSubband
        self.rlcName = "rlc"
        self.macgName = "macg"
        self.flowManagerName = "FlowManagerBS"

class UT(BCHUnit):
    __plugin__ = 'lte.controlplane.BCHUnit.UT'
    schedulerName = None

    def __init__(self, mode, parentLogger = None):
        BCHUnit.__init__(self, mode, parentLogger)
        self.schedulerName = mode.modeName + mode.separator + "resourceSchedulerTX"

class No(openwns.FUN.FunctionalUnit):
    __plugin__ = 'lte.controlplane.BCHUnit.No'
    def __init__(self, mode):
        super(No,self).__init__(functionalUnitName = mode.modeName + mode.separator + 'bch',
                                commandName = 'noBCH')
