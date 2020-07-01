#include "mainwindow.h"

#include <QApplication>
#include <array>
#include <iostream>

template<typename T>
T getHash(const std::string& key, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4, uint8_t c5){
    T hash = 0;

    for(const char& ch : key){
        hash += c0*ch;
        if(c1 != 0) hash *= c1*ch;
        hash ^= c2*ch;
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
Result check(const std::vector<std::string>& keys, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4, uint8_t c5){
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

template<typename T>
std::array<uint8_t, 7> search(const std::vector<std::string>& keys){
    std::array<uint8_t, 7> best_coeffs;
    Result best_result = COLLISION;
    uint8_t best_nonzero_coeffs = 0;

    for(uint8_t c3 = 0; c3 < 34; c3++){
    for(uint8_t c2 = 0; c2 < 34; c2++){
    for(uint8_t c1 = 0; c1 < 34; c1++){
    for(uint8_t c4 = 0; c4 < 6*sizeof(T); c4++){
    for(uint8_t c5 = 0; c5 < 6*sizeof(T); c5++){
    for(uint8_t c0 = 0; c0 < 60; c0++){
        uint8_t nonzero_coeffs = c0!=0 + c1!=0 + c2!=0 + c3!=0 + c4!=0 + c5!=0;
        Result result = check<T>(keys, c0, c1, c2, c3, c4, c5);

        if(result == MINIMAL_BY_DEFAULT){
            return {c0, c1, c2, c3, c4, c5, MINIMAL_BY_DEFAULT};
        }else if(result > best_result || (result == best_result && nonzero_coeffs < best_nonzero_coeffs)){
            best_result = result;
            best_coeffs = {c0, c1, c2, c3, c4, c5, static_cast<uint8_t>(result)};
            best_nonzero_coeffs = nonzero_coeffs;
        }
    }}}}}}

    return best_coeffs;
}

std::array<uint8_t, 8> findHashCoeffs(const std::vector<std::string>& keys){
    if(keys.size() <= 256){
        std::array<uint8_t, 7> result = search<uint8_t>(keys);
        if(result.back() != COLLISION){
            std::array<uint8_t, 8> result_and_size;
            for(int i = 0; i < 7; i++) result_and_size[i] = result[i];
            result_and_size[7] = 0;

            return result_and_size;
        }
    }

    if(keys.size() <= 65536){
        std::array<uint8_t, 7> result = search<uint16_t>(keys);
        if(result.back() != COLLISION){
            std::array<uint8_t, 8> result_and_size;
            for(int i = 0; i < 7; i++) result_and_size[i] = result[i];
            result_and_size[7] = 1;

            return result_and_size;
        }
    }

    if(keys.size() <= 4294967296){
        std::array<uint8_t, 7> result = search<uint32_t>(keys);
        if(result.back() != COLLISION){
            std::array<uint8_t, 8> result_and_size;
            for(int i = 0; i < 7; i++) result_and_size[i] = result[i];
            result_and_size[7] = 2;

            return result_and_size;
        }
    }

    std::array<uint8_t, 7> result = search<uint64_t>(keys);

    std::array<uint8_t, 8> result_and_size;
    for(int i = 0; i < 7; i++) result_and_size[i] = result[i];
    result_and_size[7] = 3;

    return result_and_size;
}

std::string getHashFunctionSource(const std::array<uint8_t, 8>& c, const std::string& type){
    std::string out;

    out += type + " getHash(const std::string& key){\n"
           "    " + type + " hash = 0;\n"
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

std::string writePerfectHash(const std::array<uint8_t, 8>& c,
                             const std::vector<std::string>& keys,
                             const std::vector<std::string>& vals,
                             const std::string& return_type,
                             const std::string& default_value){
    if(c[6]==COLLISION) return "Failed to find perfect hash function";

    uint8_t max_length = 0;
    uint8_t min_length = 255;
    uint64_t min_hash = std::numeric_limits<uint64_t>::max();
    uint64_t max_hash = 0;
    for(const std::string& key : keys){
        if(key.size() > max_length) max_length = key.size();
        if(key.size() < min_length) min_length = key.size();

        for(const std::string& key : keys){
            uint64_t hash = c[7]==0 ? getHash<uint8_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            c[7]==1 ? getHash<uint16_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            c[7]==2 ? getHash<uint32_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            getHash<uint64_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]);
            if(hash < min_hash) min_hash = hash;
        }
    }

    std::string type = c[7]==0 ? "uint8_t" :
                       c[7]==1 ? "uint16_t" :
                       c[7]==2 ? "uint32_t" :
                                 "uint64_t";

    std::string out = getHashFunctionSource(c, type);

    out += "\n\n";

    if(c[6] >= MINIMAL_BY_MODULUS){
        out += "const std::string keys[" + std::to_string(keys.size()) + "] {\n";

        std::vector<std::string> sorted_hashes;
        std::vector<std::string> sorted_vals;
        sorted_hashes.resize(keys.size());
        sorted_vals.resize(keys.size());

        for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
            const std::string& key = keys[i];
            uint64_t hash = c[7]==0 ? getHash<uint8_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            c[7]==1 ? getHash<uint16_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            c[7]==2 ? getHash<uint32_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            getHash<uint64_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]);
            if(c[6] == MINIMAL_BY_MODULUS) hash %= keys.size();
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

    out += "    " + type + " hash = getHash(key)";

    if(c[6]==MINIMAL_BY_MODULUS){
        bool power_of_2 = keys.size() != 0 && (keys.size() & (keys.size()-1)) == 0;

        if(power_of_2){
            int log2;
            std::vector<std::string>::size_type x = keys.size();
            for (log2=0; x != 1; x>>=1,log2++);
            out += " & " + std::to_string(log2-1);
        }else{
            out += " % " + std::to_string(keys.size());
        }
    }else if(c[6]==MINIMAL_BY_DEFAULT){
        out += " - " + std::to_string(min_hash);
    }

    out += ";\n\n";

    if(c[6] >= MINIMAL_BY_MODULUS){
        out += "    return (";
        if(c[6] == MINIMAL_BY_DEFAULT)
            out += "hash < " + std::to_string(keys.size()) + " && ";
        out += "key == keys[hash]) ? vals[hash] : " + default_value + ";\n";
    }else{
        out += "    switch(hash){\n";
        for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
            const std::string& key = keys.at(i);
            uint64_t hash = c[7]==0 ? getHash<uint8_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            c[7]==1 ? getHash<uint16_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            c[7]==2 ? getHash<uint32_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]) :
                            getHash<uint64_t>(key, c[0], c[1], c[2], c[3], c[4], c[5]);

            out += "      case " + std::to_string(hash) + ": return key==\"" + key + "\" ? " + vals.at(i) +
                   " : " + default_value + ";\n";
        }
        out += "        default: return " + default_value + ";\n";
        out += "    }\n";
    }

    out += "}";

    return out;
}

int main(int argc, char *argv[]){
    /*
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
    */

    static const std::vector<std::string> keys = {"alpha", "beta", "gamma", "theta", "pi"};
    static const std::vector<std::string> vals = {"α", "β", "γ", "θ", "π"};

    if(keys.size() != vals.size()){
        std::cout << "Each key must have a value";
    }

    auto result = findHashCoeffs(keys);

    std::cout << "Result: " << std::to_string(result[6]) << "\nCoefficients: ";
    for(int i = 0; i < 6; i++) std::cout << std::to_string(result[i]) << ' ';
    std::cout << std::endl;

    std::cout << writePerfectHash(result, keys, vals, "QString", "QString()") << std::endl;

    return 0;
}
