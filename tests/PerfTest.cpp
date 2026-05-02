#include <unordered_map>

#include "Dictorium/Dictorium.h"
#include "Utils/TestUtils.h"

using namespace dtr;

#define DTR_TEST_STL_UNORDERED_MAP std::unordered_map
#define DTR_TEST_DTR_CUCKOO_HASH CuckooHashDictionary

int main() {
    start_perf_compare<
        DTR_TEST_STL_UNORDERED_MAP,
        DTR_TEST_DTR_CUCKOO_HASH,
        std::string
    >(
        DTR_STR(DTR_TEST_STL_UNORDERED_MAP),
        DTR_STR(DTR_TEST_DTR_CUCKOO_HASH),
        1'000'000,
        10
    );
}
