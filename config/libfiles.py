# -*- coding: utf-8 -*-
libname = 'lte'

srcFiles = [
    'src/lteModule.cpp',
    'src/phy/PhyUser.cpp',
    ]

hppFiles = [
    'src/lteModule.hpp',
    'src/helper/HasModeName.hpp',
    'src/phy/PhyCommand.hpp',
    'src/phy/PhyUser.hpp',
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
