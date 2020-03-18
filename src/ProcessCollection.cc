#include "MGcpp/GenericMEComputer/interface/ProcessCollection.h"

#include <algorithm>

ProcessCollection::ProcessCollection()
{
}

ProcessCollection::~ProcessCollection()
{
}

double ProcessCollection::sigmaSumPermutations(const std::vector<int> & pdgIds, const std::vector <double *> & momenta, int subprocess) 
{
    assert(pdgIds.size() == momenta.size());
    unsigned int nFirst = nInitial(subprocess), nExt = pdgIds.size(); // subprocesses may have different number of legs in case of X+jets reweighting
    std::vector<int> iperm(nExt-nFirst);
    std::vector<int> permPdgIds(pdgIds);
    std::vector<double *> permMomenta(momenta);
    for (unsigned int i = 0; i < iperm.size(); ++i) {
        iperm[i] = nFirst+i;
    }
    double sum = 0;
    do {
        for (unsigned int i = 0; i < iperm.size(); ++i) {
            permPdgIds[nFirst+i]  = pdgIds[iperm[i]];
            permMomenta[nFirst+i] = momenta[iperm[i]];
        }
        sum += sigma(permPdgIds, permMomenta, subprocess);
    } while(std::next_permutation(iperm.begin(), iperm.end()));
    return sum;
}
