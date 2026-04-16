#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <stdexcept>
#include <random>
#include <string>
#include <chrono>

using DataStr = std::vector<std::pair<std::string, double>>;
using DateTime = std::chrono::time_point<std::chrono::steady_clock>;

std::string GenerateStr(std::mt19937& rng, size_t length);
DataStr GenerateDataStr(size_t count, size_t length);

template <typename TKey>
std::vector<std::pair<TKey, double>> GenerateDataNum(const size_t count) {
    static std::mt19937 rng(std::random_device{}());
    using limits = std::numeric_limits<TKey>;

    std::vector<std::pair<TKey, double>> result;
    result.reserve(count);

    if constexpr (std::is_integral_v<TKey>) {
        const size_t range = static_cast<size_t>(limits::max()) - static_cast<size_t>(limits::lowest()) + 1;
        if (static_cast<size_t>(count) > range) throw std::invalid_argument("Too many unique keys for this integer type");

        std::vector<TKey> values(count);
        TKey start = std::numeric_limits<TKey>::lowest();

        for (size_t i = 0; i < count; ++i) {
            values[i] = static_cast<TKey>(start + i);
        }
        std::shuffle(values.begin(), values.end(), rng);

        for (size_t i = 0; i < count; ++i) {
            result.emplace_back(values[i], i*1.1);
        }
    }
    else {
        for (size_t i = 0; i < count; ++i) {
            result.emplace_back(static_cast<TKey>(i), i*1.1);
        }
    }
    return result;
}

DateTime GetNow();
size_t DurationNs(DateTime start, DateTime end);

template <typename Func, typename TKey>
size_t Benchmark(const std::vector<std::pair<TKey, double>>& data, Func&& func) {
    volatile double sink = 0;

    const auto start =GetNow();

    for (const auto& [k, v] : data) {
        sink = func(k);
    }
    return DurationNs(start, GetNow());
}

#endif //TESTUTILS_H
