#ifndef RIVET_PseudoTop_HH
#define RIVET_PseudoTop_HH

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Particle.hh"
#include "Rivet/Particle.fhh"
#include "Rivet/Event.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/ChargedLeptons.hh"
#include "Rivet/Projections/DressedLeptons.hh"
#include "Rivet/Projections/VetoedFinalState.hh"
#include "Rivet/Projections/IdentifiedFinalState.hh"
#include "Rivet/Projections/VisibleFinalState.hh"
#include "Rivet/Projections/MissingMomentum.hh"
#include "Rivet/Tools/RivetHepMC.hh"

namespace Rivet {

  /// @brief Get only particles with parents
  ///
  /// This is projection used to filter out photons without parents in CMS prunedGenParticles.
  /// It should not be necessary to use on "real" HepMC events.
  class HasLeptonParentFinalState : public FinalState {
    public:
      /// The default constructor. May specify cuts
      HasLeptonParentFinalState(const FinalState& fs)
        : FinalState()
      {
        setName("HasLeptonParentFinalState");
        addProjection(fs, "FS");
      }
      
      /// Clone on the heap.
      virtual Projection* clone() const override { return new HasLeptonParentFinalState(*this); }
      
    public:
      void project(const Event& event) {
        _theParticles.clear();
        
        const Particles particles = applyProjection<FinalState>(event, "FS").particles();
        
        int nAll = 0;
        int nFromLepton = 0;
        int nFromLepton2 = 0;
        int nFromHadron = 0;
        int nFromProton = 0;
        int nFromQuark = 0;
        int nFromGluon = 0;
        int nOrphans = 0;
        for (const Particle& p : particles) {
          nAll++;
          if (hasLeptonParents(p)) nFromLepton++;
          if (hasLeptonParents2(p)) nFromLepton2++;
          if (p.fromHadron()) nFromHadron++;
          if (hasProtonParents(p)) nFromProton++;
          if (hasQuarkParents(p)) nFromQuark++;
          if (hasGluonParents(p)) nFromGluon++;
          if (not hasParents(p)) nOrphans++;
          
          if (hasLeptonParents(p)) // to be replaced with hasParentWith(...) in Rivet >= 2.5 (CMSSW 81X)
            _theParticles.push_back(p);
        }
        std::cout << "photons: " << nAll << ", from lepton: " << nFromLepton << ", from lepton2: " << nFromLepton2 << ", from hadron: " << nFromHadron << ", from proton: " << nFromProton << ", from quark: " << nFromQuark << ", from gluon: " << nFromGluon << ", orphaned: " << nOrphans << std::endl;
      }
      
    private:
      // Derived from Rivet >= 2.5 Particle::parents()
      bool hasLeptonParents(Particle particle) const {
        GenVertex* gv = particle.genParticle()->production_vertex();
        for (GenVertex::particle_iterator it = gv->particles_begin(HepMC::parents); it != gv->particles_end(HepMC::parents); ++it) {
          const Particle p(*it);
          if (p.isLepton())
            return true;
        }
        return false;
      }
      bool hasLeptonParents2(Particle particle) const {
        GenVertex* gv = particle.genParticle()->production_vertex();
        for (GenVertex::particle_iterator it = gv->particles_begin(HepMC::parents); it != gv->particles_end(HepMC::parents); ++it) {
          const Particle p(*it);
          if (p.abspid() >=11 and p.abspid() <= 18)
            return true;
        }
        return false;
      }
      bool hasQuarkParents(Particle particle) const {
        GenVertex* gv = particle.genParticle()->production_vertex();
        for (GenVertex::particle_iterator it = gv->particles_begin(HepMC::parents); it != gv->particles_end(HepMC::parents); ++it) {
          const Particle p(*it);
          if (abs(p.pid()) <= 6)
            return true;
        }
        return false;
      }
      bool hasProtonParents(Particle particle) const {
        GenVertex* gv = particle.genParticle()->production_vertex();
        for (GenVertex::particle_iterator it = gv->particles_begin(HepMC::parents); it != gv->particles_end(HepMC::parents); ++it) {
          const Particle p(*it);
          if (p.pid() == 2212)
            return true;
        }
        return false;
      }
      bool hasGluonParents(Particle particle) const {
        GenVertex* gv = particle.genParticle()->production_vertex();
        for (GenVertex::particle_iterator it = gv->particles_begin(HepMC::parents); it != gv->particles_end(HepMC::parents); ++it) {
          const Particle p(*it);
          if (p.pid() == 21)
            return true;
        }
        return false;
      }
      bool hasParents(Particle particle) const {
        GenVertex* gv = particle.genParticle()->production_vertex();
        for (GenVertex::particle_iterator it = gv->particles_begin(HepMC::parents); it != gv->particles_end(HepMC::parents); ++it) {
          return true;
        }
        return false;
      }
  };
  
  // @brief Pseudo top finder
  // 
  // Find top quark in the particle level.
  // The definition is based on the agreement at the LHC working group.
  class PseudoTop : public FinalState {
    public:
      /// @name Standard constructors and destructors.
      //@{

      /// The default constructor. May specify the minimum and maximum
      /// pseudorapidity \f$ \eta \f$ and the min \f$ p_T \f$ (in GeV).
      PseudoTop(const edm::ParameterSet& pset)
      : FinalState(-MAXDOUBLE, MAXDOUBLE, 0*GeV),
      
      _lepR(pset.getParameter<double>("leptonConeSize")),
      _lepMinPt(pset.getParameter<double>("minLeptonPt")),
      _lepMaxEta(pset.getParameter<double>("maxLeptonEta")),
      
      _jetR(pset.getParameter<double>("jetConeSize")),
      _jetMinPt(pset.getParameter<double>("minJetPt")),
      _jetMaxEta(pset.getParameter<double>("maxJetEta")),

      _minLeptonPtDilepton(pset.getParameter<double>("minLeptonPtDilepton")),
      _maxLeptonEtaDilepton(pset.getParameter<double>("maxLeptonEtaDilepton")),
      _minDileptonMassDilepton(pset.getParameter<double>("minDileptonMassDilepton")),

      _minLeptonPtSemilepton(pset.getParameter<double>("minLeptonPtSemilepton")),
      _maxLeptonEtaSemilepton(pset.getParameter<double>("maxLeptonEtaSemilepton")),
      _minVetoLeptonPtSemilepton(pset.getParameter<double>("minVetoLeptonPtSemilepton")),
      _maxVetoLeptonEtaSemilepton(pset.getParameter<double>("maxVetoLeptonEtaSemilepton")),
      _minMETSemiLepton(pset.getParameter<double>("minMETSemiLepton")),
      _minMtWSemiLepton(pset.getParameter<double>("minMtWSemiLepton")),
      
      _tMass(pset.getParameter<double>("tMass")),
      _wMass(pset.getParameter<double>("wMass"))
      {
        setName("PseudoTop");
        
        // Cuts
        Cut particle_cut = (Cuts::abseta < 6.0) and (Cuts::pT > 0.0*MeV);
        Cut lepton_cut   = (Cuts::abseta < _lepMaxEta) and (Cuts::pT > _lepMinPt*GeV);
        
        // Generic final state
        FinalState fs(Cuts::abseta < 5.);
        
        // Dressed leptons
        ChargedLeptons charged_leptons(fs);
        IdentifiedFinalState photons(fs);
        photons.acceptIdPair(PID::PHOTON);
        //HasLeptonParentFinalState photonsFromLepton(photons);
        DressedLeptons dressed_leptons(photons, charged_leptons, _lepR, lepton_cut, true, false);
        addProjection(dressed_leptons, "DressedLeptons");
        
        // Jets
        VetoedFinalState fsForJets(fs);
        fsForJets.addVetoOnThisFinalState(dressed_leptons);
        addProjection(FastJets(fsForJets, FastJets::ANTIKT, _jetR), "Jets");
        
        // Neutrinos
        IdentifiedFinalState neutrinos(fs);
        neutrinos.acceptNeutrinos();
        addProjection(neutrinos, "Neutrinos");
        
        // MET
        addProjection(MissingMomentum(fs), "MET");
      }

      enum TTbarMode {CH_NONE=-1, CH_FULLHADRON = 0, CH_SEMILEPTON, CH_FULLLEPTON};
      enum DecayMode {CH_HADRON = 0, CH_MUON, CH_ELECTRON};

      TTbarMode mode() const {
        if (!_isValid) return CH_NONE;
        if (_mode1 == CH_HADRON && _mode2 == CH_HADRON) return CH_FULLHADRON;
        else if ( _mode1 != CH_HADRON && _mode2 != CH_HADRON) return CH_FULLLEPTON;
        else return CH_SEMILEPTON;
      }
      DecayMode mode1() const {return _mode1;}
      DecayMode mode2() const {return _mode2;}
      
      /// Clone on the heap.
      /*virtual std::unique_ptr<Projection> clone() const {
        return std::unique_ptr<Projection>(new PseudoTop(*this));
      }*/
      virtual Projection* clone() const override { return new PseudoTop(*this); }

      //@}

    public:
      Particle t1() const {return _t1;}
      Particle t2() const {return _t2;}
      Particle b1() const {return _b1;}
      Particle b2() const {return _b2;}
      Particle w1() const {return _w1;}
      Particle w2() const {return _w2;}
      ParticleVector wDecays1() const {return _wDecays1;}
      ParticleVector wDecays2() const {return _wDecays2;}
      vector<DressedLepton> leptons() const {return _leptons;}
      ParticleVector neutrinos() const {return _neutrinos;}
      Jets jets() const {return _jets;}
      Jets bjets() const {return _bjets;}
      Jets ljets() const {return _ljets;}
      Vector3 met() const {return _met;}

    protected:
      // Apply the projection to the event
      void project(const Event& event) override;

    private:
      const double _lepR, _lepMinPt, _lepMaxEta;
      const double _jetR, _jetMinPt, _jetMaxEta;

      const double _minLeptonPtDilepton, _maxLeptonEtaDilepton;
      const double _minDileptonMassDilepton;
      const double _minLeptonPtSemilepton, _maxLeptonEtaSemilepton;
      const double _minVetoLeptonPtSemilepton, _maxVetoLeptonEtaSemilepton;
      const double _minMETSemiLepton, _minMtWSemiLepton;

      double _tMass = 172.5;
      double _wMass = 80.4;

    private:
      bool _isValid;
      DecayMode _mode1, _mode2;

      Particle _t1, _t2;
      Particle _b1, _b2;
      Particle _w1, _w2;
      ParticleVector _wDecays1, _wDecays2;
      
      vector<DressedLepton> _leptons;
      ParticleVector _neutrinos;
      Jets _jets, _bjets, _ljets;
      Vector3 _met;

  };

}

#endif
