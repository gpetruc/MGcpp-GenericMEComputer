#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "FWCore/Utilities/interface/transform.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <regex>
#include <algorithm>
#include <cctype>
#include <string>


#include "MGcpp/GenericMEComputer/interface/SLHAInfo.h"
#include "MGcpp/GenericMEComputer/interface/ProcessCollectionFactory.h"
//#include "MGcpp/GenericMEComputer/src/WpH_SMEFTsimB/main.h"

namespace {

std::string to_lower(const std::string & str) {
    std::string data(str);
    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c){ return std::tolower(c); });
    return data;
}


class MGGenMatrixElementTableProducer : public edm::stream::EDProducer<> {
    public:
        MGGenMatrixElementTableProducer( edm::ParameterSet const & iConfig )  ;
        ~MGGenMatrixElementTableProducer() override {} // nothing to do

        void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override ;

    protected:
        const std::vector<edm::InputTag> lheLabel_;
        const std::vector<edm::EDGetTokenT<LHEEventProduct>> lheTag_;

        const std::string name_, processCollection_, slha_;

        struct Worker {
            Worker(const std::string & n, ProcessCollection *p) : name(n), doc(), slha(), me(p) {} 
            Worker(const std::string & n, std::unique_ptr<ProcessCollection> &&p) : name(n), doc(), slha(), me(std::move(p)) {} 
            Worker(const std::string & n, const std::string & slha_file, ProcessCollection *p) : name(n), doc(), slha(), me(p) { slha.read_slha_file(slha_file); } 
            Worker(const std::string & n, const std::string & slha_file, std::unique_ptr<ProcessCollection> &&p) : name(n), doc(), slha(), me(std::move(p)) { slha.read_slha_file(slha_file); } 
            Worker(const std::string & n, const SLHAInfo & slha_info, ProcessCollection * p) : name(n), slha(slha_info), me(p) { } 
            Worker(const std::string & n, const SLHAInfo & slha_info, std::unique_ptr<ProcessCollection> && p) : name(n), slha(slha_info), me(std::move(p)) { } 
            std::string name, doc;
            SLHAInfo slha;
            std::unique_ptr<ProcessCollection> me;
        };
        std::vector<Worker> workers;
    private:
}; // class


MGGenMatrixElementTableProducer::MGGenMatrixElementTableProducer( edm::ParameterSet const & iConfig ) :
    lheLabel_(iConfig.getParameter<std::vector<edm::InputTag>>("lheInfo")),
    lheTag_(edm::vector_transform(lheLabel_, [this](const edm::InputTag & tag) { return mayConsume<LHEEventProduct>(tag); })),
    name_(iConfig.getParameter<std::string>("name")),
    processCollection_(iConfig.getParameter<std::string>("processCollection")),
    slha_(iConfig.getParameter<std::string>("slha"))
{
    produces<nanoaod::FlatTable>();
    workers.emplace_back("SM", slha_, ProcessCollectionFactory::get()->create(processCollection_));

    for (const auto & pset : iConfig.getParameter<std::vector<edm::ParameterSet>>("scanPoints")) {
        workers.emplace_back(pset.getParameter<std::string>("name"), workers.front().slha, ProcessCollectionFactory::get()->create(processCollection_));
        auto & slha = workers.back().slha;
        //std::cout << pset.getParameter<std::string>("name") << " created" << std::endl;
        for (const auto & pcoup : pset.getParameter<std::vector<edm::ParameterSet>>("params")) {
            std::string block = to_lower(pcoup.getParameter<std::string>("block"));
            if (pcoup.existsAs<std::string>("name")) {
                slha.set_block_entry(block, to_lower(pcoup.getParameter<std::string>("name")), pcoup.getParameter<double>("value"));
                //std::cout << "  " << pset.getParameter<std::string>("name") << ": set " << block << "." << to_lower(pcoup.getParameter<std::string>("name")) << " = " << pcoup.getParameter<double>("value") << std::endl;
            } else if (pcoup.existsAs<int32_t>("index")) {
                slha.set_block_entry(block, pcoup.getParameter<int32_t>("index"), pcoup.getParameter<double>("value"));
                //std::cout << "  " << pset.getParameter<std::string>("name") << ": set " << block << "." << pcoup.getParameter<int32_t>("index") << " = " << pcoup.getParameter<double>("value") << std::endl;
            } else  {
                throw cms::Exception("Configuration") << "Bad config for " << pset.getParameter<std::string>("name");
            }
        }
    }
    // now call init
    for (auto & w : workers) {
        w.me->initIndependent(w.slha);
    }
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

    auto & sm = * workers.front().me;
    std::vector<double>  p4unroll(sm.nExternal() * 4);
    std::vector<double*> p4s(sm.nExternal());
    for (int i = 0; i < sm.nExternal(); ++i) {
        p4s[i] = &p4unroll[4*i];
    }
    std::vector<int> ids;
    double alphaS;

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
    
    alphaS = hepeup.AQCDUP;

    double ref = -1;
    for (unsigned int i = 0, n = workers.size(); i < n; ++i) {
        auto & me = workers[i].me;
        me->initDependent(alphaS);
        double ret = me->sigma(ids, p4s);
        if (i == 0) {
            ref = ret;
            if (ref == 0) std::cout << "Warning: ref ME is ZERO" << std::endl;
        } else {
            std::cout << "ME " << workers[i].name << "  value = " <<  std::scientific << ret << " (relative: " << std::fixed << std::setprecision(8) << (ref > 0 ? ret/ref : -1) << ")" << std::endl;
            out->addColumnValue<float>(workers[i].name, (ref > 0 ? ret/ref : -1), workers[i].doc, nanoaod::FlatTable::FloatColumn);
        }
    }

    iEvent.put(std::move(out));
}

} // namespace

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(MGGenMatrixElementTableProducer);

