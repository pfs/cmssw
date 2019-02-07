import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
process = cms.Process('CTPPSFastSimulation', eras.ctpps_2016)

# minimal logger settings
process.MessageLogger = cms.Service("MessageLogger",
    statistics = cms.untracked.vstring(),
    destinations = cms.untracked.vstring('cerr'),
    cerr = cms.untracked.PSet(
        threshold = cms.untracked.string('WARNING')
    )
)

# number of events
process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(
  input = cms.untracked.int32($n_events)
)

# particle-data table
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")

# particle generator
process.load("SimCTPPS.Generators.PPXZGenerator_cfi")
process.generator.verbosity = 0
process.generator.m_X = $mass
process.generator.m_XZ_min = $mass + 100
process.generator.decayX = True

# random seeds
process.RandomNumberGeneratorService = cms.Service("RandomNumberGeneratorService",
    sourceSeed = cms.PSet(initialSeed =cms.untracked.uint32(98765)),
    generator = cms.PSet(initialSeed = cms.untracked.uint32(98766)),
    SmearingGenerator = cms.PSet(initialSeed =cms.untracked.uint32(3849)),
    beamDivergenceVtxGenerator = cms.PSet(initialSeed =cms.untracked.uint32(3849))
)

# geometry
process.load("Geometry.VeryForwardGeometry.geometryRPFromDD_2017_cfi")
del(process.XMLIdealGeometryESSource_CTPPS.geomXMLFiles[-1])
process.XMLIdealGeometryESSource_CTPPS.geomXMLFiles.append("Validation/CTPPS/test_2017/RP_Dist_Beam_Cent.xml")

# beam-smearing settings
process.load("IOMC.EventVertexGenerators.beamDivergenceVtxGenerator_cfi")
process.beamDivergenceVtxGenerator.src = cms.InputTag("generator", "unsmeared")

process.beamDivergenceVtxGenerator.simulateBeamDivergence = True
process.beamDivergenceVtxGenerator.simulateVertex = True

# values in rad
process.beamDivergenceVtxGenerator.beamDivergenceX = 20E-6 # TODO: updated with experimental value
process.beamDivergenceVtxGenerator.beamDivergenceY = 20E-6

# values in cm
process.beamDivergenceVtxGenerator.vertexMeanX = 0.
process.beamDivergenceVtxGenerator.vertexMeanY = 0.
process.beamDivergenceVtxGenerator.vertexMeanZ = 0.

process.beamDivergenceVtxGenerator.vertexSigmaX = 10E-4
process.beamDivergenceVtxGenerator.vertexSigmaY = 10E-4
process.beamDivergenceVtxGenerator.vertexSigmaZ = 5

# fast simulation
process.load('SimCTPPS.OpticsParameterisation.year_2017_OF.ctppsFastProtonSimulation_cfi')
process.ctppsFastProtonSimulation.verbosity = 0
process.ctppsFastProtonSimulation.hepMCTag = cms.InputTag('beamDivergenceVtxGenerator')
process.ctppsFastProtonSimulation.produceScoringPlaneHits = False
process.ctppsFastProtonSimulation.produceRecHits = True
process.ctppsFastProtonSimulation.useEmpiricalApertures = True
process.ctppsFastProtonSimulation.checkApertures = False
process.ctppsFastProtonSimulation.produceHitsRelativeToBeam = False
process.ctppsFastProtonSimulation.roundToPitch = True

process.ctppsFastProtonSimulation.xangle = 140 # make sure it corresponds to process.ctppsLHCInfoESSource.xangle
process.ctppsFastProtonSimulation.empiricalAperture45_xi0 = 0.153
process.ctppsFastProtonSimulation.empiricalAperture56_xi0 = 0.19

# effective value to reproduce real RP resolution
process.ctppsFastProtonSimulation.pitchStrips = 66E-3 * 12 / 19
process.ctppsFastProtonSimulation.pitchPixelsHor = cms.double(60e-3)
process.ctppsFastProtonSimulation.pitchPixelsVer = cms.double(80e-3)

# local track reco
process.load('RecoCTPPS.TotemRPLocal.totemRPUVPatternFinder_cfi')
process.totemRPUVPatternFinder.tagRecHit = cms.InputTag('ctppsFastProtonSimulation')

process.load('RecoCTPPS.TotemRPLocal.totemRPLocalTrackFitter_cfi')

process.load("RecoCTPPS.PixelLocal.ctppsPixelLocalTracks_cfi")
process.ctppsPixelLocalTracks.label = "ctppsFastProtonSimulation"

process.load('RecoCTPPS.TotemRPLocal.ctppsLocalTrackLiteProducer_cff')
process.ctppsLocalTrackLiteProducer.includeDiamonds = False

# proton reconstruction
process.load("RecoCTPPS.ProtonReconstruction.year_2017_OF.ctppsProtonReconstructionOF_cfi")
process.ctppsProtonReconstructionOFDB.tagLocalTrackLite = cms.InputTag('ctppsLocalTrackLiteProducer')
process.ctppsProtonReconstructionOFDB.applyExperimentalAlignment = False

process.ctppsLHCInfoESSource = cms.ESSource("CTPPSLHCInfoESSource",
  beamEnergy = cms.double(6500),
  xangle = cms.double(140)
)

# XY distribution plotter
process.ctppsTrackDistributionPlotter = cms.EDAnalyzer("CTPPSTrackDistributionPlotter",
  tracksTag = cms.InputTag("ctppsLocalTrackLiteProducer"),
  outputFile = cms.string("output_shape_smear.root")
)

# generator plots
process.load("SimCTPPS.Generators.PPXZGeneratorValidation_cfi")
process.ppxzGeneratorValidation.hepMCTag = cms.InputTag("generator", "unsmeared")
process.ppxzGeneratorValidation.recoTracksTag = cms.InputTag("ctppsLocalTrackLiteProducer")
process.ppxzGeneratorValidation.recoProtonsTag = cms.InputTag("ctppsProtonReconstructionOFDB")
process.ppxzGeneratorValidation.referenceRPDecId_45 = cms.uint32(23)
process.ppxzGeneratorValidation.referenceRPDecId_56 = cms.uint32(123)
process.ppxzGeneratorValidation.outputFile = "ppxzGeneratorValidation.root"

# reconstruction validation
process.load("Validation.CTPPS.ctppsProtonReconstructionValidator_cfi")
process.ctppsProtonReconstructionValidator.tagHepMCBeforeSmearing = cms.InputTag("generator", "unsmeared")
process.ctppsProtonReconstructionValidator.tagHepMCAfterSmearing = cms.InputTag("beamDivergenceVtxGenerator")
process.ctppsProtonReconstructionValidator.tagRecoProtons = cms.InputTag("ctppsProtonReconstructionOFDB")

# processing path
process.p = cms.Path(
    process.generator

    * process.beamDivergenceVtxGenerator

    * process.ctppsFastProtonSimulation

    * process.totemRPUVPatternFinder
    * process.totemRPLocalTrackFitter
    * process.ctppsPixelLocalTracks
    * process.ctppsLocalTrackLiteProducer
    * process.ctppsProtonReconstructionOFDB

    * process.ctppsTrackDistributionPlotter
    * process.ppxzGeneratorValidation
    * process.ctppsProtonReconstructionValidator
)


# output configuration
process.output = cms.OutputModule("PoolOutputModule",
  fileName = cms.untracked.string("file:./ntuple.root"),
  splitLevel = cms.untracked.int32(0),
  eventAutoFlushCompressedSize=cms.untracked.int32(-900),
  compressionAlgorithm=cms.untracked.string("LZMA"),
  compressionLevel=cms.untracked.int32(4),
  outputCommands = cms.untracked.vstring(
    'drop *',
    'keep edmHepMCProduct_*_*_*',
    'keep CTPPSLocalTrackLites_*_*_*',
    'keep recoProtonTracks_*_*_*'
  )
)

process.outpath = cms.EndPath(process.output)
