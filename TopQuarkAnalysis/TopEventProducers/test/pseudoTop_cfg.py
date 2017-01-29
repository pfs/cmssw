import FWCore.ParameterSet.Config as cms

process = cms.Process("TQAF")

process.load("FWCore.MessageLogger.MessageLogger_cfi")

from TopQuarkAnalysis.TopEventProducers.tqafInputFiles_cff import relValTTbar
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(relValTTbar)
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
process.load("GeneratorInterface.RivetInterface.genParticles2HepMC_cfi")
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
    fileName = cms.untracked.string("out.root"),
    outputCommands = cms.untracked.vstring(
        "drop *",
        "keep *_genParticles_*_*",
        "keep *_pseudoTop_*_*",
    ),
)
process.outPath = cms.EndPath(process.out)
