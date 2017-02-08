import FWCore.ParameterSet.Config as cms

process = cms.Process("TQAF")

process.load("FWCore.MessageLogger.MessageLogger_cfi")

from TopQuarkAnalysis.TopEventProducers.tqafInputFiles_cff import relValTTbar
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
      #'/store/mc/RunIISpring16MiniAODv2/TT_TuneCUETP8M2T4_13TeV-powheg-pythia8/MINIAODSIM/premix_withHLT_80X_mcRun2_asymptotic_v14-v1/00000/042D62D1-C597-E611-8FA4-549F3525B9A0.root'
      #'file:C85C0DCE-E293-E611-9F90-02163E0115E8.root'
      'file:TT_TuneCUETP8M2T4_13TeV-powheg-pythia8-MiniAOD.root'
      #'file:TT_TuneCUETP8M2T4_13TeV-powheg-pythia8-MiniAOD-allHadrons.root'
    )
)

## define maximal number of events to loop over
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)
## configure process options
process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(True),
    wantSummary      = cms.untracked.bool(True)
)

#process.load("TopQuarkAnalysis.TopEventProducers.producers.pseudoTop_cfi")
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.mergedGenParticles = cms.EDProducer("MergedGenParticleProducer",
    inputPruned = cms.InputTag("prunedGenParticles"),
    inputPacked = cms.InputTag("packedGenParticles"),
)
process.genParticles2HepMC = cms.EDProducer("GenParticles2HepMCConverter",
    genParticles = cms.InputTag("mergedGenParticles"),
    genEventInfo = cms.InputTag("generator"),
)
process.pseudoTop = cms.EDProducer("PseudoTopRivetProducer",
    src = cms.InputTag("genParticles2HepMC:unsmeared"),

    minLeptonPt = cms.double(15),
    maxLeptonEta = cms.double(2.5),
    minJetPt = cms.double(30),
    maxJetEta = cms.double(2.4),
    leptonConeSize = cms.double(0.1),
    jetConeSize = cms.double(0.4),
    wMass = cms.double(80.4),
    tMass = cms.double(172.5),

    minLeptonPtDilepton = cms.double(20),
    maxLeptonEtaDilepton = cms.double(2.4),
    minDileptonMassDilepton = cms.double(20),
    minLeptonPtSemilepton = cms.double(20),
    maxLeptonEtaSemilepton = cms.double(2.4),
    minVetoLeptonPtSemilepton = cms.double(15),
    maxVetoLeptonEtaSemilepton = cms.double(2.5),
    minMETSemiLepton = cms.double(30),
    minMtWSemiLepton = cms.double(30),
)

#process.p = cms.Path(process.pseudoTop)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string("out-miniaod.root"),
    outputCommands = cms.untracked.vstring(
        "drop *",
        #"keep *_genParticles_*_*",
        "keep *_pseudoTop_*_*",
    ),
)
process.outPath = cms.EndPath(process.out)
