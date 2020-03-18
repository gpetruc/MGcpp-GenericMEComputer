#ifndef READ_SLHA_H
#define READ_SLHA_H

#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>

namespace slha_util {
    struct ci_less {
        // case-independent (ci) compare_less binary function
        struct nocase_compare {
            bool operator()(const unsigned char& c1, const unsigned char& c2) const { return tolower(c1) < tolower(c2); }
        };
        bool operator()(const std::string & s1, const std::string & s2) const {
            return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare()); 
        }
    };
} // namespace 

class SLHABlock
{
  public:
    SLHABlock(const std::string & name = ""){_name = name;}
    ~SLHABlock(){}

    void add_entry(int index, double value) { 
        _entries[index] = value; 
    }
    void set_entry(int index, double value) { 
        auto match = _entries.find(index);
        if (match == _entries.end()) throwMissing(index);
        else match->second = value; 
    }
    double get_entry(int index, double def_val = 0) const {
        auto match = _entries.find(index);
        return match != _entries.end() ? match->second : def_val;
    }
    bool has_entry(int index) const { 
        return _entries.find(index) != _entries.end();
    }

    void add_entry(const std::vector<int> &indices, double value);
    void set_entry(const std::vector<int> &indices, double value);
    double get_entry(const std::vector<int> &indices, double def_val = 0) const {
        return get_entry(get_index(indices), def_val);
    }
    bool has_entry(const std::vector<int> &indices) const { 
        return _byMultiIndex.find(indices) != _byMultiIndex.end();
    }
    int get_index(const std::vector<int> &indices) const { 
        auto match = _byMultiIndex.find(indices);
        return match != _byMultiIndex.end() ? match->second : -1;
    }


    void add_entry(const std::string & name, double value);
    void set_entry(const std::string & name, double value);
    double get_entry(const std::string & name, double def_val = 0) const {
        return get_entry(get_index(name), def_val);
    }
    bool has_entry(const std::string & name) const { 
        return _byName.find(name) != _byName.end();
    }
    int get_index(const std::string & name) const { 
        auto match = _byName.find(name);
        return match != _byName.end() ? match->second : -1;
    }

    void add_entry(const std::string & name, int index, double value) ;
    void add_entry(const std::string & name, const std::vector<int> &indices, double value) ;
 
    void set_name(const std::string & name) {_name = name;}
    const std::string & get_name() const {return _name;}

    int get_indices() const { return _nIndices;}

    double & operator[](int index) { return _entries[index]; }
    double operator[](int index) const { return get_entry(index); }

  private:
    void throwMissing(int index) const ;
    int next_free_index() const ;

    std::string _name;
    std::map<int, double> _entries;
    std::map<std::vector<int>, int> _byMultiIndex;
    std::map<std::string, int, slha_util::ci_less> _byName;
    unsigned int _nIndices;
};

class SLHAInfo
{
  public:
    SLHAInfo() ;
    ~SLHAInfo() ;
    
    void read_slha_file(const std::string & file_name);

    const SLHABlock & get_block(const std::string & name) const {
        auto match = _blocks.find(name);
        if (match == _blocks.end()) throwMissingBlock(name);
        return match->second;
    }
    SLHABlock & get_block(const std::string & name) {
        auto match = _blocks.find(name);
        if (match == _blocks.end()) {
            SLHABlock & ret = _blocks[name];
            ret.set_name(name);
            return ret;
        } else {
            return match->second;
        }
    }


    double get_block_entry(const std::string & block_name, const std::vector<int> & indices, double def_val = 0) const {
        auto match = _blocks.find(block_name);
        return match != _blocks.end() ? match->second.get_entry(indices, def_val) : def_val;
    }
    double get_block_entry(const std::string & block_name, int index, double def_val = 0) const {
        auto match = _blocks.find(block_name);
        return match != _blocks.end() ? match->second.get_entry(index, def_val) : def_val;
    }
    double get_block_entry(const std::string & block_name, const std::string & name, double def_val = 0) const {
        auto match = _blocks.find(block_name);
        return match != _blocks.end() ? match->second.get_entry(name, def_val) : def_val;
    }

    void set_block_entry(const std::string & block_name, const std::vector<int> & indices, double value) {
        return get_block(block_name).set_entry(indices, value);
    }
    void set_block_entry(const std::string & block_name, const std::string & name, double value) {
        return get_block(block_name).set_entry(name, value);
    }
    void set_block_entry(const std::string & block_name, int index, double value) {
        return get_block(block_name).set_entry(index, value);
    }
    void add_block_entry(const std::string & block_name, const std::vector<int> & indices, double value) {
        return get_block(block_name).add_entry(indices, value);
    }
    void add_block_entry(const std::string & block_name, const std::string & name, double value) {
        return get_block(block_name).add_entry(name, value);
    }
    void add_block_entry(const std::string & block_name, int index, double value) {
        return get_block(block_name).add_entry(index, value);
    }
    void add_block_entry(const std::string & block_name, const std::string & name, const std::vector<int> & indices, double value) {
        return get_block(block_name).add_entry(name, indices, value);
    }
    void add_block_entry(const std::string & block_name, const std::string & name, int index, double value) {
        return get_block(block_name).add_entry(name, index, value);
    }
   

  private:
    std::map<std::string, SLHABlock, slha_util::ci_less> _blocks;

    void throwMissingBlock(const std::string & block_name) const ;

};

#endif
