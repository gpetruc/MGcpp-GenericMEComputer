#ifndef MGcpp_GenericMEComputer_MEInitFromLHE_h
#define MGcpp_GenericMEComputer_MEInitFromLHE_h

#include <vector>
#include <algorithm>
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"

class MEInitFromLHE {
    public:
        MEInitFromLHE() : motherParticle_(0), undecayedIds_(), leptonIDSquash_(0) {}
    
        static MEInitFromLHE ProductionME() { return MEInitFromLHE(); }
        static MEInitFromLHE ProductionME(const std::vector<int> & undecayedIds) { return MEInitFromLHE(0,undecayedIds); }
        static MEInitFromLHE DecayME(int motherParticle) { return MEInitFromLHE(motherParticle, std::vector<int>()); }
        static MEInitFromLHE DecayME(int motherParticle, const std::vector<int> & undecayedIds) { return MEInitFromLHE(motherParticle, undecayedIds); }

        // if zero, do nothing
        // if 11 or 13, this is the preferred pdgId for leptons in input:
        // - if there's only one lepton kind in the input, make it the preferred one
        // - if there's two, make one a muon and the other an electron
        void setLeptonIDSquash(int pdgId=13) ;

        bool readLHE(const LHEEventProduct & lhe);
        void printLHE(const LHEEventProduct & lhe);

        double alphaS() const { return alphaS_; }
        const std::vector<int> & pdgIds() const { return pdgIds_; }
        const std::vector<double*> & p4s() const { return p4s_; }

    private:
        MEInitFromLHE(int motherParticle, const std::vector<int> & undecayedIds) :
            motherParticle_(motherParticle), undecayedIds_(undecayedIds), leptonIDSquash_(0) {}

        // config data 
        int motherParticle_;
        std::vector<int> undecayedIds_;
        int leptonIDSquash_;

        // event data
        double alphaS_;
        std::vector<int> pdgIds_;
        std::vector<double*> p4s_;
        std::vector<double>  p4data_; // holds the doubles pointed to by p4s_;
       
        // utilities 
        void pickParticle_(const lhef::HEPEUP &hepeup, int i) ;
        bool readDecay_();
        void recursivePickDaughters_(const lhef::HEPEUP &hepeup, int imom);
        bool has_(const std::vector<int> & v, int id) const { 
            return (std::find(v.begin(), v.end(), id) != v.end());
        }
        void makeP4s_();
};
 
#endif
