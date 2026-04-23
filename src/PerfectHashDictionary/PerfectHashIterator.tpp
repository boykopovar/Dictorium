#include <vector>

namespace dtr{

template<typename TKey, typename TValue>
struct DictSlot;

template<typename TKey, typename TValue>
class PerfectHashIterator {
public:
    PerfectHashIterator(const std::vector<DictSlot<TKey, TValue>>& slots, size_t index)
        : _slots(&slots), _index(index) { _skipDead(); }

    const std::pair<TKey, TValue>& operator*() const {
        return (*_slots)[_index].Item;
    }

    const std::pair<TKey, TValue>* operator->() const {
        return &(*_slots)[_index].Item;
    }

    PerfectHashIterator& operator++() {
        ++_index;
        _skipDead();
        return *this;
    }

    bool operator==(const PerfectHashIterator& other) const {return _index == other._index;}
    bool operator!=(const PerfectHashIterator& other) const {return _index != other._index;}

private:
    const std::vector<DictSlot<TKey, TValue>>* _slots;
    size_t _index;

    void _skipDead(){
        while(_index < _slots->size() && !(*_slots)[_index].Exists){
            ++_index;
        }
    }
};

}