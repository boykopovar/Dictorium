#include <unordered_map>
#include <vector>

#include "Dictorium/Dictorium.h"
#include "../Utils/TestUtils.h"
#include "CorrectnessTests.h"

using namespace dtr;
using namespace dtr::test;

template<typename TKey, typename TValue>
using ChainingHash2 = ChainingHashDictionary<TKey, TValue>;

template<typename TKey, typename TValue>
using LinearProbing2 = LinearProbingDictionary<TKey, TValue>;

template<typename TKey, typename TValue>
using QuadraticProbing2 = QuadraticProbingDictionary<TKey, TValue>;

template<typename TKey, typename TValue>
using RobinHood2 = RobinHoodHashDictionary<TKey, TValue>;

template<typename TKey, typename TValue>
using DoubleHash2 = DoubleHashDictionary<TKey, TValue>;

#define DTR_CORRECT_TEST(results, Dict, Key) \
(results).push_back(start_correctness_test<Dict, Key>(DTR_STR(Dict), 10'000, 8))

#define DTR_CORRECT_TEST_BOTH(results, Dict) \
DTR_CORRECT_TEST(results, Dict, int);    \
DTR_CORRECT_TEST(results, Dict, std::string)

int main() {
    std::vector<SuiteResult> results;

    DTR_CORRECT_TEST_BOTH(results, AvlDictionary);
    DTR_CORRECT_TEST_BOTH(results, SkipListDictionary);
    DTR_CORRECT_TEST_BOTH(results, LinearDictionary);
    DTR_CORRECT_TEST_BOTH(results, ChainingHash2);
    DTR_CORRECT_TEST_BOTH(results, LinearProbing2);
    DTR_CORRECT_TEST_BOTH(results, QuadraticProbing2);
    DTR_CORRECT_TEST_BOTH(results, RobinHood2);
    DTR_CORRECT_TEST_BOTH(results, DoubleHash2);
    DTR_CORRECT_TEST_BOTH(results, CuckooHashDictionary);
    DTR_CORRECT_TEST_BOTH(results, PerfectHashDictionary);

    printTotalSummary(results);
}