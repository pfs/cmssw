#ifndef TopQuarkAnalysis_TopEventProducers_PseudoTopProducer_H
#define TopQuarkAnalysis_TopEventProducers_PseudoTopProducer_H

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "TopQuarkAnalysis/TopEventProducers/interface/RivetWrapper.h"

class PseudoTopProducer : public edm::stream::EDProducer<>
{
public:
  PseudoTopProducer(const edm::ParameterSet& pset);
  void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

private:
  template<typename T> reco::Candidate::LorentzVector p4(const T& p) const
  {
    return reco::Candidate::LorentzVector(p.px(), p.py(), p.pz(), p.energy());
  }

  const edm::EDGetTokenT<edm::HepMCProduct> srcToken_;
  
  const std::string projection_;

  reco::Particle::Point genVertex_;
  
  Rivet::RivetWrapper rivet_;

};

#endif
