# -*- coding: utf-8 -*-
libname = 'lte'

srcFiles = [
    'src/lteModule.cpp',

    'src/main/Layer2.cpp',

    'src/upperconvergence/eNB.cpp',
    'src/upperconvergence/UE.cpp',

    'src/rlc/eNB.cpp',
    'src/rlc/UE.cpp',

    'src/macg/MACg.cpp',
    'src/macg/MACgUT.cpp',
    'src/macg/MACgBS.cpp',
    'src/macg/modeselection/Best.cpp',

    'src/macr/PhyUser.cpp',

    'src/controlplane/AssociationsProxy.cpp',
    'src/controlplane/associationHandler/AssociationHandler.cpp',
    'src/controlplane/associationHandler/AssociationHandlerBS.cpp',
    'src/controlplane/associationHandler/AssociationHandlerUT.cpp',
    'src/controlplane/flowmanagement/FlowManager.cpp',
    'src/controlplane/flowmanagement/flowhandler/FlowHandler.cpp',
    'src/controlplane/flowmanagement/flowhandler/FlowHandlerBS.cpp',
    'src/controlplane/flowmanagement/flowhandler/FlowHandlerUT.cpp',

    'src/timing/TimingScheduler.cpp',
    'src/timing/events/Base.cpp',
    'src/timing/events/rap/Events.cpp',
    'src/timing/events/ut/Events.cpp',

    'src/helper/idprovider/Distance.cpp',
    'src/helper/idprovider/QoSClass.cpp',
    'src/helper/idprovider/PeerId.cpp',
    'src/helper/Keys.cpp',
    'src/helper/SwitchConnector.cpp',
]

hppFiles = [
    'src/lteModule.hpp',

    'src/main/Layer2.hpp',
    'src/main/IRANG.hpp',

    'src/upperconvergence/eNB.hpp',
    'src/upperconvergence/UE.hpp',

    'src/rlc/RLCCommand.hpp',
    'src/rlc/eNB.hpp',
    'src/rlc/UE.hpp',

    'src/macg/MACgCommand.hpp',
    'src/macg/MACg.hpp',
    'src/macg/MACgUT.hpp',
    'src/macg/MACgBS.hpp',
    'src/macg/modeselection/Strategy.hpp',
    'src/macg/modeselection/Best.hpp',

    'src/macr/PhyUser.hpp',
    'src/macr/PhyCommand.hpp',
    'src/macr/Mode.hpp',
    'src/macr/RACHInterface.hpp',
    'src/macr/CQIMeasurementInterface.hpp',
    'src/macr/ScorerInterface.hpp',

    'src/controlplane/MapHandlerInterface.hpp',
    'src/controlplane/bch/BCHUnitInterface.hpp',

    'src/controlplane/AssociationsProxyInterface.hpp',
    'src/controlplane/AssociationsProxy.hpp',
    'src/controlplane/associationHandler/IAssociationHandler.hpp',
    'src/controlplane/associationHandler/AssociationHandler.hpp',
    'src/controlplane/associationHandler/AssociationHandlerBS.hpp',
    'src/controlplane/associationHandler/AssociationHandlerUT.hpp',
    'src/controlplane/flowmanagement/IFlowManager.hpp',
    'src/controlplane/flowmanagement/FlowManager.hpp',
    'src/controlplane/flowmanagement/flowhandler/FlowHandler.hpp',
    'src/controlplane/flowmanagement/flowhandler/FlowHandlerBS.hpp',
    'src/controlplane/flowmanagement/flowhandler/FlowHandlerUT.hpp',

    'src/timing/TimingScheduler.hpp',
    'src/timing/ResourceSchedulerInterface.hpp',
    'src/timing/events/Base.hpp',
    'src/timing/events/rap/Events.hpp',
    'src/timing/events/ut/Events.hpp',

    'src/helper/HasModeName.hpp',
    'src/helper/MIProviderInterface.hpp',
    'src/helper/idprovider/HopCount.hpp',
    'src/helper/idprovider/Distance.hpp',
    'src/helper/idprovider/QoSClass.hpp',
    'src/helper/Keys.hpp',
    'src/helper/QoSClasses.hpp',
    'src/helper/idprovider/PeerId.hpp',
    'src/helper/Route.hpp',
    'src/helper/TransactionID.hpp',
    'src/helper/SwitchConnector.hpp',
    'src/helper/SwitchLink.hpp',
]

pyconfigs = [

    'lte/__init__.py',
    'lte/creators/__init__.py',
    'lte/creators/fdd/__init__.py',
    'lte/creators/fdd/BS.py',
    'lte/creators/fdd/UE.py',
    'lte/modes/__init__.py',
    'lte/modes/hasModeName.py',
    'lte/modes/mode.py',
    'lte/modes/taskfun/__init__.py',
    'lte/modes/taskfun/default/__init__.py',
    'lte/modes/taskfun/default/BS.py',
    'lte/modes/taskfun/default/UT.py',
    'lte/nodes/__init__.py',
    'lte/nodes/BS.py',
    'lte/nodes/UE.py',
    'lte/phy/__init__.py',
    'lte/partitioning/__init__.py',
    'lte/partitioning/fdd.py',
    'lte/partitioning/service.py',
    'lte/phy/plm/__init__.py',
    'lte/phy/plm/fdd/__init__.py',
    'lte/phy/plm/fdd/default.py',
    'lte/phy/plm/fdd/lteFDD10.py',
    'lte/phy/ofdma.py',
    'lte/dll/__init__.py',
    'lte/dll/component.py',
    'lte/dll/upperConvergence.py',
    'lte/dll/rlc.py',
    'lte/dll/macg.py',
    'lte/dll/phyUser.py',
    'lte/dll/controlplane/__init__.py',
    'lte/dll/controlplane/association.py',
    'lte/dll/controlplane/flowmanager.py',
    'lte/support/__init__.py',
    'lte/support/helper.py',

]

dependencies = []
# Put in any external lib here as you would pass it to a -l compiler flag, e.g.
# dependencies = ['boost_date_time']
Return('libname srcFiles hppFiles pyconfigs dependencies')
