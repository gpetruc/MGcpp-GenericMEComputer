#include "MGcpp/GenericMEComputer/interface/MEInitFromLHE.h"
#include <cstdio>
#include <iostream>

void MEInitFromLHE::printLHE(const LHEEventProduct & lhe) {
    const lhef::HEPEUP &hepeup = lhe.hepeup();
    for (int i = 0; i < hepeup.NUP; ++i) {
        printf("Particle %2d  pdgId %+4d  status %+2d xyzem ",
                    i, hepeup.IDUP[i], hepeup.ISTUP[i]);
        for (int j = 0; j < 5; ++j) {
            printf("%+14.9e  ", hepeup.PUP[i][j]);
        }
        printf("  mothers %2d  %2d\n", hepeup.MOTHUP[i].first-1, hepeup.MOTHUP[i].second-1);
    }
}

void MEInitFromLHE::printSelected() {
     for (unsigned int i = 0, n = pdgIds_.size(); i < n; ++i) {
        printf("Selected particle %2d  pdgId %+4d   xyze  %+14.9e  %+14.9e  %+14.9e  %+14.9e\n",
            i, pdgIds_[i], p4s_[i][1], p4s_[i][2], p4s_[i][3], p4s_[i][0]);
     }
}

bool MEInitFromLHE::readLHE(const LHEEventProduct & lhe) 
{
    pdgIds_.clear(); p4data_.clear(); p4s_.clear();
    const lhef::HEPEUP &hepeup = lhe.hepeup();
    alphaS_ = hepeup.AQCDUP;
    bool ret = true;
    if (motherParticle_ != 0) {
        ret = false;
        for (int i = 0; i < hepeup.NUP; ++i) {
            if (hepeup.IDUP[i] == motherParticle_) {
                ret = true; 
                pickParticle_(hepeup, i);
                recursivePickDaughters_(hepeup, i);
                break;
            }
        }
    } else {
        std::vector<int> vetoMoms;
        for (int i = 0; i < hepeup.NUP; ++i) { 
            if (hepeup.ISTUP[i] == -1) {
                pickParticle_(hepeup, i);
            } else if (has_(vetoMoms, hepeup.MOTHUP[i].first-1)) {
                if (hepeup.ISTUP[i] == 2) vetoMoms.push_back(i);
            } else if (hepeup.ISTUP[i] == 2) {
                if (has_(undecayedIds_, std::abs(hepeup.IDUP[i]))) {
                    pickParticle_(hepeup, i);
                    vetoMoms.push_back(i+1);
                }
            } else if (hepeup.ISTUP[i] == +1) {
                pickParticle_(hepeup, i);
            }
        }
    }
    makeP4s_();
    return ret;
}

void MEInitFromLHE::pickParticle_(const lhef::HEPEUP &hepeup, int i) 
{
    pdgIds_.push_back(hepeup.IDUP[i]);
    p4data_.push_back(hepeup.PUP[i][3]);
    for (unsigned int j = 0; j < 3; ++j) {
        p4data_.push_back(hepeup.PUP[i][j]);
    } 
}

void MEInitFromLHE::recursivePickDaughters_(const lhef::HEPEUP &hepeup, int imom) {
    for (int i = imom+1; i < hepeup.NUP; ++i) { 
        if (hepeup.MOTHUP[i].first-1 != imom) continue;
        if (hepeup.ISTUP[i] == 2) {
            if (has_(undecayedIds_, std::abs(hepeup.IDUP[i]))) {
                pickParticle_(hepeup, i);
            } else {
                recursivePickDaughters_(hepeup, i);
            }
        } else {
            pickParticle_(hepeup, i);
        }
    }
}

void MEInitFromLHE::makeP4s_()
{
    p4s_.resize(pdgIds_.size());
    for (unsigned int i = 0, n = pdgIds_.size(); i < n; ++i) {
        p4s_[i] = & p4data_[4*i];
    }
}
