#include <unordered_map>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

#define DICT_PERF_KEYS 1'000'000
#define DICT_PERF_KEY_LEN 100

using namespace dtr;

int main() {
    auto data = GenerateDataStr(DICT_PERF_KEYS, DICT_PERF_KEY_LEN);

    std::unordered_map<std::string, double> umap(data.begin(), data.end());
    PerfectHashDictionary<std::string, double> dict(data.begin(), data.end(), data.size());

    const auto umapTime = Benchmark(data, [&](const std::string& key) {
        return umap[key];
    });


    const auto phTime = Benchmark(data, [&](const std::string& key) {
        return dict.GetValue(key);
    });

    std::cout << "unordered_map: " << umapTime / DICT_PERF_KEYS << " ns/op\n";
    std::cout << "PerfectHash: " << phTime / DICT_PERF_KEYS << " ns/op\n";

    const auto ratio = static_cast<double>(std::max(umapTime, phTime))
                        / static_cast<double>(std::min(umapTime, phTime));

    std::cout<< (phTime < umapTime ? "PerfectHash" : "unordered_map")
        << " faster" << " x" << ratio << "\n";
}
