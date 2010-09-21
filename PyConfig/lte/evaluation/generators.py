from openwns.evaluation import *
from lte.modes.hasModeName import HasModeName
import lte.dll.qos

# MAC.StationType
# 1 : BS
#            MAC.task
# 2 : RN---- 0 : RNBS
#         +- 1 : RNUT
# 3 : UT


class SeparateOnlyBSs(ITreeNodeGenerator):
    """ Separate only BSs.
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [5], names = ['BS'], format='%s')(pathname):
            yield node

class SeparateOnlyRNs(ITreeNodeGenerator):
    """ Separate only RNs. RNBS and RNUT
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [2], names = ['RN'], format='%s')(pathname):
            yield node

class SeparateOnlyRNBSs(ITreeNodeGenerator):
    """ Separate only RNBSs
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.task', keys = [0], names = ['RNBS'], format='%s')(pathname):
            yield node

class SeparateOnlyRNUTs(ITreeNodeGenerator):
    """ Separate only RNUTs
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.task', keys = [1], names = ['RNUT'], format='%s')(pathname):
            yield node

class SeparateOnlyUTs(ITreeNodeGenerator):
    """ Separate only UTs, but not RNUTs
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [6], names = ['UT'], format='%s')(pathname):
            yield node

class SeparateByBSandUT(ITreeNodeGenerator):
    """ Separate by BS and UT (ignore RN)
    """

    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [5, 6], names = ['BS', 'UT'], format='%s')(pathname):
            yield node

class SeparateByBSRNandUT(ITreeNodeGenerator):
    """ Separate by BS, RN and UT
    """

    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [5, 2, 6], names = ['BS', 'RN', 'UT'], format='%s')(pathname):
            yield node

class SeparateByBSRNUTandRNTasks(ITreeNodeGenerator):
    """ Separate by BS, RN and UT. Further Separate the RN by its task.
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [5, 6], names = ['BS', 'UT'], format='%s')(pathname):
            yield node
        for node in Enumerated(by = 'MAC.StationType', keys = [2], names = ['RN'], format='')(pathname):
            node.appendChildren(Enumerated(by = 'MAC.task', keys = [0, 1], names = ['RNBS', 'RNUT'], format='%s'))
            yield node

class SeparateByBSandRNBS(ITreeNodeGenerator):
    """ Separate only RAPs. BS and RNBS are RAPs. UTs and RNUTs are UTs.
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [5], names = ['BS'], format='%s')(pathname):
            yield node
        for node in Enumerated(by = 'MAC.StationType', keys = [2], names = ['RN'], format='')(pathname):
            node.appendChildren(Enumerated(by = 'MAC.task', keys = [0], names = ['RNBS'], format='%s'))
            yield node

class SeparateByUTandRNUT(ITreeNodeGenerator):
    """ Separate only UTs and RNUTs. BS and RNBS are RAPs.
    """
    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.StationType', keys = [6], names = ['UT'], format='%s')(pathname):
            yield node
        for node in Enumerated(by = 'MAC.StationType', keys = [2], names = ['RN'], format='')(pathname):
            node.appendChildren(Enumerated(by = 'MAC.task', keys = [1], names = ['RNUT'], format='%s'))
            yield node

class SeparateByHopCount(ITreeNodeGenerator):
    """ Separate by HopCount.
    MAC.HopCount
    1 : SingleHop
    2 : Multihop
    """

    def __call__(self, pathname):
        for node in Enumerated(by = 'MAC.HopCount', keys = [1, 2], names = ['SingleHop', 'MultiHop'], format='%s')(pathname):
            yield node

class SeparateByMode(ITreeNodeGenerator):
    """ Separate by Mode. Mapping between mode name and number is in HasModeName.py
    MAC.mode
    """
    def __call__(self, pathname):
        _keys = []
        _names = []
        for (n,k) in HasModeName.mappingOfModeNamesToNumbers.items():
            _names.append(n)
            _keys.append(k)
        for node in Enumerated(by = 'MAC.mode', keys = _keys, names = _names, format='%s')(pathname):
            yield node

class SeparateByQoSClass(ITreeNodeGenerator):
    """ Separate by QoS class. Mapping between priority of the the class and its name.
    MAC.QoSClass
    see QoSClasses defined in winprost.qos
    """
    def __call__(self, pathname):
        qosClassMapping = lte.qos.QoSClasses()
        _keys =[]
        _names = []
        for qosClass in qosClassMapping.mapEntries:
            _keys.append(qosClass.number)
            _names.append(qosClass.name)

        for node in Enumerated(by = 'MAC.QoSClass', keys = _keys, names = _names, format='%s')(pathname):
            yield node
