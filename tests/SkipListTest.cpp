#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

#define DICT_SKIP_KEY_TYPE std::string
#define DICT_SKIP_KEYS 1'000'000
#define DICT_SKIP_KEY_LEN 10
#define DICT_SKIP_TEST_INIT true
#define DICT_SKIP_INSERT_OPS 10'000
#define DICT_SKIP_REMOVE_OPS 100'000

using namespace dtr;

std::string StlLabel = "std::map";
std::string DictLabel = "SkipListDictionary";

auto GenerateData() {
    if constexpr (std::is_same_v<DICT_SKIP_KEY_TYPE, std::string>) {
        return GenerateDataStr(DICT_SKIP_KEYS, DICT_SKIP_KEY_LEN);
    } else {
        return GenerateDataNum<DICT_SKIP_KEY_TYPE>(DICT_SKIP_KEYS);
    }
}

static void PrintInitTimes(size_t stlNs, size_t dictNs) {
    std::cout << StlLabel << " Init time: " << stlNs / 1'000'000.0 << "ms\n";
    std::cout << DictLabel << " Init time: " << dictNs / 1'000'000.0 << "ms\n";

    const auto initRatio =
        static_cast<double>(std::max(stlNs, dictNs)) / static_cast<double>(std::min(stlNs, dictNs));

    std::cout << (dictNs < stlNs ? DictLabel : StlLabel) << " init faster x" << initRatio << "\n\n";
}

static void PrintCompare(const std::string& opLabel, size_t stlNs, size_t dictNs, size_t ops) {
    std::cout << StlLabel << ": " << static_cast<double>(stlNs) / static_cast<double>(ops) << " ns/op\n";
    std::cout << DictLabel << ": " << static_cast<double>(dictNs) / static_cast<double>(ops) << " ns/op\n";

    const auto ratio =
        static_cast<double>(std::max(stlNs, dictNs)) / static_cast<double>(std::min(stlNs, dictNs));

    std::cout << (dictNs < stlNs ? DictLabel : StlLabel) << ' ' << opLabel << " faster x" << ratio << "\n\n";
}

int main() {
    auto data = GenerateData();
    std::cout << "Data generating finish\n";

    const auto mapInitStart = GetNow();
    std::map<DICT_SKIP_KEY_TYPE, double> mapDict(data.begin(), data.end());
    const auto mapInitTime = DurationNs(mapInitStart, GetNow());

    const auto skipInitStart = GetNow();
    SkipListDictionary<DICT_SKIP_KEY_TYPE, double> skipDict(data.begin(), data.end());
    const auto skipInitTime = DurationNs(skipInitStart, GetNow());

    const auto mapReadFunc = [&](const DICT_SKIP_KEY_TYPE& key) { return mapDict.at(key); };
    const auto skipReadFunc = [&](const DICT_SKIP_KEY_TYPE& key) { return skipDict.GetValue(key); };

    const auto skipTime1 = Benchmark(data, skipReadFunc);
    const auto mapTime1 = Benchmark(data, mapReadFunc);
    const auto skipTime2 = Benchmark(data, skipReadFunc);
    const auto mapTime2 = Benchmark(data, mapReadFunc);

    const auto mapReadTime = (mapTime1 + mapTime2) / 2;
    const auto skipReadTime = (skipTime1 + skipTime2) / 2;

    std::cout << "N = " << DICT_SKIP_KEYS << '\n';

    if (DICT_SKIP_TEST_INIT) {
        PrintInitTimes(mapInitTime, skipInitTime);
    }

    PrintCompare("read", mapReadTime, skipReadTime, DICT_SKIP_KEYS);

    volatile size_t sink = 0;

    const auto mapContainsStart = GetNow();
    for (const auto& [k, v] : data) {
        (void)v;
        sink += mapDict.find(k) != mapDict.end() ? 1 : 0;
    }
    const auto mapContainsTime = DurationNs(mapContainsStart, GetNow());

    const auto skipContainsStart = GetNow();
    for (const auto& [k, v] : data) {
        (void)v;
        sink += skipDict.ContainsKey(k) ? 1 : 0;
    }
    const auto skipContainsTime = DurationNs(skipContainsStart, GetNow());

    PrintCompare("contains", mapContainsTime, skipContainsTime, DICT_SKIP_KEYS);

    std::vector<std::pair<DICT_SKIP_KEY_TYPE, double>> insertData;
    insertData.reserve(DICT_SKIP_INSERT_OPS);
    const size_t offset = data.size() * 2 + 1;
    for (size_t i = 0; i < DICT_SKIP_INSERT_OPS; ++i) {
        insertData.emplace_back("ins_" + std::to_string(offset + i), static_cast<double>(i) * 1.1);
    }

    const auto mapInsertStart = GetNow();
    {
        std::map<DICT_SKIP_KEY_TYPE, double> m(data.begin(), data.end());
        for (const auto& [k, v] : insertData) {
            m.insert_or_assign(k, v);
        }
        sink += m.size();
    }
    const auto mapInsertTime = DurationNs(mapInsertStart, GetNow());

    const auto skipInsertStart = GetNow();
    {
        SkipListDictionary<DICT_SKIP_KEY_TYPE, double> d(data.begin(), data.end());
        for (const auto& [k, v] : insertData) {
            d.InsertOrAssign(k, v);
        }
        sink += d.Count();
    }
    const auto skipInsertTime = DurationNs(skipInsertStart, GetNow());

    PrintCompare("insert", mapInsertTime, skipInsertTime, DICT_SKIP_INSERT_OPS);

    const auto mapIterStart = GetNow();
    {
        volatile double local = 0;
        for (const auto& [k, v] : mapDict) {
            (void)k;
            local += v;
        }
        sink += static_cast<size_t>(local);
    }
    const auto mapIterTime = DurationNs(mapIterStart, GetNow());

    const auto skipIterStart = GetNow();
    {
        volatile double local = 0;
        for (const auto& [k, v] : skipDict) {
            (void)k;
            local += v;
        }
        sink += static_cast<size_t>(local);
    }
    const auto skipIterTime = DurationNs(skipIterStart, GetNow());

    PrintCompare("iterate", mapIterTime, skipIterTime, DICT_SKIP_KEYS);

    std::vector<DICT_SKIP_KEY_TYPE> removeKeys;
    const size_t removeOps = std::min(static_cast<size_t>(DICT_SKIP_REMOVE_OPS), data.size());
    removeKeys.reserve(removeOps);
    for (size_t i = 0; i < removeOps; ++i) {
        removeKeys.push_back(data[i].first);
    }

    const auto mapRemoveStart = GetNow();
    {
        std::map<DICT_SKIP_KEY_TYPE, double> m(data.begin(), data.end());
        for (const auto& k : removeKeys) {
            m.erase(k);
        }
        sink += m.size();
    }
    const auto mapRemoveTime = DurationNs(mapRemoveStart, GetNow());

    const auto skipRemoveStart = GetNow();
    {
        SkipListDictionary<DICT_SKIP_KEY_TYPE, double> d(data.begin(), data.end());
        for (const auto& k : removeKeys) {
            d.Remove(k);
        }
        sink += d.Count();
    }
    const auto skipRemoveTime = DurationNs(skipRemoveStart, GetNow());

    PrintCompare("remove", mapRemoveTime, skipRemoveTime, removeOps);

    std::cout << DictLabel << " max level: " << skipDict.MaxLevel() << '\n';

    (void)sink;
    return 0;
}
