// Author: Felice Pantaleo - felice.pantaleo@cern.ch
// Date: 09/2018
// Copyright CERN

#ifndef __RecoHGCal_TICL_PRbyCA_H__
#define __RecoHGCal_TICL_PRbyCA_H__
#include <algorithm>
#include <iostream>
#include "RecoHGCal/TICL/interface/PatternRecognitionAlgoBase.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "RecoHGCal/TICL/interface/TICLConstants.h"
#include "DataFormats/Math/interface/normalizedPhi.h"
#include "HGCGraph.h"
#include "HGCDoublet.h"



class PatternRecognitionbyCA final : public PatternRecognitionAlgoBase {
public:
    PatternRecognitionbyCA(const edm::ParameterSet& conf) : PatternRecognitionAlgoBase(conf) {
      min_cos_theta_ = (float)conf.getParameter<double>("min_cos_theta");
      min_cos_pointing_ = (float)conf.getParameter<double>("min_cos_pointing");
      missing_layers_ = conf.getParameter<int>("missing_layers");
      min_clusters_per_ntuplet_ = conf.getParameter<int>("min_clusters_per_ntuplet");
        // TODO get number of bins from configuration
        // eta min 1.5, max 3.0
        // phi min -pi, max +pi
        // bins of size 0.05 in eta/phi -> 30 bins in eta, 126 bins in phi
        histogram_.resize(nLayers_);
        auto nBins = nEtaBins_*nPhiBins_;
        for(int i = 0; i < nLayers_; ++i)
        {
            histogram_[i].resize(nBins);
        }
    }


    void fillHistogram(const std::vector<reco::CaloCluster>& layerClusters,
            const std::vector<std::pair<unsigned int, float> >& mask);


    void makeTracksters(
        const edm::Event& ev,
        const edm::EventSetup& es,
        const std::vector<reco::CaloCluster>& layerClusters,
        const std::vector<std::pair<unsigned int, float> >& mask, std::vector<Trackster>& result) override;

private:

    int getEtaBin(float eta) const {
        constexpr float etaRange = ticlConstants::maxEta - ticlConstants::minEta;
        static_assert(etaRange>=0.f);
        float r = nEtaBins_/etaRange;
        int etaBin = (std::abs(eta) - ticlConstants::minEta)*r;
        etaBin = std::min(etaBin,nEtaBins_);
        etaBin = std::max(etaBin,0);
        return etaBin;
    }

    int getPhiBin(float phi) const {
        auto normPhi = normalizedPhi(phi);
        float r = nPhiBins_*M_1_PI*0.5f;
        int phiBin = (normPhi + M_PI)*r;

        return phiBin;
    }

    int globalBin(int etaBin, int phiBin) const {
        return phiBin + etaBin*nPhiBins_;
    }

    void clearHistogram()
    {
        auto nBins = nEtaBins_*nPhiBins_;
        for(int i = 0; i < nLayers_; ++i)
        {
            for(int j = 0; j< nBins; ++j)  histogram_[i][j].clear();
        }
    }


    std::vector< std::vector< std::vector<unsigned int> > > histogram_; // a histogram of layerClusters IDs per layer

    hgcal::RecHitTools rhtools_;

    const int nEtaBins_ = 30;
    const int nPhiBins_ = 126;
    const int nLayers_ = 104;
    HGCGraph theGraph_;
    float min_cos_theta_;
    float min_cos_pointing_;
    int missing_layers_;
    int min_clusters_per_ntuplet_;
};

#endif
