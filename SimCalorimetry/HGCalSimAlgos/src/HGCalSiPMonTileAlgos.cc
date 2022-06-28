#include "SimCalorimetry/HGCalSimProducers/interface/HGCDigitizerBase.h"
#include "DataFormats/HGCDigi/interface/HGCDigiCollections.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "SimCalorimetry/HGCalSimAlgos/interface/HGCalSciNoiseMap.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "SimCalorimetry/HGCalSimProducers/interface/HGCDigitizerPluginFactory.h"

#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandGaussQ.h"
#include "vdt/vdtMath.h"


/**
   @class HGCCEHSiPMTileDigitizer
   @short specialization of HGCDigitizerBase for the CE-H SiPM-on-tile compartment 
*/
class HGCCEHSiPMTileDigitizer : public HGCDigitizerBase {
public:
  HGCCEHSiPMTileDigitizer(const edm::ParameterSet& ps);
  void run(std::unique_ptr<HGCalDigiCollection>& digiColl,
           hgc::HGCSimHitDataAccumulator& simData,
           const CaloSubdetectorGeometry* theGeom,
           const std::unordered_set<DetId>& validIds,
           CLHEP::HepRandomEngine* engine) override;
  void updateOutput(std::unique_ptr<hgc_digi::DColl>& coll,
                    const hgc_digi::DFr& rawDataFrame) override;
  ~HGCCEHSiPMTileDigitizer() override;

private:
  HGCalSciNoiseMap scal_;
};


using namespace hgc_digi;

//
HGCCEHSiPMTileDigitizer::HGCCEHSiPMTileDigitizer(const edm::ParameterSet& ps) : HGCDigitizerBase(ps) {

  this->det_ = DetId::HGCalHSc;

  //FIXME: move all of this to a scal_->configure(myCfg_);
  edm::ParameterSet scalerCfg = digiCfg_.template getParameter<edm::ParameterSet>("scalerCfg");
  int scaleByDoseAlgo         = scalerCfg.template getParameter<uint32_t>("scaleByDoseAlgo");
  double scaleByDoseFactor    = scalerCfg.template getParameter<double>("scaleByDoseFactor");
  std::string doseMapFile     = scalerCfg.template getParameter<std::string>("doseMap");
  std::string sipmMapFile     = scalerCfg.template getParameter<std::string>("sipmMap");
  double refIdark             = scalerCfg.template getParameter<double>("referenceIdark");
  double xTalk                = scalerCfg.template getParameter<double>("referenceXtalk");
  scal_.setDoseMap(doseMapFile, scaleByDoseAlgo);
  scal_.setReferenceDarkCurrent(refIdark);
  scal_.setFluenceScaleFactor(scaleByDoseFactor);
  scal_.setSipmMap(sipmMapFile);
  scal_.setReferenceCrossTalk(xTalk);
  //FIXME
  //this->myFEelectronics_->setTDCfsc(2 * scal_.getNPeInSiPM());
}

//
void HGCCEHSiPMTileDigitizer::run(std::unique_ptr<HGCalDigiCollection>& digiColl,
                                  HGCSimHitDataAccumulator& simData,
                                  const CaloSubdetectorGeometry* theGeom,
                                  const std::unordered_set<DetId>& validIds,
                                  CLHEP::HepRandomEngine* engine) {

  HGCSimHitData chargeColl, toa;
  // this represents a cell with no signal charge
  HGCCellInfo zeroData;
  zeroData.hit_info[0].fill(0.f);  //accumulated energy
  zeroData.hit_info[1].fill(0.f);  //time-of-flight

  // needed to compute the radiation and geometry scale factors
  scal_.setGeometry(theGeom);

  for (const auto& id : validIds) {

    if (id.det() != DetId::HGCalHSc) continue;

    chargeColl.fill(0.f);
    toa.fill(0.f);
    HGCSimHitDataAccumulator::iterator it = simData.find(id);
    HGCCellInfo& cell = (simData.end() == it ? zeroData : it->second);
    addCellMetadata(cell, theGeom, id);
    
    HGCScintillatorDetId scId(id.rawId());
    double radius = scal_.computeRadius(scId);
    auto opChar = scal_.scaleByDose(scId, radius);
    float scaledPePerMip = opChar.s;
    float tunedNoise = opChar.n;
    float nTotalPixels = opChar.ntotalPE;
    float xTalk = opChar.xtalk;

    //set mean for poissonian noise
    float meanN = std::pow(tunedNoise, 2);

    for (size_t i = 0; i < cell.hit_info[0].size(); ++i) {

      //FIXME
      //convert total energy keV->MIP, since converted to keV in accumulator
      float totalIniMIPs(cell.hit_info[0][i]); // * keV2MIP_);

      //generate the number of photo-electrons from the energy deposit
      const uint32_t npeS = std::floor(CLHEP::RandPoissonQ::shoot(engine, totalIniMIPs * scaledPePerMip) + 0.5);

      //generate the noise associated to the dark current
      const uint32_t npeN = std::floor(CLHEP::RandPoissonQ::shoot(engine, meanN) + 0.5);

      //total number of pe from signal + noise  (not subtracting pedestal)
      const uint32_t npe = npeS + npeN;

      //take into account SiPM saturation
      uint32_t nPixel(npe);
      if (xTalk >= 0) {
        const float x = vdt::fast_expf(-((float)npe) / nTotalPixels);
        if (xTalk * x != 1)
          nPixel = (uint32_t)std::max(nTotalPixels * (1.f - x) / (1.f - xTalk * x), 0.f);
      }

      //take into account the gain fluctuations of each pixel
      //just a note for now, parameters to be defined
      //const float nPixelTot = nPixel + sqrt(nPixel) * CLHEP::RandGaussQ::shoot(engine, 0., 0.05);

      //subtract back the pedestal
      float pedestal(meanN);
      if (scal_.ignoreAutoPedestalSubtraction())
        pedestal = 0.f;
      chargeColl[i] = std::max(nPixel - pedestal, 0.f);
    
      //update time of arrival
      toa[i] = cell.hit_info[1][i];
      if (totalIniMIPs > 0)
        toa[i] = cell.hit_info[1][i] / totalIniMIPs;
    }
    
    //update FE config
    //int gainIdx = opChar.gain;
    //float adcLsb = scal_.getLSBPerGain()[gainIdx];
    //uint32_t thrADC = opChar.thrADC;
    //float maxADC = scal_.getMaxADCPerGain()[gainIdx] - 1e-6;
    //this->frontend()->configureADC(maxADC,);
    //this->frontend()->configureTOA(fsc,onset,toaJitterParam);
    //this->frontend()->configureTOT(fsc,onset,chargeDrainParam,bxUndershoot);
    this->frontend()->configureNoise(0.f); //sipm noise is already simulated above

    //init a new data frame and run FE emulation
    hgc_digi::DFr dfr;
    this->frontend()->run(dfr, chargeColl, toa, engine,hgc_digi::intimeSample);

    //prepare the output
    this->updateOutput(digiColl, dfr);
  }
}

//
void HGCCEHSiPMTileDigitizer::updateOutput(std::unique_ptr<hgc_digi::DColl>& coll, const DFr& rawDataFrame) {

  //FIXME
  // if in time amplitude is above threshold
  // , then don't push back the dataframe
  //if ((!rawDataFrame.threshold())) {
  //  return;
  // }

  coll->push_back(rawDataFrame);
}


//
HGCCEHSiPMTileDigitizer::~HGCCEHSiPMTileDigitizer() {}

DEFINE_EDM_PLUGIN(HGCDigitizerPluginFactory, HGCCEHSiPMTileDigitizer, "HGCCEHSiPMTileDigitizer");
