#ifndef PERFECTDICTIONARY_TPP
#define PERFECTDICTIONARY_TPP

#define PERFHASH_MAX_ATTEMPTS 100


template <CHashable TKey, typename TValue>
template<CPairIterator<TKey, TValue> TIter>
void PerfectHashDictionary<TKey, TValue>::_build(TIter begin, TIter end, size_t size) {
    if (size == 0) throw std::invalid_argument("PerfectHashDictionary cannot be empty");

    this->_count = size;
    this->_tableSize = size;
    auto buckets_list = std::vector<std::vector<std::pair<TKey, TValue>>>(_tableSize);

    uint64_t currentAttempt = 0;
    while (currentAttempt < PERFHASH_MAX_ATTEMPTS) {
        ++currentAttempt;
        for (auto& bucket : buckets_list) bucket.clear();
        this->_globalSeed = _randomNum();

        for (auto it = begin; it != end; ++it) {
            buckets_list[_hash(it->first, _globalSeed, _tableSize)].push_back(*it);
        }

        size_t countSqSum = 0;
        for (auto& bucket : buckets_list) {
            countSqSum += bucket.size() * bucket.size();
        }
        if (countSqSum > 2*buckets_list.size()) continue;

        for (auto& bucket : buckets_list) {
            if (bucket.size() <= 1) continue;
            for (size_t i = 0; i < bucket.size(); ++i) {
                for (size_t j = i + 1; j < bucket.size(); ++j) {
                    if (bucket[i].first == bucket[j].first)
                        throw std::invalid_argument("Duplicate key detected");
                }
            }
        }

        this->_buckets.assign(_tableSize, PhBucket{0, 0, 0});
        bool needNewGlobalSeed = false;

        for (auto& bucket : buckets_list) {
            if (bucket.empty()) continue;
            auto bucketInnerSize = _nextPrime(bucket.size() * bucket.size());

            auto globalIndex = _hash(bucket[0].first, _globalSeed, _tableSize);
            auto bucketSeed = _findSeed(bucket, bucketInnerSize);
            if (bucketSeed == 0) {
                needNewGlobalSeed = true;
                break;
            }

            _buckets[globalIndex].Size = bucketInnerSize;
            _buckets[globalIndex].Seed = bucketSeed;
        }

        if (needNewGlobalSeed) {
            _tableSize *= 2;
            buckets_list.resize(_tableSize);
            continue;
        }

        size_t flatSize = 0;
        for (auto& bucket : _buckets) {
            bucket.Offset = flatSize;
            flatSize += bucket.Size;
        }
        _values.assign(flatSize, {{TKey{}, TValue{}}, false});

        for (auto& bucket : buckets_list) {
            if (bucket.empty()) continue;
            auto globalIndex = _hash(bucket[0].first, _globalSeed, _tableSize);
            auto& bucketFromFlat = _buckets[globalIndex];

            for (auto& [key, value] : bucket) {
                size_t flatIndex = bucketFromFlat.Offset + _hash(key, bucketFromFlat.Seed, bucketFromFlat.Size);
                _values[flatIndex].Item.first = key;
                _values[flatIndex].Item.second = value;
                _values[flatIndex].Exists = true;
            }
        }
        return;
    }
    throw std::runtime_error("Failed to build PerfectHash in " + std::to_string(PERFHASH_MAX_ATTEMPTS) + " attempts");
}

template<CHashable TKey, typename TValue>
[[nodiscard]] uint64_t PerfectHashDictionary<TKey, TValue>::_randomNum() const {
    static std::mt19937_64 generator(std::random_device{}());
    return generator();
}

template<CHashable TKey, typename TValue>
uint64_t PerfectHashDictionary<TKey, TValue>::_findSeed(const std::vector<std::pair<TKey, TValue>>& bucket, size_t tableSize) const {
    std::vector<bool> occupied(tableSize, false);
    for (size_t i = 0; i < PERFHASH_MAX_ATTEMPTS; ++i) {
        std::fill(occupied.begin(), occupied.end(), false);
        auto seed = _randomNum();
        bool collision = false;

        for (auto& [key, value] : bucket) {
            auto slot = _hash(key, seed, tableSize);
            if (occupied[slot]){
                collision = true;
                break;
            }
            occupied[slot] = true;
        }
        if (!collision) return seed;
    }
    return 0;
}

template<CHashable TKey, typename TValue>
[[nodiscard]] size_t PerfectHashDictionary<TKey, TValue>::_nextPrime(size_t n) const {
    if (n<=2) return 2;
    if (n%2 == 0) ++n;

    while (true) {
        bool prime = true;
        for (size_t i = 3; i * i <= n; i+=2) {
            if (n%i == 0) {
                prime = false;
                break;
            }
        }
        if (prime) return n;
        n+=2;
    }
}



#endif // PERFECTDICTIONARY_TPP
