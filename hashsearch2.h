#ifndef HASHSEARCH2_H
#define HASHSEARCH2_H

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include "hashutil.h"

typedef uint16_t SeedType;

uint32_t hash2(const std::string& key, const SeedType& coeff){
    uint32_t h = 0;

    for(const char& ch : key)
        h = h*coeff + ch;

    return h;
}

uint32_t hash2(size_t x, const SeedType& coeff){
    x = ((x >> 7) ^ x) * coeff;
    x = (x >> 7) ^ x;
    return x;
}

std::string hashStr2(const std::string&, uint16_t seed, size_t n1, size_t n2, const std::string& default_value, std::string map_name, std::string key_type, bool nonKeyLookups){
    std::string hash =
        "    static inline uint32_t hash(const std::string& key, const uint32_t& coeff) noexcept{\n"
        "        uint32_t h = 0;\n"
        "\n"
        "        for(size_t i = 0; i < key.size(); i++)\n"
        "            h = h*coeff + key[i];\n"
        "\n"
        "        return h;\n"
        "    }\n"
        "};\n"
        "\n"
        "std::string_view " + map_name + "::lookup(const " + key_type + "& key) noexcept{\n"
        "    constexpr uint32_t s0 = " + std::to_string(seed) + ";\n"
        "    const size_t h1 = hash(key, s0) & " + std::to_string(n1) + ";\n"
        "    const uint32_t& s1 = seeds[h1];\n"
        "    const size_t bin = hash(key, s1) & " + std::to_string(n2) + ";\n";
    if(nonKeyLookups) hash +=
        "    return checkBin(key, bin) ? std::string_view(&flat_vals[val_start[bin]], val_size[bin]) : \"" + default_value + "\";\n";
    else hash +=
        "    #ifndef NDEBUG\n"
        "    assert(checkBin(key, bin));\n"
        "    #endif\n\n"
        "    return std::string_view(&flat_vals[val_start[bin]], val_size[bin]);\n";

    hash += "}\n\n";

    return hash;
}

std::string hashStr2(size_t, uint16_t seed, size_t n1, size_t n2, const std::string& default_value, std::string map_name, std::string key_type, bool nonKeyLookups){
    std::string hash =
        "    static inline constexpr uint32_t hash(size_t x, const uint32_t& coeff) noexcept{\n"
        "        x = ((x >> 7) ^ x) * coeff;\n"
        "        x = (x >> 7) ^ x;\n"
        "        return x;\n"
        "    }\n"
        "};\n"
        "\n"
        "constexpr std::string_view " + map_name + "::lookup(const " + key_type + "& key) noexcept{\n"
        "    constexpr uint32_t s0 = " + std::to_string(seed) + ";\n"
        "    const size_t h1 = hash(key, s0) & " + std::to_string(n1) + ";\n"
        "    const uint32_t& s1 = seeds[h1];\n"
        "    const size_t bin = hash(key,s1) & " + std::to_string(n2) + ";\n";
    if(nonKeyLookups) hash +=
        "    return keys[bin] == key ? std::string_view(&flat_vals[val_start[bin]], val_size[bin]) : \"" + default_value + "\";\n";
    else hash +=
        "    #ifndef NDEBUG\n"
        "    assert(keys[bin] == key);\n"
        "    #endif\n\n"
        "    return std::string_view(&flat_vals[val_start[bin]], val_size[bin]);\n";

    hash += "}\n\n";

    return hash;
}

template<typename KeyType>
struct Bin{
    uint32_t generating_hash;
    SeedType seed;
    std::vector<KeyType> keys;

    bool operator<(const Bin& other){
        return keys.size() > other.keys.size();
    }
};

template<typename KeyType>
bool testSeed(const Bin<KeyType>& bin, size_t n2, std::vector<bool>& final_layer){
    for(size_t i = bin.keys.size()-1; i < std::numeric_limits<size_t>::max(); i--){
        const KeyType& key = bin.keys[i];
        const uint32_t h = hash2(key, bin.seed) & n2;
        if(final_layer[h]){
            for(size_t j = i + 1; j < bin.keys.size(); j++){
                const KeyType& key = bin.keys[j];
                const uint32_t h = hash2(key, bin.seed) & n2;
                final_layer[h] = false;
            }

            return false;
        }

        final_layer[h] = true;
    }

    return true;
}

template<typename KeyType>
bool findSeed(Bin<KeyType>& bin, size_t n2, std::vector<bool>& final_layer){
    for(bin.seed = 0; bin.seed < std::numeric_limits<SeedType>::max(); bin.seed++)
        if(testSeed(bin, n2, final_layer)) return true;

    return false;
}

template<typename KeyType>
void writeHash2(const std::vector<KeyType>& keys,
               const SeedType& seed,
               size_t n1,
               size_t n2,
               const std::vector<std::string>& vals,
               std::vector<Bin<KeyType>> layer1,
               std::string& hash_str,
               std::string map_name,
               const std::string& default_value,
               bool nonKeyLookups){

    struct LayerSort{
        inline bool operator() (const Bin<KeyType>& a, const Bin<KeyType>& b){
            return (a.generating_hash < b.generating_hash);
        }
    };
    std::sort(layer1.begin(), layer1.end(), LayerSort());

    std::vector<int> mapping(n2+1, -1);
    for(size_t i = 0; i < keys.size(); i++){
        const KeyType& key = keys[i];
        uint32_t h = hash2(key, seed) & n1;
        uint32_t s1 = layer1[h].seed;
        size_t final = hash2(key, s1) & n2;
        mapping[final] = i;
    }

    hash_str = getCommonCodeGen(keys, vals, mapping, n2, map_name, nonKeyLookups);

    hash_str += "    static constexpr std::array<uint16_t, " + std::to_string(n1+1) + "> seeds {\n        ";

    size_t i = 0;
    for(const auto& bin : layer1){
        if(i && i%entries_per_row==0) hash_str += "\n        ";
        i++;
        hash_str += std::to_string(bin.seed) + ",";
    }
    hash_str += "\n    };\n\n";

    hash_str += hashStr2(keys[0], seed, n1, n2, default_value, map_name, typeStr(keys[0]), nonKeyLookups);

    std::string upper_name = map_name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), toupper);
    hash_str += "#endif // POIFECT_" + upper_name + "_H\n";
}

template<typename KeyType>
static bool testSeed(const std::vector<KeyType>& keys,
                     const SeedType& seed,
                     size_t n1,
                     size_t n2,
                     const std::vector<std::string>& vals,
                     std::string& hash_str,
                     std::string map_name,
                     const std::string& default_value,
                     bool nonKeyLookups){
    std::vector<Bin<KeyType>> layer1(n1+1);
    for(size_t i = 0; i <= n1; i++) layer1[i].generating_hash = i;

    for(const KeyType& key : keys){
        const uint32_t h = hash2(key, seed) & n1;
        layer1[h].keys.push_back(key);
    }

    struct LayerSort{
        inline bool operator() (const Bin<KeyType>& a, const Bin<KeyType>& b){
            return (a.keys.size() > b.keys.size());
        }
    };
    std::sort(layer1.begin(), layer1.end(), LayerSort());

    const size_t max_keys1 = 10;

    if(layer1[0].keys.size() == 1) return true;
    else if(layer1[0].keys.size() >= max_keys1) return false;

    std::vector<bool> final_layer(n2+1, false);

    for(auto& bin : layer1)
        if(!findSeed<KeyType>(bin, n2, final_layer)) return false;

    writeHash2<KeyType>(keys, seed, n1, n2, vals, layer1, hash_str, map_name, default_value, nonKeyLookups);
    return true;
}

template<typename KeyType>
bool hashSearch2(const std::vector<KeyType>& keys,
                 const std::vector<std::string>& vals,
                 std::string& hash_str,
                 std::string map_name = "PoifectMap",
                 std::string default_value = "",
                 uint8_t expansion = 1,
                 uint8_t reduction = 1,
                 bool nonKeyLookups = true){
    assert(keys.size() > 1);
    assert(!hasDuplicates(keys));
    assert(vals.size() == keys.size());

    const size_t n1 = getModulusBitmask(keys.size()*expansion/reduction);
    const size_t n2 = getModulusBitmask(keys.size());

    const uint8_t primes[32] = {  0,   1,   2,   3,   5,
                                  7,  11,  13,  17,  19,
                                 23,  29,  31,  37,  41,
                                 43,  47,  53,  59,  61,
                                 67,  71,  73,  79,  83,
                                 89,  97, 101, 103, 107,
                                109, 113};

    for(SeedType seed = 0; seed < 32; seed++)
        if(testSeed<KeyType>(keys, primes[seed], n1, n2, vals, hash_str, map_name, default_value, nonKeyLookups))
            return true;

    return false;
}

#endif // HASHSEARCH2_H
