#ifndef PERFECTDICTIONARYSETTERS_TPP
#define PERFECTDICTIONARYSETTERS_TPP

namespace  dtr
{

template <CHashable TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    auto flatIndex = _findIndex(key);
    if (flatIndex != -1) {
        auto& slot = _values[flatIndex];
        if (slot.Item.first == key) {
            if (slot.Exists)
                throw std::invalid_argument("Key already exists");
            slot.Item.second = value;
            slot.Exists = true;
            return;
        }
    }

    std::vector<std::pair<TKey, TValue>> data;
    data.reserve(_count + 1);

    for (auto& [k, v] : *this) {
        data.emplace_back(k, v);
    }
    data.emplace_back(key, value);

    Clear();
    _build(data.begin(), data.end(), data.size());
}

template <CHashable TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    auto flatIndex = _findIndex(key);
    if (flatIndex != -1) {
        auto& slot = _values[flatIndex];
        if (slot.Item.first == key) {
            if (!slot.Exists) ++_count;

            slot.Item.second = value;
            slot.Exists = true;
            return;
        }
    }

    std::vector<std::pair<TKey, TValue>> data;
    data.reserve(_count + 1);
    for (auto& [k, v] : *this)
        data.emplace_back(k, v);
    data.emplace_back(key, value);

    Clear();
    _build(data.begin(), data.end(), data.size());
}

template<CHashable TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.Exists || slot.Item.first != key) return false;

    slot.Exists = false;
    --_count;
    return true;
}

template<CHashable TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Clear() {
    for (auto& slot : _values) {
        slot.Exists = false;
    }

    _count = 0;
    _tableSize = 0;
}

}
#endif // PERFECTDICTIONARYSETTERS_TPP