#ifndef HASHBENCHMARK_H
#define HASHBENCHMARK_H

#include <chrono>
#include <iostream>
#include <vector>

template<class Map, typename KeyType>
void runBenchmark(const std::vector<KeyType> keys){
    constexpr size_t n = 100000;
    const auto start = std::chrono::high_resolution_clock::now();

    for(size_t i = 0; i < n; i++)
        for(const auto& key : keys)
            Map::lookup(key);

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "average lookup time: " << 1000*duration.count() / (double)(keys.size()*n) << "ns" << std::endl;
}

#endif // HASHBENCHMARK_H
