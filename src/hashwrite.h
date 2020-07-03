#ifndef HASHWRITE_H
#define HASHWRITE_H

#include "hashsearch.h"
#include <string>

static uint64_t getUnderFlowValue(std::string type, uint64_t subtrahend){
    if(subtrahend==0){
        return 0;
    }else if(type=="uint8_t"){
        return std::numeric_limits<uint8_t>::max() - subtrahend + 1;
    }else if(type=="uint16_t"){
        return std::numeric_limits<uint16_t>::max() - subtrahend + 1;
    }else if(type=="uint32_t"){
        return std::numeric_limits<uint32_t>::max() - subtrahend + 1;
    }else if(type=="uint64_t"){
        return std::numeric_limits<uint64_t>::max() - subtrahend + 1;
    }else{
        return 0;
    }
}

static std::string getHashFunctionSource(const SearchResult& result, size_t table_sze, uint64_t min_hash){
    const std::array<uint8_t, 6>& c = result.coeffs;

    std::string out;

    out += result.type + " getHash(const std::string& key){\n"
           "    " + result.type + " hash = ";
    if(result.code == MINIMAL_BY_DEFAULT || result.code == DENSE_BY_DEFAULT){
        out += std::to_string(getUnderFlowValue(result.type, min_hash));
    }else{
        out += "0";
    }
    out += ";\n"
           "\n"
           "    for(const char& ch : key){\n";
    if(c[0] != 0){
        out += "        hash += ";
        if(c[0]!=1) out += std::to_string(c[0]) + "*";
        out += "ch;\n";
    }
    if(c[1] != 0){
        out += "        hash *= ";
        if(c[1]!=1) out += std::to_string(c[1]) + "*";
        out += "ch;\n";
    }
    if(c[2] != 0){
        out += "        hash ^= ";
        if(c[2]!=1) out += std::to_string(c[2]) + "*";
        out += "ch;\n";
    }
    if(c[3] != 0 || c[4] != 0 || c[5] != 0){
        out += "        hash ^= ";
        bool add = false;
        if(c[4] != 0){
            out += "hash << " + std::to_string(c[4]);
            add = true;
        }

        if(c[5] != 0){
            if(add) out += " + ";
            out += "hash >> " + std::to_string(c[5]);
            add = true;
        }

        if(c[3] != 0){
            if(add) out += " + ";
            out += "hash";
            if(c[3]!=1) out += "*" + std::to_string(c[3]);
        }
        out += ";\n";
    }

    out += "    }\n"
           "\n"
           "    return hash";

    if(result.code == MINIMAL_BY_MOD || result.code == DENSE_BY_MOD){
        bool power_of_2 = table_sze != 0 && (table_sze & (table_sze-1)) == 0;

        if(power_of_2){
            out += " & " + std::to_string(table_sze-1);
        }else{
            out += " % " + std::to_string(table_sze);
        }
    }

    out += ";\n"
           "}";

    return out;
}

template<void(*Callback)(void)>
SearchResult writePerfectHash(const std::vector<std::string>& keys,
                             const std::vector<std::string>& vals,
                             uint64_t acceptable_empties,
                             const std::string& return_type,
                             const std::string& default_value,
                             uint8_t& progress,
                             const bool& terminate,
                             std::string& msg){
    if(keys.size() != vals.size()) return SearchResult("Each key must have a value");

    auto result = findHashCoeffs<Callback>(keys, acceptable_empties, progress, terminate, msg);
    progress = terminate ? 0 : 100;

    if(result.code==COLLISION){
        if(terminate){
            msg = "🤫   Search cancelled";
            return SearchResult("Terminated");
        }else{
            msg = "😳   Failed to find perfect hash";
            return SearchResult("Failed to find perfect hash function");
        }
    }

    std::string::size_type max_length = 0;
    std::string::size_type min_length = 255;
    uint64_t min_hash = std::numeric_limits<uint64_t>::max();
    for(const std::string& key : keys){
        if(key.size() > max_length) max_length = key.size();
        if(key.size() < min_length) min_length = key.size();

        for(const std::string& key : keys){
            uint64_t hash = result.hash_fn(key);
            if(hash < min_hash) min_hash = hash;
        }
    }

    std::string out = getHashFunctionSource(result, keys.size()+result.num_empty_entries, min_hash);

    out += "\n\n";

    if(result.code >= DENSE_BY_MOD){
        uint64_t sze = keys.size() + result.num_empty_entries;

        out += "const std::string keys[" + std::to_string(sze) + "] {\n";

        std::vector<std::string> sorted_hashes;
        std::vector<std::string> sorted_vals;
        sorted_hashes.resize(sze);
        sorted_vals.resize(sze);

        for(std::vector<std::string>::size_type i = 0; i < sze; i++){
            sorted_hashes.at(i) = "";
            sorted_vals.at(i) = default_value;
        }

        for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
            const std::string& key = keys[i];
            uint64_t hash = result.hash_fn(key);
            if(result.code == MINIMAL_BY_MOD || result.code == DENSE_BY_MOD) hash %= sze;
            else hash -= min_hash;

            sorted_hashes.at(hash) = key;
            sorted_vals.at(hash) = vals.at(i);
        }

        for(const std::string& key : sorted_hashes){
            out += "    \"" + key + "\",\n";
        }
        out += "};\n\n";

        out += "const " + return_type + " vals[" + std::to_string(sze) + "] {\n";
        for(const std::string& val : sorted_vals){
            out += "    " + val + ",\n";
        }
        out += "};\n\n";
    }

    out += return_type + " lookup(const std::string& key){\n";
    if(max_length == min_length) out += "    if(key.size() != " + std::to_string(max_length) + ") return -1;\n\n";

    out += "    " + result.type + " hash = getHash(key);\n\n";

    if(result.code >= DENSE_BY_MOD){
        out += "    return (";
        if(result.code == MINIMAL_BY_DEFAULT || result.code == DENSE_BY_DEFAULT)
            out += "hash < " + std::to_string(keys.size() + result.num_empty_entries) + " && ";
        out += "key == keys[hash]) ? vals[hash] : " + default_value + ";\n";
    }else{
        out += "    switch(hash){\n";
        for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
            const std::string& key = keys.at(i);
            uint64_t hash = result.hash_fn(key);

            out += "        case " + std::to_string(hash) + ": return key==\"" + key + "\" ? " + vals.at(i) +
                   " : " + default_value + ";\n";
        }
        out += "        default: return " + default_value + ";\n";
        out += "    }\n";
    }

    out += "}";

    return out;
}

#endif // HASHWRITE_H
