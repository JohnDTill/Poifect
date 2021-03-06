#ifndef HASHUTIL_H
#define HASHUTIL_H

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <vector>

constexpr int entries_per_row = 10;

template<typename KeyType>
static bool hasDuplicates(const std::vector<KeyType>& keys){
    for(size_t i = keys.size()-1; i < std::numeric_limits<size_t>::max(); i--)
        for(size_t j = i-1; j < std::numeric_limits<size_t>::max(); j--)
            if(keys[i] == keys[j]) return true;

    return false;
}

size_t getModulusBitmask(size_t size){
    assert(size > 1);

    for(size_t i = 1; i < 64; i++)
        if((1 << i) >= size) return (1 << i) - 1;

    return std::numeric_limits<size_t>::max();
}

void writeKeys(std::string& str,
               const std::vector<std::string>& keys,
               const std::vector<int>& mapping,
               size_t n){
    size_t num_chars = 0;
    std::vector<size_t> sze;
    std::vector<size_t> start;

    for(const std::string& key : keys){
        start.push_back(num_chars);
        num_chars += key.size();
        sze.push_back(key.size());
    }

    str += "    static constexpr std::array<char, " + std::to_string(num_chars) + "> flat_keys {\n";
    for(const std::string& key : keys){
        for(uint8_t i = 0; i < 8; i++) str.push_back(' ');
        for(const char& ch : key){
            str.push_back('\'');
            str.push_back(ch);
            str.push_back('\'');
            str.push_back(',');
        }
        str.push_back('\n');
    }
    str += "    };\n\n";

    str += "    static constexpr std::array<size_t, " + std::to_string(n+1) + "> key_start {\n        ";
    size_t i = 0;
    for(const int& val : mapping){
        if(i && i%entries_per_row == 0) str += "\n        ";
        i++;
        if(val == -1) str += "0,";
        else str += std::to_string(start[val]) + ",";
    }
    str += "\n    };\n\n";

    str += "    static constexpr std::array<size_t, " + std::to_string(n+1) + "> key_size {\n        ";
    i = 0;
    for(const int& val : mapping){
        if(i && i%entries_per_row == 0) str += "\n        ";
        i++;
        if(val == -1) str += "0,";
        else str += std::to_string(sze[val]) + ",";
    }
    str += "\n    };\n\n";

    str += "    static inline bool checkBin(const std::string& key, size_t bin) noexcept{\n"
           "        const auto& size = key_size[bin];\n"
           "        if(size != key.size()) return false;\n"
           "        const auto& start = key_start[bin];\n"
           "        for(size_t i = size-1; i < std::numeric_limits<size_t>::max(); i--)\n"
           "            if(key[i] != flat_keys[start+i]) return false;\n"
           "        return true;\n"
           "    }\n";
}

template<typename KeyType>
void writeKeys(std::string& str,
               const std::vector<KeyType>& keys,
               std::string key_type,
               const std::vector<int>& mapping,
               size_t n){
    str += "    static constexpr std::array<" + key_type + ", " + std::to_string(n+1) + "> keys {\n        ";

    size_t i = 0;
    for(const int& val : mapping){
        if(i && i%entries_per_row==0) str += "\n        ";
        i++;
        if(val == -1) str += "0,";
        else    str += std::to_string(keys[val]) + ",";
    }
    str += "\n    };\n";
}

void writeKeys(std::string& str, const std::vector<uint64_t>& keys, const std::vector<int>& mapping, size_t n){
    writeKeys<uint64_t>(str, keys, "uint64_t", mapping, n);
}

void writeKeys(std::string& str, const std::vector<uint32_t>& keys, const std::vector<int>& mapping, size_t n){
    writeKeys<uint32_t>(str, keys, "uint32_t", mapping, n);
}

void writeKeys(std::string& str, const std::vector<uint16_t>& keys, const std::vector<int>& mapping, size_t n){
    writeKeys<uint16_t>(str, keys, "uint16_t", mapping, n);
}

void writeKeys(std::string& str, const std::vector<uint8_t>& keys, const std::vector<int>& mapping, size_t n){
    writeKeys<uint8_t>(str, keys, "uint8_t", mapping, n);
}

std::string typeStr(const std::string&){
    return "std::string";
}

std::string typeStr(uint64_t){
    return "uint64_t";
}

std::string typeStr(uint32_t){
    return "uint32_t";
}

std::string typeStr(uint16_t){
    return "uint16_t";
}

std::string typeStr(uint8_t){
    return "uint8_t";
}

template<typename KeyType>
std::string getCommonCodeGen(const std::vector<KeyType>& keys,
                             const std::vector<std::string> vals,
                             const std::vector<int>& mapping,
                             size_t n,
                             std::string map_name,
                             bool nonKeyLookups){
    std::string upper_name = map_name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), toupper);

    std::string str = "//CODEGEN FILE\n"
                      "#ifndef POIFECT_" + upper_name + "_H\n"
                      "#define POIFECT_" + upper_name + "_H\n"
                      "#include <array>\n";

    if(!nonKeyLookups) str += "#include <cassert>\n";

    str += "#include <limits>\n"
           "#include <string>\n\n";

    str += "class " + map_name + " final{\n"
    "public:\n"
    "    static " + (typeStr(keys[0])!="std::string" ? "constexpr " : "")
            + "std::string_view lookup(const " + typeStr(keys[0]) + "& key) noexcept;\n"
    "\n"
    "private:\n";

    if(!nonKeyLookups) str += "    #ifndef NDEBUG\n";
    writeKeys(str, keys, mapping, n);
    if(!nonKeyLookups) str += "    #endif\n";
    str += "\n";

    size_t num_chars = 0;
    std::vector<size_t> sze;
    std::vector<size_t> start;

    for(const std::string& val : vals){
        start.push_back(num_chars);
        num_chars += val.size();
        sze.push_back(val.size());
    }

    str += "    static constexpr char flat_vals[" + std::to_string(num_chars+1) + "] = ";
    for(const std::string& val : vals){
        str.push_back('\n');
        for(uint8_t i = 0; i < 8; i++) str.push_back(' ');
        str += '"' + val + '"';
    }
    str += ";\n\n";

    str += "    static constexpr std::array<size_t, " + std::to_string(n+1) + "> val_start {\n        ";
    size_t i = 0;
    for(const int& val : mapping){
        if(i && i%entries_per_row == 0) str += "\n        ";
        i++;
        if(val == -1) str += "0,";
        else str += std::to_string(start[val]) + ",";
    }
    str += "\n    };\n\n";

    str += "    static constexpr std::array<size_t, " + std::to_string(n+1) + "> val_size {\n        ";
    i = 0;
    for(const int& val : mapping){
        if(i && i%entries_per_row == 0) str += "\n        ";
        i++;
        if(val == -1) str += "0,";
        else str += std::to_string(sze[val]) + ",";
    }
    str += "\n    };\n\n";

    return str;
}

//This is a function I was playing around with to identify possible optimizations.
//I love the idea of unwrapping the loop over the string, but it's not possible for every key set.
//I'm leaving this as a note since I don't want highly specialized optimizations yet.
#include <iostream>
#include <unordered_set>
void analyze(const std::vector<std::string>& keys){
    size_t min = std::numeric_limits<size_t>::max();
    size_t max = 0;

    for(const std::string& key : keys){
        min = std::min(key.size(), min);
        max = std::max(key.size(), max);
    }

    if(min == max){
        std::cout << "All keys have length " << min << ". You don't need a key map." << std::endl;
        return;
    }

    std::unordered_set<std::string> set;
    for(const std::string& key : keys){
        std::string subkey = key.substr(0, min);
        if(set.find(subkey) != set.end()) break;
        set.insert(subkey);
    }
    if(set.size() == keys.size()) std::cout << "First " << min << " chars are unique" << std::endl;

    set.clear();
    for(const std::string& key : keys){
        std::string subkey = key.substr(key.size()-min);
        if(set.find(subkey) != set.end()) break;
        set.insert(subkey);
    }
    if(set.size() == keys.size()) std::cout << "Last " << min << " chars are unique" << std::endl;

    size_t low = min/2;
    size_t high = min - low;
    set.clear();
    for(const std::string& key : keys){
        std::string subkey = key.substr(0, min/2) + key.substr(key.size()-high);
        if(set.find(subkey) != set.end()) break;
        set.insert(subkey);
    }
    if(set.size() == keys.size()) std::cout << "First " << low << " with last " << high << " chars are unique" << std::endl;

    set.clear();
    for(const std::string& key : keys){
        std::string subkey = key.substr(0, min) + "⁜" + char(key.size());
        if(set.find(subkey) != set.end()) break;
        set.insert(subkey);
    }
    if(set.size() == keys.size()) std::cout << "First " << min << " chars with size are unique" << std::endl;

    set.clear();
    for(const std::string& key : keys){
        std::string subkey = key.substr(key.size()-min) + "⁜" + char(key.size());
        if(set.find(subkey) != set.end()) break;
        set.insert(subkey);
    }
    if(set.size() == keys.size()) std::cout << "Last " << min << " chars are unique" << std::endl;
}

#endif // HASHUTIL_H
