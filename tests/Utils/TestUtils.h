#ifndef TESTUTILS_H
#define TESTUTILS_H

#define DTR_HASHTAG(x) #x
#define DTR_STR(x) DTR_HASHTAG(x)

#define DICT_PERF_TEST_INIT true

#include <stdexcept>
#include <random>
#include <string>
#include <chrono>
#include <iostream>

#define DICT_PERF_KEYS 1'000'000
#define DICT_PERF_KEY_LEN 10

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

template<typename TKey>
auto GenerateData(const size_t keysCount, const size_t keyLen) {
    if constexpr (std::is_same_v<TKey, std::string>)
        return GenerateDataStr(keysCount, keyLen);
    else return GenerateDataNum<TKey>(keysCount);
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

template<
    template<typename, typename> class StlDict,
    template<typename, typename> class DtrDict,
    typename TKey
>
void start_perf_compare(
    const std::string& stlName,
    const std::string& dtrName,
    const size_t keysCount,
    const size_t keyLen = DICT_PERF_KEY_LEN
) {
    auto data = GenerateData<TKey>(keysCount, keyLen);
    std::cout << "Data generating finish\n";

    const auto umapInitStart = GetNow();
    StlDict<TKey, double> umap(data.begin(), data.end());
    const auto umapInitTime = DurationNs(umapInitStart, GetNow());

    const auto dictInitStart = GetNow();
    DtrDict<TKey, double> dict(data.begin(), data.end());
    const auto dictInitTime = DurationNs(dictInitStart, GetNow());

    const auto umapFunc = [&](const TKey& key) {
        return umap[key];
    };

    const auto dictFunc = [&](const TKey& key) {
        return dict.GetValue(key);
    };

    const auto phTime1 = Benchmark(data, dictFunc);
    const auto umapTime1 = Benchmark(data, umapFunc);

    const auto phTime2 = Benchmark(data, dictFunc);
    const auto umapTime2 = Benchmark(data, umapFunc);

    const auto umapTime = (umapTime1 + umapTime2) / 2;
    const auto phTime = (phTime1 + phTime2) / 2;

    std::cout << "N = " << DICT_PERF_KEYS << '\n';
    if (DICT_PERF_TEST_INIT) {
        std::cout << stlName << " Init time: " << umapInitTime / 1'000'000.0 << "ms\n";
        std::cout << dtrName << " Init time: " << dictInitTime / 1'000'000.0 << "ms\n";

        const auto initRation = std::max(umapInitTime, dictInitTime)
                    / static_cast<double>(std::min(umapInitTime, dictInitTime));
        std::cout << (dictInitTime < umapInitTime ? dtrName : stlName) << " init faster x" << initRation << "\n\n";
    }

    std::cout << stlName <<": " << umapTime / DICT_PERF_KEYS << " ns/op\n";
    std::cout << dtrName << ": " << phTime / DICT_PERF_KEYS << " ns/op\n";

    const auto ratio = static_cast<double>(std::max(umapTime, phTime))
                        / static_cast<double>(std::min(umapTime, phTime));

    std::cout<< (phTime < umapTime ? dtrName : stlName)
        << " read faster" << " x" << ratio << "\n";
}

#endif //TESTUTILS_H
