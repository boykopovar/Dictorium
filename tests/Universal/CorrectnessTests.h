#ifndef CORRECTNESSTESTS_H
#define CORRECTNESSTESTS_H

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>

#include "../Utils/TestUtils.h"

namespace dtr::test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

inline void _printResult(const TestResult& r) {
    std::cout << (r.passed ? "  [PASS] " : "  [FAIL] ") << r.name;
    if (!r.passed && !r.message.empty()) std::cout << "  =>  " << r.message;
    std::cout << '\n';
}

inline void _printSuite(const std::string& dictName, const std::string& keyType) {
    std::cout << "\n+----------------------------------------------------------+\n";
    std::cout << "  " << dictName << "  <TKey=" << keyType << ", TValue=double>\n";
    std::cout << "+----------------------------------------------------------+\n";
}

template<
    template<typename, typename> class DtrDict,
    typename TKey
>
void start_correctness_test(
    const std::string& dictName,
    const size_t keysCount,
    const size_t keyLen = DICT_PERF_KEY_LEN
) {
    const std::string keyType = std::is_same_v<TKey, std::string> ? "std::string" : typeid(TKey).name();
    _printSuite(dictName, keyType);

    auto data = GenerateData<TKey>(keysCount, keyLen);

    std::unordered_map<TKey, double> ref(data.begin(), data.end());
    DtrDict<TKey, double> dict(data.begin(), data.end());

    const size_t halfIdx = data.size() / 2;
    std::vector<std::pair<TKey, double>> firstHalf(data.begin(), data.begin() + halfIdx);
    std::vector<std::pair<TKey, double>> secondHalf(data.begin() + halfIdx, data.end());

    {
        const bool ok = dict.Count() == ref.size();
        _printResult({"Count after construction",
                      ok,
                      "expected " + std::to_string(ref.size()) + ", got " + std::to_string(dict.Count())});
    }

    {
        bool ok = true;
        for (const auto& [k, v] : data) {
            if (!dict.ContainsKey(k)) { ok = false; break; }
        }
        _printResult({"ContainsKey  (all inserted keys present)", ok, "unexpectedly returned false"});
    }

    {
        bool ok = true;
        std::string msg;
        double got = 0;
        for (const auto& [k, v] : data) {
            if (!dict.TryGetValue(k, got)) { ok = false; msg = "returned false for existing key"; break; }
            if (got != ref.at(k))          { ok = false; msg = "value mismatch vs std::unordered_map"; break; }
        }
        _printResult({"TryGetValue  (values match unordered_map)", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : data) {
            try {
                if (dict.GetValue(k) != ref.at(k)) { ok = false; msg = "value mismatch vs std::unordered_map"; break; }
            } catch (...) { ok = false; msg = "threw for existing key"; break; }
        }
        _printResult({"GetValue     (values match unordered_map)", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : data) {
            const double updated = v * 3.7 + 1.0;
            ref[k] = updated;
            dict.InsertOrAssign(k, updated);
        }
        for (const auto& [k, v] : data) {
            if (dict.GetValue(k) != ref.at(k)) { ok = false; msg = "value mismatch after update"; break; }
        }
        if (ok && dict.Count() != ref.size()) {
            ok = false;
            msg = "Count changed after InsertOrAssign on existing keys";
        }
        _printResult({"InsertOrAssign (update existing, count stable)", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : data) {
            try { dict.Add(k, v); ok = false; msg = "no exception on duplicate key"; break; }
            catch (const std::invalid_argument&) {}
            catch (...) { ok = false; msg = "wrong exception type on duplicate"; break; }
        }
        _printResult({"Add duplicate throws invalid_argument", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : firstHalf) {
            ref.erase(k);
            if (!dict.Remove(k)) { ok = false; msg = "Remove returned false for existing key"; break; }
        }
        if (ok && dict.Count() != ref.size()) {
            ok = false;
            msg = "Count mismatch: expected " + std::to_string(ref.size()) + ", got " + std::to_string(dict.Count());
        }
        _printResult({"Remove       (existing keys, count matches)", ok, msg});
    }

    {
        bool ok = true;
        for (const auto& [k, v] : firstHalf) {
            if (dict.ContainsKey(k)) { ok = false; break; }
        }
        _printResult({"ContainsKey  (removed keys absent)", ok, "ContainsKey true after Remove"});
    }

    {
        bool ok = true;
        for (const auto& [k, v] : secondHalf) {
            if (!dict.ContainsKey(k)) { ok = false; break; }
        }
        _printResult({"ContainsKey  (non-removed keys still present)", ok, "ContainsKey false for surviving key"});
    }

    {
        bool ok = true;
        for (const auto& [k, v] : firstHalf) {
            if (dict.Remove(k)) { ok = false; break; }
        }
        _printResult({"Remove       (already removed returns false)", ok, "Remove returned true for missing key"});
    }

    {
        bool ok = true;
        std::string msg;
        double got = 0;
        for (const auto& [k, v] : firstHalf) {
            if (dict.TryGetValue(k, got)) { ok = false; msg = "TryGetValue true for removed key"; break; }
        }
        _printResult({"TryGetValue  (removed keys return false)", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : secondHalf) {
            try {
                if (dict.GetValue(k) != ref.at(k)) { ok = false; msg = "value mismatch vs unordered_map"; break; }
            } catch (...) { ok = false; msg = "threw for surviving key"; break; }
        }
        _printResult({"GetValue     (surviving keys match unordered_map)", ok, msg});
    }

    {
        dict.Clear();
        const bool okCount = dict.Count() == 0;
        bool okContains = true;
        if (okCount) {
            for (const auto& [k, v] : data) {
                if (dict.ContainsKey(k)) { okContains = false; break; }
            }
        }
        const bool ok = okCount && okContains;
        std::string msg;
        if (!okCount)    msg = "Count != 0 after Clear";
        if (!okContains) msg = "ContainsKey true after Clear";
        _printResult({"Clear        (count=0, all keys absent)", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : data) {
            try { dict.Add(k, v); }
            catch (...) { ok = false; msg = "Add threw after Clear"; break; }
        }
        if (ok && dict.Count() != data.size()) {
            ok = false;
            msg = "Count mismatch after re-Add: expected " + std::to_string(data.size()) + ", got " + std::to_string(dict.Count());
        }
        _printResult({"Add after Clear (all keys, count matches)", ok, msg});
    }

    {
        bool ok = true;
        std::string msg;
        for (const auto& [k, v] : data) {
            double got = 0;
            if (!dict.TryGetValue(k, got)) { ok = false; msg = "key missing after re-Add"; break; }
            if (got != v) { ok = false; msg = "value mismatch after re-Add"; break; }
        }
        _printResult({"TryGetValue  (correct values after re-Add)", ok, msg});
    }
}

}

#endif //CORRECTNESSTESTS_H
