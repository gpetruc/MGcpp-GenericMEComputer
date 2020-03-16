#ifndef MGcpp_GenericMEComputer_MEMultiWeightTool_h
#define MGcpp_GenericMEComputer_MEMultiWeightTool_h

#include <memory>
#include "MGcpp/GenericMEComputer/interface/SLHAInfo.h"
#include "MGcpp/GenericMEComputer/interface/ProcessCollection.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class MEMultiWeightTool {
    public:
        MEMultiWeightTool(const std::string & processCollection, const std::string & refSLHA_, const std::vector<edm::ParameterSet> & scanPoints) ;
        MEMultiWeightTool(const std::string & processCollection, const std::string & refSLHA_) ;
        ~MEMultiWeightTool();

        double evalRef(double alphaS, const std::vector<int> & pdgIds, const std::vector <double *> & momenta, bool permuteFS=false) ;
        std::vector<double> evalAll(double alphaS, const std::vector<int> & pdgIds, const std::vector <double *> & momenta, bool relative=true, bool permuteFS=false) ;

        unsigned int size() const { return workers_.size(); }
        const std::string & name(unsigned int i) const { return workers_[i].name; }
        const std::string & doc(unsigned int i) const { return workers_[i].doc; }

        const ProcessCollection & proc(unsigned int i=0) const { return *workers_[i].me; }
        const SLHAInfo & slha(unsigned int i=0) const { return workers_[i].slha; }

        // for programmatic modifications
        void addPoint() ;
        void addPoints(unsigned int n) { 
            if (n > size()*4) workers_.reserve(n);
            for (unsigned int i = 0; i < n; ++i) addPoint();
        }
        void setNameAndDoc(unsigned int i, const std::string & name, const std::string & doc) { 
            workers_[i].name = name; workers_[i].doc = doc; 
        }
        SLHAInfo & slha(unsigned int i) { workers_[i].isInit = false; return workers_[i].slha; }

    private:
        std::string processCollection_, slha_;

        struct Worker {
            Worker(const std::string & n, ProcessCollection *p) : name(n), doc(), slha(), me(p), isInit(false) {} 
            Worker(const std::string & n, std::unique_ptr<ProcessCollection> &&p) : name(n), doc(), slha(), me(std::move(p)), isInit(false) {} 
            Worker(const std::string & n, const std::string & slha_file, ProcessCollection *p) : name(n), doc(), slha(), me(p), isInit(false) { slha.read_slha_file(slha_file); } 
            Worker(const std::string & n, const std::string & slha_file, std::unique_ptr<ProcessCollection> &&p) : name(n), doc(), slha(), me(std::move(p)), isInit(false) { slha.read_slha_file(slha_file); } 
            Worker(const std::string & n, const SLHAInfo & slha_info, ProcessCollection * p) : name(n), slha(slha_info), me(p), isInit(false) { } 
            Worker(const std::string & n, const SLHAInfo & slha_info, std::unique_ptr<ProcessCollection> && p) : name(n), slha(slha_info), me(std::move(p)), isInit(false) { } 
            std::string name, doc;
            SLHAInfo slha;
            std::unique_ptr<ProcessCollection> me;
            bool isInit;
            void init(double alphaS) {
                if (!isInit) { me->initIndependent(slha); isInit = true; }
                me->initDependent(alphaS);
            }
            double eval(const std::vector<int> & pdgIds, const std::vector <double *> & momenta) { return me->sigma(pdgIds, momenta); } 
        };
        std::vector<Worker> workers_;

};

#endif
