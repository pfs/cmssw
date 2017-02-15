#include "TopQuarkAnalysis/TopEventProducers/interface/PseudoTopProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "RecoJets/JetProducers/interface/JetSpecific.h"
#include "CommonTools/Utils/interface/PtComparator.h"

#include "Rivet/Analysis.hh"
#include "TopQuarkAnalysis/TopEventProducers/interface/PseudoTop.hh"

using namespace std;
using namespace edm;
using namespace reco;
using namespace Rivet;

PseudoTopProducer::PseudoTopProducer(const edm::ParameterSet& pset):
  srcToken_(consumes<edm::HepMCProduct>(pset.getParameter<edm::InputTag>("src"))),
  projection_(pset.getParameter<std::string>("projection"))
{
  genVertex_ = reco::Particle::Point(0,0,0);

  produces<reco::GenParticleCollection>("neutrinos");
  produces<reco::GenJetCollection>("leptons");
  produces<reco::GenJetCollection>("jets");
  produces<reco::GenParticleCollection>("consts");

  produces<reco::GenParticleCollection>();
  
  // Init projections added in RivetWrapper
  rivet_.init(pset);
}

void PseudoTopProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup)
{
  using namespace Rivet;
  typedef reco::Candidate::LorentzVector LorentzVector;

  std::unique_ptr<reco::GenParticleCollection> neutrinos(new reco::GenParticleCollection);
  std::unique_ptr<reco::GenJetCollection> leptons(new reco::GenJetCollection);
  std::unique_ptr<reco::GenJetCollection> jets(new reco::GenJetCollection);
  std::unique_ptr<reco::GenParticleCollection> consts(new reco::GenParticleCollection);
  auto neutrinosRefHandle = event.getRefBeforePut<reco::GenParticleCollection>("neutrinos");
  auto leptonsRefHandle = event.getRefBeforePut<reco::GenJetCollection>("leptons");
  auto jetsRefHandle = event.getRefBeforePut<reco::GenJetCollection>("jets");
  auto constsRefHandle = event.getRefBeforePut<reco::GenParticleCollection>("consts");

  std::unique_ptr<reco::GenParticleCollection> pseudoTopCands(new reco::GenParticleCollection);
  auto pseudoTopRefHandle = event.getRefBeforePut<reco::GenParticleCollection>();

  edm::Handle<HepMCProduct> srcHandle;
  event.getByToken(srcToken_, srcHandle);

  // Write usual Rivet analyzer like codes below using the Rivet::Event event
  Rivet::Event genEvent(srcHandle->GetEvent());

  const PseudoTop& pseudoTop = rivet_.applyProjection<PseudoTop>(genEvent, projection_);

  // Convert into edm objects
  for ( auto p : pseudoTop.neutrinos() ) {
    neutrinos->push_back(reco::GenParticle(p.charge(), p4(p), genVertex_, p.pdgId(), 1, true));
  }
  std::sort(neutrinos->begin(), neutrinos->end(), GreaterByPt<reco::Candidate>());

  int iConstituent = 0;
  for ( auto lepton : pseudoTop.leptons() ) {
    reco::GenJet lepJet;
    lepJet.setP4(p4(lepton));
    lepJet.setVertex(genVertex_);
    lepJet.setPdgId(lepton.pdgId());
    lepJet.setCharge(lepton.charge());
    
    const auto cl = lepton.constituentLepton();
    consts->push_back(reco::GenParticle(cl.charge(), p4(cl), genVertex_, cl.pdgId(), 1, true));
    lepJet.addDaughter(edm::refToPtr(reco::GenParticleRef(constsRefHandle, iConstituent)));
    ++iConstituent;
    
    for ( auto p : lepton.constituentPhotons()) {
      consts->push_back(reco::GenParticle(p.charge(), p4(p), genVertex_, p.pdgId(), 1, true));
      lepJet.addDaughter(edm::refToPtr(reco::GenParticleRef(constsRefHandle, iConstituent)));
      ++iConstituent;
    }
    
    leptons->push_back(lepJet);
  }
  std::sort(leptons->begin(), leptons->end(), GreaterByPt<reco::GenJet>());

  for ( auto jet : pseudoTop.jets() ) {
    const auto pjet = jet.pseudojet();

    reco::GenJet genJet;
    genJet.setP4(p4(jet));
    genJet.setVertex(genVertex_);
    if ( jet.bTagged() ) genJet.setPdgId(5);
    genJet.setJetArea(pjet.has_area() ? pjet.area() : 0);
    
    for ( auto p : jet.particles()) {
      consts->push_back(reco::GenParticle(p.charge(), p4(p), genVertex_, p.pdgId(), 1, true));
      genJet.addDaughter(edm::refToPtr(reco::GenParticleRef(constsRefHandle, iConstituent)));
      ++iConstituent;
    }

    jets->push_back(genJet);
  }

  if ( pseudoTop.mode() == PseudoTop::CH_FULLLEPTON or pseudoTop.mode() == PseudoTop::CH_SEMILEPTON ) {
    reco::GenParticle t1(pseudoTop.t1().charge(), p4(pseudoTop.t1()), genVertex_, pseudoTop.t1().pdgId(), 3, false);
    reco::GenParticle w1(pseudoTop.w1().charge(), p4(pseudoTop.w1()), genVertex_, pseudoTop.w1().pdgId(), 3, true);
    reco::GenParticle b1(pseudoTop.b1().charge(), p4(pseudoTop.b1()), genVertex_, pseudoTop.b1().pdgId(), 1, true);
    reco::GenParticle l1(pseudoTop.wDecays1()[0].charge(), p4(pseudoTop.wDecays1()[0]), genVertex_, pseudoTop.wDecays1()[0].pdgId(), 1, true);
    reco::GenParticle n1(pseudoTop.wDecays1()[1].charge(), p4(pseudoTop.wDecays1()[1]), genVertex_, pseudoTop.wDecays1()[1].pdgId(), 1, true);

    reco::GenParticle t2(pseudoTop.t2().charge(), p4(pseudoTop.t2()), genVertex_, pseudoTop.t2().pdgId(), 3, false);
    reco::GenParticle w2(pseudoTop.w2().charge(), p4(pseudoTop.w2()), genVertex_, pseudoTop.w2().pdgId(), 3, true);
    reco::GenParticle b2(pseudoTop.b2().charge(), p4(pseudoTop.b2()), genVertex_, pseudoTop.b2().pdgId(), 1, true);
    reco::GenParticle l2(pseudoTop.wDecays2()[0].charge(), p4(pseudoTop.wDecays2()[0]), genVertex_, pseudoTop.wDecays2()[0].pdgId(), 1, true);
    reco::GenParticle n2(pseudoTop.wDecays2()[1].charge(), p4(pseudoTop.wDecays2()[1]), genVertex_, pseudoTop.wDecays2()[1].pdgId(), 1, true);

    pseudoTopCands->push_back(t1);
    pseudoTopCands->push_back(t2);

    pseudoTopCands->push_back(w1);
    pseudoTopCands->push_back(b1);
    pseudoTopCands->push_back(l1);
    pseudoTopCands->push_back(n1);

    pseudoTopCands->push_back(w2);
    pseudoTopCands->push_back(b2);
    pseudoTopCands->push_back(l2);
    pseudoTopCands->push_back(n2);

    // t->W+b
    pseudoTopCands->at(0).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 2)); // t->W
    pseudoTopCands->at(0).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 3)); // t->b
    pseudoTopCands->at(2).addMother(reco::GenParticleRef(pseudoTopRefHandle, 0)); // t->W
    pseudoTopCands->at(3).addMother(reco::GenParticleRef(pseudoTopRefHandle, 0)); // t->b

    // W->lv or W->jj
    pseudoTopCands->at(2).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 4));
    pseudoTopCands->at(2).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 5));
    pseudoTopCands->at(4).addMother(reco::GenParticleRef(pseudoTopRefHandle, 2));
    pseudoTopCands->at(5).addMother(reco::GenParticleRef(pseudoTopRefHandle, 2));

    // tbar->W-b
    pseudoTopCands->at(1).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 6));
    pseudoTopCands->at(1).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 7));
    pseudoTopCands->at(6).addMother(reco::GenParticleRef(pseudoTopRefHandle, 1));
    pseudoTopCands->at(7).addMother(reco::GenParticleRef(pseudoTopRefHandle, 1));

    // W->jj
    pseudoTopCands->at(6).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 8));
    pseudoTopCands->at(6).addDaughter(reco::GenParticleRef(pseudoTopRefHandle, 9));
    pseudoTopCands->at(8).addMother(reco::GenParticleRef(pseudoTopRefHandle, 6));
    pseudoTopCands->at(9).addMother(reco::GenParticleRef(pseudoTopRefHandle, 6));

  }

  event.put(std::move(neutrinos), "neutrinos");
  event.put(std::move(leptons), "leptons");
  event.put(std::move(jets), "jets");
  event.put(std::move(consts), "consts");

  event.put(std::move(pseudoTopCands));
}

