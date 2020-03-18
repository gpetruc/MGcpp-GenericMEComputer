import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process('GEN',eras.Run2_2018,eras.run2_nanoAOD_102Xv1)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.GeometrySimDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedRealistic25ns13TeVEarly2018Collision_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('PhysicsTools.NanoAOD.nano_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)

# Input source
process.source = cms.Source("LHESource",
        fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7.bis/WpH_HToMuNu-SMEFTsim-B/Events/run_01/unweighted_events.lhe',)
)

process.options = cms.untracked.PSet( 
        #wantSummary = cms.untracked.bool(True),
        #numberOfStreams = cms.untracked.uint32(0),
        #numberOfThreads = cms.untracked.uint32(2),
)

# Output definition
process.RAWSIMoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('runjob_step')
    ),
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(1),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(20971520),
    fileName = cms.untracked.string('step1.root'),
    outputCommands = process.RAWSIMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# Other statements
process.XMLFromDBSource.label = cms.string("Extended")
process.genstepfilter.triggerConditions=cms.vstring("runjob_step")

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '102X_upgrade2018_realistic_v11', '')

from Configuration.Generator.Pythia8CommonSettings_cfi import pythia8CommonSettingsBlock
from Configuration.Generator.MCTunes2017.PythiaCP5Settings_cfi import pythia8CP5SettingsBlock
from Configuration.Generator.PSweightsPythia.PythiaPSweightsSettings_cfi import pythia8PSweightsSettingsBlock

process.generator = cms.EDFilter("Pythia8HadronizerFilter",
    comEnergy = cms.double(13000.0),
    filterEfficiency = cms.untracked.double(1.0),
    maxEventsToPrint = cms.untracked.int32(0),
    pythiaHepMCVerbosity = cms.untracked.bool(False),
    pythiaPylistVerbosity = cms.untracked.int32(0),
    PythiaParameters = cms.PSet(
        pythia8CommonSettingsBlock,
        pythia8CP5SettingsBlock,
        pythia8PSweightsSettingsBlock,
        parameterSets = cms.vstring('pythia8CommonSettings',
                                    'pythia8CP5Settings',
                                    'pythia8PSweightsSettings', 
                                    )
    )
)

process.NANOAODSIMoutput = cms.OutputModule("NanoAODOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('runjob_step')
    ),
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(9),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('NANOAODSIM'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('step1_NANO.root'),
    outputCommands = process.NANOAODSIMEventContent.outputCommands
)

process.RAWSIMoutput.outputCommands = [
    'drop *',
    'keep *_source_*_*',
    'keep nanoaodFlatTable_*Table_*_*',
    'keep String_*_genModel_*',
    'keep nanoaodMergeableCounterTable_*Table_*_*',
    'keep nanoaodUniqueString_nanoMetadata_*_*',
]
process.NANOAODSIMoutput.outputCommands = [
    'drop *',
    'keep nanoaodFlatTable_*Table_*_*',
    'keep String_*_genModel_*',
    'keep nanoaodMergeableCounterTable_*Table_*_*',
    'keep nanoaodUniqueString_nanoMetadata_*_*',
]

process.finalGenParticles.src = 'genParticles'
process.genParticles2HepMC.genParticles = 'genParticles'
process.genParticles2HepMCHiggsVtx.genParticles = 'genParticles'
process.particleLevelSequence.remove(process.mergedGenParticles)



War_points_CPeven = [
        ('HBox_up', dict(cHbox=+1)),
        ('HBox_dn', dict(cHbox=-1)),
        ('HW_up', dict(cHW=+0.1)),
        ('HW_dn', dict(cHW=-0.1)),
       #('HB_up', dict(cHB=+0.1)),
       #('HB_dn', dict(cHB=-0.1)),
       #('HWHB_up', dict(cHW=+0.1,cHB=+0.1)),
       #('HWHB_dn', dict(cHW=-0.1,cHB=-0.1)),
       #('NoZZ_up', dict(cHW=+0.469333471898234, cHB=-1.6326252710332367)),
       #('NoZZ_dn', dict(cHW=-0.469333471898234, cHB=+1.6326252710332367)),
]
War_points = War_points_CPeven + [
       ('HWtil_up', dict(cHWtil=+1)),
       #('HWtil_dn', dict(cHWtil=-1)),
       #('HWtil_half', dict(cHWtil=+0.5)),
       #('HBtil_up', dict(cHBtil=+1)),
       #('HBtil_dn', dict(cHBtil=-1)),
       #('HBtil_half', dict(cHBtil=+0.5)),
       #('HWBtil_up', dict(cHWBtil=+1)),
       #('HWBtil_dn', dict(cHWBtil=-1)),
       #('HWBtil_half', dict(cHWBtil=+0.5)),
]


import sys
args = sys.argv[:]
if args[0].startswith('cmsRun'): args = args[1:]
sample = args[1] if len(args) > 1 else 'WpH_HToMuNu-SMEFTsim-B'
mother = 0; undecayed = [ 25 ]
scanPoints = cms.VPSet()
from MGcpp.GenericMEComputer.utils import singleBlockPoints2PSet, genericPoints2PSet, warsawSMEFTsimPoints2SMEFTatNLO

## ==== WH ====
if sample == "WpH_HToMuNu-SMEFTsim-B":  
    process.source.fileNames = [ 'file:/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7/WpH_SMEFTsimB-no_b_mass/Events/run_01/unweighted_events.lhe' ]
    paramCard = "/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7.bis/WpH_HToMuNu-SMEFTsim-B/Cards/param_card.dat"
    mgProcessCollection = "MGME_WpH_SMEFTsimB"
    #myWeights = [ 'rwHBox_up', 'rwHBox_dn', 'rwHW_up', 'rwHW_dn', 'rwHWtil_up', 'rwHWtil_dn', 'rwHWtil_half', ]
    myWeights = [ 'nhrwHW_up', 'nhrwHW_dn', 'nhrwHBox_up', 'nhrwHBox_dn', 'nhrwHWtil_up' ]
    scanPoints = singleBlockPoints2PSet("newcoup", War_points)
elif sample == "WpH_HToMuNu-SMEFTsim-B-with-SMEFTatNLO-01j":  
    process.source.fileNames = [ 'file:/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7/WpH_SMEFTsimB-no_b_mass/Events/run_01/unweighted_events.lhe' ]
    paramCard = "/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7/cpp_WpH-SMEFTatNLO-01j/Cards/param_card.dat"
    mgProcessCollection = "MGME_WpH_SMEFTatNLO_01j"
    #myWeights = [ 'rwHBox_up', 'rwHBox_dn', 'rwHW_up', 'rwHW_dn', 'rwHWtil_up', 'rwHWtil_dn', 'rwHWtil_half', ]
    myWeights = [ 'nhrwHW_up', 'nhrwHW_dn', 'nhrwHBox_up', 'nhrwHBox_dn', ]  
    scanPoints = genericPoints2PSet(warsawSMEFTsimPoints2SMEFTatNLO(War_points_CPeven))
elif sample == "ggH_HTo4L-SMEFTsim-B":
    process.source.fileNames = [ 'file:/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7/ggH_HTo4L-SMEFTsim-B/Events/run_02/unweighted_events.lhe' ]
    paramCard = "/afs/cern.ch/work/g/gpetrucc/eft/EFT2Obs/EFT2Obs/MG5_aMC_v2_6_7/ggH_HTo4L-SMEFTsim-B/Cards/param_card.dat"
    mgProcessCollection = "MGME_HTo4L_SMEFTsimB"
    mother = 25; undecayed = []
    #myWeights = [ 'rwHBox_up', 'rwHBox_dn', 'rwHW_up', 'rwHW_dn', 'rwHWtil_up', 'rwHWtil_dn', 'rwHWtil_half', ]
    #myWeights += ["nh"+w for w in myWeights if (w.startswith("rw") and ("SM" not in w)) ]
    myWeights = [ 'nhrwHW_up', 'nhrwHW_dn', 'nhrwHWtil_up' ]  
    scanPoints = singleBlockPoints2PSet("newcoup", War_points)
print scanPoints.dumpPython()

process.NANOAODSIMoutput.fileName = 'step1-%s_NANO.root' % sample
process.RAWSIMoutput.fileName = 'v2-step1-%s.root' % sample

process.genWeightsTable.namedWeightIDs = myWeights
process.genWeightsTable.namedWeightLabels = [ ("nh"+w[4:] if w.startswith("nhrw") else w[2:]) for w in myWeights ]
#process.genWeightsTable.printNamedWeights = cms.untracked.bool(True)

process.gen_minimal = cms.Sequence(
    process.generator * 
    process.VtxSmeared *
    process.generatorSmeared *
    process.genParticles
)

process.pre_nanoAOD = cms.Sequence(
                                    process.genParticleSequence + 
                                    process.particleLevelSequence +
                                    process.genTable + 
                                    process.genWeightsTable + 
                                    process.genParticleTables + 
                                    process.particleLevelTables + 
                                    process.lheInfoTable 
)

process.mgmeTable = cms.EDProducer("MGGenMatrixElementTableProducer",
   lheInfo = cms.VInputTag(cms.InputTag("externalLHEProducer"), cms.InputTag("source")),
   name = cms.string("MG"),
   processCollection = cms.string(mgProcessCollection),
   slha = cms.string(paramCard),
   motherParticle = cms.int32(mother),
   undecayedPdgIds = cms.vuint32(*undecayed),
   scanPoints = scanPoints,
)



process.runjob_step = cms.Path(process.gen_minimal * process.pre_nanoAOD * process.mgmeTable)
#process.runjob_step = cms.Path(process.lheInfoTable * process.mgmeTable)
process.RAWSIMoutput_step = cms.EndPath(process.RAWSIMoutput)
process.NANO_step = cms.EndPath(process.NANOAODSIMoutput)
#process.schedule = cms.Schedule(process.runjob_step,process.RAWSIMoutput_step)
process.schedule = cms.Schedule(process.runjob_step,process.RAWSIMoutput_step,process.NANO_step)

