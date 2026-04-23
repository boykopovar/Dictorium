#ifndef DICTORIUM_H
#define DICTORIUM_H

#include <cstdint>

#ifdef _MSC_VER
    #include <xmmintrin.h>
    #define DTR_PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
    #else
    #define DTR_PREFETCH(addr) __builtin_prefetch(addr, 0, 1)
#endif

namespace dtr {
    static inline uint64_t FastRange(uint64_t a, uint64_t b) {
#ifdef _MSC_VER
        uint64_t high;
        _umul128(a, b, &high);
        return high;
#else
        return static_cast<uint64_t>((static_cast<__uint128_t>(a) * b) >> 64);
#endif
    }
}

#include "Contracts/Contracts.h"
#include "Entities/LinearDictionary.h"
#include "Entities/PerfectHashDictionary.h"
#include "Entities/CuckooHashDictionary.h"
#include "Entities/Console.h"

#endif //DICTORIUM_H
