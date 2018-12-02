import FWCore.ParameterSet.Config as cms

from SimCTPPS.OpticsParameterisation.year_2017_OF.ctppsDetectorPackages_cff import detectorPackages
from SimCTPPS.OpticsParameterisation.year_2017_OF.lhcBeamConditions_cff import lhcBeamConditions

ctppsFastProtonSimulation = cms.EDProducer('CTPPSFastProtonSimulationOF',
    verbosity = cms.untracked.uint32(0),

    hepMCTag = cms.InputTag('generator', 'unsmeared'),

    beamConditions = lhcBeamConditions,
    detectorPackages = detectorPackages,

    xangle = cms.double(0), # in urad

    xangle1 = cms.double(140),
    opticsFile1 = cms.FileInPath("CondFormats/CTPPSOpticsObjects/data/2017/optical_functions_2017_140urad.root"),

    xangle2 = cms.double(120),
    opticsFile2 = cms.FileInPath("CondFormats/CTPPSOpticsObjects/data/2017/optical_functions_2017_120urad.root"),

    produceScoringPlaneHits = cms.bool(True),
    produceRecHits = cms.bool(True),

    produceHitsRelativeToBeam = cms.bool(False),

    checkApertures = cms.bool(False),

    useEmpiricalApertures = cms.bool(False),
    empiricalAperture45_xi0 = cms.double(0.140),
    empiricalAperture45_a = cms.double(+210),
    empiricalAperture56_xi0 = cms.double(0.175),
    empiricalAperture56_a = cms.double(+290),

    roundToPitch = cms.bool(False),
    checkIsHit = cms.bool(True),

    pitchStrips = cms.double(66e-3), # mm
    insensitiveMarginStrips = cms.double(34e-3), # mm

    pitchPixelsHor = cms.double(100e-3), # mm, x in local coordinates
    pitchPixelsVer = cms.double(150e-3), # mm, y in local coordinates
)
