import FWCore.ParameterSet.Config as cms

def singleBlockPoints2PSet(blockname, points):
    return cms.VPSet([
                cms.PSet(name = cms.string(name),
                        params = cms.VPSet(
                            cms.PSet(block = cms.string(blockname),
                                     name  = cms.string(key),
                                     value = cms.double(val)) for (key,val) in point.iteritems()))
                for (name,point) in points])
def genericPoints2PSet(points):
    return cms.VPSet([
                cms.PSet(name = cms.string(name),
                        params = cms.VPSet(
                            cms.PSet(block = cms.string(blockname),
                                     name  = cms.string(key),
                                     value = cms.double(val)) for ((blockname,key),val) in point.iteritems()))
                for (name,point) in points])


def warsawSMEFTsimPoints2SMEFTatNLO(points):
    mapSub = {
        'LambdaSMEFT' : ('dim6','Lambda'),
        'cHDD' : ('dim6','cpDC'),  
        'cHWB' : ('dim6','cpWB'),  
        'cHbox' : ('dim6','cdp'),   
        'cH' : ('dim6','cp'),    
        'cW' : ('dim6','cWWW'),  
        'cHWB' : ('dim6','cG'),    
        'cHG' : ('dim6','cpG'),   
        'cHW' : ('dim6','cpW'),   
        'cHB' : ('dim6','cpBB'),  
    }
    # I'm to lazy now to do the rest
    return [ (name, dict((mapSub[key],val) for (key,val) in point.iteritems())) for (name, point) in points ]

