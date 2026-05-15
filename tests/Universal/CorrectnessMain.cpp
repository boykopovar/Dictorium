#include <unordered_map>

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

#define DTR_CORRECT_TEST(Dict, Key) \
    start_correctness_test<Dict, Key>(DTR_STR(Dict), 10'000, 8)

#define DTR_CORRECT_TEST_BOTH(Dict) \
    DTR_CORRECT_TEST(Dict, int); \
    DTR_CORRECT_TEST(Dict, std::string)

int main() {
    DTR_CORRECT_TEST_BOTH(AvlDictionary);
    DTR_CORRECT_TEST_BOTH(SkipListDictionary);
    DTR_CORRECT_TEST_BOTH(LinearDictionary);

    DTR_CORRECT_TEST_BOTH(ChainingHash2);
    DTR_CORRECT_TEST_BOTH(LinearProbing2);
    DTR_CORRECT_TEST_BOTH(QuadraticProbing2);
    DTR_CORRECT_TEST_BOTH(RobinHood2);
    DTR_CORRECT_TEST_BOTH(DoubleHash2);
    DTR_CORRECT_TEST_BOTH(CuckooHashDictionary);
    DTR_CORRECT_TEST_BOTH(PerfectHashDictionary);
}
