#include <map>
#include <cmath>
#include <iostream>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

// ── Parameters ────────────────────────────────────────────────────────────────
#define AVL_PERF_KEY_TYPE int
#define AVL_PERF_KEYS     500'000

using namespace dtr;

auto GenerateData() {
    if constexpr (std::is_same_v<AVL_PERF_KEY_TYPE, std::string>)
        return GenerateDataStr(AVL_PERF_KEYS, 10);
    else
        return GenerateDataNum<AVL_PERF_KEY_TYPE>(AVL_PERF_KEYS);
}

static void printSeparator() {
    std::cout << "---------------------------------------------\n";
}

static void printRatio(const std::string& faster, const std::string& slower,
                        size_t fastTime, size_t slowTime)
{
    const double ratio = static_cast<double>(slowTime) / static_cast<double>(fastTime);
    std::cout << "  -> " << faster << " is faster by " << ratio << "x\n";
}

int main() {
    std::cout << "\n=== Benchmark: AvlDictionary vs std::map ===\n";
    std::cout << "  Key type: " << typeid(AVL_PERF_KEY_TYPE).name() << '\n';
    std::cout << "  N:        " << AVL_PERF_KEYS << '\n';
    std::cout << '\n';

    auto data = GenerateData();
    std::cout << "  Data generated.\n\n";

    // ── 1. Initialization ─────────────────────────────────────────────────────
    printSeparator();
    std::cout << "1. Initialization (insert N elements)\n";

    const auto mapInitStart = GetNow();
    std::map<AVL_PERF_KEY_TYPE, double> stdMap(data.begin(), data.end());
    const auto mapInitTime = DurationNs(mapInitStart, GetNow());

    const auto avlInitStart = GetNow();
    AvlDictionary<AVL_PERF_KEY_TYPE, double> avlDict(data.begin(), data.end());
    const auto avlInitTime = DurationNs(avlInitStart, GetNow());

    std::cout << "  std::map      : " << mapInitTime / 1'000'000.0 << " ms\n";
    std::cout << "  AvlDictionary : " << avlInitTime / 1'000'000.0 << " ms\n";
    if (avlInitTime < mapInitTime)
        printRatio("AvlDictionary", "std::map", avlInitTime, mapInitTime);
    else
        printRatio("std::map", "AvlDictionary", mapInitTime, avlInitTime);

    // ── 2. Read (random order) ────────────────────────────────────────────────
    printSeparator();
    std::cout << "2. Read N elements (random order)\n";

    const auto mapReadFunc = [&](const AVL_PERF_KEY_TYPE& k) { return stdMap.at(k); };
    const auto avlReadFunc = [&](const AVL_PERF_KEY_TYPE& k) { return avlDict.GetValue(k); };

    // Warmup
    Benchmark(data, mapReadFunc);
    Benchmark(data, avlReadFunc);

    const auto mapRead = (Benchmark(data, mapReadFunc) + Benchmark(data, mapReadFunc)) / 2;
    const auto avlRead = (Benchmark(data, avlReadFunc) + Benchmark(data, avlReadFunc)) / 2;

    std::cout << "  std::map      : " << mapRead / AVL_PERF_KEYS << " ns/op\n";
    std::cout << "  AvlDictionary : " << avlRead / AVL_PERF_KEYS << " ns/op\n";
    if (avlRead < mapRead)
        printRatio("AvlDictionary", "std::map", avlRead, mapRead);
    else
        printRatio("std::map", "AvlDictionary", mapRead, avlRead);

    // ── 3. ContainsKey ────────────────────────────────────────────────────────
    printSeparator();
    std::cout << "3. ContainsKey (all keys exist)\n";

    const auto mapContainsFunc = [&](const AVL_PERF_KEY_TYPE& k) -> double {
        return static_cast<double>(stdMap.count(k));
    };
    const auto avlContainsFunc = [&](const AVL_PERF_KEY_TYPE& k) -> double {
        return static_cast<double>(avlDict.ContainsKey(k));
    };

    const auto mapContains = Benchmark(data, mapContainsFunc);
    const auto avlContains = Benchmark(data, avlContainsFunc);

    std::cout << "  std::map      : " << mapContains / AVL_PERF_KEYS << " ns/op\n";
    std::cout << "  AvlDictionary : " << avlContains / AVL_PERF_KEYS << " ns/op\n";
    if (avlContains < mapContains)
        printRatio("AvlDictionary", "std::map", avlContains, mapContains);
    else
        printRatio("std::map", "AvlDictionary", mapContains, avlContains);

    // ── 4. Tree height ────────────────────────────────────────────────────────
    printSeparator();
    std::cout << "4. Tree height\n";
    std::cout << "  AvlDictionary : " << avlDict.Height() << " levels\n";
    std::cout << "  Theoretical AVL max: ~"
              << static_cast<int>(std::ceil(1.45 * std::log2(AVL_PERF_KEYS + 1)))
              << " levels for N=" << AVL_PERF_KEYS << "\n";

    // ── 5. Full in-order iteration ────────────────────────────────────────────
    printSeparator();
    std::cout << "5. Full traversal (in-order iteration)\n";

    volatile double sink = 0;

    const auto mapIterStart = GetNow();
    for (auto& [k, v] : stdMap) sink = v;
    const auto mapIterTime = DurationNs(mapIterStart, GetNow());

    const auto avlIterStart = GetNow();
    for (auto& [k, v] : avlDict) sink = v;
    const auto avlIterTime = DurationNs(avlIterStart, GetNow());

    std::cout << "  std::map      : " << mapIterTime / 1'000'000.0 << " ms\n";
    std::cout << "  AvlDictionary : " << avlIterTime / 1'000'000.0 << " ms\n";
    if (avlIterTime < mapIterTime)
        printRatio("AvlDictionary", "std::map", avlIterTime, mapIterTime);
    else
        printRatio("std::map", "AvlDictionary", mapIterTime, avlIterTime);

    printSeparator();
    std::cout << '\n';
    return 0;
}