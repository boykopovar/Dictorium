#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <random>
#include <string>
#include <chrono>

using DataStr = std::vector<std::pair<std::string, double>>;
using DateTime = std::chrono::time_point<std::chrono::system_clock>;

std::string GenerateStr(std::mt19937& rng, size_t length);
DataStr GenerateDataStr(size_t count, size_t length);

DateTime GetNow();
size_t DurationNs(DateTime start, DateTime end);

template <typename Func>
long long Benchmark(const DataStr& data, Func&& func) {
    volatile double sink = 0;

    const auto start =GetNow();

    for (const auto& [k, v] : data) {
        sink = func(k);
    }
    return DurationNs(start, GetNow());
}

#endif //TESTUTILS_H
