#include <unordered_map>
#include <vector>

#include "Dictorium/Dictorium.h"
#include "../Utils/TestUtils.h"
#include "CorrectnessTests.h"

using namespace dtr;
using namespace dtr::test;

template<typename TKey, typename TValue> using ChainingHash2 = ChainingHashDictionary<TKey, TValue>;
template<typename TKey, typename TValue> using LinearProbing2 = LinearProbingDictionary<TKey, TValue>;
template<typename TKey, typename TValue> using QuadraticProbing2 = QuadraticProbingDictionary<TKey, TValue>;
template<typename TKey, typename TValue> using RobinHood2 = RobinHoodHashDictionary<TKey, TValue>;
template<typename TKey, typename TValue> using DoubleHash2 = DoubleHashDictionary<TKey, TValue>;

#define DTR_DICT_LIST(X) \
    X(AvlDictionary) \
    X(SkipListDictionary) \
    X(LinearDictionary) \
    X(ChainingHash2) \
    X(LinearProbing2) \
    X(QuadraticProbing2) \
    X(RobinHood2) \
    X(DoubleHash2) \
    X(CuckooHashDictionary) \
    X(PerfectHashDictionary)

#define DTR_CORRECT_TEST(results, Dict, Key) \
    (results).push_back(start_correctness_test<Dict, Key>(DTR_STR(Dict), 10'000, 8))

#define DTR_RUN(Dict) \
    DTR_CORRECT_TEST(results, Dict, int); \
    DTR_CORRECT_TEST(results, Dict, std::string);

int main() {
    std::vector<SuiteResult> results;

    DTR_DICT_LIST(DTR_RUN)

    printTotalSummary(results);
}