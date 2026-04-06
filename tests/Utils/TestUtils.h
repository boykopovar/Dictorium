#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <random>
#include <string>
#include <chrono>

using DataStr = std::vector<std::pair<std::string, double>>;

std::string GenerateStr(std::mt19937& rng, size_t length);
DataStr GenerateDataStr(size_t count, size_t length);

template <typename Func>
long long Benchmark(const DataStr& data, const Func&& func) {
    volatile double sink = 0;

    const auto start = std::chrono::high_resolution_clock::now();

    for (const auto& [k, v] : data) {
        sink = func(k);
    }
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

#endif //TESTUTILS_H
