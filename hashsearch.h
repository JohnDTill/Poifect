#ifndef HASHSEARCH_H
#define HASHSEARCH_H

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include "hashutil.h"

std::array<uint32_t, 6> c;
std::array<uint32_t, 6> c_min {0, 0, 0, 0, 0, 0};
std::array<uint32_t, 6> c_max {7, 7, 7, 7, 7, 7};

static uint8_t checkNonzeroCoeffs(){
    uint8_t active_coeffs = 0;
    for(size_t i = c.size()-1; i < std::numeric_limits<size_t>::max(); i--)
        active_coeffs += c[i]!=0;

    return active_coeffs;
}

static uint32_t hash(uint32_t a){
    a =  (a ^ c[0]) ^ (a >> c[1]);
    a += (a << c[2])*(c[2]!=0);
    a ^= (a >> c[3])*(c[3]!=0);
    a *= c[4]+1;
    a ^= (a >> c[5])*(c[5]!=0);
    return a;
}

static uint32_t hash(const std::string& key){
    uint32_t h = 0;

    for(const char& ch : key)
        h ^= hash(ch);

    return h;
}

std::string hashStr(uint32_t){
    std::string str =
"    static inline constexpr uint32_t hash(uint32_t a) noexcept{\n";
    if(c[0] && c[1])
        str += "        a =  (a ^ " + std::to_string(c[0]) + ") ^ (a >> " + std::to_string(c[1]) + ");\n";
    else if(c[0])
        str += "        a ^= a ^ " + std::to_string(c[0]) + ";\n";
    else if(c[1])
        str += "        a ^= a >> " + std::to_string(c[1]) + ";\n";

    if(c[2]) str += "        a += a << " + std::to_string(c[2]) + ";\n";
    if(c[3]) str += "        a ^= a >> " + std::to_string(c[3]) + ";\n";
    if(c[4]) str += "        a *= " + std::to_string(c[4]+1) + ";\n";
    if(c[5]) str += "        a ^= a >> " + std::to_string(c[5]) + ";\n";

    str += "        return a;\n"
           "    }\n";

    return str;
}

std::string hashStr(uint32_t, size_t n, const std::string& default_value, std::string map_name, std::string key_type, bool nonKeyLookups){
    std::string hash = hashStr(uint32_t()) +
        "};\n"
        "\n"
        "constexpr std::string_view " + map_name + "::lookup(const " + key_type + "& key) noexcept{\n"
        "    const size_t h = hash(key) & " + std::to_string(n) + ";\n";
    if(nonKeyLookups) hash +=
        "    return keys[h] == key ? std::string_view(&flat_vals[val_start[h]], val_size[h]) : \"" + default_value + "\";\n";
    else hash +=
        "    #ifndef NDEBUG\n"
        "    assert(keys[h] == key);\n"
        "    #endif\n\n"
        "    return std::string_view(&flat_vals[val_start[h]], val_size[h]);\n";

    hash += "}\n\n";

    return hash;
}

std::string hashStr(const std::string&, size_t n, const std::string& default_value, std::string map_name, std::string key_type, bool nonKeyLookups){
    std::string hash = hashStr(uint32_t()) + "\n"
"    static inline uint32_t hash(const " + key_type + "& key) noexcept{\n"
"        uint32_t h = 0;\n"
"\n"
"        for(size_t i = key.size()-1; i < std::numeric_limits<size_t>::max(); i--)\n"
"            h ^= hash(key[i]);\n"
"\n"
"        return h;\n"
"    }\n"
"};\n"
"\n"
"std::string_view " + map_name + "::lookup(const " + key_type + "& key) noexcept{\n"
"    const size_t h = hash(key) & " + std::to_string(n) + ";\n";
    if(nonKeyLookups) hash +=
"    return checkBin(key, h) ? std::string_view(&flat_vals[val_start[h]], val_size[h]) : \"" + default_value + "\";\n";
    else hash +=
"    #ifndef NDEBUG\n"
"    assert(checkBin(key, h));\n"
"    #endif\n\n"
"    return std::string_view(&flat_vals[val_start[h]], val_size[h]);\n";

    hash += "}\n\n";

    return hash;
}

template<typename KeyType>
static bool hasCollisions(const std::vector<KeyType>& keys, size_t n, std::vector<bool>& hash_table){
    for(size_t i = n; i < std::numeric_limits<size_t>::max(); i--)
        hash_table[i] = false;

    for(size_t i = keys.size()-1; i < std::numeric_limits<size_t>::max(); i--){
        uint32_t h = hash(keys[i]) & n;
        if(hash_table[h]) return true;
        hash_table[h] = true;
    }

    return false;
}

template<typename KeyType>
bool hashSearch(const std::vector<KeyType>& keys,
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

    size_t n = getModulusBitmask(keys.size() * expansion / reduction);
    std::vector<bool> hash_table(n+1, false);
    uint8_t best_num_c = c.size()+1;
    std::array<uint32_t, c.size()> best_c;

    #define ITERATE_INDEX(i) for(c[i] = c_min[i]; c[i] <= c_max[i]; c[i]++)

    ITERATE_INDEX(0)
    ITERATE_INDEX(1)
    ITERATE_INDEX(2)
    ITERATE_INDEX(3)
    ITERATE_INDEX(4)
    ITERATE_INDEX(5)
    {
        const uint8_t num_c = checkNonzeroCoeffs();
        if(num_c < best_num_c && !hasCollisions(keys, n, hash_table)){
            best_num_c = num_c;
            best_c = c;
        }
    }

    if(best_num_c == c.size()+1) return false;

    c = best_c;
    std::vector<int> mapping(n+1, -1);
    for(size_t i = keys.size()-1; i < std::numeric_limits<size_t>::max(); i--)
        mapping[hash(keys[i])&n] = i;

    hash_str = getCommonCodeGen(keys, vals, mapping, n, map_name, nonKeyLookups);

    hash_str += hashStr(keys[0], n, default_value, map_name, typeStr(keys[0]), nonKeyLookups);

    std::string upper_name = map_name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), toupper);
    hash_str += "#endif // POIFECT_" + upper_name + "_H\n";

    return true;
}

#endif // HASHSEARCH_H
