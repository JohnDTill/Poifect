#include <iostream>

#include <string>
#include <vector>
typedef uint32_t Key;
typedef uint16_t Hash;
typedef uint16_t Moduland;

static constexpr int num_keys = 125;
static constexpr Moduland moduland_max = 512;
static constexpr Moduland moduland_min = 256;
static Moduland moduland;
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
static bool hash_table[moduland_max];
#endif

static constexpr int shft_max = 8*sizeof(Hash)-1;

static constexpr int digits = 6;
static Hash c[digits];
static constexpr Hash c_min[digits] = {0, 0, 0, 0, 0, 0};
static constexpr Hash c_max[digits] = {254, shft_max, shft_max, shft_max, 254, shft_max};

static uint8_t checkNonzeroCoeffs(){
    uint8_t active_coeffs = 0;
    uint8_t i = digits;
    while(i --> 0) active_coeffs += (c[i]!=0);

    return active_coeffs;
}

static Hash hash(Key a){
    a = (a ^ c[0]) ^ (a >> c[1]);
    a = a + (a << c[2]);
    a = a ^ (a >> c[3]);
    a = a * c[4];
    a = a ^ (a >> c[5]);
    return a & (moduland-1);
}

static bool hasCollisions(){
    #ifndef CHECK_HASHES_SPARSE
    Hash j = moduland;
    while(j --> 0) hash_table[j] = false;
    #endif

    uint8_t i = num_keys;
    while(i --> 0){
        #ifdef CHECK_HASHES_SPARSE
        hashes[i] = hash(keys[i]) % moduland;

        uint8_t j = num_keys;
        while(j --> i+1)
            if(hashes[j] == hashes[i]) return true;
        #else
        Hash h = hash(keys[i]);
        if(hash_table[h]) return true;
        hash_table[h] = true;
        #endif
    }

    return false;
}

static int countCollisions(){
    int collisions = 0;

    #ifndef CHECK_HASHES_SPARSE
    int j = moduland;
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
        Hash h = hash(keys[i]);
        collisions += hash_table[h];
        hash_table[h] = true;
        #endif
    }

    return collisions;
}

std::string getHashString(){
    return
"static Hash hash(Key a){\n"
"    a = (a ^ " + std::to_string(c[0]) + ") ^ (a >> " + std::to_string(c[1]) + ");\n"
"    a = a + (a << " + std::to_string(c[2]) + ");\n"
"    a = a ^ (a >> " + std::to_string(c[3]) + ");\n"
"    a = a * " + std::to_string(c[4]) + ";\n"
"    a = a ^ (a >> " + std::to_string(c[5]) + ");\n"
"    return a;\n"
"}\n";
}

std::string getHashTable(){
    int hash_table[moduland_max];
    int j = moduland_max;
    while(j --> 0) hash_table[j] = -1;

    std::string tbl = "";
    int i = num_keys;
    while(i --> 0){
        Key k = keys[i];
        Hash h = hash(k);

        tbl += std::to_string(k);
        for(uint32_t j = 10000; j > 1; j /= 10) if(k/j==0) tbl += ' ';
        tbl += ", " + std::to_string(h);
        for(uint32_t j = 10000; j > 1; j = j /= 10) if(h/j==0) tbl += ' ';
        if(hash_table[h] >= 0) tbl += ", collides w/ " + std::to_string(hash_table[h]);
        tbl += '\n';

        hash_table[h] = k;
    }

    return tbl;
}

static constexpr int dims = 4;
static constexpr int num_iter = 1;
static int active_dims[dims];
static Hash best_c[digits];
static Hash best_moduland = std::numeric_limits<Hash>::max();
static uint16_t best_collisions = std::numeric_limits<uint16_t>::max();
static uint8_t best_num_c = std::numeric_limits<uint8_t>::max();
static bool keep_searching;

static void runLoop(int i){
    int k = active_dims[i];
    for(c[k] = c_min[k]; c[k] <= c_max[k]; c[k]++){
        if(i==0){
            for(moduland = moduland_min; moduland <= moduland_max; moduland*=2){
                const uint8_t num_c = checkNonzeroCoeffs();
                if(best_collisions==0 &&
                   (moduland > best_moduland || (moduland==best_moduland && num_c >= best_num_c)))
                    continue;

                auto collisions = countCollisions();

                if(collisions < best_collisions ||
                   (collisions == best_collisions && (moduland < best_moduland ||
                   (moduland == best_moduland && num_c < best_num_c)))){
                    best_collisions = collisions;
                    best_moduland = moduland;
                    best_num_c = num_c;
                    for(int j = 0; j < dims; j++) best_c[j] = c[active_dims[j]];
                    keep_searching = true;

                    std::cout << "--IMPROVEMENT-- Collisions: " << collisions
                              << ", Moduland: " << (int)moduland
                              << ", Non-zero coeffs: " << (int)num_c
                              << ", Coeffs: {" << (int)c[0];
                    for(int p = 1; p < digits; p++) std::cout << ", " << (int)c[p];
                    std::cout << '}' << std::endl;
                }
            }
        }else{
            runLoop(i-1);
        }
    }
}

void generalizedLineSearch(){
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

            for(int j = 0; j < dims; j++) best_c[j] = c[active_dims[j]];

            runLoop(dims-1);

            for(int j = 0; j < dims; j++) c[active_dims[j]] = best_c[j];
        }
    }

    moduland = best_moduland;
}

#define ITERATE_INDEX(i) for(c[i] = c_min[i]; c[i] <= c_max[i]; c[i]++)

void bruteForceSearch(){
    best_moduland = std::numeric_limits<Hash>::max();
    best_num_c = std::numeric_limits<uint8_t>::max();

    ITERATE_INDEX(0){
        std::cout << c[0]*100 / c_max[0] << '%' << std::endl;
    ITERATE_INDEX(1)
    ITERATE_INDEX(2)
    ITERATE_INDEX(3)
    ITERATE_INDEX(4)
    ITERATE_INDEX(5)
    for(moduland = moduland_min; moduland <= moduland_max; moduland*=2){
        const uint8_t num_c = checkNonzeroCoeffs();
        if(moduland > best_moduland
           || (moduland==best_moduland && num_c >= best_num_c)
           || hasCollisions()) continue;

        best_collisions = 0;
        best_moduland = moduland;
        best_num_c = num_c;
        for(int j = 0; j < digits; j++) best_c[j] = c[j];

        std::cout << "--IMPROVEMENT-- Moduland: " << (int)moduland
                  << ", Non-zero coeffs: " << (int)num_c
                  << ", Coeffs: {" << (int)c[0];
        for(int p = 1; p < digits; p++) std::cout << ", " << (int)c[p];
        std::cout << '}' << std::endl;
    }
    }

    for(int j = 0; j < digits; j++) c[j] = best_c[j];
}

#include <chrono>
using namespace std::chrono;

int main(){
    //Randomly initialize
    for(int i = 0; i < digits; i++) c[i] = rand() % (c_max[i]-c_min[i]) + c_min[i];
    moduland = moduland_min;
    best_collisions = countCollisions();
    best_moduland = moduland;
    best_num_c = checkNonzeroCoeffs();

    //Search
    auto start = high_resolution_clock::now();
    for(int i = 0; i < num_iter; i++) generalizedLineSearch();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    //Report
    std::cout << "\nBest result: " << best_collisions << " collisions\n"
                 "Runtime: " << duration.count() << "s\n"
                 "Coeffs: {";
    for(int i = 0; i < digits; i++) std::cout << (int)c[i] << ", ";
    std::cout << "}\nModuland: " << best_moduland << std::endl;
    std::cout << getHashString() << std::endl;
    std::cout << getHashTable() << std::endl;

    return 0;
}
