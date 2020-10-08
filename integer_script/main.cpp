#include <iostream>

#include <vector>
typedef uint32_t Key;
typedef uint32_t Hash;

static constexpr int num_keys = 125;
static constexpr Hash moduland_max = 512;
static constexpr Hash moduland_min = 256;
static Hash moduland;
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

//#define CHECK_HASHES_SPARSE
#ifdef CHECK_HASHES_SPARSE
static Hash hashes[num_keys];
#else
static bool hash_table[moduland_max+1];
#endif

static constexpr int digits = 6;
static Hash c[digits];
static constexpr Hash c_min[digits] = {0, 0, 0, 0, 0, 0};
static constexpr Hash c_max[digits] = {512, 31, 31, 31, 512, 31};

static uint8_t checkNonzeroCoeffs(){
    uint8_t active_coeffs = 0;
    uint8_t i = digits;
    while(i --> 0) active_coeffs += (c[i]!=0);

    return active_coeffs;
}

Hash hash(Key a){
    a = (a ^ c[0]) ^ (a >> c[1]);
    a = a + (a << c[2]);
    a = a ^ (a >> c[3]);
    a = a * c[4];
    a = a ^ (a >> c[5]);
    return a;
}

/*
Hash hash(Key x)
{
    x = ((x >> c[0]) ^ x) * c[1];
    x = ((x >> c[2]) ^ x) * c[3];
    x = (x >> c[4]) ^ x;
    return x;
}
*/

int checkCollisions(){
    int collisions = 0;

    #ifndef CHECK_HASHES_SPARSE
    int j = moduland+1;
    while(j --> 0) hash_table[j] = false;
    #endif

    uint8_t i = num_keys;
    while(i --> 0){
        #ifdef CHECK_HASHES_SPARSE
        hashes[i] = hash(keys[i]) % moduland;

        uint8_t j = num_keys;
        while(j --> i+1){
            if(hashes[j] == hashes[i]){
                collisions++;
                break;
            }
        }
        #else
        Hash h = hash(keys[i]) & (moduland-1);
        collisions += hash_table[h];
        hash_table[h] = true;
        #endif
    }

    return collisions;
}

static constexpr int dims = 3;
static constexpr int num_iter = 5;
static int active_dims[dims];
static Hash best_ci[dims];
static Hash best_moduland = std::numeric_limits<Hash>::max();
static uint16_t best_collisions = std::numeric_limits<uint16_t>::max();
static uint8_t best_num_c = std::numeric_limits<uint8_t>::max();
static bool keep_searching;

void runLoop(int i){
    int k = active_dims[i];
    for(c[k] = c_min[k]; c[k] <= c_max[k]; c[k]++){
        if(i==0){
            for(moduland = moduland_min; moduland <= moduland_max; moduland*=2){
                auto collisions = checkCollisions();
                const uint8_t num_c = checkNonzeroCoeffs();

                if(collisions < best_collisions ||
                   (collisions == best_collisions && (moduland < best_moduland ||
                   (moduland == best_moduland && num_c < best_num_c)))){
                    best_collisions = collisions;
                    best_moduland = moduland;
                    best_num_c = num_c;
                    for(int j = 0; j < dims; j++) best_ci[j] = c[active_dims[j]];
                    keep_searching = true;

                    std::cout << "Collisions: " << best_collisions << ", Non-zero coeffs: " << (int)num_c
                              << ", Moduland: " << moduland << std::endl;

                    for(int p = 0; p < digits; p++) std::cout << c[p] << ", ";
                    std::cout << std::endl;
                }
            }
        }else{
            runLoop(i-1);
        }
    }
}

void search(){
    keep_searching = true;

    while(keep_searching){
        keep_searching = false;

        for(int i = digits-1; i >= 0; i--){
            active_dims[0] = i;
            for(int j = 1; j < dims; j++){
                bool get_dim = true;
                while(get_dim){
                    active_dims[j] = rand() % digits;
                    get_dim = false;
                    for(int k = j-1; k >= 0; k--) get_dim = get_dim || active_dims[j]==active_dims[k];
                }
            }

            for(int j = 0; j < dims; j++) best_ci[j] = c[active_dims[j]];

            runLoop(dims-1);

            for(int j = 0; j < dims; j++) c[active_dims[j]] = best_ci[j];
        }
    }
}

#include <chrono>
using namespace std::chrono;

int main(){
    //Randomly initialize
    for(int i = 0; i < digits; i++) c[i] = rand() % (c_max[i]-c_min[i]) + c_min[i];
    moduland = moduland_min;
    best_collisions = checkCollisions();
    best_moduland = moduland;
    best_num_c = checkNonzeroCoeffs();

    //Search
    auto start = high_resolution_clock::now();
    for(int i = 0; i < num_iter; i++) search();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    //Report
    std::cout << "\nBest result: " << best_collisions << " collisions\n"
                 "Runtime: " << duration.count() << "s\n"
                 "Coeffs: {";
    for(int i = 0; i < digits; i++) std::cout << c[i] << ", ";
    std::cout << "}\nModuland: " << best_moduland << std::endl;

    return 0;
}
