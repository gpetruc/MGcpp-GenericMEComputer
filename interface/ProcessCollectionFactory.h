#ifndef MGcpp_GenericMEComputer_ProcessCollectionFactory_h
#define MGcpp_GenericMEComputer_ProcessCollectionFactory_h

#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "MGcpp/GenericMEComputer/interface/ProcessCollection.h"

typedef edmplugin::PluginFactory<ProcessCollection*(void)> ProcessCollectionFactory;

#endif
