#ifndef ProcessCollection_H
#define ProcessCollection_H

#include <map>
#include <vector>
#include <cassert>
#include <memory>
#include <iostream>
#include <iomanip>
#include <boost/ptr_container/ptr_vector.hpp>

#include "MGcpp/GenericMEComputer/interface/SLHAInfo.h"

class ProcessCollection
{
  public:
    ProcessCollection() ;
    virtual ~ProcessCollection() ;

    virtual void initIndependent(const SLHAInfo & info) = 0;
    virtual void initDependent(double aS) = 0;

    virtual double sigma(const std::vector<int> & pdgIds, const std::vector <double *> & momenta, int subprocess=-1) = 0;
    double sigmaSumPermutations(const std::vector<int> & pdgIds, const std::vector <double *> & momenta, int subprocess=-1) ;

    virtual int nInitial(int subprocess=-1) const = 0;
    virtual int nExternal(int subprocess=-1) const = 0;
    virtual int nProcesses(int subprocess=-1) const = 0;
};

template<typename Parameters, typename ProcessBase> 
class ProcessCollectionT : public ProcessCollection {
    public:
        ProcessCollectionT() {}
        ~ProcessCollectionT() override {}

        void initIndependent(const SLHAInfo & info) override {
            if (!params_) params_.reset(new Parameters());
            params_->setIndependentParameters(info);
            params_->setIndependentCouplings(); 
            //params_->printIndependentParameters(); 
            //params_->printIndependentCouplings();  
            for (auto & p : processes_) {
                p.second.initBlock(*params_);
            }
        }

        void initDependent(double aS) override {
            params_->aS = aS;
            params_->setDependentParameters(); 
            params_->setDependentCouplings(); 
            //params_->printDependentParameters(); 
            //params_->printDependentCouplings(); 
        }

        double sigma(const std::vector<int> & pdgIds, const std::vector <double *> & momenta, int subprocess=-1) override ;

        int nInitial(int subprocess=-1) const override {
            if (subprocess == -1) {
                int ret = -1;
                for (const auto & p : processes_) {
                    if (ret == -1) ret = p.second.nInitial;
                    else assert(ret == p.second.nInitial);
                }
                return ret;
            } else {
                auto match = processes_.find(subprocess);
                return (match == processes_.end() ? -1 : match->second.nInitial);
            }
        }

        int nExternal(int subprocess=-1) const override {
            if (subprocess == -1) {
                int ret = -1;
                for (const auto & p : processes_) {
                    if (ret == -1) ret = p.second.nExternal;
                    else assert(ret == p.second.nExternal);
                }
                return ret;
            } else {
                auto match = processes_.find(subprocess);
                return (match == processes_.end() ? -1 : match->second.nExternal);
            }
        }

        int nProcesses(int subprocess=-1) const override {
            if (subprocess == -1) {
                int ret = 0;
                for (const auto & p : processes_) {
                    ret += p.second.nProcesses;
                }
                return ret;
            } else {
                auto match = processes_.find(subprocess);
                return (match == processes_.end() ? -1 : match->second.nProcesses);
            }
        }

    protected:
        struct ProcessBlock {
            ProcessBlock() : nInitial(-1), nExternal(-1), nProcesses(0) {}

            boost::ptr_vector<ProcessBase> subprocesses;
            int nInitial, nExternal, nProcesses;

            void initBlock(const Parameters &pars) { 
                for (auto & s : subprocesses) s.initProc(pars);
            }

            double sigma(const std::vector<int> & pdgIds, const std::vector < double * > & momenta) {
                assert(pdgIds.size() == momenta.size());
                assert(nInitial <= 2);
                double ret = 0;
                if (int(pdgIds.size()) == nExternal) {
                    for (auto & s : subprocesses) {
                        //std::cout << "Trying process " << s.code() << " " << s.name() << std::endl;
                        if (!s.matchParticles(pdgIds)) continue;
                        s.setInitial(pdgIds[0], pdgIds[1]);
                        s.setMomenta(momenta);
                        s.sigmaKin(); // compute
                        //std::cout << "process " << s.code() << " " << s.name() << " -->  " << std::scientific << std::setprecision(6) << s.sigmaHat() << std::endl;
                        ret += s.sigmaHat();
                    }
                } else {
                    //std::cout << "Process block " << subprocesses.front().code() << " skipped as it has nExternal = " << nExternal << " while input has " << pdgIds.size() << " particles." << std::endl;
                }
                return ret;
            }
            void addProcess(ProcessBase *p) {
                nProcesses++;
                if (nInitial == -1) { 
                    nInitial = p->nInitial();
                    nExternal = p->nExternal();
                } else {
                    assert(nInitial == p->nInitial());
                    assert(nExternal = p->nExternal());
                }
                subprocesses.push_back(p);
            }
        };

        std::unique_ptr<Parameters> params_;
        std::map<int, ProcessBlock> processes_;

        void addProcess(ProcessBase *p) {
            processes_[p->code()].addProcess(p);
        }
};

template<typename Parameters, typename ProcessBase>
double ProcessCollectionT<Parameters,ProcessBase>::sigma(const std::vector<int> & pdgIds, const std::vector < double * > & momenta, int subprocess) {
    if (subprocess == -1) {
        double ret = 0;
        for (auto & p : processes_) {
            ret += p.second.sigma(pdgIds, momenta);
        }
        return ret;
    } else {
        auto match = processes_.find(subprocess);
        return (match == processes_.end() ? -1 : match->second.sigma(pdgIds, momenta));
    }
}

#endif
