import openwns.evaluation

def installPhyTrace(sim):
    sourceName = "phyTrace"
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.getLeafs().appendChildren(openwns.evaluation.JSONTrace(key="__json__", description="JSON testing in PhyUser"))
