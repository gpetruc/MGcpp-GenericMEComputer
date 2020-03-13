#!/usr/bin/env python
import sys, re, os

from optparse import OptionParser
parser = OptionParser(usage="usage: %prog name srcdir")
parser.add_option("-o", "--out", dest="out", default=None, help="Output dir")
(options, args) = parser.parse_args()
if len(args) != 2 or (not os.path.isdir(args[1])) or os.path.isdir(args[0]):
    parser.print_usage()
    exit(1)

name, srcdir = args
outdir = options.out if options.out else name+".dir";


src = dict((k,[ f for f in os.listdir(srcdir+"/src") if os.path.isfile(srcdir+"/src/"+f) and f.endswith("."+k)]) for k in ("h","cc") )
print "Source files: ", src

model = [f for f in src['h'] if f.startswith("HelAmps_") ][0][len("HelAmps_"):-2]
print "Model: ", model

subprocesses = [ d for d in os.listdir(srcdir+"/SubProcesses") if os.path.isdir(srcdir+"/SubProcesses/"+d) and re.match(r"P\d+_.*",d) ]
print "Number of subprocesses: ",len(subprocesses)


PDGIDs = dict(h=25, g=21, a=22, z=23)
PDGIDs["w+"] = +24; PDGIDs["w-"] = -24
for i,q in enumerate("duscbt"): 
    PDGIDs[q]     = +(i+1)
    PDGIDs[q+"~"] = -(i+1)
for i,l in enumerate("e ve mu vm ta vt".split()):
    p,ap = ("","~") if l[0] == "v" else ("-","+")
    PDGIDs[l+p ] = +(i+11)
    PDGIDs[l+ap] = -(i+11)

def copy_helamps(srcdir,outdir,name,model):
    oldguard = "HelAmps_%s_H" % model
    newguard = "%s_HelAmps_H" % name
    newns = "MGME_"+name
    oldns = "MG5_"+model
    # start with h file
    fin  = "%s/src/%s_%s.h" % (srcdir,"HelAmps",model)
    fout = "%s/%s_%s.h" % (outdir,name,"HelAmps")
    sout = open(fout,'w');
    for line in open(fin, 'r'):
        line = line.replace(oldguard, newguard)
        line = line.replace("namespace "+oldns, "namespace "+newns)
        sout.write(line)
    print "Done "+fout
    sout.close()
    # now cc file
    fin  = "%s/src/%s_%s.cc" % (srcdir,"HelAmps",model)
    fout = "%s/%s_%s.cc" % (outdir,name,"HelAmps")
    sout = open(fout,'w');
    oldinc = "HelAmps_%s.h" % model
    newinc = "%s_HelAmps.h" % name
    for line in open(fin, 'r'):
        line = line.replace(oldinc, newinc)
        line = line.replace("namespace "+oldns, "namespace "+newns)
        sout.write(line)
    print "Done "+fout
    sout.close()


def copy_param_class(srcdir,outdir,name,model):
    ## Header
    fin  = "%s/src/%s_%s.h" % (srcdir,"Parameters",model)
    fout = "%s/%s_%s.h" % (outdir,"Parameters",model)
    ident = "MGME_"+name
    sout = open(fout,'w');
    hguard = None; opened = False
    for line in open(fin, 'r'):
        if line[0] == "#":
            m = re.match(r"#(ifndef|define)\s+(\w+)", line.strip())
            if m: 
                hguard = m.group(2)
                line = "#%s %s_%s\n" % (m.group(1), ident, m.group(2))
            elif hguard and opened:
                m = re.match(r"#endif\s+//\s*"+hguard, line.strip())
                if m: 
                    sout.write("} // namespace %s\n\n" % ident)
                    line = "#endif // %s_%s\n" % (ident, hguard)
        elif line.startswith("class Parameters_"+model):
            sout.write("namespace %s {\n\n" % ident)
            opened = True
        elif re.match(r"\s*static Parameters_%s\s+\*\s+getInstance().*" % model,line): 
            # replace static constructor with default one
            line = "    Parameters_%s() {}\n" % model
        elif re.match(r"\s*static Parameters_%s\s+\*\s+instance.*" % model,line): 
            # remove static instance
            continue
        line = line.replace("read_slha.h", "MGcpp/GenericMEComputer/interface/SLHAInfo.h")
        line = line.replace("SLHAReader&", "const SLHAInfo &")
        line = line.replace("SLHAReader", "SLHAInfo")
        m = re.match(r"(\s*void.*print(Ind|D)ependent\w+\s*\(\s*\)\s*);", line)
        if m: line = "%s const ;\n" % m.group(1)
        sout.write(line)
    print "Done "+fout
    sout.close()
    ## CC
    fin  = "%s/src/%s_%s.cc" % (srcdir,"Parameters",model)
    fout = "%s/%s_%s.cc" % (outdir,"Parameters",model)
    sout = open(fout,'w');
    stage = 0
    for line in open(fin, 'r'):
        #print "[%d]< %s" % (stage, line.rstrip())
        if "Initialize static instance" in line:
            if stage != 0: raise RuntimeError("Bad stage %s" % stage)
            stage = 1
            line = "using %s::Parameters_%s;\n" % (ident, model)
        elif stage == 1 and "::instance" in line:
            stage = 2
            continue
        elif "Function to get static instance" in line:
            if stage != 2: raise RuntimeError("Bad stage %s" % stage)
            stage = 3
            continue
        elif stage == 3:
            if line.strip() == "}": 
                stage = 4
            continue
        else:
            line = line.replace("SLHAReader&", "const SLHAInfo &")
            line = line.replace("SLHAReader", "SLHAInfo")
            m = re.match(r"(void.*::print(Ind|D)ependent\w+\s*\(\s*\))\s*", line)
            if m: line = "%s const\n" % m.group(1)
            #print "[%d]> %s" % (stage, line.rstrip())
            pass
        sout.write(line)
    print "Done "+fout
    sout.close()

def make_cpprocess(outdir,name,model):
    ident = "MGME_"+name
    fout = "%s/%s.h" % (outdir,"CPPProcess")
    sout = open(fout,'w');
    code = ("""
#ifndef <HEADER>
#define <HEADER>

#include "<PARAMETERS>.h"

namespace <NAMESPACE> {

class CPPProcess
{
  public:

    // Constructor.
    CPPProcess() {}

    // Destructor
    virtual ~CPPProcess() {}

    // Initialize process.
    virtual void initProc(const <PARAMETERS> & params) = 0; 

    // Calculate flavour-independent parts of cross section.
    virtual void sigmaKin() = 0;

    // Evaluate sigmaHat(sHat).
    virtual double sigmaHat() = 0;

    // Info on the subprocess.
    virtual std::string name() const = 0;
    virtual int code() const = 0;


    virtual const std::vector<double> & getMasses() const = 0;

    // Get and set momenta for matrix element evaluation
    virtual std::vector < double * > & getMomenta() = 0;
    virtual void setMomenta(const std::vector < double * > & momenta) = 0;
    virtual void setInitial(int inid1, int inid2) = 0;

    virtual int nInitial() const = 0; 
    virtual int nExternal() const = 0; 

    // Get matrix element vector
    virtual const double * getMatrixElements() const = 0;

    // True if the pdgIds of the external legs match with this process
    virtual bool matchParticles(const std::vector<int> & pdgIds) const = 0;
};

} // namespace <NAMESPACE>

#endif // <HEADER>""").strip()
    code = code.replace("<HEADER>","%s_CPPProcess_h" % ident)
    code = code.replace("<PARAMETERS>", "Parameters_%s" % model)
    code = code.replace("<NAMESPACE>", ident)
    sout.write(code+"\n")
    print "Done "+fout
    sout.close()

def copy_subprocess(srcdir,outdir,subprocess,name,model):
    ## Header
    fin  = "%s/SubProcesses/%s/%s.h" % (srcdir,subprocess,"CPPProcess")
    fout = "%s/%s.h" % (outdir,subprocess)
    ident = "MGME_"+name
    sout = open(fout,'w');
    particles = []
    stage = 0
    for line in open(fin, 'r'):
        if re.match("#include.*Parameters_%s" % model, line):
            if stage != 0: raise RuntimeError("Bad stage %s" % stage)
            line += '#include "CPPProcess.h"\n'
            stage = 1
        elif stage == 1:
            if "using namespace std" in line:
                continue
            m = re.match(r"// Process:\s+((?:\w+[\+\-~]?\s+)+)>\s*((?:\w+[\+\-~]?\s+)+)(\w+<?=\d)*(\s+@\d+)?", line)
            if m:
                particles.append(tuple(m.group(i).strip().split() for i in (1,2)))
                print "\tfor %s, found reaction %r -> %r" % (subprocess, particles[-1][0], particles[-1][1])
            elif "// Process:" in line:
                raise RuntimeError("Can't parse process line %s in subprocess %s\n", line.strip(), subprocess)
            if "class CPPProcess" in line:
                line = "namespace %s {\n\nclass %s : public CPPProcess\n" % (ident,subprocess);
                stage = 2
        elif stage == 2 and "CPPProcess()" in line:
            l0 = line
            line =  l0.replace("CPPProcess", subprocess)+"\n"
            line += l0.replace("CPPProcess", "~"+subprocess).replace("{}", " override {}");

            stage = 3
        elif stage == 3:
            line = line.replace("initProc(string param_card_name)","initProc(Parameters_%s & params)" % model) # const added below
            line = line.replace("vector","std::vector")
            line = line.replace("Parameters_%s" % model,"const Parameters_%s" % model)
            if "virtual " in line:
                line = line.replace("virtual ","")
                if "{" in line: line = line.replace("{","override {",1)
                else:           line = line.replace(";"," override ;",1)
            elif re.match(r"\s+.*(getMasses|getMomenta|setMomenta|setInitial|getMatrixElements).*\n",line):
                line = line.replace("{"," override {")
                line = line.replace("getMomenta()","& getMomenta()")
                if "setMomenta" in line: 
                    line = line.replace("std::vector","const std::vector")
            elif re.match(r"\s+double\s+\*\s+jamp2\[nprocesses\];.*", line):
                line = "    std::vector<std::vector<double>> jamp2;\n";
            elif "Constants for array limits" in line:
                line += "    int nInitial() const override { return ninitial; }\n" 
                line += "    int nExternal() const override { return nexternal; }\n" 
            elif "private:" in line:
                line = ("    // True if the pdgIds of the external legs match with this process\n"+
                        "    bool matchParticles(const std::vector<int> & pdgIds) const override ;\n\n"+
                        line)
            elif line.strip() == "};":
                stage = 4
        elif stage == 4:
            line += "} // namespace %s\n" % ident
            stage = 5
        sout.write(line)
    if len(particles) == 0:
        raise RuntimeError("Couldn't find external leg particles for %s" % subprocess)
    print "Done "+fout
    sout.close()
    ## Source
    fin  = "%s/SubProcesses/%s/%s.cc" % (srcdir,subprocess,"CPPProcess")
    fout = "%s/%s.cc" % (outdir,subprocess)
    ident = "MGME_"+name
    oldns = "MG5_"+model
    sout = open(fout,'w');
    stage = 0;  
    for line in open(fin, 'r'):
        line = line.replace("CPPProcess::",subprocess+"::")
        if "#include" in line:
            if stage != 0: raise RuntimeError("Bad stage %s" % stage)
            line = line.replace("CPPProcess.h", "%s.h" % subprocess)
            line = line.replace("HelAmps_%s.h" % model, "%s_HelAmps.h" % name)
        elif "using namespace "+oldns in line:
            if stage != 0: raise RuntimeError("Bad stage %s" % stage)
            line = line.replace("namespace "+oldns, "namespace "+ident)
            stage = 1
        elif "::initProc" in line:
            if stage != 1: raise RuntimeError("Bad stage %s" % stage)
            line = line.replace("initProc(string param_card_name)","initProc(const Parameters_%s & params)" % model)
            stage = 2
        elif stage == 2 or stage == 2.1:
            m = re.match(r"(\s*)jamp2\[(\d+)\]\s+=\s+new\s+double\[(\d+)\];\s*", line)
            if m:
                line = "%sjamp2[%s].resize(%s);\n" % (m.group(1), m.group(2), m.group(3));
                if stage == 2:
                    line = ("%sjamp2.resize(nprocesses);\n" % m.group(1)) + line
                stage = 2.1
            elif stage == 2.1:
                stage = 3
            else:
                line = line.replace("Parameters_%s::getInstance()" % model,"& params");
                if "SLHAReader" in line: continue
                line = line.replace("pars->setIndependent","//pars->setIndependent")
                line = line.replace("pars->printIndependent","//pars->printIndependent")
        elif stage == 3:
            if "if (firsttime)" in line:
                line = line.replace("firsttime","false")
            elif "firsttime" in line: 
                continue
            # comment out printouts 
            line = line.replace("pars->setDependent","//pars->setDependent")
            line = line.replace("pars->printDependent","//pars->printDependent")
            # make thread-safe
            line = line.replace("static int", "int")
            line = line.replace("static bool", "bool")
            # fix bogus initialization
            line = line.replace("goodhel[ncomb] = {ncomb * false};", "goodhel[ncomb]; std::fill_n(goodhel, ncomb, false);")
        sout.write(line)
    sout.write("\n")
    sout.write("bool %s::matchParticles(const std::vector<int> & pdgIds) const {\n" % subprocess)
    for (inc,out) in particles:
        sequences = [inc+out]
        if len(inc) == 2: 
            sequences.append([inc[1],inc[0]] + out)
        for seq in sequences:
            sout.write("    if (")
            for i,pname in enumerate(seq):
                if pname not in PDGIDs: raise RuntimeError("Particle %r not in known PDG IDs\n" % pname)
                if i: sout.write(" && ");
                sout.write("pdgIds[%d] == %+d" % (i, PDGIDs[pname]))
            sout.write(") return true;\n");
    sout.write("    return false;\n");
    sout.write("}\n");
    print "Done "+fout
    sout.close()

def make_mainprocess(outdir,name,model,subprocesses):
    ident = "MGME_"+name
    fout = "%s/%s.h" % (outdir,"main")
    sout = open(fout,'w');
    code = ("""
#ifndef <HEADER>
#define <HEADER>

#include "MGcpp/GenericMEComputer/interface/ProcessCollection.h"
#include "<PARAMETERS>.h"
#include "CPPProcess.h"

namespace <NAMESPACE> {

class main : public ProcessCollectionT<<PARAMETERS>,CPPProcess>
{
  public:

    // Constructor
    main() ;
    ~main() {}
};

} // namespace <NAMESPACE>

#endif // <HEADER>""").strip()
    code = code.replace("<HEADER>","%s_main_h" % ident)
    code = code.replace("<PARAMETERS>", "Parameters_%s" % model)
    code = code.replace("<NAMESPACE>", ident)
    sout.write(code+"\n")
    print "Done "+fout
    sout.close()
    fout = "%s/%s.cc" % (outdir,"main")
    sout = open(fout,'w');
    code = ("""
#include "main.h"

<INCLUDE_SUBPROCESSES>

<NAMESPACE>::main::main() {
    <ADD_SUBPROCESSES>
}
""").strip()
    code = code.replace("<HEADER>","%s_CPPProcess_h" % ident)
    code = code.replace("<PARAMETERS>", "Parameters_%s" % model)
    code = code.replace("<NAMESPACE>", ident)
    code = code.replace("<INCLUDE_SUBPROCESSES>", "\n".join('#include "%s.h"' % s for s in subprocesses))   
    code = code.replace("<ADD_SUBPROCESSES>", "\n    ".join('addProcess(new %s());' % s for s in subprocesses))   
    sout.write(code+"\n")
    print "Done "+fout
    sout.close()


os.mkdir(outdir)
copy_helamps(srcdir,outdir,name,model)
copy_param_class(srcdir,outdir,name,model)
make_cpprocess(outdir,name,model)
for s in subprocesses:
    copy_subprocess(srcdir,outdir,s,name,model)
make_mainprocess(outdir,name,model,subprocesses)
