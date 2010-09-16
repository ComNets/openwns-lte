class ModeCreator:

    def __init__(self, modetype, *args, **kwargs):
        self.modetype = modetype
        self.args = args
        self.kwargs = kwargs

    def __call__(self, **kwargs):
        self.kwargs.update(kwargs)
        return self.modetype(*self.args, **self.kwargs)

def getModeCreator(modename):
    import lte.modes.mode
    modeMap = {
        #'ltefdd1p4' : ModeCreator(lte.modes.mode.LTEFDD1p4),
        #'ltefdd3'   : ModeCreator(lte.modes.mode.LTEFDD3),
        #'ltefdd5'   : ModeCreator(lte.modes.mode.LTEFDD5),
        'ltefdd10'  : ModeCreator(lte.modes.mode.LTEFDD10),
        #'ltefdd15'  : ModeCreator(lte.modes.mode.LTEFDD15),
        #'ltefdd20'  : ModeCreator(lte.modes.mode.LTEFDD20),
        }

    if not modeMap.has_key(modename):
        print "ERROR! Unknown Mode"
        print "Choose one of the following"
        for key in modeMap.keys():
            print key
    return modeMap[modename]
