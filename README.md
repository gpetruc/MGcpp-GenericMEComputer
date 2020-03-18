# MGcpp-GenericMEComputer
Setup for evaluating LHE-level matrix elements from Madgraph in CMSSW

## Installation instructions 

Tested in `CMSSW_10_2_20_UL` on lxplus7, using matrix elements generated from Madgraph 2.6.5 and 2.6.7 at leading order.
````
git clone https://github.com/gpetruc/MGcpp-GenericMEComputer.git MGcpp/GenericMEComputer
````

## To add matrix element codes:

1. launch madgraph, generate one or more processes, and export the matrix element as c++ code
    - if you have different processes, you must tag them with `@0`, `@1`, especially if they have a dfferent number of legs
    - you can also generate decay-only matrx elements (`generate h > mu+ mu- e+ e-`)
````
   import model your-favourite-ufo 
   generate p p > some-final-state @0 
   add process p p >  another-final-state @1 
    ... 
   output standalone_cpp some-directory-name
````   
2. convert the c++ code and export it (e.g. to a subdirectory of `MGcpp/GenericMEComputer/plugins`)
````
    mg5_cpp_repack.py name path/to/some-directory-name -o /path/to/MGcpp/GenericMEComputer/plugins/name --plugin
````

## Example usage

See [MGGenMatrixElementTableProducer](https://github.com/gpetruc/MGcpp-GenericMEComputer/blob/master/plugins/MGGenMatrixElementTableProducer.cc) as a starting point

Two helper files are available:
 * [MEMultiWeightTool](https://github.com/gpetruc/MGcpp-GenericMEComputer/blob/master/interface/MEMultiWeightTool.h) allows you to load a matrix element set and instance several copies of it for different points in parameter space
 * [MEInitFromLHE](https://github.com/gpetruc/MGcpp-GenericMEComputer/blob/master/interface/MEInitFromLHE.h) allows to parse the CMSSW LHE event record and create the vector of pdgIds and 4-vectors for evaluating the ME. It can be configured to extract only the decay tree for a specified particle, or to treat some particle as undecayed.
