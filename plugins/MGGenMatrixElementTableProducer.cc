#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "FWCore/Utilities/interface/transform.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"

#include "MGcpp/GenericMEComputer/interface/MEMultiWeightTool.h"
#include "MGcpp/GenericMEComputer/interface/ProcessCollectionFactory.h"

namespace {

class MGGenMatrixElementTableProducer : public edm::stream::EDProducer<> {
    public:
        MGGenMatrixElementTableProducer( edm::ParameterSet const & iConfig )  ;
        ~MGGenMatrixElementTableProducer() override {} // nothing to do

        void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override ;

    protected:
        const std::vector<edm::InputTag> lheLabel_;
        const std::vector<edm::EDGetTokenT<LHEEventProduct>> lheTag_;

        const std::string name_;
        MEMultiWeightTool meTool_;

     private:
}; // class


MGGenMatrixElementTableProducer::MGGenMatrixElementTableProducer( edm::ParameterSet const & iConfig ) :
    lheLabel_(iConfig.getParameter<std::vector<edm::InputTag>>("lheInfo")),
    lheTag_(edm::vector_transform(lheLabel_, [this](const edm::InputTag & tag) { return mayConsume<LHEEventProduct>(tag); })),
    name_(iConfig.getParameter<std::string>("name")),
    meTool_(iConfig.getParameter<std::string>("processCollection"), 
            iConfig.getParameter<std::string>("slha"), 
            iConfig.getParameter<std::vector<edm::ParameterSet>>("scanPoints"))
{
    produces<nanoaod::FlatTable>();
}


void MGGenMatrixElementTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    auto out = std::make_unique<nanoaod::FlatTable>(1, "LHE_"+name_, true);
    out->setDoc("MG weights at LHE level");

    edm::Handle<LHEEventProduct> lheInfo;
    for (const auto & lheTag: lheTag_) {
        iEvent.getByToken(lheTag, lheInfo);
        if (lheInfo.isValid()) {
            break;
        }
    }

    unsigned int nExt = meTool_.proc().nExternal();
    std::vector<double>  p4unroll(nExt * 4);
    std::vector<double*> p4s(nExt);
    for (unsigned int i = 0; i < nExt; ++i) {
        p4s[i] = &p4unroll[4*i];
    }
    std::vector<int> ids;

    const auto & hepeup = lheInfo->hepeup();
    const auto & pup = hepeup.PUP;
    for (unsigned int i = 0, k = 0, n = pup.size(); i  < n; ++i) {
        /*std::cout << "Particle " << i << "  pdgId " << std::showpos << std::setw(3) << hepeup.IDUP[i] << " status " << std::setw(2) << hepeup.ISTUP[i] << 
            std::scientific << std::setprecision(10) << 
            "   XYZEM  " << pup[i][0] << "   " << pup[i][1]  << "   " << pup[i][2]  << "   " << pup[i][3]  << "   " << pup[i][4] <<
            std::fixed << std::setprecision(8) <<
            std::endl; */
        if (std::abs(hepeup.ISTUP[i]) != 1) continue;
        p4s[k][0] = pup[i][3]; // E
        for (unsigned int j = 0; j < 3; ++j) {
            p4s[k][j+1] = pup[i][j]; // PX PY PZ
        }
        
        ids.push_back(hepeup.IDUP[i]);
        k++;
    }
    
    double alphaS = hepeup.AQCDUP;
    std::vector<double> vals = meTool_.evalAll(alphaS, ids, p4s, true);
    for (unsigned int i = 0, n = vals.size(); i < n; ++i) {
        out->addColumnValue<float>(meTool_.name(i+1), vals[i], meTool_.doc(i+1), nanoaod::FlatTable::FloatColumn);
    }

    iEvent.put(std::move(out));
}

} // namespace

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(MGGenMatrixElementTableProducer);

