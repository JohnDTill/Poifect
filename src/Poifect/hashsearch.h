#ifndef HASHSEARCH_H
#define HASHSEARCH_H

#include <array>
#include <functional>
#include <vector>

template<typename T>
static T getHash(const std::string& key,
                 uint8_t c0,
                 uint8_t c1,
                 uint8_t c2,
                 uint8_t c3,
                 uint8_t c4,
                 uint8_t c5){
    T hash = 0;

    for(const char& ch : key){
        hash += c0*static_cast<T>(ch);
        if(c1 != 0) hash *= c1*static_cast<T>(ch);
        hash ^= c2*static_cast<T>(ch);
        hash ^= (c4!=0)*(hash << c4) + (c5!=0)*(hash >> c5) + c3*hash;
    }

    return hash;
}

enum Result{
    COLLISION,
    PERFECT,
    MINIMAL_BY_MODULUS,
    MINIMAL_BY_DEFAULT
};

template<typename T>
static Result check(const std::vector<std::string>& keys,
             uint8_t c0,
             uint8_t c1,
             uint8_t c2,
             uint8_t c3,
             uint8_t c4,
             uint8_t c5){
    std::vector<T> hashes;

    T max_hash = std::numeric_limits<T>::min();
    T min_hash = std::numeric_limits<T>::max();

    for(const std::string& key : keys){
        T hash = getHash<T>(key, c0, c1, c2, c3, c4, c5);
        if(std::find(hashes.begin(), hashes.end(), hash) != hashes.end()) return COLLISION;
        hashes.push_back(hash);

        if(hash > max_hash) max_hash = hash;
        if(hash < min_hash) min_hash = hash;
    }

    if(max_hash - min_hash == keys.size() - 1) return MINIMAL_BY_DEFAULT;

    std::vector<T> mod_hashes;

    for(T hash : hashes){
        T mod_hash = hash % keys.size();

        if(std::find(mod_hashes.begin(), mod_hashes.end(), mod_hash) != mod_hashes.end())
            return PERFECT;
        mod_hashes.push_back(mod_hash);
    }

    return MINIMAL_BY_MODULUS;
}

struct SearchResult{
    std::array<uint8_t, 6> coeffs;
    Result code;
    std::string type;
    std::function<uint64_t(const std::string&)> hash_fn;
    std::string hash_source;

    SearchResult(std::array<uint8_t, 6> coeffs, Result result)
        : coeffs(coeffs), code(result){}
    SearchResult(std::string msg) : hash_source(msg){}
};

template<void(*Callback)(void), typename T>
static SearchResult search(const std::vector<std::string>& keys, uint8_t& progress, const bool& terminate, std::string& msg){
    std::array<uint8_t, 6> best_coeffs;
    Result best_result = COLLISION;
    uint8_t best_nonzero_coeffs = 0;

    for(uint8_t c3 = 0; c3 < 34; c3++){
    for(uint8_t c2 = 0; c2 < 34; c2++){
    for(uint8_t c1 = 0; c1 < 34; c1++){
        progress = static_cast<uint8_t>(
                    100*static_cast<uint16_t>(c3) / 34 +
                    100*static_cast<uint16_t>(c2) / (34*34) +
                    100*static_cast<uint16_t>(c1) / (34*34*34)
                   );
        switch (best_result) {
            case COLLISION: break;
            case PERFECT: msg = "🙂   Found perfect hash"; break;
            case MINIMAL_BY_MODULUS: msg = "😁   Found perfect minimal hash"; break;
            case MINIMAL_BY_DEFAULT: msg = "😂   Found perfect minimal hash w/out modulus"; break;
        }
    for(uint8_t c4 = 0; c4 < 6*sizeof(T); c4++){
        if(terminate) return SearchResult(best_coeffs, best_result);
        Callback();
    for(uint8_t c5 = 0; c5 < 6*sizeof(T); c5++){
    for(uint8_t c0 = 0; c0 < 60; c0++){
        uint8_t nonzero_coeffs = c0!=0 + c1!=0 + c2!=0 + c3!=0 + c4!=0 + c5!=0;
        Result result = check<T>(keys, c0, c1, c2, c3, c4, c5);

        if(result > best_result || (result == best_result && nonzero_coeffs < best_nonzero_coeffs)){
            best_result = result;
            best_coeffs = {c0, c1, c2, c3, c4, c5};
            best_nonzero_coeffs = nonzero_coeffs;
        }
    }}}}}}

    Callback();

    return SearchResult(best_coeffs, best_result);
}

template<void(*Callback)(void)>
SearchResult findHashCoeffs(const std::vector<std::string>& keys, uint8_t& progress, const bool& terminate, std::string& msg){
    msg = "🧐   Searching...";

    if(keys.size() <= 256){
        SearchResult result = search<Callback, uint8_t>(keys, progress, terminate, msg);
        if(terminate || result.code != COLLISION){
            result.type = "uint8_t";
            uint8_t c0 = result.coeffs[0];
            uint8_t c1 = result.coeffs[1];
            uint8_t c2 = result.coeffs[2];
            uint8_t c3 = result.coeffs[3];
            uint8_t c4 = result.coeffs[4];
            uint8_t c5 = result.coeffs[5];
            result.hash_fn = [c0,c1,c2,c3,c4,c5](const std::string& key){
                return getHash<uint8_t>(key, c0, c1, c2, c3, c4, c5);
            };
            return result;
        }else{
            msg = "🤔   Failed hashing to uint8_t, trying uint16_t...";
        }
    }

    if(keys.size() <= 65536){
        SearchResult result = search<Callback, uint16_t>(keys, progress, terminate, msg);
        if(terminate || result.code != COLLISION){
            result.type = "uint16_t";
            uint8_t c0 = result.coeffs[0];
            uint8_t c1 = result.coeffs[1];
            uint8_t c2 = result.coeffs[2];
            uint8_t c3 = result.coeffs[3];
            uint8_t c4 = result.coeffs[4];
            uint8_t c5 = result.coeffs[5];
            result.hash_fn = [c0,c1,c2,c3,c4,c5](const std::string& key){
                return getHash<uint16_t>(key, c0, c1, c2, c3, c4, c5);
            };
            return result;
        }else{
            msg = "🤔   Failed hashing to uint16_t, trying uint32_t...";
        }
    }

    if(keys.size() <= 4294967296){
        SearchResult result = search<Callback, uint32_t>(keys, progress, terminate, msg);
        if(terminate || result.code != COLLISION){
            result.type = "uint32_t";
            uint8_t c0 = result.coeffs[0];
            uint8_t c1 = result.coeffs[1];
            uint8_t c2 = result.coeffs[2];
            uint8_t c3 = result.coeffs[3];
            uint8_t c4 = result.coeffs[4];
            uint8_t c5 = result.coeffs[5];
            result.hash_fn = [c0,c1,c2,c3,c4,c5](const std::string& key){
                return getHash<uint32_t>(key, c0, c1, c2, c3, c4, c5);
            };
            return result;
        }else{
            msg = "🤔   Failed hashing to uint32_t, trying uint64_t...";
        }
    }

    SearchResult result = search<Callback, uint64_t>(keys, progress, terminate, msg);
    result.type = "uint64_t";
    uint8_t c0 = result.coeffs[0];
    uint8_t c1 = result.coeffs[1];
    uint8_t c2 = result.coeffs[2];
    uint8_t c3 = result.coeffs[3];
    uint8_t c4 = result.coeffs[4];
    uint8_t c5 = result.coeffs[5];
    result.hash_fn = [c0,c1,c2,c3,c4,c5](const std::string& key){
        return getHash<uint64_t>(key, c0, c1, c2, c3, c4, c5);
    };

    return result;
}

#endif // HASHSEARCH_H
