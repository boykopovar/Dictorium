# TreapDictionary

`TreapDictionary<TKey, TValue>` — упорядоченный словарь на основе **декартова дерева (treap)**: бинарное дерево поиска по ключу и куча (max-heap) по случайному приоритету `priority`.

Реализация наследует `IDictionary<TKey, TValue>` и поддерживает обход пар через `begin()` / `end()` (in-order).

---

## Сложности операций

| Метод | O (верхняя граница) | Ω (нижняя граница, ожидаемая) | Амортизированная | Пояснение |
|-------|---------------------|-------------------------------|------------------|-----------|
| `ContainsKey` | O(n) | Ω(log n) | O(log n) | Итеративный BST-поиск по `_root` |
| `TryGetValue` | O(n) | Ω(log n) | O(log n) | Тот же поиск; при успехе копирует `value` |
| `GetValue` | O(n) | Ω(log n) | O(log n) | Поиск + доступ; иначе `std::out_of_range` |
| `InsertOrAssign` (ключ есть) | O(n) | Ω(log n) | O(log n) | `_find` + присвоение `data.second` |
| `InsertOrAssign` (новый ключ) | O(n) | Ω(log n) | O(log n) | `_insert`: BST-вставка + вращения, пока приоритет ребёнка больше |
| `Add` | O(n) | Ω(log n) | O(log n) | `_find`; при дубликате — `std::invalid_argument`, иначе `_insert` |
| `Remove` | O(n) | Ω(log n) | O(log n) | `_remove`: спуск к узлу + вращения по приоритету / снятие листа |
| `Clear` | O(n) | Ω(n) | O(n) | Рекурсивное удаление всех узлов |
| `Count` | O(1) | Ω(1) | O(1) | Поле `_count` |
| `initializer_list` / конструктор диапазона | O(n²) | Ω(n log n) | O(n log n) | `initializer_list` → `Add` на пару; диапазон → `InsertOrAssign` |
| Итерация (`begin` / `end`) | O(n) | Ω(n) | O(n) | In-order обход со стеком |
| `operator[]` (из `IDictionary`) | O(n) | Ω(log n) | O(log n) | Через `GetValue` / `InsertOrAssign` |

> **Обозначения:** n — число элементов. Ожидаемая высота дерева — O(log n) при равномерных случайных приоритетах.

> **Худший случай O(n):** если приоритеты упорядочены так, что дерево вырождается в цепочку (крайне маловероятно при рандомной генерации приоритетов).

---

## Устройство

У каждого узла:

- `data` — пара `(key, value)`;
- `priority` — случайное `uint32_t` (max-heap: у родителя приоритет ≥ детей);
- `left`, `right` — дочерние ссылки.

Инварианты:

1. **BST по ключу** — левое поддерево меньше, правое больше.
2. **Куча по приоритету** — у родителя приоритет не меньше, чем у детей.

### Вставка (`_insert`)

1. Вставить узел как в BST.
2. Поднять узел вращениями (`_rotateLeft` / `_rotateRight`), если приоритет ребёнка выше.

### Удаление (`_remove`)

1. Найти узел по ключу.
2. Если 0–1 ребёнок — удалить и перелинковать.
3. Если 2 ребёнка — вращать узел вниз по более приоритетному ребёнку, затем удалить на листе.

### Вращения

Те же, что у AVL: `_rotateLeft`, `_rotateRight` — локальная перестройка за O(1).

---

## Примеры использования

```cpp
#include "Dictorium/Dictorium.h"

using namespace dtr;

int main() {
    TreapDictionary<std::string, int> dict = {
        {"beta", 2},
        {"alpha", 1},
        {"gamma", 3},
    };

    dict.InsertOrAssign("delta", 4);
    std::cout << dict["alpha"] << '\n';

    for (const auto& [key, value] : dict) {
        std::cout << key << ": " << value << '\n';
    }

    dict.Remove("beta");
    std::cout << dict;
}
```

---

## Итог

`TreapDictionary` — вероятностное сбалансированное дерево с **ожидаемым O(log n)** на поиск, вставку и удаление.
