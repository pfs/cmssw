#ifndef TopQuarkAnalysis_TopEventProducers_PseudoTopRivetProducer_H
#define TopQuarkAnalysis_TopEventProducers_PseudoTopRivetProducer_H

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "TopQuarkAnalysis/TopEventProducers/interface/RivetWrapper.h"

class PseudoTopRivetProducer : public edm::stream::EDProducer<>
{
public:
  PseudoTopRivetProducer(const edm::ParameterSet& pset);
  void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

private:
  template<typename T> reco::Candidate::LorentzVector p4(const T& p) const
  {
    return reco::Candidate::LorentzVector(p.px(), p.py(), p.pz(), p.energy());
  }

  const edm::EDGetTokenT<edm::HepMCProduct> srcToken_;

  const double leptonConeSize_, jetConeSize_;
  const double wMass_, tMass_;
  const double minLeptonPt_, maxLeptonEta_, minJetPt_, maxJetEta_;
  const double minLeptonPtDilepton_, maxLeptonEtaDilepton_;
  const double minDileptonMassDilepton_;
  const double minLeptonPtSemilepton_, maxLeptonEtaSemilepton_;
  const double minVetoLeptonPtSemilepton_, maxVetoLeptonEtaSemilepton_;
  const double minMETSemiLepton_, minMtWSemiLepton_;

  reco::Particle::Point genVertex_;
  
  Rivet::RivetWrapper rivet_;

};

#endif
