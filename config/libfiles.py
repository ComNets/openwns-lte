libname = 'lte'

srcFiles = [
    'src/lteModule.cpp',
    'src/SimulationModel.cpp',
    ]

hppFiles = [
    'src/lteModule.hpp',
    'src/SimulationModel.hpp',
    ]

pyconfigs = [
    'lte/__init__.py',
    'lte/simulationmodel.py',
]

dependencies = []
# Put in any external lib here as you would pass it to a -l compiler flag, e.g.
# dependencies = ['boost_date_time']
Return('libname srcFiles hppFiles pyconfigs dependencies')
