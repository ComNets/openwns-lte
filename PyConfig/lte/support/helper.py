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

def setupPhy(simulator, plmName, scenario):
    import rise.scenario.Pathloss
    from openwns.interval import Interval

    if scenario == "InH":
        setupPhyDetail(simulator, 3400, rise.scenario.Pathloss.ITUInH(), dBm(24), dBm(24), plmName, dB(5), dB(7))
    elif scenario == "UMa":
        setupPhyDetail(simulator, 2000, rise.scenario.Pathloss.ITUUMa(), dBm(46), dBm(24), plmName, dB(5), dB(7))
    elif scenario == "UMi":
        setupPhyDetail(simulator, 2500, rise.scenario.Pathloss.ITUUMi(), dBm(41), dBm(24), plmName, dB(5), dB(7))
    elif scenario == "RMa":
        setupPhyDetail(simulator, 800, rise.scenario.Pathloss.ITURMa(), dBm(46), dBm(24), plmName, dB(5), dB(7))
    elif scenario == "SMa":
        setupPhyDetail(simulator, 2000, rise.scenario.Pathloss.ITUSMa(), dBm(46), dBm(24), plmName, dB(5), dB(7))
    else:
        raise "Unknown scenario %s" % scenario

def setupPhyDetail(simulator, freq, pathloss, bsTxPower, utTxPower, plmName, rxNoiseBS, rxNoiseUT):

    from ofdmaphy.OFDMAPhy import OFDMASystem
    import rise.Scenario
    from rise.scenario import Shadowing
    from rise.scenario import FastFading
    import openwns.Scheduler
    import math
    import scenarios.channelmodel

    plm = lte.phy.plm.getByName(plmName)

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")

    ofdmaPhySystem = OFDMASystem('ofdma')
    ofdmaPhySystem.Scenario = rise.Scenario.Scenario()
    simulator.modules.ofdmaPhy.systems.append(ofdmaPhySystem)

    # Large Scale fading model
    for node in bsNodes + utNodes:
        # TX frequency
        node.phy.ofdmaStation.txFrequency = freq
        node.phy.ofdmaStation.rxFrequency = freq

    # Noise figure 
    for bs in bsNodes:
        bs.phy.ofdmaStation.receiver[0].receiverNoiseFigure = rxNoiseBS

    power = fromdBm(bsTxPower)
    bsNominalTxPower = dBm(power - 10 * math.log10(plm.numSubchannels))
    
    bsPower = openwns.Scheduler.PowerCapabilities(bsTxPower, bsNominalTxPower, bsNominalTxPower)
    
    # Per subchannel nominal power equals the max power since we assume UTs only use few subchannels
    utPower = openwns.Scheduler.PowerCapabilities(utTxPower, utTxPower, utTxPower)
    
    #for bs in bsNodes:
    #    bs.dll.dlscheduler.config.txScheduler.registry.powerCapabilitiesAP = bsPower
    #    bs.dll.dlscheduler.config.txScheduler.registry.powerCapabilitiesUT = utPower
    #    bs.dll.ulscheduler.config.rxScheduler.registry.powerCapabilitiesAP = bsPower
    #    bs.dll.ulscheduler.config.rxScheduler.registry.powerCapabilitiesUT = utPower
        
        # For broadcasts
    #    bs.phy.ofdmaStation.txPower = bsTxPower
    
    #for ut in utNodes:
    #    ut.dll.ulscheduler.config.txScheduler.registry.powerCapabilitiesAP = bsPower
    #    ut.dll.ulscheduler.config.txScheduler.registry.powerCapabilitiesUT = utPower
    #    ut.phy.ofdmaStation.txPower = bsTxPower

def setupUL_APC(simulator, modes, alpha, pNull):
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
                    assert isinstance(
                        fu.strategy.apcstrategy, 
                        openwns.scheduler.APCStrategy.LTE_UL), "Uplink APC strategy is not of type LTE_UL"
                    fu.strategy.apcstrategy.alpha = alpha
                    fu.strategy.apcstrategy.pNull = pNull
                    found = True    
        assert found, "Could not find uplink master scheduler in BS"

def setupScheduler(simulator, sched, modes):
    setupDLScheduler(simulator, sched, modes)
    setupULScheduler(simulator, sched, modes)

def setupULScheduler(simulator, sched, modes):
    setupSchedulerDetail(simulator, sched, "UL", modes)

def setupDLScheduler(simulator, sched):
    setupSchedulerDetail(simulator, sched, "DL", modes)

def setupSchedulerDetail(simulator, sched, direction, modes):
    if direction == "UL":
        suffix = "RX"
    elif direction == "DL":
        suffix == "TX"
    else:
        assert false, "Unknown direction"

    import lte.modes
    import openwns.scheduler.DSAStrategy
    import openwns.Scheduler

    if sched == "Fixed":
        Strat = openwns.Scheduler.DSADrivenRR
        DSA = openwns.scheduler.DSAStrategy.Fixed
    elif sched == "Random":
        Strat = openwns.Scheduler.RoundRobin
        DSA = openwns.scheduler.DSAStrategy.Random
    elif sched == "ExhaustiveRR":
        Strat = openwns.Scheduler.ExhaustiveRoundRobin
        DSA = openwns.scheduler.DSAStrategy.LinearFFirst

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")

    for n in bsNodes:
        found = False
        fun = n.dll.fun.functionalUnit
        for mt in modes:
            modeCreator = lte.modes.getModeCreator(mt)
            aMode = modeCreator(default = False)
            for fu in fun:
                if aMode.modeBase + aMode.separator + "resourceScheduler" + suffix in fu.commandName:
                    fu.strategy.dsastrategy = DSA(oneUserOnOneSubChannel = True)
                    fu.strategy.dsafbstrategy = DSA(oneUserOnOneSubChannel = True)
                    fu.strategy.subStrategies[4] = Strat(useHARQ = True)
                    fu.strategy.subStrategies[5] = Strat(useHARQ = True)
                    fu.strategy.subStrategies[6] = Strat(useHARQ = True)
                    fu.strategy.subStrategies[7] = Strat(useHARQ = True)
                    found = True    
        assert found, "Could not find scheduler in BS"

    
    
def setupFTFading(simulator, scenario, modes):

    if scenario == "InH":
        setupFTFadingDetails(simulator, modes, 3.0, 3.4E9)
    elif scenario == "UMa":
        setupFTFadingDetails(simulator, modes, 30.0, 2.0E9)
    elif scenario == "UMi":
        setupFTFadingDetails(simulator, modes, 3.0, 2.5E9)
    elif scenario == "RMa":
        setupFTFadingDetails(simulator, modes, 120.0, 0.8E9)
    elif scenario == "SMa":
        setupFTFadingDetails(simulator, modes, 90.0, 2.0E9)
    else:
        raise "Unknown scenario %s" % scenario      


def setupFTFadingDetails(simulator, modes, speed, freq):
    for mode in modes:
        modeCreator = lte.modes.getModeCreator(mode)
        aMode = modeCreator(default = False)

        dlFrequency = freq
        if aMode.modeName.find("tdd") >= 0:
            ulFrequency = dlFrequency
        else:
            ulFrequency = dlFrequency - 0.1E9
      
        speed = (speed * 1000.0) / 3600.0
        ulDoppler = speed / 3E8 * (ulFrequency)
        dlDoppler = speed / 3E8 * (dlFrequency)
        
        import rise.scenario.FTFading
        
        bsNodes = simulator.simulationModel.getNodesByProperty("Type", "eNB")
        utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")

        assert len(bsNodes) > 0, "No BS in scenario. First setup your scenario!"
        
        for node in bsNodes:
            node.phys[aMode.modeName].ofdmaStation.receiver[0].FTFadingStrategy = rise.scenario.FTFading.FTFadingFneighbourCorrelation(
                    samplingTime = 0.001,
                    neighbourCorrelationFactor = 0.8,
                    dopFreq = ulDoppler,
                    numWaves = 100,
                    numSubCarriers =
                    node.phys[mode].ofdmaStation.numberOfSubCarrier)
            node.phys[aMode.modeName].ofdmaStation.receiver[0].doMeasurementUpdates = True
            node.phys[aMode.modeName].ofdmaStation.receiver[0].measurementUpdateInterval = 0.001

        for node in utNodes:
            node.phys[aMode.modeName].ofdmaStation.receiver[0].FTFadingStrategy = rise.scenario.FTFading.FTFadingFneighbourCorrelation(
                    samplingTime = 0.001,
                    neighbourCorrelationFactor = 0.8,
                    dopFreq = dlDoppler,
                    numWaves = 100,
                    numSubCarriers = node.phys[mode].ofdmaStation.numberOfSubCarrier)
            node.phys[aMode.modeName].ofdmaStation.receiver[0].doMeasurementUpdates = True
            node.phys[aMode.modeName].ofdmaStation.receiver[0].measurementUpdateInterval = 0.001

