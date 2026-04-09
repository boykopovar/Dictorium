#ifndef PERFECTDICTIONARY_TPP
#define PERFECTDICTIONARY_TPP


template <typename TKey, typename TValue>
template<PairIterator<TKey, TValue> TIter>
void PerfectHashDictionary<TKey, TValue>::_build(TIter begin, TIter end, size_t size) {
    this->_count = size;
    this->_tableSize = size;
    auto buckets_list = std::vector<std::vector<std::pair<TKey, TValue>>>(_tableSize);

    while (true) {
        for (auto& bucket : buckets_list) bucket.clear();
        this->_globalSeed = _randomNum();

        for (auto it = begin; it != end; ++it) {
            buckets_list[_hash(it->first, _globalSeed, _tableSize)].push_back(*it);
        }

        size_t countSqSum = 0;
        for (auto& bucket : buckets_list) {
            countSqSum += bucket.size() * bucket.size();
        }

        for (auto& bucket : buckets_list) {
            if (bucket.size() <= 1) continue;
            for (size_t i = 0; i < bucket.size(); ++i) {
                for (size_t j = i + 1; j < bucket.size(); ++j) {
                    if (bucket[i].first == bucket[j].first)
                        throw std::invalid_argument("Duplicate key detected");
                }
            }
        }

        if (countSqSum <= 2*buckets_list.size()) {
            this->_buckets = std::vector<PhBucket<TKey, TValue>>(_tableSize);
            bool needNewGlobalSeed = false;

            for (auto& bucket : buckets_list) {
                if (bucket.size() == 0) continue;
                auto bucketInnerSize = _nextPrime(bucket.size() * bucket.size());

                auto globalIndex = _hash(bucket[0].first, _globalSeed, _tableSize);
                auto bucketSeed = _findSeed(bucket, bucketInnerSize);
                if (bucketSeed == 0) {
                    needNewGlobalSeed = true;
                    break;
                }

                _buckets[globalIndex].Values.resize(bucketInnerSize);
                _buckets[globalIndex].Seed = bucketSeed;

                for (auto& [key, value] : bucket){
                    _buckets[globalIndex].Values[_hash(key, bucketSeed, bucketInnerSize)] = {value, true};
                }
            }

            if (needNewGlobalSeed) {
                _tableSize *= 2;
                buckets_list.resize(_tableSize);
                continue;
            }
            return;
        }
    }
}

template<typename TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::_hash(const TKey& key, const uint64_t seed, const size_t tableSize) const {
    auto keyHash = std::hash<TKey>{}(key);

    uint64_t a = seed * PERFECTHASH_SALT + 1;
    uint64_t b = seed * PERFECTHASH_SALT;

    return (a * keyHash + b) % tableSize;
}

template<typename TKey, typename TValue>
[[nodiscard]] uint64_t PerfectHashDictionary<TKey, TValue>::_randomNum() const {
    static std::mt19937_64 generator(std::random_device{}());
    return generator();
}

template<typename TKey, typename TValue>
uint64_t PerfectHashDictionary<TKey, TValue>::_findSeed(const std::vector<std::pair<TKey, TValue>>& bucket, size_t tableSize) const {
    constexpr size_t maxAttempts = 1000;
    std::vector<bool> occupied(tableSize, false);
    for (size_t i = 0; i < maxAttempts; ++i) {
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

template<typename TKey, typename TValue>
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
