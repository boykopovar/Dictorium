#ifndef PERFECTDICTIONARY_TPP
#define PERFECTDICTIONARY_TPP
#include <Dictorium/Entities/PerfectHashDictionary.h>


template <typename TKey, typename TValue>
template <SizedIterable TContainer>
void PerfectHashDictionary<TKey, TValue>::_build(TContainer& init) {
    this->_count = init.size();
    while (true) {
        auto buckets_list = std::vector<std::vector<std::pair<TKey, TValue>>>(_count);
        this->_globalSeed = _randomNum();

        for (auto& pair : init) {
            buckets_list[_hash(pair.first, _globalSeed, _count)].push_back(pair);
        }

        size_t countSqSum = 0;
        for (auto& bucket : buckets_list) {
            countSqSum += bucket.size() * bucket.size();
        }
        if (countSqSum <= 2*buckets_list.size()) {
            this->_buckets = std::vector<PhBucket<TKey, TValue>>(_count);
            bool needNewGlobalSeed = false;

            for (auto& bucket : buckets_list) {
                if (bucket.size() == 0) continue;
                auto bucketInnerSize = _nextPrime(std::pow(bucket.size(), 2));

                auto globalIndex = _hash(bucket[0].first, _globalSeed, _count);
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

            if (needNewGlobalSeed) continue;
            return;
        }
    }
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    throw std::logic_error("Not implemented");
}


template<typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    throw std::logic_error("Not implemented");
}

template<typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Clear() {
    throw std::logic_error("Not implemented");
}

template<typename TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::Count() const {
    throw std::logic_error("Not implemented");
}



#endif // PERFECTDICTIONARY_TPP
