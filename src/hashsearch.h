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
        hash *= c1*static_cast<T>(ch) + (c1 == 0);
        hash ^= c2*static_cast<T>(ch);
        hash ^= (c4!=0)*(hash << c4) + (c5!=0)*(hash >> c5) + c3*hash;
    }

    return hash;
}

enum Result{
    COLLISION,
    PERFECT,
    DENSE_BY_MOD,
    DENSE_BY_DEFAULT,
    MINIMAL_BY_MOD,
    MINIMAL_BY_DEFAULT
};

static Result check8_t(const std::vector<std::string>& keys,
             uint8_t c0,
             uint8_t c1,
             uint8_t c2,
             uint8_t c3,
             uint8_t c4,
             uint8_t c5,
             uint8_t acceptable_empties,
             uint8_t& empty_entries){
    bool hashes[256];
    std::fill_n(hashes, 256, false);

    for(const std::string& key : keys){
        uint8_t hash = getHash<uint8_t>(key, c0, c1, c2, c3, c4, c5);
        if(hashes[hash]) return COLLISION;
        hashes[hash] = true;
    }

    uint8_t max_hash;
    for(max_hash = 255; !hashes[max_hash]; max_hash--);
    uint8_t min_hash;
    for(min_hash = 0; !hashes[min_hash]; min_hash++);

    empty_entries = max_hash - min_hash - (keys.size() - 1);
    if(empty_entries == 0) return MINIMAL_BY_DEFAULT;

    bool minimal_by_mod = true;
    bool mod_hashes[256];
    std::fill_n(mod_hashes, keys.size(), false);
    for(uint16_t quotient = 0; quotient <= max_hash; quotient += keys.size()){
        for(uint8_t modulus = 0; modulus < keys.size() && quotient+modulus <= max_hash; modulus++){
            if(!hashes[quotient + modulus]) continue;
            if(mod_hashes[modulus]){
                minimal_by_mod = false;
                quotient = 256;
                break;
            }
            mod_hashes[modulus] = true;
        }
    }

    if(minimal_by_mod){
        empty_entries = 0;
        return MINIMAL_BY_MOD;
    }else if(empty_entries <= acceptable_empties){
        return DENSE_BY_DEFAULT;
    }

    uint8_t sze = keys.size() + acceptable_empties;
    std::fill_n(mod_hashes, sze, false);
    for(uint16_t quotient = 0; quotient <= max_hash; quotient += sze){
        for(uint8_t modulus = 0; modulus < sze && quotient + modulus <= max_hash; modulus++){
            if(!hashes[quotient + modulus]) continue;
            if(mod_hashes[modulus]) return PERFECT;
            mod_hashes[modulus] = true;
        }
    }

    empty_entries = acceptable_empties;
    return DENSE_BY_MOD;
}

#include <set>
template<typename T>
static Result check(const std::vector<std::string>& keys,
             uint8_t c0,
             uint8_t c1,
             uint8_t c2,
             uint8_t c3,
             uint8_t c4,
             uint8_t c5,
             T acceptable_empties,
             T& empty_entries){
    std::set<T> hashes;

    for(const std::string& key : keys){
        T hash = getHash<T>(key, c0, c1, c2, c3, c4, c5);
        if(!hashes.insert(hash).second) return COLLISION;
    }

    T min_hash = *hashes.begin();
    T max_hash = *hashes.rbegin();

    empty_entries = max_hash - min_hash - (keys.size() - 1);
    if(empty_entries == 0) return MINIMAL_BY_DEFAULT;

    bool minimal_by_mod = true;
    std::set<T> mod_hashes;
    for(T hash : hashes){
        T mod_hash = hash % keys.size();

        if(!mod_hashes.insert(mod_hash).second){
            minimal_by_mod = false;
            break;
        }
    }

    if(minimal_by_mod){
        empty_entries = 0;
        return MINIMAL_BY_MOD;
    }else if(empty_entries <= acceptable_empties){
        return DENSE_BY_DEFAULT;
    }

    mod_hashes.clear();
    for(T hash : hashes){
        T mod_hash = hash % (keys.size() + acceptable_empties);

        if(!mod_hashes.insert(mod_hash).second) return PERFECT;
    }

    empty_entries = acceptable_empties;
    return DENSE_BY_MOD;
}

struct SearchResult{
    std::array<uint8_t, 6> coeffs;
    Result code;
    std::string type;
    std::function<uint64_t(const std::string&)> hash_fn;
    std::string hash_source;
    uint64_t num_empty_entries;

    SearchResult(std::array<uint8_t, 6> coeffs, Result result, uint64_t num_empty_entries)
        : coeffs(coeffs), code(result), num_empty_entries(num_empty_entries) {}
    SearchResult(std::string msg) : hash_source(msg){}
};

template<void(*Callback)(void)>
static SearchResult search8_t(const std::vector<std::string>& keys,
                           uint8_t acceptable_empties,
                           uint8_t& progress,
                           const bool& terminate,
                           std::string& msg){
    std::array<uint8_t, 6> best_coeffs;
    Result best_result = COLLISION;
    uint8_t best_nonzero_coeffs = 0;
    uint8_t best_empty_entries = 255;

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
            case DENSE_BY_MOD: msg = "😊   Found perfect dense hash"; break;
            case DENSE_BY_DEFAULT: msg = "🤭   Found perfect dense hash w/out modulus"; break;
            case MINIMAL_BY_MOD: msg = "😁   Found perfect minimal hash"; break;
            case MINIMAL_BY_DEFAULT: msg = "😍   Found perfect minimal hash w/out modulus"; break;
        }
    for(uint8_t c4 = 0; c4 < 6; c4++){
        if(terminate) return SearchResult(best_coeffs, best_result, best_empty_entries);
        Callback();
    for(uint8_t c5 = 0; c5 < 6; c5++){
    for(uint8_t c0 = 0; c0 < 60; c0++){
        uint8_t nonzero_coeffs = c0!=0 + c1!=0 + c2!=0 + c3!=0 + c4!=0 + c5!=0;
        uint8_t num_empty_entries;
        Result result = check8_t(keys, c0, c1, c2, c3, c4, c5, acceptable_empties, num_empty_entries);

        if(result > best_result ||
           (result == best_result && num_empty_entries < best_empty_entries) ||
           (result == best_result && num_empty_entries == best_empty_entries && nonzero_coeffs < best_nonzero_coeffs)){
            best_result = result;
            best_coeffs = {c0, c1, c2, c3, c4, c5};
            best_empty_entries = num_empty_entries;
            best_nonzero_coeffs = nonzero_coeffs;
        }
    }}}}}}

    Callback();

    return SearchResult(best_coeffs, best_result, best_empty_entries);
}


template<void(*Callback)(void), typename T>
static SearchResult search(const std::vector<std::string>& keys,
                           T acceptable_empties,
                           uint8_t& progress,
                           const bool& terminate,
                           std::string& msg){
    std::array<uint8_t, 6> best_coeffs;
    Result best_result = COLLISION;
    uint8_t best_nonzero_coeffs = 0;
    T best_empty_entries = std::numeric_limits<T>::max();

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
            case DENSE_BY_MOD: msg = "😊   Found perfect dense hash"; break;
            case DENSE_BY_DEFAULT: msg = "🤭   Found perfect dense hash w/out modulus"; break;
            case MINIMAL_BY_MOD: msg = "😁   Found perfect minimal hash"; break;
            case MINIMAL_BY_DEFAULT: msg = "😍   Found perfect minimal hash w/out modulus"; break;
        }
    for(uint8_t c4 = 0; c4 < 6*sizeof(T); c4++){
        if(terminate) return SearchResult(best_coeffs, best_result, best_empty_entries);
        Callback();
    for(uint8_t c5 = 0; c5 < 6*sizeof(T); c5++){
    for(uint8_t c0 = 0; c0 < 60; c0++){
        uint8_t nonzero_coeffs = c0!=0 + c1!=0 + c2!=0 + c3!=0 + c4!=0 + c5!=0;
        T num_empty_entries;
        Result result = check<T>(keys, c0, c1, c2, c3, c4, c5, acceptable_empties, num_empty_entries);

        if(result > best_result ||
           (result == best_result && num_empty_entries < best_empty_entries) ||
           (result == best_result && num_empty_entries == best_empty_entries && nonzero_coeffs < best_nonzero_coeffs)){
            best_result = result;
            best_coeffs = {c0, c1, c2, c3, c4, c5};
            best_empty_entries = num_empty_entries;
            best_nonzero_coeffs = nonzero_coeffs;
        }
    }}}}}}

    Callback();

    return SearchResult(best_coeffs, best_result, best_empty_entries);
}

template<void(*Callback)(void)>
SearchResult findHashCoeffs(const std::vector<std::string>& keys, uint64_t acceptable_empties, uint8_t& progress, const bool& terminate, std::string& msg){
    msg = "🧐   Searching...";

    if(keys.size() <= 256){
        SearchResult result = keys.size() < 10 ?
                              search<Callback, uint8_t>(keys, acceptable_empties, progress, terminate, msg) :
                              search8_t<Callback>(keys, acceptable_empties, progress, terminate, msg);
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
        SearchResult result = search<Callback, uint16_t>(keys, acceptable_empties, progress, terminate, msg);
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
        SearchResult result = search<Callback, uint32_t>(keys, acceptable_empties, progress, terminate, msg);
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

    SearchResult result = search<Callback, uint64_t>(keys, acceptable_empties, progress, terminate, msg);
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
