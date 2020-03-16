#include "MGcpp/GenericMEComputer/interface/MEMultiWeightTool.h"
#include "MGcpp/GenericMEComputer/interface/ProcessCollectionFactory.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <sstream>
#include <iomanip>

MEMultiWeightTool::MEMultiWeightTool(const std::string & processCollection, const std::string & refSLHA, const std::vector<edm::ParameterSet> & scanPoints) :
    processCollection_(processCollection), slha_(refSLHA)
{
    workers_.reserve(scanPoints.size()+1);
    workers_.emplace_back("SM", slha_, ProcessCollectionFactory::get()->create(processCollection_));   

    for (const auto & pset : scanPoints) {
        workers_.emplace_back(pset.getParameter<std::string>("name"), workers_.front().slha, ProcessCollectionFactory::get()->create(processCollection_));
        auto & slha = workers_.back().slha;
        std::stringstream doc; doc << std::setprecision(8) << std::scientific;
        //std::cout << pset.getParameter<std::string>("name") << " created" << std::endl;
        for (const auto & pcoup : pset.getParameter<std::vector<edm::ParameterSet>>("params")) {
            std::string block = pcoup.getParameter<std::string>("block");
            if (pcoup.existsAs<std::string>("name")) {
                slha.set_block_entry(block, pcoup.getParameter<std::string>("name"), pcoup.getParameter<double>("value"));
                doc << block << "." << pcoup.getParameter<std::string>("name") << " = " << pcoup.getParameter<double>("value") << ";  ";
                //std::cout << "  " << pset.getParameter<std::string>("name") << ": set " << block << "." << pcoup.getParameter<std::string>("name") << " = " << pcoup.getParameter<double>("value") << std::endl;
            } else if (pcoup.existsAs<int32_t>("index")) {
                slha.set_block_entry(block, pcoup.getParameter<int32_t>("index"), pcoup.getParameter<double>("value"));
                doc << block << "[" << pcoup.getParameter<int32_t>("index") << "] = " << pcoup.getParameter<double>("value") << ";  ";
                //std::cout << "  " << pset.getParameter<std::string>("name") << ": set " << block << "." << pcoup.getParameter<int32_t>("index") << " = " << pcoup.getParameter<double>("value") << std::endl;
            } else  {
                throw cms::Exception("Configuration") << "Bad config for " << pset.getParameter<std::string>("name");
            }
        }
        workers_.back().doc = doc.str();
    }
}

MEMultiWeightTool::MEMultiWeightTool(const std::string & processCollection, const std::string & refSLHA) :
    processCollection_(processCollection), slha_(refSLHA)
{
    workers_.emplace_back("SM", slha_, ProcessCollectionFactory::get()->create(processCollection_));   
}

MEMultiWeightTool::~MEMultiWeightTool()
{
}

double MEMultiWeightTool::evalRef(double alphaS, const std::vector<int> & pdgIds, const std::vector <double *> & momenta, bool permuteFS) 
{
    auto & ref = workers_.front();
    ref.init(alphaS);
    return ref.eval(pdgIds, momenta);
}

std::vector<double> MEMultiWeightTool::evalAll(double alphaS, const std::vector<int> & pdgIds, const std::vector <double *> & momenta, bool relative, bool permuteFS) 
{
    std::vector<double> vals; 
    unsigned int n = size();
    for (auto & w : workers_) w.init(alphaS);
    if (relative) {
        vals.resize(n-1);
        double ref = workers_.front().eval(pdgIds, momenta);    
        if (ref <= 0) edm::LogWarning("MEMultiWeightTool") << "Evaluating reference ME for " << processCollection_ << " init from " << slha_ << ": ME is " << ref << std::endl; 
        for (unsigned int i = 1; i < n; ++i) {
            vals[i-1] = (ref > 0) ? workers_[i].eval(pdgIds, momenta)/ref : 0;
        }
    } else {
        vals.resize(n);
        for (unsigned int i = 0; i < n; ++i) {
            vals[i] = workers_[i].eval(pdgIds, momenta);
        }
    }
    return vals;
}

void MEMultiWeightTool::addPoint() 
{
    workers_.emplace_back("", workers_.front().slha, ProcessCollectionFactory::get()->create(processCollection_));
}
