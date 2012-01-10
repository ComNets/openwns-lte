# -*- coding: iso-8859-1 -*-
import lte.phy.plm

from openwns import dBm, dB, fromdB, fromdBm

def connectFUs(pairList):
    """
    pairList= [(A,B), (B,C)]
    
    results in a FUN like this
    A - B - C
    """
    for source, destination in pairList:
        source.connect(destination)

def setupPhy(simulator, modes, scenario):
    import rise.scenario.Pathloss
    from openwns.interval import Interval

    if scenario == "InH":
        setupPhyDetail(simulator, 3400, rise.scenario.Pathloss.ITUInH(), dBm(1), dBm(1), modes, dB(5), dB(7))
    elif scenario == "UMa":
        setupPhyDetail(simulator, 2000, rise.scenario.Pathloss.ITUUMa(), dBm(29), dBm(3), modes, dB(5), dB(7))
    elif scenario == "UMi":
        setupPhyDetail(simulator, 2500, rise.scenario.Pathloss.ITUUMi(), dBm(24), dBm(3), modes, dB(5), dB(7))
    elif scenario == "RMa":
        setupPhyDetail(simulator, 800, rise.scenario.Pathloss.ITURMa(), dBm(29), dBm(3), modes, dB(5), dB(7))
    elif scenario == "SMa":
        setupPhyDetail(simulator, 2000, rise.scenario.Pathloss.ITUSMa(), dBm(29), dBm(3), modes, dB(5), dB(7))
    else:
        raise "Unknown scenario %s" % scenario

def setupPhyDetail(simulator, freq, pathloss, bsTxPower, utTxPower, modes, rxNoiseBS, rxNoiseUT):

    from ofdmaphy.OFDMAPhy import OFDMASystem
    import rise.Scenario
    from rise.scenario import Shadowing
    from rise.scenario import FastFading
    import openwns.Scheduler
    import math
    import scenarios.channelmodel

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")

    # Large Scale fading model
    for node in bsNodes:
        # TX frequency
        for phy in node.phys.values():
            phy.ofdmaStation.txFrequency = freq
            phy.ofdmaStation.rxFrequency = freq + 120

    for node in utNodes:
        # TX frequency
        for phy in node.phys.values():
            phy.ofdmaStation.txFrequency = freq + 120
            phy.ofdmaStation.rxFrequency = freq


    # Noise figure 
    for bs in bsNodes:
        for phy in bs.phys.values():
            phy.ofdmaStation.receiver[0].receiverNoiseFigure = rxNoiseBS

    power = fromdBm(bsTxPower)
    bsMaxTxPower = dBm(power + 10 * math.log10(getMaxNumberOfSubchannels(modes)))
    
    bsPower = openwns.Scheduler.PowerCapabilities(bsMaxTxPower, bsTxPower, bsMaxTxPower)
    
    power = fromdBm(utTxPower)
    utMaxTxPower = dBm(power + 10 * math.log10(getMaxNumberOfSubchannels(modes)))
    utTxPower = dBm(power + 10 * math.log10(getMaxNumberOfSubchannels(modes) / 5))
    utPower = openwns.Scheduler.PowerCapabilities(utMaxTxPower, utTxPower, utMaxTxPower)

    for bs in bsNodes:
        fu = getMasterSchedulerFU(simulator, bs, "DL", modes)
        fu.registry.powerCapabilitiesBS = bsPower        
        fu.registry.powerCapabilitiesUT = utPower
        fu = getMasterSchedulerFU(simulator, bs, "UL", modes)
        fu.registry.powerCapabilitiesBS = bsPower
        fu.registry.powerCapabilitiesUT = utPower        

        # For broadcasts
        for phy in bs.phys.values():
            phy.ofdmaStation.txPower = bsTxPower
    
    for ut in utNodes:
        fu = getSlaveSchedulerFU(simulator, ut, modes)
        fu.registry.powerCapabilitiesBS = bsPower
        fu.powerCapabilitiesUT = utPower
        for phy in ut.phys.values():
            phy.ofdmaStation.txPower = utTxPower

    setupUL_APC(simulator, modes, 0.0, utTxPower, True)

def getMaxNumberOfSubchannels(modes):
    import lte.modes

    nMax = 0
    
    for mt in modes:
        modeCreator = lte.modes.getModeCreator(mt)
        aMode = modeCreator(default = False)
        n = aMode.plm.numSubchannels
        if n > nMax:
            nMax = n

    return nMax


def associateByGeometry(simulator):
  eNBNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
  for bs in eNBNodes:
    for ut in simulator.simulationModel.getNodesByProperty("BS", bs):
        ut.dll.associateTo(bs.dll.address)



def setupUL_APC(simulator, modes, alpha, pNull, onlyIfAlphaNull = False):
    import lte.modes
    import openwns.scheduler.APCStrategy

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")

    for n in bsNodes:
        found = False
        fun = n.dll.fun.functionalUnit
        for mt in modes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(default = False)
            for fu in fun:
                if aMode.modeBase + aMode.separator + "resourceSchedulerRX" in fu.commandName:
                    if(not isinstance(fu.strategy.apcstrategy, openwns.scheduler.APCStrategy.LTE_UL)):
                        print "Uplink APC strategy is not of type LTE_UL"
                        return
                    if(onlyIfAlphaNull == False or fu.strategy.apcstrategy.alpha == 0.0):
                        fu.strategy.apcstrategy.alpha = alpha
                        fu.strategy.apcstrategy.pNull = pNull
                        for sstrat in fu.strategy.subStrategies:
                            if isinstance(sstrat, openwns.Scheduler.PersistentVoIP):
                                sstrat.resourceGrid.linkAdaptation.alpha = alpha
                                sstrat.resourceGrid.linkAdaptation.pNull = pNull
                    found = True    
        assert found, "Could not find uplink master scheduler in BS"

def getSlaveSchedulerFU(simulator, ue, modes):
    return getMasterSchedulerFU(simulator, ue, "DL", modes)

def getMasterSchedulerFU(simulator, bs, direction, modes):
    if direction == "UL":
        suffix = "RX"
    elif direction == "DL":
        suffix = "TX"
    else:
        assert false, "Unknown direction"

    fun = bs.dll.fun.functionalUnit
    for mt in modes:
        modeCreator = lte.modes.getModeCreator(mt)
        aMode = modeCreator(default = False)
        for fu in fun:
            if aMode.modeBase + aMode.separator + "resourceScheduler" + suffix in fu.commandName:    
                return fu

    assert false, "Could not find scheduler in BS"
    return None        

def setHARQRetransmissionLimit(simulator, modes, limit):
    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")

    for bs in bsNodes:
        fu = getMasterSchedulerFU(simulator, bs, "DL", modes)
        fu.harq.retransmissionLimit = limit
        fu.harq.harqEntity.retransmissionLimit = limit

    ueNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")

    for ue in ueNodes:
        fu = getSlaveSchedulerFU(simulator, ue, modes)
        fu.harq.retransmissionLimit = limit
        fu.harq.harqEntity.retransmissionLimit = limit

def setQueueSize(simulator, modes, size):

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")

    for bs in bsNodes:
        fu = getMasterSchedulerFU(simulator, bs, "DL", modes)
        fu.registry.queueSize = size

    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")

    for ut in utNodes:
        fu = getSlaveSchedulerFU(simulator, ut, modes)
        fu.registry.queueSize = size


def setupScheduler(simulator, sched, modes):
    setupDLScheduler(simulator, sched, modes)
    setupULScheduler(simulator, sched, modes)

def setupULScheduler(simulator, sched, modes):
    setupSchedulerDetail(simulator, sched, "UL", modes)
 

def setupDLScheduler(simulator, sched, modes):
    setupSchedulerDetail(simulator, sched, "DL", modes)

def setupSchedulerDetail(simulator, sched, direction, modes):


    import lte.modes
    import openwns.scheduler.DSAStrategy
    import openwns.Scheduler

    if sched == "Fixed":
        Strat = openwns.Scheduler.DSADrivenRR
        DSA = openwns.scheduler.DSAStrategy.Fixed
    elif sched == "Random":
        Strat = openwns.Scheduler.RoundRobin
        DSA = openwns.scheduler.DSAStrategy.Random
    elif sched == "PersistentVoIP":
        Strat = openwns.Scheduler.PersistentVoIP
        DSA = openwns.scheduler.DSAStrategy.BestEffSINR
    elif sched == "ExhaustiveRR":
        Strat = openwns.Scheduler.ExhaustiveRoundRobin
        DSA = openwns.scheduler.DSAStrategy.LinearFFirst

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")

    for bs in bsNodes:
        fu = getMasterSchedulerFU(simulator, bs, direction, modes)
    
        fu.strategy.dsastrategy = DSA(oneUserOnOneSubChannel = True)
        fu.strategy.dsafbstrategy = DSA(oneUserOnOneSubChannel = True)
        for i in xrange(4, 8):
            strat = Strat(useHARQ = True)
            strat.setParentLogger(fu.strategy.logger)
            fu.strategy.subStrategies[i] = strat

        # HARQ is involved fro within PersistentVoIP
        if sched == "PersistentVoIP":
            fu.strategy.subStrategies[1] = openwns.Scheduler.Disabled()
            if direction == "DL":
                HARQStrat = openwns.Scheduler.HARQRetransmission
            else:
                HARQStrat = openwns.Scheduler.HARQUplinkRetransmission

            strat = HARQStrat()
            strat.setParentLogger(fu.strategy.logger)
            for i in xrange(4, 8):
                fu.strategy.subStrategies[i].harq = strat

def setupPersistentVoIPScheduler(simulator, la, tbc, reduceMCS, modes, fallback = None, returnRandom = False):
    import openwns.Scheduler
    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
    for direction in ["UL", "DL"]:
        for bs in bsNodes:
            fu = getMasterSchedulerFU(simulator, bs, direction, modes)
    
            for i in xrange(4, 8):
                if isinstance(fu.strategy.subStrategies[i], openwns.Scheduler.PersistentVoIP):
                    if(tbc == "Previous"):
                        assert fallback != None, "Need fallback strategy"
                        fu.strategy.subStrategies[i].resourceGrid.tbChoser = \
                            openwns.Scheduler.PersistentVoIP.ResourceGrid.PreviousTBC(fallback, returnRandom)
                    else:
                        fu.strategy.subStrategies[i].resourceGrid.tbChoser.__plugin__ = tbc
                        fu.strategy.subStrategies[i].resourceGrid.tbChoser.returnRandom = returnRandom

                    fu.strategy.subStrategies[i].resourceGrid.linkAdaptation.__plugin__ = la    
                    fu.strategy.subStrategies[i].resourceGrid.linkAdaptation.reduceMCS = reduceMCS    

def getInititalICacheValues(simulator):
    import dll.Services
    import openwns.scheduler.metascheduler
    
    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
    icache = None
    for s in bsNodes[0].dll.managementServices:
        if isinstance(s, dll.Services.InterferenceCache):
            icache = s

    assert icache != None, "Could not find ICache"
    assert isinstance(icache.notFoundStrategy, dll.Services.ConstantValue), \
      "NotFoundStrategy must be ConstantValue"

    initVal = openwns.scheduler.metascheduler.InitVals()
    initVal.pl = icache.notFoundStrategy.averagePathloss
    initVal.c = icache.notFoundStrategy.averageCarrier
    initVal.i = icache.notFoundStrategy.averageInterference

    return initVal


# TODO: Separate MetaScheduler for up- and downlink. ATM only one metascheduler and thereby only one strategy may be applied
def setupMetaScheduler(simulator, direction, modes, metaSched="NoMetaScheduler"):

    import lte.modes
    import openwns.Scheduler
   

    if metaSched == "NoMetaScheduler":
        return
        
    elif metaSched == "GreedyMetaScheduler":
        Strat = openwns.Scheduler.DSADrivenRR
        DSA = openwns.scheduler.DSAStrategy.DSAMeta
        Meta = openwns.scheduler.metascheduler.GreedyMetaScheduler
    
    elif metaSched == "MaxRegretMetaScheduler":
        Strat = openwns.Scheduler.DSADrivenRR
        DSA = openwns.scheduler.DSAStrategy.DSAMeta
        Meta = openwns.scheduler.metascheduler.MaxRegretMetaScheduler
        
    elif metaSched == "HighCwithHighIMetaScheduler":
        Strat = openwns.Scheduler.DSADrivenRR
        DSA = openwns.scheduler.DSAStrategy.DSAMeta
        Meta = openwns.scheduler.metascheduler.HighCwithHighIMetaScheduler
        
    disablePhyUnicastTransmissionDetail(simulator, modes, direction)  
    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
    for bs in bsNodes:
        fu = getMasterSchedulerFU(simulator, bs, direction, modes)
    
        fu.strategy.dsastrategy = DSA(oneUserOnOneSubChannel = True)
        fu.strategy.dsafbstrategy = DSA(oneUserOnOneSubChannel = True)
        fu.metaScheduler = Meta(getInititalICacheValues(simulator))
        
        for i in xrange(4, 8):
            strat = Strat(useHARQ = True)
            strat.setParentLogger(fu.strategy.logger)
            fu.strategy.subStrategies[i] = strat
  
  
def setupFastFading(scenario, modes, rxAntennas):
    setupULFastFading(scenario, modes, rxAntennas)
    setupDLFastFading(scenario, modes, rxAntennas)

def setupULFastFading(scenario, modes, rxAntennas):
    setupFastFadingDetails(scenario, modes, rxAntennas, "UL")

def setupDLFastFading(scenario, modes, rxAntennas):
    setupFastFadingDetails(scenario, modes, rxAntennas, "DL")
    
def setupFastFadingDetails(scenario, modes, rxAntennas, direction):

    
    if scenario == "InH":
        speed = 3.0
        freq = 3.4E9
    elif scenario == "UMa":
        speed = 30.0
        freq = 2.0E9
    elif scenario == "UMi":
        speed = 3.0
        freq = 2.5E9
    elif scenario == "RMa":
        speed = 120.0
        freq = 0.8E9
    elif scenario == "SMa":
        speed = 90.0
        freq = 2.0E9
    else:
        raise "Unknown scenario %s" % scenario      

    try:
        import imtaphy
        setupIMTAPhyFastFadingDetails(modes, scenario, speed, freq, rxAntennas, direction)
    except ImportError:
        print "IMTAPhy modul not available, using Jakes FastFading"
        setupJakesFastFadingDetails(modes, speed, freq, direction)
    
def setupIMTAPhyFastFadingDetails(modes, scenario, speed, freq, rxAntennas, direction):
    import rise.scenario.FastFading
    import rise.scenario.Propagation
    import lte.modes
    from imtaphy.SCM import SISORiseWrapper, SIMORiseWrapper, NoRiseWrapper

    modeCreator = lte.modes.getModeCreator(modes[0])
    aMode = modeCreator(default = False)

    bsType = aMode.plm.phy.eNBTransceiverType
    utType = aMode.plm.phy.utTransceiverType

    nsc = lte.support.helper.getMaxNumberOfSubchannels(modes)
    prop = rise.scenario.Propagation.PropagationSingleton.getInstance()

    if rxAntennas == 1:
        RiseWrapper = SISORiseWrapper
    elif rxAntennas == 2:
        RiseWrapper = SIMORiseWrapper
    else:
        assert false, "Only 1 or 2 antennas supported"

    if direction == "UL":
        prop.setPair(utType, bsType).fastFading = RiseWrapper(scenario, speed, freq, nsc, direction)
    if direction == "DL":
        prop.setPair(bsType, utType).fastFading = RiseWrapper(scenario, speed, freq, nsc, direction)

def setupJakesFastFadingDetails(modes, speed, freq, direction):
    import rise.scenario.FastFading
    import rise.scenario.Propagation
    import lte.modes

    numSC = getMaxNumberOfSubchannels(modes)

    modeCreator = lte.modes.getModeCreator(modes[0])
    aMode = modeCreator(default = False)

    dlFrequency = freq
    if aMode.modeName.find("tdd") >= 0:
        ulFrequency = dlFrequency
    else:
        ulFrequency = dlFrequency - 0.1E9

    if direction == "DL":
        frequency = dlFrequency
    if direction == "UL":
        frequency = ulFrequency
  
    speed = (speed * 1000.0) / 3600.0
    doppler = speed / 3E8 * (frequency)
            
    ftf = rise.scenario.FastFading.FTFadingFneighbourCorrelation(
                    samplingTime = 0.001,
                    neighbourCorrelationFactor = 0.8,
                    dopFreq = doppler,
                    numWaves = numSC,
                    numSubCarriers = numSC)

    bsType = aMode.plm.phy.eNBTransceiverType
    utType = aMode.plm.phy.utTransceiverType

    prop = rise.scenario.Propagation.PropagationSingleton.getInstance()
    if direction == "UL":
        prop.setPair(utType, bsType).fastFading = rise.scenario.FastFading.Jakes(ftf)
    if direction == "DL":
        prop.setPair(bsType, utType).fastFading = rise.scenario.FastFading.Jakes(ftf)

def disablePhyUnicastTransmission(simulator, modes):
    disablePhyUnicastULTransmission(simulator, modes)
    disablePhyUnicastDLTransmission(simulator, modes)

def disablePhyUnicastULTransmission(simulator, modes):
    disablePhyUnicastTransmissionDetail(simulator, modes, "UL")

def disablePhyUnicastDLTransmission(simulator, modes):
    disablePhyUnicastTransmissionDetail(simulator, modes, "DL")

def disablePhyUnicastTransmissionDetail(simulator, modes, direction):
    if direction == "DL":
        nodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
    elif direction == "UL":
        nodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
    else:
        assert false, "Unknown direction"

    for node in nodes:
        fun = node.dll.fun.functionalUnit
        for mt in modes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(default = False)
            for fu in fun:
                if aMode.modeBase + aMode.separator + "phyUser" in fu.commandName:    
                    fu.sendAllBroadcast = True

try:
    import applications.clientSessions
    import applications.serverSessions
    import applications.codec
    import applications.component

    ### Application VoIP
    def createDLVoIPTraffic(simulator, codecType = applications.codec.AMR_12_2(), 
            comfortNoiseChoice = True, settlingTime = 0.0, trafficStartDelay = 0.0,
            probeEndTime = 1E12):
        rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
        rang = rangs[0]

        voipDL = applications.serverSessions.VoIP(codecType = codecType,
                                                comfortNoiseChoice = comfortNoiseChoice, 
                                                settlingTime = settlingTime, 
                                                trafficStartDelay = trafficStartDelay,
                                                probeEndTime = probeEndTime,
                                                parentLogger = rang.logger)

        tlListenerBinding = applications.component.TLListenerBinding(rang.nl.domainName, "127.0.0.1", 1028,
                                                                    lte.dll.qos.conversationalQosClass, 1028, voipDL,
                                                                    parentLogger = rang.logger)
        rang.load.addListenerBinding(tlListenerBinding)
    
    def createULVoIPTraffic(simulator, codecType = applications.codec.AMR_12_2(), comfortNoiseChoice = True,
                            settlingTime = 0.0, minStartDelay = 0.1, maxStartDelay = 1.0, probeEndTime = 1E12):
        rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
        rang = rangs[0]
        utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
                
        for ut in utNodes:
            voipUL = applications.clientSessions.VoIP(codecType = codecType,
                                                    comfortNoiseChoice = comfortNoiseChoice, settlingTime = settlingTime,
                                                    minStartDelay = minStartDelay, maxStartDelay = maxStartDelay,
                                                    probeEndTime = probeEndTime,
                                                    parentLogger = ut.logger)
    
            tlBinding = applications.component.TLBinding(ut.nl.domainName, rang.nl.domainName,
                                                        1028, lte.dll.qos.conversationalQosClass,
                                                        1028, parentLogger = ut.logger)
            ut.load.addTraffic(tlBinding, voipUL)

except ImportError:
    pass


