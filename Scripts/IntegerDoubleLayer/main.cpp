#include <iostream>

#include <algorithm>
#include <array>
#include <string>
#include <vector>
typedef uint32_t Key;
typedef uint16_t Hash;
typedef uint16_t Moduland;

static constexpr int num_keys = 125;
static constexpr int num_intd = 32;
static constexpr int num_final = 128;

static constexpr Key keys[num_keys] = {8501 ,
                              64 ,
                              92 ,
                              124 ,
                              8757 ,
                              8502 ,
                              8745 ,
                              94 ,
                              58 ,
                              44 ,
                              8743 ,
                              8750 ,
                              8746 ,
                              8224 ,
                              8788 ,
                              176 ,
                              8744 ,
                              247 ,
                              36 ,
                              8901 ,
                              8214 ,
                              8225 ,
                              8252 ,
                              8811 ,
                              8810 ,
                              8450 ,
                              8518 ,
                              8461 ,
                              8520 ,
                              8469 ,
                              8473 ,
                              8474 ,
                              8477 ,
                              8484 ,
                              8709 ,
                              61 ,
                              8801 ,
                              8364 ,
                              33 ,
                              8707 ,
                              8704 ,
                              47 ,
                              62 ,
                              8805 ,
                              8459 ,
                              8712 ,
                              8734 ,
                              8747 ,
                              8466 ,
                              10216 ,
                              8592 ,
                              91 ,
                              123 ,
                              8968 ,
                              10218 ,
                              10214 ,
                              8970 ,
                              40 ,
                              60 ,
                              8804 ,
                              8614 ,
                              9205 ,
                              9204 ,
                              45 ,
                              8723 ,
                              42 ,
                              8711 ,
                              10 ,
                              8715 ,
                              172 ,
                              8800 ,
                              8708 ,
                              8815 ,
                              8713 ,
                              8814 ,
                              8716 ,
                              8836 ,
                              8840 ,
                              10752 ,
                              8855 ,
                              8706 ,
                              37 ,
                              46 ,
                              8869 ,
                              8462 ,
                              43 ,
                              177 ,
                              35 ,
                              163 ,
                              8826 ,
                              8733 ,
                              9632 ,
                              10217 ,
                              8594 ,
                              8658 ,
                              93 ,
                              125 ,
                              8969 ,
                              10219 ,
                              10215 ,
                              8971 ,
                              41 ,
                              59 ,
                              8333 ,
                              8331 ,
                              8330 ,
                              8334 ,
                              8834 ,
                              8838 ,
                              8827 ,
                              8317 ,
                              8315 ,
                              8314 ,
                              8318 ,
                              11389 ,
                              7610 ,
                              8230 ,
                              8756 ,
                              10727 ,
                              39 ,
                              126 ,
                              215 ,
                              8868 ,
                              8921 ,
                              8920};

static constexpr Hash seed_min = 0;
static constexpr Hash seed_max = std::numeric_limits<uint16_t>::max() - 1;

static Hash seed = 403;
static Hash local_seeds[num_intd];

static Hash hashLayer(Hash x, Hash coeff) {
    x = ((x >> 8) ^ x) * coeff;
    x = (x >> 8) ^ x;
    return x;
}

static bool compare(const std::vector<Key>& a, const std::vector<Key>& b){
    return a.size() > b.size();
}

static void hashAll(){
    std::array<std::vector<Key>, num_intd> intermediate_table;

    for(const Key& key : keys){
        const Hash hash = hashLayer(key, seed) % num_intd;
        intermediate_table[hash].push_back(key);
    }

    std::sort(intermediate_table.begin(), intermediate_table.end(), compare);

    static bool final_table[num_final];
    for(int i = 0; i < num_final; i++) final_table[i] = false;

    for(int i = 0; i < num_intd; i++){
        const std::vector<Key>& bucket = intermediate_table.at(i);
        bool seed_ok;

        for(Hash local_seed = seed_min; local_seed <= seed_max; local_seed++){
            seed_ok = true;

            for(unsigned i = 0; i < bucket.size(); i++){
                const Key& key = bucket.at(i);
                const Hash hash = hashLayer(key, local_seed) % num_final;
                if(final_table[hash]){
                    for(int j = i-1; j >= 0; j--){
                        const Key& key = bucket.at(j);
                        const Hash hash = hashLayer(key, local_seed) % num_final;
                        final_table[hash] = false;
                    }
                    seed_ok = false;
                    break;
                }else{
                    final_table[hash] = true;
                }
            }

            if(seed_ok){
                local_seeds[i] = local_seed;
                break;
            }
        }

        if(!seed_ok){
            std::cout << "Failed to find perfect hash" << std::endl;
            return;
        }
    }
}

#include <chrono>
using namespace std::chrono;

int main(){
    //Search
    auto start = high_resolution_clock::now();
    hashAll();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    //Report
    std::cout << "Runtime: " << duration.count() << "s" << std::endl;

    return 0;
}
