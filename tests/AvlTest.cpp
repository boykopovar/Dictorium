#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include "Dictorium/Dictorium.h"

using namespace dtr;

using Key   = int;
using Value = double;
using Clock = std::chrono::steady_clock;

constexpr std::size_t N = 200'000;
constexpr std::size_t READ_OPS = 1'000'000;
constexpr std::size_t INSERT_OPS = 10'000;
constexpr int RUNS = 10;

constexpr std::uint64_t DATA_SEED    = 0xC001D00DULL;
constexpr std::uint64_t READ_SEED    = 0xA11CE5ULL;
constexpr std::uint64_t CONTAINS_SEED = 0xBADC0FFEEULL;
constexpr std::uint64_t INSERT_SEED   = 0xC0FFEEULL;

volatile std::uint64_t g_sink = 0;

static void sep() {
    std::cout << "---------------------------------------------\n";
}

static void faster(const char* a, const char* b, double ta, double tb) {
    const double r = tb / ta;
    std::cout << "  -> " << a << " faster x" << std::fixed << std::setprecision(2) << r << "\n";
}

template <class F>
static double MeasureNs(F&& f) {
    const auto s = Clock::now();
    f();
    const auto e = Clock::now();
    return std::chrono::duration<double, std::nano>(e - s).count();
}

template <class F>
static double AverageNs(int runs, F&& f) {
    double sum = 0.0;
    for (int i = 0; i < runs; ++i) {
        sum += MeasureNs(f);
    }
    return sum / runs;
}

template <class F>
static double AverageValue(int runs, F&& f) {
    double sum = 0.0;
    for (int i = 0; i < runs; ++i) {
        sum += f();
    }
    return sum / runs;
}

static std::vector<std::pair<Key, Value>> MakeData(std::size_t n) {
    std::mt19937_64 rng(DATA_SEED);

    std::vector<Key> keys(n);
    std::iota(keys.begin(), keys.end(), 0);
    std::shuffle(keys.begin(), keys.end(), rng);

    std::vector<std::pair<Key, Value>> data;
    data.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        data.emplace_back(keys[i], static_cast<Value>(i));
    }

    return data;
}

static std::vector<Key> MakeLookupKeys(const std::vector<std::pair<Key, Value>>& data,
                                       std::size_t count,
                                       std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::size_t> pick(0, data.size() - 1);

    std::vector<Key> keys;
    keys.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        keys.push_back(data[pick(rng)].first);
    }

    return keys;
}

static std::vector<std::pair<Key, Value>> MakeInsertData(std::size_t count, Key startKey) {
    std::mt19937_64 rng(INSERT_SEED);
    std::uniform_real_distribution<Value> valDist(0.0, 1'000'000.0);

    std::vector<std::pair<Key, Value>> inserts;
    inserts.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        inserts.emplace_back(startKey + static_cast<Key>(i), valDist(rng));
    }

    return inserts;
}

template <class DictT>
static double BenchInit(const std::vector<std::pair<Key, Value>>& data) {
    return MeasureNs([&] {
        DictT dict(data.begin(), data.end());
        g_sink ^= static_cast<std::uint64_t>(dict.Count());
    });
}

template <>
double BenchInit<std::map<Key, Value>>(const std::vector<std::pair<Key, Value>>& data) {
    return MeasureNs([&] {
        std::map<Key, Value> dict(data.begin(), data.end());
        g_sink ^= static_cast<std::uint64_t>(dict.size());
    });
}

template <class DictT>
static double BenchRead(const std::vector<std::pair<Key, Value>>& data,
                        const std::vector<Key>& keys) {
    return MeasureNs([&] {
        DictT dict(data.begin(), data.end());

        std::uint64_t local = 0;
        for (Key k : keys) {
            local += static_cast<std::uint64_t>(dict.GetValue(k));
        }

        g_sink ^= local;
    });
}

template <>
double BenchRead<std::map<Key, Value>>(const std::vector<std::pair<Key, Value>>& data,
                                       const std::vector<Key>& keys) {
    return MeasureNs([&] {
        std::map<Key, Value> dict(data.begin(), data.end());

        std::uint64_t local = 0;
        for (Key k : keys) {
            local += static_cast<std::uint64_t>(dict.at(k));
        }

        g_sink ^= local;
    });
}

template <class DictT>
static double BenchContains(const std::vector<std::pair<Key, Value>>& data,
                            const std::vector<Key>& keys) {
    return MeasureNs([&] {
        DictT dict(data.begin(), data.end());

        std::uint64_t local = 0;
        for (Key k : keys) {
            local += dict.ContainsKey(k) ? 1ULL : 0ULL;
        }

        g_sink ^= local;
    });
}

template <>
double BenchContains<std::map<Key, Value>>(const std::vector<std::pair<Key, Value>>& data,
                                           const std::vector<Key>& keys) {
    return MeasureNs([&] {
        std::map<Key, Value> dict(data.begin(), data.end());

        std::uint64_t local = 0;
        for (Key k : keys) {
            local += (dict.find(k) != dict.end()) ? 1ULL : 0ULL;
        }

        g_sink ^= local;
    });
}

template <class DictT>
static double BenchInsertBatch(const std::vector<std::pair<Key, Value>>& data,
                               const std::vector<std::pair<Key, Value>>& inserts) {
    DictT dict(data.begin(), data.end());

    const double totalNs = MeasureNs([&] {
        for (const auto& [k, v] : inserts) {
            dict.InsertOrAssign(k, v);
        }
        g_sink ^= static_cast<std::uint64_t>(dict.Count());
    });

    return totalNs / static_cast<double>(inserts.size());
}

template <>
double BenchInsertBatch<std::map<Key, Value>>(const std::vector<std::pair<Key, Value>>& data,
                                              const std::vector<std::pair<Key, Value>>& inserts) {
    std::map<Key, Value> dict(data.begin(), data.end());

    const double totalNs = MeasureNs([&] {
        for (const auto& [k, v] : inserts) {
            dict.insert_or_assign(k, v);
        }
        g_sink ^= static_cast<std::uint64_t>(dict.size());
    });

    return totalNs / static_cast<double>(inserts.size());
}

template <class DictT>
static double BenchIteration(const std::vector<std::pair<Key, Value>>& data) {
    DictT dict(data.begin(), data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const auto& [k, v] : dict) {
            local += static_cast<std::uint64_t>(k) ^ static_cast<std::uint64_t>(v);
        }
        g_sink ^= local;
    });
}

template <>
double BenchIteration<std::map<Key, Value>>(const std::vector<std::pair<Key, Value>>& data) {
    std::map<Key, Value> dict(data.begin(), data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const auto& [k, v] : dict) {
            local += static_cast<std::uint64_t>(k) ^ static_cast<std::uint64_t>(v);
        }
        g_sink ^= local;
    });
}

template <class DictT>
static double BenchHeight(const std::vector<std::pair<Key, Value>>& data) {
    DictT dict(data.begin(), data.end());
    return static_cast<double>(dict.Height());
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n=== AvlDictionary vs std::map, N=" << N << ", runs=" << RUNS << " ===\n\n";

    const auto data = MakeData(N);
    const auto readKeys = MakeLookupKeys(data, READ_OPS, READ_SEED);
    const auto containsKeys = MakeLookupKeys(data, READ_OPS, CONTAINS_SEED);
    const auto insertData = MakeInsertData(INSERT_OPS, static_cast<Key>(N * 10 + 1));

    // 1. Initialization
    sep();
    std::cout << "1. Initialization (construct from N elements)\n";

    const double mapInit = AverageNs(RUNS, [&] {
        return BenchInit<std::map<Key, Value>>(data);
    });
    const double avlInit = AverageNs(RUNS, [&] {
        return BenchInit<AvlDictionary<Key, Value>>(data);
    });

    std::cout << "  std::map      : " << (mapInit / 1e6) << " ms\n";
    std::cout << "  AvlDictionary : " << (avlInit / 1e6) << " ms\n";
    if (avlInit < mapInit) faster("AvlDictionary", "std::map", avlInit, mapInit);
    else                   faster("std::map", "AvlDictionary", mapInit, avlInit);

    // 2. Read
    sep();
    std::cout << "2. Read (GetValue on existing keys, random order)\n";

    const double mapRead = AverageNs(RUNS, [&] {
        return BenchRead<std::map<Key, Value>>(data, readKeys);
    });
    const double avlRead = AverageNs(RUNS, [&] {
        return BenchRead<AvlDictionary<Key, Value>>(data, readKeys);
    });

    std::cout << "  std::map      : " << (mapRead / READ_OPS) << " ns/op\n";
    std::cout << "  AvlDictionary : " << (avlRead / READ_OPS) << " ns/op\n";
    if (avlRead < mapRead) faster("AvlDictionary", "std::map", avlRead, mapRead);
    else                   faster("std::map", "AvlDictionary", mapRead, avlRead);

    // 3. ContainsKey
    sep();
    std::cout << "3. ContainsKey (all keys exist)\n";

    const double mapContains = AverageNs(RUNS, [&] {
        return BenchContains<std::map<Key, Value>>(data, containsKeys);
    });
    const double avlContains = AverageNs(RUNS, [&] {
        return BenchContains<AvlDictionary<Key, Value>>(data, containsKeys);
    });

    std::cout << "  std::map      : " << (mapContains / READ_OPS) << " ns/op\n";
    std::cout << "  AvlDictionary : " << (avlContains / READ_OPS) << " ns/op\n";
    if (avlContains < mapContains) faster("AvlDictionary", "std::map", avlContains, mapContains);
    else                           faster("std::map", "AvlDictionary", mapContains, avlContains);

    // 4. Insert element
    sep();
    std::cout << "4. Insert element (avg over " << INSERT_OPS << " inserts, fresh tree each run)\n";

    const double mapInsert = AverageNs(RUNS, [&] {
        return BenchInsertBatch<std::map<Key, Value>>(data, insertData);
    });
    const double avlInsert = AverageNs(RUNS, [&] {
        return BenchInsertBatch<AvlDictionary<Key, Value>>(data, insertData);
    });

    std::cout << "  std::map      : " << mapInsert << " ns/op\n";
    std::cout << "  AvlDictionary : " << avlInsert << " ns/op\n";
    if (avlInsert < mapInsert) faster("AvlDictionary", "std::map", avlInsert, mapInsert);
    else                       faster("std::map", "AvlDictionary", mapInsert, avlInsert);

    // 5. Height
    sep();
    std::cout << "5. Tree height\n";

    const double avlHeight = AverageValue(RUNS, [&] {
        return BenchHeight<AvlDictionary<Key, Value>>(data);
    });

    const double avlTheory = std::ceil(1.45 * std::log2(static_cast<double>(N) + 1.0));
    const double rbTheory  = std::ceil(2.0  * std::log2(static_cast<double>(N) + 1.0));

    std::cout << "  AvlDictionary : " << avlHeight << " levels (avg)\n";
    std::cout << "  AVL max (theory) : ~" << avlTheory << " levels for N=" << N << "\n";
    std::cout << "  std::map (RB max): ~" << rbTheory << " levels for N=" << N << "\n";

    // 6. Iteration
    sep();
    std::cout << "6. Full in-order iteration\n";

    const double mapIter = AverageNs(RUNS, [&] {
        return BenchIteration<std::map<Key, Value>>(data);
    });
    const double avlIter = AverageNs(RUNS, [&] {
        return BenchIteration<AvlDictionary<Key, Value>>(data);
    });

    std::cout << "  std::map      : " << (mapIter / 1e6) << " ms\n";
    std::cout << "  AvlDictionary : " << (avlIter / 1e6) << " ms\n";
    if (avlIter < mapIter) faster("AvlDictionary", "std::map", avlIter, mapIter);
    else                   faster("std::map", "AvlDictionary", mapIter, avlIter);

    sep();
    std::cout << "\n";
    return 0;
}