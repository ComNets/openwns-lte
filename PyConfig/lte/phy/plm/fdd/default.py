import openwns.Scheduler

class MACParams:
    duplex              = None # 'TDD' or 'FDD'
    duplexGuardInterval = None
    symbolLength        = None
    cpLength            = None
    fullSymbolDur       = None # symbolLength+cpLength
    symbolsPerChunk     = None
    symbolsPerMap       = None
    symbolsPerFrame     = None
    ulSymbols           = None
    dlSymbols           = None
    chunkLength         = None # [s]
    chunksPerFrame      = None
    switchingPoint      = None # [symbols]
    subCarriersPerChunk = None
    usedSubChannels     = None
    frameLength         = None # [s]
    framesPerSuperFrame = None
    symbolsPerPreamble  = None
    preambleGuard       = None
    symbolsPerRACH      = None
    mapLength           = None
    preambleLength      = None
    RACHLength          = None
    superFrameLength    = None # [s]
    safetyFraction      = None
    ulGuardSubCarriers  = None # NEW for OFDMA uplink, not used yet
    switchingPointOffset = None # [s] TDD: duration of DataTx phase; FDD: duration of 1st duplexGroup phase
    symbolsPerBlock      = None
    lowestPhyMode       = None # set later when mac+phy are combined into plm class
    schedulingOffset    = None # number of frames that the scheduling ios done in advance
    numberOfFramesToSchedule = None # how many frames are scheduled when StartMap is called
    useMapResourcesInUL      = None
    ulSegmentSize            = None # for SAR
    dlSegmentSize            = None

    def __init__(self, usedSubCarriers, subCarriersPerChunk):
        self.subCarriersPerChunk = subCarriersPerChunk
        self.usedSubChannels     = usedSubCarriers / subCarriersPerChunk
        # WINNER-specific:
        self.framesPerSuperFrame = 8
        self.symbolsPerPreamble  = 7
        self.safetyFraction      = 1e-8
        self.switchingPointOffset = 0
        self.symbolsPerBlock     = 3
        self.symbolsPerMap       = self.symbolsPerBlock

    def setMapLength(self):
        self.mapLength = (self.symbolLength + self.cpLength) * self.symbolsPerMap

    def setPreambleLength(self):
        self.preambleLength = (self.symbolLength + self.cpLength) * self.symbolsPerPreamble

    def setRACHLength(self):
        self.RACHLength = (self.symbolLength + self.cpLength) * self.symbolsPerRACH

    def setSuperFrameLength(self):
        self.superFrameLength = self.RACHLength + self.preambleGuard + self.preambleLength + (self.framesPerSuperFrame * self.frameLength)
        self.fullSymbolDur = self.symbolLength + self.cpLength

    def setSwitchingPoint(self, switchingSymbol):
        self.switchingPoint      = switchingSymbol
        self.ulSymbols           = self.symbolsPerFrame - self.switchingPoint
        self.dlSymbols           = self.symbolsPerFrame - self.ulSymbols - self.symbolsPerMap
        # to be correct, duplexGuardInterval should be added so that propagation delay etc is waited for
        self.switchingPointOffset = (self.switchingPoint - self.symbolsPerMap) * self.fullSymbolDur + self.duplexGuardInterval

class MACLTEFDD(MACParams):
    def __init__(self, usedSubCarriers, subCarriersPerChunk):
        MACParams.__init__(self, usedSubCarriers, subCarriersPerChunk)
        self.duplex              = 'FDD'

        self.framesPerSuperFrame = 10
        self.symbolsPerPreamble  = 0
        self.symbolsPerRACH      = 0
        self.symbolsPerMap       = 3
        self.safetyFraction      = 1E-8
        self.duplexGuardInterval = 0.0
        self.symbolLength        = 200e-6/3.0
        self.cpLength            = 4.761904761904763001313656278767894036719E-6
        self.preambleGuard       = self.duplexGuardInterval
        self.symbolsPerChunk     = 7
        self.chunkLength         = self.symbolsPerChunk * (self.symbolLength + self.cpLength)
        self.chunksPerFrame      = 2
        self.symbolsPerFrame     = self.symbolsPerChunk * self.chunksPerFrame
        self.switchingPoint      = 0
        self.frameLength         = 1e-3

        self.setMapLength()
        self.setPreambleLength()
        self.setRACHLength()
        self.setSuperFrameLength()
        assert (self.superFrameLength - 10e-3) < 1e-6

        self.ulSymbols           = self.symbolsPerFrame - self.symbolsPerMap
        self.dlSymbols           = self.symbolsPerFrame - self.symbolsPerMap

        # specify necessary guard frequency bands between transmitted bands of UTs in the uplink:
        self.symbolsPerBlock     = self.symbolsPerFrame - self.symbolsPerMap
        self.schedulingOffset         = 0 # start one frame ahead
        self.numberOfFramesToSchedule = 1 # schedulingOffset+numberOfFramesToSchedule >= RN taskPhase period

        self.dlSegmentSize = 12 # Bits
        self.ulSegmentSize = 12 # Bits
        self.useMapResourcesInUL = True
        if (self.useMapResourcesInUL == True):
            self.ulSymbols = self.symbolsPerFrame
            self.ulSegmentSize = 16 # Bits

class PHYLTEFDD:
    def __init__(self, usedSubCarriers):
        self.usedSubCarriers     = usedSubCarriers
        self.phyResourceSize     = 12 # 12 SC per SCh

        # Frequencies taken from draft LTE-A proposal to ITU (June 2009)
        # Deployment in the 2500-2690 MHz band
        self.ulCenterFreq        = 2500E6 
        self.dlCenterFreq        = 2620E6 
        self.subcarrierSpacing   = 15000 # Hz

        # UL == DL: UE receives other UE's BCH compounds, wrong SINR calculations!
        assert self.ulCenterFreq != self.dlCenterFreq, 'UL frequency == DL frequency!'

class LTE:

    def __init__(self):
        self.subCarrierBandwidth = 0.015 # [MHz]

        self.subCarrierPerSubChannel = 12

        self.subChannelBandwidth = self.subCarrierPerSubChannel * self.subCarrierBandwidth

        assert self.numSubchannels is not None, "You need to set numSubchannels in your subclass"

        self.bandwidth = self.subChannelBandwidth * self.numSubchannels

        self.subCarriersPerSubChannel = 12

        self.mac = MACLTEFDD(self.numSubchannels * self.subCarriersPerSubChannel, self.subCarriersPerSubChannel)

        self.phy = PHYLTEFDD(self.numSubchannels * self.subCarriersPerSubChannel)

        #self.mac.lowestPhyMode                = winprost.LLMapping.ScaleNetLowestPhyMode # from LLMapping.py
        #self.mac.lowestPhyMode.symbolDuration = self.mac.fullSymbolDur
        #self.mac.lowestPhyMode.subCarriersPerSubChannel = self.phy.phyResourceSize

        # Power of 4.0 dBm for nominalPerSubband is calculated for 20 MHz case
        # Power UT/BS according to ITU-R M.2135
        self.phy.txPwrUT = openwns.Scheduler.PowerCapabilities( maxPerSubband     = "39.0 dBm",
                                                                nominalPerSubband = "29.0 dBm",
                                                                maxOverall        = "49.0 dBm")
        self.phy.txPwrBS = openwns.Scheduler.PowerCapabilities( maxPerSubband     = "39.0 dBm",
                                                                nominalPerSubband = "29.0 dBm",
                                                                maxOverall        = "49.0 dBm")
