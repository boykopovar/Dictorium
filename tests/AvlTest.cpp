#include <algorithm>
#include <atomic>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Dictorium/Dictorium.h"

using namespace dtr;

using Value = double;
using Clock = std::chrono::steady_clock;

constexpr std::size_t N = 200'000;
constexpr std::size_t READ_OPS = 1'000'000;
constexpr std::size_t INSERT_OPS = 10'000;
constexpr int RUNS = 10;

volatile std::uint64_t g_sink = 0;

struct BigKey {
    std::uint64_t part[8]{};

    auto operator<=>(const BigKey& other) const noexcept {
        for (int i = 0; i < 8; ++i) {
            auto cmp = part[i] <=> other.part[i];
            if (cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }

    bool operator==(const BigKey& other) const noexcept {
        for (int i = 0; i < 8; ++i) {
            if (part[i] != other.part[i]) return false;
        }
        return true;
    }
};

template <class Key>
struct Scenario {
    std::vector<std::pair<Key, Value>> data;
    std::vector<Key> readKeys;
    std::vector<Key> containsKeys;
    std::vector<std::pair<Key, Value>> insertData;
};

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

static std::uint64_t Mix64(std::uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

static std::uint64_t MakeRandomSeed() {
    static std::atomic<std::uint64_t> counter{0};

    const auto now = static_cast<std::uint64_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );

    std::random_device rd;
    std::uint64_t seed =
            now ^
            (static_cast<std::uint64_t>(rd()) << 1) ^
            (counter.fetch_add(1) << 32);

    return Mix64(seed);
}

template <class Key>
static Key MakeBaseKey(std::size_t i, std::uint64_t seed) {
    if constexpr (std::is_same_v<Key, std::string>) {
        (void)seed;
        return "key_" + std::to_string(i);
    } else if constexpr (std::is_same_v<Key, BigKey>) {
        BigKey k{};

        const std::uint64_t group = static_cast<std::uint64_t>(i / 16);
        std::uint64_t x = Mix64(seed ^ group);

        for (int j = 0; j < 7; ++j) {
            x = Mix64(x + 0x9e3779b97f4a7c15ULL + static_cast<std::uint64_t>(j));
            k.part[j] = x;
        }

        k.part[7] = static_cast<std::uint64_t>(i);
        return k;
    } else {
        static_assert(std::is_integral_v<Key>, "Supported key types: integral types, std::string, BigKey");
        (void)seed;
        return static_cast<Key>(i);
    }
}

template <class Key>
static Key MakeInsertKey(std::size_t i, std::uint64_t seed) {
    if constexpr (std::is_same_v<Key, std::string>) {
        (void)seed;
        return "ins_" + std::to_string(i);
    } else if constexpr (std::is_same_v<Key, BigKey>) {
        return MakeBaseKey<BigKey>(i, seed);
    } else {
        static_assert(std::is_integral_v<Key>, "Supported key types: integral types, std::string, BigKey");
        (void)seed;
        return static_cast<Key>(i);
    }
}

template <class Key>
static std::uint64_t KeyToU64(const Key& key) {
    if constexpr (std::is_same_v<Key, std::string>) {
        return static_cast<std::uint64_t>(std::hash<std::string>{}(key));
    } else if constexpr (std::is_same_v<Key, BigKey>) {
        std::uint64_t x = 0;
        for (std::uint64_t p : key.part) {
            x ^= Mix64(p + 0x9e3779b97f4a7c15ULL);
        }
        return x;
    } else {
        static_assert(std::is_integral_v<Key>, "Supported key types: integral types, std::string, BigKey");
        return static_cast<std::uint64_t>(key);
    }
}

template <class Key>
static std::vector<std::pair<Key, Value>> MakeData(std::size_t n, std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<Value> valueDist(0.0, 1'000'000.0);

    std::vector<Key> keys;
    keys.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        keys.push_back(MakeBaseKey<Key>(i, seed));
    }

    std::shuffle(keys.begin(), keys.end(), rng);

    std::vector<std::pair<Key, Value>> data;
    data.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        data.emplace_back(std::move(keys[i]), valueDist(rng));
    }

    return data;
}

template <class Key>
static std::vector<Key> MakeLookupKeys(
        const std::vector<std::pair<Key, Value>>& data,
        std::size_t count,
        std::uint64_t seed
) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::size_t> pick(0, data.size() - 1);

    std::vector<Key> keys;
    keys.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        keys.push_back(data[pick(rng)].first);
    }

    return keys;
}

template <class Key>
static std::vector<std::pair<Key, Value>> MakeInsertData(std::size_t count, std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<Value> valueDist(0.0, 1'000'000.0);

    std::vector<std::pair<Key, Value>> inserts;
    inserts.reserve(count);

    const std::size_t base = N * 10 + 1;
    for (std::size_t i = 0; i < count; ++i) {
        inserts.emplace_back(MakeInsertKey<Key>(base + i, seed), valueDist(rng));
    }

    return inserts;
}

template <class Key>
static Scenario<Key> MakeScenario(std::uint64_t seed) {
    Scenario<Key> s;
    s.data = MakeData<Key>(N, Mix64(seed ^ 0x1111111111111111ULL));
    s.readKeys = MakeLookupKeys<Key>(s.data, READ_OPS, Mix64(seed ^ 0x2222222222222222ULL));
    s.containsKeys = MakeLookupKeys<Key>(s.data, READ_OPS, Mix64(seed ^ 0x3333333333333333ULL));
    s.insertData = MakeInsertData<Key>(INSERT_OPS, Mix64(seed ^ 0x4444444444444444ULL));
    return s;
}

template <class Key>
static double BenchInitMap(const Scenario<Key>& s) {
    return MeasureNs([&] {
        std::map<Key, Value> dict(s.data.begin(), s.data.end());
        g_sink ^= static_cast<std::uint64_t>(dict.size());
    });
}

template <class Key>
static double BenchInitAvl(const Scenario<Key>& s) {
    return MeasureNs([&] {
        AvlDictionary<Key, Value> dict(s.data.begin(), s.data.end());
        g_sink ^= static_cast<std::uint64_t>(dict.Count());
    });
}

template <class Key>
static double BenchReadMap(const Scenario<Key>& s) {
    std::map<Key, Value> dict(s.data.begin(), s.data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const Key& k : s.readKeys) {
            local += static_cast<std::uint64_t>(dict.at(k));
        }
        g_sink ^= local;
    });
}

template <class Key>
static double BenchReadAvl(const Scenario<Key>& s) {
    AvlDictionary<Key, Value> dict(s.data.begin(), s.data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const Key& k : s.readKeys) {
            local += static_cast<std::uint64_t>(dict.GetValue(k));
        }
        g_sink ^= local;
    });
}

template <class Key>
static double BenchContainsMap(const Scenario<Key>& s) {
    std::map<Key, Value> dict(s.data.begin(), s.data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const Key& k : s.containsKeys) {
            local += (dict.find(k) != dict.end()) ? 1ULL : 0ULL;
        }
        g_sink ^= local;
    });
}

template <class Key>
static double BenchContainsAvl(const Scenario<Key>& s) {
    AvlDictionary<Key, Value> dict(s.data.begin(), s.data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const Key& k : s.containsKeys) {
            local += dict.ContainsKey(k) ? 1ULL : 0ULL;
        }
        g_sink ^= local;
    });
}

template <class Key>
static double BenchInsertMap(const Scenario<Key>& s) {
    std::map<Key, Value> dict(s.data.begin(), s.data.end());

    const double totalNs = MeasureNs([&] {
        for (const auto& [k, v] : s.insertData) {
            dict.insert_or_assign(k, v);
        }
        g_sink ^= static_cast<std::uint64_t>(dict.size());
    });

    return totalNs / static_cast<double>(s.insertData.size());
}

template <class Key>
static double BenchInsertAvl(const Scenario<Key>& s) {
    AvlDictionary<Key, Value> dict(s.data.begin(), s.data.end());

    const double totalNs = MeasureNs([&] {
        for (const auto& [k, v] : s.insertData) {
            dict.InsertOrAssign(k, v);
        }
        g_sink ^= static_cast<std::uint64_t>(dict.Count());
    });

    return totalNs / static_cast<double>(s.insertData.size());
}

template <class Key>
static double BenchHeightAvl(const Scenario<Key>& s) {
    AvlDictionary<Key, Value> dict(s.data.begin(), s.data.end());
    return static_cast<double>(dict.Height());
}

template <class Key>
static double BenchIterMap(const Scenario<Key>& s) {
    std::map<Key, Value> dict(s.data.begin(), s.data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const auto& [k, v] : dict) {
            local += KeyToU64(k) ^ static_cast<std::uint64_t>(v);
        }
        g_sink ^= local;
    });
}

template <class Key>
static double BenchIterAvl(const Scenario<Key>& s) {
    AvlDictionary<Key, Value> dict(s.data.begin(), s.data.end());

    return MeasureNs([&] {
        std::uint64_t local = 0;
        for (const auto& [k, v] : dict) {
            local += KeyToU64(k) ^ static_cast<std::uint64_t>(v);
        }
        g_sink ^= local;
    });
}

template <class Key>
static void RunForType(const char* typeName) {
    std::cout << "\n Key type: " << typeName << " \n";

    double initMapSum = 0.0;
    double initAvlSum = 0.0;
    double readMapSum = 0.0;
    double readAvlSum = 0.0;
    double containsMapSum = 0.0;
    double containsAvlSum = 0.0;
    double insertMapSum = 0.0;
    double insertAvlSum = 0.0;
    double heightSum = 0.0;
    double iterMapSum = 0.0;
    double iterAvlSum = 0.0;

    for (int run = 0; run < RUNS; ++run) {
        const auto seed = MakeRandomSeed();
        const Scenario<Key> s = MakeScenario<Key>(seed);

        initMapSum += BenchInitMap<Key>(s);
        initAvlSum += BenchInitAvl<Key>(s);

        readMapSum += BenchReadMap<Key>(s);
        readAvlSum += BenchReadAvl<Key>(s);

        containsMapSum += BenchContainsMap<Key>(s);
        containsAvlSum += BenchContainsAvl<Key>(s);

        insertMapSum += BenchInsertMap<Key>(s);
        insertAvlSum += BenchInsertAvl<Key>(s);

        heightSum += BenchHeightAvl<Key>(s);

        iterMapSum += BenchIterMap<Key>(s);
        iterAvlSum += BenchIterAvl<Key>(s);
    }

    const double initMap = initMapSum / RUNS;
    const double initAvl = initAvlSum / RUNS;
    const double readMap = readMapSum / RUNS;
    const double readAvl = readAvlSum / RUNS;
    const double containsMap = containsMapSum / RUNS;
    const double containsAvl = containsAvlSum / RUNS;
    const double insertMap = insertMapSum / RUNS;
    const double insertAvl = insertAvlSum / RUNS;
    const double heightAvg = heightSum / RUNS;
    const double iterMap = iterMapSum / RUNS;
    const double iterAvl = iterAvlSum / RUNS;

    std::cout << "1. Initialization (construct from N elements)\n";
    std::cout << "  std::map      : " << (initMap / 1e6) << " ms\n";
    std::cout << "  AvlDictionary : " << (initAvl / 1e6) << " ms\n";
    if (initAvl < initMap) faster("AvlDictionary", "std::map", initAvl, initMap);
    else faster("std::map", "AvlDictionary", initMap, initAvl);

    std::cout << "2. Read (GetValue on existing keys, random order)\n";
    std::cout << "  std::map      : " << (readMap / READ_OPS) << " ns/op\n";
    std::cout << "  AvlDictionary : " << (readAvl / READ_OPS) << " ns/op\n";
    if (readAvl < readMap) faster("AvlDictionary", "std::map", readAvl, readMap);
    else faster("std::map", "AvlDictionary", readMap, readAvl);

    std::cout << "3. ContainsKey (all keys exist)\n";
    std::cout << "  std::map      : " << (containsMap / READ_OPS) << " ns/op\n";
    std::cout << "  AvlDictionary : " << (containsAvl / READ_OPS) << " ns/op\n";
    if (containsAvl < containsMap) faster("AvlDictionary", "std::map", containsAvl, containsMap);
    else faster("std::map", "AvlDictionary", containsMap, containsAvl);

    std::cout << "4. Insert element (avg over " << INSERT_OPS << " inserts, fresh tree each run)\n";
    std::cout << "  std::map      : " << insertMap << " ns/op\n";
    std::cout << "  AvlDictionary : " << insertAvl << " ns/op\n";
    if (insertAvl < insertMap) faster("AvlDictionary", "std::map", insertAvl, insertMap);
    else faster("std::map", "AvlDictionary", insertMap, insertAvl);

    std::cout << "5. Tree height\n";
    const double avlTheory = std::ceil(1.45 * std::log2(static_cast<double>(N) + 1.0));
    const double rbTheory  = std::ceil(2.0  * std::log2(static_cast<double>(N) + 1.0));
    std::cout << "  AvlDictionary : " << heightAvg << " levels (avg)\n";
    std::cout << "  AVL max (theory) : ~" << avlTheory << " levels for N=" << N << "\n";
    std::cout << "  std::map (RB max): ~" << rbTheory << " levels for N=" << N << "\n";

    std::cout << "6. Full in-order iteration\n";
    std::cout << "  std::map      : " << (iterMap / N) << " ns/op\n";
    std::cout << "  AvlDictionary : " << (iterAvl / N) << " ns/op\n";
    if (iterAvl < iterMap) faster("AvlDictionary", "std::map", iterAvl, iterMap);
    else faster("std::map", "AvlDictionary", iterMap, iterAvl);
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n AvlDictionary vs std::map, runs=" << RUNS;

    RunForType<int>("int");
    RunForType<long long>("long long");
    RunForType<std::string>("std::string");
    RunForType<BigKey>("BigKey (64 bytes)");

    std::cout << '\n';
    return 0;
}
