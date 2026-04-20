#include "TestUtils.h"

#include <unordered_map>
#include <unordered_set>


std::string GenerateStr(std::mt19937& rng, const size_t length) {
    std::uniform_int_distribution<> dist('a', 'z');
    std::string key;
    key.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        key.push_back(static_cast<char>(dist(rng)));
    }
    return key;
}


DataStr GenerateDataStr(const size_t count, const size_t length) {
    static std::mt19937 rng(std::random_device{}());
    std::unordered_map<std::string, double> seen;

    seen.reserve(count);
    DataStr data;
    data.reserve(count);

    size_t i = 0;
    while (seen.size() < count) {
        seen.emplace(GenerateStr(rng, length), static_cast<double>(i++)*1.1);
    }
    return {seen.begin(), seen.end()};
}


DateTime GetNow() {
    return std::chrono::steady_clock::now();
}

size_t DurationNs(const DateTime start, const DateTime end) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


