#include <unordered_map>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

#define DICT_PERF_KEYS 1'000
#define DICT_PERF_KEY_LEN 100
#define DICT_PERF_TEST_INIT false

using namespace dtr;

std::string StlLabel = "unordered_map";
std::string DictLabel = "PerfectHash";

int main() {
    auto data = GenerateDataStr(DICT_PERF_KEYS, DICT_PERF_KEY_LEN);

    const auto umapInitStart = GetNow();
    std::unordered_map<std::string, double> umap(data.begin(), data.end());
    const auto umapInitTime = DurationNs(umapInitStart, GetNow());

    const auto dictInitStart = GetNow();
    PerfectHashDictionary<std::string, double> dict(data.begin(), data.end());
    const auto dictInitTime = DurationNs(dictInitStart, GetNow());

    const auto phTime1 = Benchmark(data, [&](const std::string& key) {
        return dict.GetValue(key);
    });
    const auto umapTime1 = Benchmark(data, [&](const std::string& key) {
        return umap[key];
    });

    const auto phTime2 = Benchmark(data, [&](const std::string& key) {
        return dict.GetValue(key);
    });
    const auto umapTime2 = Benchmark(data, [&](const std::string& key) {
        return umap[key];
    });

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


