#ifndef HASHWRITE_H
#define HASHWRITE_H

#include "hashsearch.h"
#include <string>

std::string getHashFunctionSource(const SearchResult& result){
    const std::array<uint8_t, 6>& c = result.coeffs;

    std::string out;

    out += result.type + " getHash(const std::string& key){\n"
           "    " + result.type + " hash = 0;\n"
           "\n"
           "    for(const char& ch : key){\n";
    if(c[0] != 0) out += "        hash += " + std::to_string(c[0]) + "*ch;\n";
    if(c[1] != 0) out += "        hash *= " + std::to_string(c[1]) + "*ch;\n";
    if(c[2] != 0) out += "        hash ^= "  + std::to_string(c[2]) + "*ch;\n";
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
            out += "hash*" + std::to_string(c[3]);
        }
        out += ";\n";
    }

    out += "    }\n"
           "\n"
           "    return hash;\n"
           "}";

    return out;
}

template<void(*Callback)(void)>
SearchResult writePerfectHash(const std::vector<std::string>& keys,
                             const std::vector<std::string>& vals,
                             const std::string& return_type,
                             const std::string& default_value,
                             uint8_t& progress,
                             const bool& terminate,
                             std::string& msg){
    if(keys.size() != vals.size()) return SearchResult("Each key must have a value");

    auto result = findHashCoeffs<Callback>(keys, progress, terminate, msg);
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

    std::string out = getHashFunctionSource(result);

    out += "\n\n";

    if(result.code >= MINIMAL_BY_MODULUS){
        out += "const std::string keys[" + std::to_string(keys.size()) + "] {\n";

        std::vector<std::string> sorted_hashes;
        std::vector<std::string> sorted_vals;
        sorted_hashes.resize(keys.size());
        sorted_vals.resize(keys.size());

        for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
            const std::string& key = keys[i];
            uint64_t hash = result.hash_fn(key);
            if(result.code == MINIMAL_BY_MODULUS) hash %= keys.size();
            else hash -= min_hash;

            sorted_hashes.at(hash) = key;
            sorted_vals.at(hash) = vals.at(i);
        }

        for(const std::string& key : sorted_hashes){
            out += "    \"" + key + "\",\n";
        }
        out += "};\n\n";

        out += "const " + return_type + " vals[" + std::to_string(keys.size()) + "] {\n";
        for(const std::string& val : sorted_vals){
            out += "    " + val + ",\n";
        }
        out += "};\n\n";
    }

    out += return_type + " lookup(const std::string& key){\n";
    if(max_length == min_length) out += "    if(key.size() != " + std::to_string(max_length) + ") return -1;\n\n";

    out += "    " + result.type + " hash = getHash(key)";

    if(result.code == MINIMAL_BY_MODULUS){
        bool power_of_2 = keys.size() != 0 && (keys.size() & (keys.size()-1)) == 0;

        if(power_of_2){
            int log2;
            std::vector<std::string>::size_type x = keys.size();
            for (log2=0; x != 1; x>>=1,log2++);
            out += " & " + std::to_string(log2-1);
        }else{
            out += " % " + std::to_string(keys.size());
        }
    }else if(result.code == MINIMAL_BY_DEFAULT){
        out += " - " + std::to_string(min_hash);
    }

    out += ";\n\n";

    if(result.code >= MINIMAL_BY_MODULUS){
        out += "    return (";
        if(result.code == MINIMAL_BY_DEFAULT)
            out += "hash < " + std::to_string(keys.size()) + " && ";
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
