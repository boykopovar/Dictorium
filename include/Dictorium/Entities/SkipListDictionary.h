#ifndef SKIPLISTDICTIONARY_H
#define SKIPLISTDICTIONARY_H

#include <array>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <random>

#include "Dictorium/Contracts/Contracts.h"

#ifdef _MSC_VER
#include <xmmintrin.h>
#define DTR_SKIPLIST_PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#else
#define DTR_SKIPLIST_PREFETCH(addr) __builtin_prefetch(addr, 0, 1)
#endif

#define DTR_SKIPLIST_MAX_LEVEL 32
#define DTR_SKIPLIST_P 0.5

namespace dtr {
namespace detail {

template <typename TKey, typename TValue>
struct SkipListNode {
    std::pair<TKey, TValue> data;
    uint8_t level = 0;
    std::array<SkipListNode<TKey, TValue>*, DTR_SKIPLIST_MAX_LEVEL + 1> forward{};

    SkipListNode() = default;

    SkipListNode(const TKey& key, const TValue& value, uint8_t nodeLevel)
        : data(key, value), level(nodeLevel) {}
};

}

template <typename TKey, typename TValue>
class IDictionary;

template <typename TKey, typename TValue>
class SkipListDictionary : public IDictionary<TKey, TValue> {
public:
    using Node = detail::SkipListNode<TKey, TValue>;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<TKey, TValue>;
        using reference = std::pair<TKey, TValue>&;
        using pointer = std::pair<TKey, TValue>*;
        using difference_type = std::ptrdiff_t;

        Iterator() : _current(nullptr) {}
        explicit Iterator(Node* node) : _current(node) {}

        reference operator*() const { return _current->data; }
        pointer operator->() const { return &_current->data; }

        Iterator& operator++() {
            _current = _current->forward[0];
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const { return _current == other._current; }
        bool operator!=(const Iterator& other) const { return _current != other._current; }

    private:
        Node* _current;
    };

    SkipListDictionary();

    SkipListDictionary(std::initializer_list<std::pair<TKey, TValue>> init)
        : SkipListDictionary() {
        for (const auto& [key, value] : init) {
            Add(key, value);
        }
    }

    template <typename TIter>
    SkipListDictionary(TIter first, TIter last);

    ~SkipListDictionary() override = default;

    SkipListDictionary(const SkipListDictionary&) = delete;
    SkipListDictionary& operator=(const SkipListDictionary&) = delete;

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;
    void Add(const TKey& key, const TValue& value) override;
    void InsertOrAssign(const TKey& key, const TValue& value) override;
    bool Remove(const TKey& key) override;
    void Clear() override;
    [[nodiscard]] size_t Count() const override;
    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;

    Iterator begin() const { return Iterator(_level >= 0 ? _head[0] : nullptr); }
    Iterator end() const { return Iterator(); }

    std::ostream& WriteToStream(std::ostream& os) const override {
        return this->_writeItems(os, *this);
    }

    [[nodiscard]] int MaxLevel() const { return _level; }

private:
    using UpdatePath = std::array<Node*, DTR_SKIPLIST_MAX_LEVEL + 1>;

    std::array<Node*, DTR_SKIPLIST_MAX_LEVEL + 1> _head{};
    int _level = -1;
    size_t _count = 0;

    std::deque<Node> _pool;
    Node* _freeList = nullptr;
    mutable uint64_t _rngState = 0;

    [[nodiscard]] uint64_t _nextRandom() const;
    [[nodiscard]] uint8_t _randomLevel() const;

    Node* _allocNode(const TKey& key, const TValue& value, uint8_t nodeLevel);
    void _freeNode(Node* node);

    Node* _search(const TKey& key, UpdatePath& update) const;
    void _linkNode(Node* node, const UpdatePath& update, uint8_t nodeLevel);
    void _unlinkNode(Node* node, const UpdatePath& update);
    void _shrinkLevel();
};

}

#include "SkipListDictionary/SkipListDictionary.tpp"

#endif // SKIPLISTDICTIONARY_H
