#include <unordered_map>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

#define DICT_CHAIN_KEY_TYPE std::string
#define DICT_CHAIN_KEYS 1'000'000
#define DICT_CHAIN_KEY_LEN 10
#define DICT_CHAIN_TEST_INIT true

auto GenerateData()
{
    if constexpr (std::is_same_v<DICT_CHAIN_KEY_TYPE, std::string>)
        return GenerateDataStr(DICT_CHAIN_KEYS, DICT_CHAIN_KEY_LEN);
    else
        return GenerateDataNum<DICT_CHAIN_KEY_TYPE>(DICT_CHAIN_KEYS);
}

using namespace dtr;

std::string StlLabel = "unordered_map";
std::string DictLabel = "ChainingHashDictionary";

int main()
{
    auto data = GenerateData();
    std::cout << "Data generating finish\n";

    // ===== INIT BENCHMARK =====
    const auto umapInitStart = GetNow();
    std::unordered_map<DICT_CHAIN_KEY_TYPE, double> umap(data.begin(), data.end());
    const auto umapInitTime = DurationNs(umapInitStart, GetNow());

    const auto dictInitStart = GetNow();
    ChainingHashDictionary<DICT_CHAIN_KEY_TYPE, double> dict(8);
    for (const auto &[k, v] : data)
    {
        dict.InsertOrAssign(k, v);
    }
    const auto dictInitTime = DurationNs(dictInitStart, GetNow());

    // ===== ACCESS FUNCS =====
    const auto umapFunc = [&](const DICT_CHAIN_KEY_TYPE &key) { return umap[key]; };

    const auto dictFunc = [&](const DICT_CHAIN_KEY_TYPE &key) { return dict.GetValue(key); };

    // ===== BENCHMARK READ =====
    const auto chainTime1 = Benchmark(data, dictFunc);
    const auto umapTime1 = Benchmark(data, umapFunc);

    const auto chainTime2 = Benchmark(data, dictFunc);
    const auto umapTime2 = Benchmark(data, umapFunc);

    const auto umapTime = (umapTime1 + umapTime2) / 2;
    const auto chainTime = (chainTime1 + chainTime2) / 2;

    std::cout << "N = " << DICT_CHAIN_KEYS << '\n';

    if (DICT_CHAIN_TEST_INIT)
    {
        std::cout << StlLabel << " Init time: " << umapInitTime / 1'000'000.0 << "ms\n";
        std::cout << DictLabel << " Init time: " << dictInitTime / 1'000'000.0 << "ms\n";

        const auto initRatio =
            std::max(umapInitTime, dictInitTime) / static_cast<double>(std::min(umapInitTime, dictInitTime));

        std::cout << (dictInitTime < umapInitTime ? DictLabel : StlLabel) << " init faster x" << initRatio << "\n\n";
    }

    std::cout << StlLabel << ": " << umapTime / DICT_CHAIN_KEYS << " ns/op\n";

    std::cout << DictLabel << ": " << chainTime / DICT_CHAIN_KEYS << " ns/op\n";

    const auto ratio =
        static_cast<double>(std::max(umapTime, chainTime)) / static_cast<double>(std::min(umapTime, chainTime));

    std::cout << (chainTime < umapTime ? DictLabel : StlLabel) << " read faster x" << ratio << "\n";

    return 0;
}