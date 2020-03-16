#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "FWCore/Utilities/interface/transform.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"

#include "MGcpp/GenericMEComputer/interface/MEMultiWeightTool.h"
#include "MGcpp/GenericMEComputer/interface/MEInitFromLHE.h"
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
        MEInitFromLHE lhe2me_;
        MEMultiWeightTool meTool_;

     private:
}; // class


MGGenMatrixElementTableProducer::MGGenMatrixElementTableProducer( edm::ParameterSet const & iConfig ) :
    lheLabel_(iConfig.getParameter<std::vector<edm::InputTag>>("lheInfo")),
    lheTag_(edm::vector_transform(lheLabel_, [this](const edm::InputTag & tag) { return mayConsume<LHEEventProduct>(tag); })),
    name_(iConfig.getParameter<std::string>("name")),
    lhe2me_(),
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

    if (lhe2me_.readLHE(*lheInfo)) {
        std::vector<double> vals = meTool_.evalAll(lhe2me_.alphaS(), lhe2me_.pdgIds(), lhe2me_.p4s(), true);
        MEInitFromLHE lhe2me_;
        for (unsigned int i = 0, n = vals.size(); i < n; ++i) {
            out->addColumnValue<float>(meTool_.name(i+1), vals[i], meTool_.doc(i+1), nanoaod::FlatTable::FloatColumn);
        }
    } else {
        edm::LogWarning("MGGenMatrixElementTableProducer") << "Could not initialize ME info from LE.";
        for (unsigned int i = 0, n = meTool_.size()-1; i < n; ++i) {
            out->addColumnValue<float>(meTool_.name(i+1), 0, meTool_.doc(i+1), nanoaod::FlatTable::FloatColumn);
        }
    }

    iEvent.put(std::move(out));
}

} // namespace

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(MGGenMatrixElementTableProducer);

