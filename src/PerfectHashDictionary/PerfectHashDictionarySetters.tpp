#ifndef PERFECTDICTIONARYSETTERS_TPP
#define PERFECTDICTIONARYSETTERS_TPP


template <typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    std::vector<std::pair<TKey, TValue>> data;
    data.reserve(_count + 1);

    for (auto& [k, v] : *this) {
        data.emplace_back(k, v);
    }
    data.emplace_back(key, value);

    Clear();
    _build(data.begin(), data.end(), data.size());
}

template <typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    auto& slot = _values[_findIndex(key)];
    if (slot.Exists && slot.Item.first == key) slot.Item.second = value;
    else {
        Add(key, value);
    }
}

template<typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.Exists) return false;

    slot.Exists = false;
    return true;
}

template<typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Clear() {
    _buckets.clear();
    _values.clear();

    _count = 0;
    _tableSize = 0;
}


#endif // PERFECTDICTIONARYSETTERS_TPP