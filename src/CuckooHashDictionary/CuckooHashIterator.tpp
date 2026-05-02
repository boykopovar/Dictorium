#ifndef CUCKOOHASHITERATOR_TPP
#define CUCKOOHASHITERATOR_TPP

#include <vector>

namespace dtr{

template<typename TKey, typename TValue>
struct DictSlot;

template<typename TKey, typename TValue, bool IsConst>
class CuckooHashIterator {
    using SlotVec = std::vector<DictSlot<TKey, TValue>>;
    using PairRef = std::conditional_t<IsConst,
          const std::pair<TKey, TValue>&,
          std::pair<TKey, TValue>
    >;
    using PairPtr = std::conditional_t<IsConst,
          const std::pair<TKey, TValue>*,
          std::pair<TKey, TValue>*
    >;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<TKey, TValue>;
    using difference_type = std::ptrdiff_t;

    CuckooHashIterator(
        const SlotVec* table1,
        const SlotVec* table2,
        size_t tableIndex,
        size_t slotIndex
    ) : _tables{table1, table2}, _tableIndex{tableIndex}, _slotIndex{slotIndex} {
        _skipDead();
    }

    PairRef operator*() const { return (*_tables[_tableIndex])[_slotIndex].Item; }
    PairPtr operator->() const { return (*_tables[_tableIndex])[_slotIndex].Item; }

    CuckooHashIterator& operator++() {
        ++_slotIndex;
        _skipDead();
        return *this;
    }

    CuckooHashIterator operator++(int) {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    bool operator==(const CuckooHashIterator& other) const {
        return _tableIndex == other._tableIndex && _slotIndex == other._slotIndex;
    }
    bool operator!=(const CuckooHashIterator& other) const {
        return !(*this == other);
    }

private:
    const SlotVec* _tables[2];
    size_t _tableIndex;
    size_t _slotIndex;

    void _skipDead(){
        while(_tableIndex < 2){
            while(_slotIndex < _tables[_tableIndex]->size()){
                if((*_tables[_tableIndex])[_slotIndex].Exists) return;
                ++_slotIndex;
            }
            ++_tableIndex;
            _slotIndex = 0;
        }
    }
};

}
#endif