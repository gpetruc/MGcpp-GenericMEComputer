#include <algorithm>
#include <iostream>
#include <fstream>
#include <regex>
#include "MGcpp/GenericMEComputer/interface/SLHAInfo.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

void SLHABlock::throwMissing(int index) const {
    throw cms::Exception("NotFound") << "Index " << index << " missing in block '" << _name << "'\n";
}

int SLHABlock::next_free_index() const {
    for (unsigned int i = _entries.size(), n = 2*i+2; i < n; ++i) {
        if (_entries.find(i) == _entries.end()) return i;
    }
    return -1;
}

void SLHABlock::add_entry(const std::vector<int> &indices, double value)
{
  if (_entries.empty()) {
    _nIndices = indices.size();
  } else if(indices.size() != _nIndices) {
    throw cms::Exception("LogicError") << "Wrong number of indices (" <<indices.size() <<") in add_entry for block '" << _name << "'\n";
  }

  int index = get_index(indices);
  if (index == -1) {
      index = next_free_index();
      _byMultiIndex[indices] = index;
  } else {
      throw cms::Exception("LogicError") << "Duplicate entry in add_entry for block '" << _name << "'\n";
  }
  _entries[index] = value;
}

void SLHABlock::set_entry(const std::vector<int> &indices, double value)
{
  if (_entries.empty() || indices.size() != _nIndices)
    throw cms::Exception("LogicError") << "Wrong number of indices (" <<indices.size() <<") in set_entry for block '" << _name << "'\n";
    
  int index = get_index(indices);
  if (index == -1) {
      throw cms::Exception("NotFound") << "Missing entry in set_entry for block '" << _name << "'\n";
  }
  _entries[index] = value;
}


void SLHABlock::add_entry(const std::string & name, double value)
{
  int index = get_index(name);
  if (index == -1) {
      index = next_free_index();
      _byName[name] = index;
  } else {
      throw cms::Exception("LogicError") << "Duplicate entry in add_entry '" << name << "' for block '" << _name << "'\n";
  }
  _entries[index] = value;
}


void SLHABlock::set_entry(const std::string & name, double value)
{
  int index = get_index(name);
  if (index == -1) {
      throw cms::Exception("NotFound") << "Missing entry in set_entry '" << name << "' for block '" << _name << "'\n";
  }
  _entries[index] = value;
}

void SLHABlock::add_entry(const std::string & name, int index, double value) 
{
  if (get_index(name) != -1) {
      throw cms::Exception("LogicError") << "Duplicate entry in add_entry '" << name << "' for block '" << _name << "'\n";
  } 

  _byName[name] = index;
  _entries[index] = value;
}
void SLHABlock::add_entry(const std::string & name, const std::vector<int> &indices, double value) 
{
  if (get_index(name) != -1  || get_index(indices) != -1) {
      throw cms::Exception("LogicError") << "Duplicate entry in add_entry '" << name << "' for block '" << _name << "'\n";
  } 

  int index = next_free_index();
  _byMultiIndex[indices] = index;
  _entries[index] = value;
}
 

SLHAInfo::SLHAInfo() 
{
}

SLHAInfo::~SLHAInfo() 
{
}

void SLHAInfo::read_slha_file(const std::string & file_name)
{
  std::ifstream param_card;
  param_card.open(file_name.c_str(), std::ifstream::in);
  if(!param_card.good())
    throw cms::Exception("Error while opening param card");
  std::cout << "Opened slha file " << file_name << " for reading" << std::endl;
  char buf[501];
  std::string line;
  std::string block("");

  std::regex idx1dline("\\s*(\\d+)\\s+(\\S+)\\s*(\\s+#\\s*(\\S.*)?)?");
  std::regex idx2dline("\\s*(\\d+)\\s+(\\d+)\\s+(\\S+)\\s*(\\s+#\\s*(\\S.*)?)?");
  std::regex blockline("\\s*block\\s+(\\S+)\\s*(\\s+#.*)?",  std::regex_constants::ECMAScript |  std::regex_constants::icase);
  std::regex decayline("\\s*decay\\s+(\\d+)\\s*(\\S+)\\s*(\\s+#.*)?",  std::regex_constants::ECMAScript |  std::regex_constants::icase);
  std::regex emptyline("\\s*#.*");
  std::smatch match;
  std::vector<int> indices(2);
  while(param_card.good()){
      param_card.getline(buf, 500);
      line = buf;
      //bool barf = true;
      if(line != "" && line[0] != '#'){
          if(block != ""){
              if (std::regex_match(line, match, idx1dline)) {
                  int index = std::stoi(match[1].str());
                  double value = std::stod(match[2].str());
                  if (match[4].matched) {
                      std::string name = match[4].str(); name.erase(name.find_last_not_of("\t\n\v\f\r ")+1);
                      add_block_entry(block, name, index, value);
                      //std::cout << "Block " << block << ": created named entry '" << name << "' index " << index << " value " << value << std::endl;
                  } else {
                      add_block_entry(block, index, value);
                      //std::cout << "Block " << block << ": created unnamed entry index " << index << " value " << value << std::endl;
                  }
                  //barf = false;
              } else if (std::regex_match(line, match, idx2dline)) {
                  indices.resize(2);
                  indices[0] = std::stoi(match[1].str());
                  indices[1] = std::stoi(match[2].str());
                  double value = std::stod(match[3].str());
                  if (match[5].matched) {
                      std::string name = match[5].str(); name.erase(name.find_last_not_of("\t\n\v\f\r ")+1);
                      add_block_entry(block, name, indices, value);
                      //std::cout << "Block " << block << ": created named entry '" << name << "' index2d ( " << indices[0] << " , " << indices[1] << " )  value " << value << std::endl;
                  } else {
                      add_block_entry(block, indices, value);
                      //std::cout << "Block " << block << ": created unnamed index2d ( " << indices[0] << " , " << indices[1] << " )  value " << value << std::endl;
                  }
                  //barf = false;
              } else if (!std::regex_match(line, match, emptyline)) {
                  //std::cout << "Block " << block << ": ended" << std::endl;
                  //barf = false;
                  block = "";
              }
          }
          if (std::regex_match(line, match, blockline)) {
              block = match[1].str();
              //std::cout << "Block " << block << ": start" << std::endl;
              //barf = false;
          } else if (std::regex_match(line, match, decayline)) {
              int pdg_code = std::stoi(match[1].str());
              double value = std::stod(match[2].str());
              add_block_entry("decay", pdg_code, value);
              block = "";
              //std::cout << "Decay for " << pdg_code << " found" << std::endl;
              //barf = false;
          } 
          //if (barf) std::cout << "barf{" << line << "}" << std::endl; 
      }
  }

  if (_blocks.size() == 0)
    throw cms::Exception("No information read from SLHA card");

  param_card.close();
}

void SLHAInfo::throwMissingBlock(const std::string & name) const
{
    throw cms::Exception("Block "+name+" missing in SLHA");
}
