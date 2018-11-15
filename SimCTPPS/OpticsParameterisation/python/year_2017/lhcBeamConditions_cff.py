import FWCore.ParameterSet.Config as cms

lhcBeamConditions = cms.PSet(
    sqrtS = cms.double(13.e3), # in GeV
    vertexSize = cms.double(10.e-6), # in m
    beamDivergence = cms.double(20.e-6), # in rad

    # vertex offset in both sectors
    #xOffsetSector45 = cms.double(0), # in m
    #xOffsetSector56 = cms.double(0), # in m
    xOffsetSector45 = cms.double(0.00055266), # in m
    xOffsetSector56 = cms.double(0.00055266), # in m

    yOffsetSector45 = cms.double(-0.0015), # in m
    yOffsetSector56 = cms.double(-0.0015), # in m

    # crossing angle
    halfCrossingAngleSector45 = cms.double(120e-6), # in rad
    halfCrossingAngleSector56 = cms.double(120e-6), # in rad
)
