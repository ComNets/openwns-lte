# -*- coding: utf-8 -*-
libname = 'lte'

srcFiles = [
    'src/lteModule.cpp',
    'src/phy/PhyUser.cpp',

    # concrete classes

    'src/lteDummy.cpp',

    'src/timing/TimingScheduler.cpp',
    'src/timing/events/Base.cpp',
    'src/timing/events/rap/Events.cpp',
    'src/timing/events/ut/Events.cpp',

    'src/macr/PhyUser.cpp',
    'src/controlplane/TaskDispatcher.cpp',

    'src/main/Layer2.cpp',
    'src/helper/idprovider/Distance.cpp',
    'src/helper/idprovider/QoSClass.cpp',
    'src/helper/idprovider/PeerId.cpp',

    ]

hppFiles = [
    'src/lteModule.hpp',
    'src/helper/HasModeName.hpp',
    'src/phy/PhyCommand.hpp',
    'src/phy/PhyUser.hpp',

    # concrete classes

    'src/lteDummy.hpp',

    'src/timing/TimingScheduler.hpp',
    'src/timing/events/Base.hpp',
    'src/timing/events/rap/Events.hpp',
    'src/timing/events/ut/Events.hpp',

    'src/macr/PhyUser.hpp',
    'src/macr/PhyCommand.hpp',
    'src/helper/MIProviderInterface.hpp',
    'src/controlplane/TaskDispatcher.hpp',

    'src/main/Layer2.hpp',
    'src/helper/idprovider/HopCount.hpp',
    'src/helper/idprovider/Distance.hpp',
    'src/helper/idprovider/QoSClass.hpp',
    'src/helper/QoSClasses.hpp',
    'src/helper/idprovider/PeerId.hpp',
    'src/helper/Route.hpp',
    'src/helper/TransactionID.hpp',
    'src/macr/Mode.hpp',

    # abstract classes

    'src/controlplane/MapHandlerInterface.hpp',
    'src/controlplane/bch/BCHUnitInterface.hpp',
    'src/macr/RACHInterface.hpp',
    'src/timing/ResourceSchedulerInterface.hpp',

    'src/macr/CQIMeasurementInterface.hpp',
    'src/controlplane/AssociationsProxyInterface.hpp',
    'src/controlplane/flowmanagement/FlowManagerInterface.hpp',
    'src/macg/MACgInterface.hpp',
    'src/rlc/RLCInterface.hpp',

    ]

pyconfigs = [

    'lte/__init__.py',
    'lte/creators/__init__.py',
    'lte/creators/fdd/__init__.py',
    'lte/creators/fdd/BS.py',
    'lte/creators/fdd/UE.py',
    'lte/nodes/__init__.py',
    'lte/nodes/BS.py',
    'lte/nodes/UE.py',
    'lte/phy/__init__.py',
    'lte/phy/plm/__init__.py',
    'lte/phy/plm/fdd/__init__.py',
    'lte/phy/plm/fdd/lteFDD10.py',
    'lte/phy/ofdma.py',
    'lte/support/__init__.py',
    'lte/support/helper.py',

]

dependencies = []
# Put in any external lib here as you would pass it to a -l compiler flag, e.g.
# dependencies = ['boost_date_time']
Return('libname srcFiles hppFiles pyconfigs dependencies')
