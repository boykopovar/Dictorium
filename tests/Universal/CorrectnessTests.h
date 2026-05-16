#ifndef CORRECTNESSTESTS_H
#define CORRECTNESSTESTS_H

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "../Utils/TestUtils.h"

namespace dtr::test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

struct SuiteResult {
    std::string dictName;
    std::string keyType;
    std::vector<TestResult> results;

    int passed() const {
        int n = 0;
        for (const auto& r : results) n += r.passed;
        return n;
    }

    int failed() const {
        return static_cast<int>(results.size()) - passed();
    }
};

inline void _printResult(const TestResult& r) {
    std::cout << (r.passed ? "  [PASS] " : "  [FAIL] ") << r.name;
    if (!r.passed && !r.message.empty()) std::cout << "  =>  " << r.message;
    std::cout << '\n';
}

inline void _printSuiteHeader(const std::string& dictName, const std::string& keyType) {
    std::cout << "\n+----------------------------------------------------------+\n";
    std::cout << "  " << dictName << "  <TKey=" << keyType << ", TValue=double>\n";
    std::cout << "+----------------------------------------------------------+\n";
}

inline void _printSuiteSummary(const SuiteResult& s) {
    const int total = static_cast<int>(s.results.size());
    std::cout << "  ----------------------------------------------------------\n";
    std::cout << "  PASSED: " << s.passed() << " / " << total << "\n";
    if (s.failed() > 0) {
        std::cout << "  FAILED: " << s.failed() << "\n";
        for (const auto& r : s.results)
            if (!r.passed)
                std::cout << "    - " << r.name << "\n";
    }
    std::cout << "  ----------------------------------------------------------\n";
}

inline void printTotalSummary(const std::vector<SuiteResult>& suites) {
    std::cout << "\n\n";

    int totalPassed = 0, totalFailed = 0;
    for (const auto& s : suites) {
        std::cout << (s.failed() == 0 ? "  [OK]   " : "  [FAIL] ")
                  << s.dictName << " <TKey=" << s.keyType << ">"
                  << "  " << s.passed() << "/" << s.results.size() << "\n";
        totalPassed += s.passed();
        totalFailed += s.failed();
    }

    std::cout << "\n  PASSED: " << totalPassed << "\n";
    std::cout << "  FAILED: " << totalFailed << "\n";

}

#define _RUN_TEST(suite, testName, body)           \
    do {                                            \
        bool _ok = true;                            \
        std::string _msg;                           \
        body                                        \
        TestResult _r{testName, _ok, _msg};         \
        _printResult(_r);                           \
        (suite).results.push_back(std::move(_r));  \
    } while(0)

template<
    template<typename, typename> class DtrDict,
    typename TKey
>
SuiteResult start_correctness_test(
    const std::string& dictName,
    const size_t keysCount,
    const size_t keyLen = DICT_PERF_KEY_LEN
) {
    const std::string keyType = std::is_same_v<TKey, std::string> ? "std::string" : typeid(TKey).name();
    _printSuiteHeader(dictName, keyType);

    SuiteResult suite{dictName, keyType, {}};

    auto data = GenerateData<TKey>(keysCount, keyLen);

    std::unordered_map<TKey, double> ref(data.begin(), data.end());
    DtrDict<TKey, double> dict(data.begin(), data.end());

    const size_t halfIdx = data.size() / 2;
    std::vector<std::pair<TKey, double>> firstHalf(data.begin(), data.begin() + halfIdx);
    std::vector<std::pair<TKey, double>> secondHalf(data.begin() + halfIdx, data.end());

    _RUN_TEST(suite, "Count after construction", {
        if (dict.Count() != ref.size()) {
            _ok = false;
            _msg = "expected " + std::to_string(ref.size()) + ", got " + std::to_string(dict.Count());
        }
    });

    _RUN_TEST(suite, "ContainsKey  (all inserted keys present)", {
        for (const auto& [k, v] : data)
            if (!dict.ContainsKey(k)) { _ok = false; _msg = "unexpectedly returned false"; break; }
    });

    _RUN_TEST(suite, "TryGetValue  (values match unordered_map)", {
        double got = 0;
        for (const auto& [k, v] : data) {
            if (!dict.TryGetValue(k, got)) { _ok = false; _msg = "returned false for existing key"; break; }
            if (got != ref.at(k))          { _ok = false; _msg = "value mismatch vs std::unordered_map"; break; }
        }
    });

    _RUN_TEST(suite, "GetValue     (values match unordered_map)", {
        for (const auto& [k, v] : data) {
            try {
                if (dict.GetValue(k) != ref.at(k)) { _ok = false; _msg = "value mismatch vs std::unordered_map"; break; }
            } catch (...) { _ok = false; _msg = "threw for existing key"; break; }
        }
    });

    _RUN_TEST(suite, "InsertOrAssign (update existing, count stable)", {
        for (const auto& [k, v] : data) {
            const double updated = v * 3.7 + 1.0;
            ref[k] = updated;
            dict.InsertOrAssign(k, updated);
        }
        for (const auto& [k, v] : data) {
            if (dict.GetValue(k) != ref.at(k)) { _ok = false; _msg = "value mismatch after update"; break; }
        }
        if (_ok && dict.Count() != ref.size()) {
            _ok = false;
            _msg = "Count changed after InsertOrAssign on existing keys";
        }
    });

    _RUN_TEST(suite, "Add duplicate throws invalid_argument", {
        for (const auto& [k, v] : data) {
            try { dict.Add(k, v); _ok = false; _msg = "no exception on duplicate key"; break; }
            catch (const std::invalid_argument&) {}
            catch (...) { _ok = false; _msg = "wrong exception type on duplicate"; break; }
        }
    });

    _RUN_TEST(suite, "Remove       (existing keys, count matches)", {
        for (const auto& [k, v] : firstHalf) {
            ref.erase(k);
            if (!dict.Remove(k)) { _ok = false; _msg = "Remove returned false for existing key"; break; }
        }
        if (_ok && dict.Count() != ref.size()) {
            _ok = false;
            _msg = "Count mismatch: expected " + std::to_string(ref.size()) + ", got " + std::to_string(dict.Count());
        }
    });

    _RUN_TEST(suite, "ContainsKey  (removed keys absent)", {
        for (const auto& [k, v] : firstHalf)
            if (dict.ContainsKey(k)) { _ok = false; _msg = "ContainsKey true after Remove"; break; }
    });

    _RUN_TEST(suite, "ContainsKey  (non-removed keys still present)", {
        for (const auto& [k, v] : secondHalf)
            if (!dict.ContainsKey(k)) { _ok = false; _msg = "ContainsKey false for surviving key"; break; }
    });

    _RUN_TEST(suite, "Remove       (already removed returns false)", {
        for (const auto& [k, v] : firstHalf)
            if (dict.Remove(k)) { _ok = false; _msg = "Remove returned true for missing key"; break; }
    });

    _RUN_TEST(suite, "TryGetValue  (removed keys return false)", {
        double got = 0;
        for (const auto& [k, v] : firstHalf)
            if (dict.TryGetValue(k, got)) { _ok = false; _msg = "TryGetValue true for removed key"; break; }
    });

    _RUN_TEST(suite, "GetValue     (surviving keys match unordered_map)", {
        for (const auto& [k, v] : secondHalf) {
            try {
                if (dict.GetValue(k) != ref.at(k)) { _ok = false; _msg = "value mismatch vs unordered_map"; break; }
            } catch (...) { _ok = false; _msg = "threw for surviving key"; break; }
        }
    });

    _RUN_TEST(suite, "Clear        (count=0, all keys absent)", {
        dict.Clear();
        if (dict.Count() != 0) { _ok = false; _msg = "Count != 0 after Clear"; }
        if (_ok) {
            for (const auto& [k, v] : data)
                if (dict.ContainsKey(k)) { _ok = false; _msg = "ContainsKey true after Clear"; break; }
        }
    });

    _RUN_TEST(suite, "Add after Clear (all keys, count matches)", {
        for (const auto& [k, v] : data) {
            try { dict.Add(k, v); }
            catch (...) { _ok = false; _msg = "Add threw after Clear"; break; }
        }
        if (_ok && dict.Count() != data.size()) {
            _ok = false;
            _msg = "Count mismatch: expected " + std::to_string(data.size()) + ", got " + std::to_string(dict.Count());
        }
    });

    _RUN_TEST(suite, "TryGetValue  (correct values after re-Add)", {
        double got = 0;
        for (const auto& [k, v] : data) {
            if (!dict.TryGetValue(k, got)) { _ok = false; _msg = "key missing after re-Add"; break; }
            if (got != v)                  { _ok = false; _msg = "value mismatch after re-Add"; break; }
        }
    });

    _printSuiteSummary(suite);
    return suite;
}

}

#endif //CORRECTNESSTESTS_H