#include <unordered_map>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

#define DICT_PERF_KEY_TYPE std::string
#define DICT_PERF_KEYS 1'000'000
#define DICT_PERF_KEY_LEN 10
#define DICT_PERF_TEST_INIT true

#define DTR_TEST_TYPE CuckooHashDictionary
#define STL_TEST_TYPE std::unordered_map


auto GenerateData() {
    if constexpr (std::is_same_v<DICT_PERF_KEY_TYPE, std::string>)
        return GenerateDataStr(DICT_PERF_KEYS, DICT_PERF_KEY_LEN);
    else return GenerateDataNum<DICT_PERF_KEY_TYPE>(DICT_PERF_KEYS);
}

using namespace dtr;

std::string StlLabel = DTR_STR(STL_TEST_TYPE);
std::string DictLabel = DTR_STR(DTR_TEST_TYPE);

int main() {
    auto data = GenerateData();
    std::cout << "Data generating finish\n";

    const auto umapInitStart = GetNow();
    STL_TEST_TYPE<DICT_PERF_KEY_TYPE, double> umap(data.begin(), data.end());
    const auto umapInitTime = DurationNs(umapInitStart, GetNow());

    const auto dictInitStart = GetNow();
    DTR_TEST_TYPE<DICT_PERF_KEY_TYPE, double> dict(data.begin(), data.end());
    const auto dictInitTime = DurationNs(dictInitStart, GetNow());

    const auto umapFunc = [&](const DICT_PERF_KEY_TYPE& key) {
        return umap[key];
    };

    IDictionary<DICT_PERF_KEY_TYPE, double>& dictRef = dict;
    const auto dictFunc = [&](const DICT_PERF_KEY_TYPE& key) {
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
        std::cout << StlLabel << " Init time: " << umapInitTime / 1'000'000.0 << "ms\n";
        std::cout << DictLabel << " Init time: " << dictInitTime / 1'000'000.0 << "ms\n";

        const auto initRation = std::max(umapInitTime, dictInitTime)
                    / static_cast<double>(std::min(umapInitTime, dictInitTime));
        std::cout << (dictInitTime < umapInitTime ? DictLabel : StlLabel) << " init faster x" << initRation << "\n\n";
    }

    std::cout << StlLabel <<": " << umapTime / DICT_PERF_KEYS << " ns/op\n";
    std::cout << DictLabel << ": " << phTime / DICT_PERF_KEYS << " ns/op\n";

    const auto ratio = static_cast<double>(std::max(umapTime, phTime))
                        / static_cast<double>(std::min(umapTime, phTime));

    std::cout<< (phTime < umapTime ? DictLabel : StlLabel)
        << " read faster" << " x" << ratio << "\n";
}


