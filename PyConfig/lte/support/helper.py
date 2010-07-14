import lte.phy.plm

from openwns import dBm, dB, fromdB, fromdBm

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
    
