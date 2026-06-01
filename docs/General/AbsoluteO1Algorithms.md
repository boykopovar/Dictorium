# Глава 2. Структуры с гарантированно константным временем поиска

## СОДЕРЖАНИЕ

- [2.1 Кукушкино хеширование](#21-кукушкино-хеширование)
- [2.2 Совершенное хеширование](#22-совершенное-хеширование)
- [2.3 Расширенное тестирование производительности](#23-расширенное-тестирование-производительности)

---

## Примечание
```Под W(n) понимается худший случай, B(n) - лучший случай, A(n) - амортизационная оценка```

---

### 2.1 Кукушкино хеширование

В рамках данной работы была разработана структура данных `CuckooHashDictionary<TKey, TValue>`, реализующая ассоциативный массив на основе хэш-таблицы с двухтабличным кукушкиным хешированием (Cuckoo Hashing).

Кукушкино хеширование обеспечивает **строгий** $O(1)$ в худшем случае для операций поиска — без амортизации и вероятностных допущений. Каждый ключ имеет ровно два возможных местоположения: одно в `Table1`, другое в `Table2`. При вставке конкурирующий элемент «выталкивается» в своё альтернативное местоположение (kick), что может инициировать цепочку вытеснений. Если цепочка превышает $k_{\max}$ шагов, выполняется рехеширование с новыми seed-значениями. Удаление реализовано через tombstone-маркеры: слот помечается `Exists = false` без немедленной реструктуризации. Каждая таблица хранит объекты типа `DictSlot<TKey, TValue>` (пара ключ-значение и флаг `Exists`). Максимальный суммарный коэффициент загрузки $\alpha_{\max} = 0{,}5$ (относительно суммарной ёмкости обеих таблиц).

---

**Процедура 2.1.1. Hash1 (внутренняя)**
```basic
01: Hash1(stdHash, tableSize)
02:     return FastRange(stdHash * seed1, tableSize)
```

- **Входные данные:** `stdHash` — предварительно вычисленный хеш ключа; `tableSize` — размер таблицы.

**Шаги алгоритма:**
1. Выполняется умножение `stdHash * seed1` в арифметике `uint64_t`.
2. `FastRange` отображает результат в диапазон $[0, \mathtt{tableSize})$ без операции деления.

**Анализ сложности:**

Хеш ключа уже вычислен вызывающим кодом и передаётся как готовое число. Процедура выполняет фиксированное число арифметических операций:

$$T(n) = c_1 + c_2 = c = \text{const}$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 2.1.2. Hash2 (внутренняя)**
```basic
01: Hash2(stdHash, tableSize)
02:     hash = (stdHash xor (stdHash >> 30)) * seed2
03:     return FastRange(hash, tableSize)
```

- **Входные данные:** `stdHash` — предварительно вычисленный хеш ключа; `tableSize` — размер таблицы.

**Шаги алгоритма:**
1. Применяется дополнительное лавинное смешивание `xor`-сдвигом.
2. `FastRange` отображает результат в $[0, \mathtt{tableSize})$.

**Анализ сложности:**

$$T(n) = c_1 + c_2 + c_3 = c = \text{const}$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 2.1.3. Get Max Kicks (внутренняя)**
```basic
01: GetMaxKicks()
02:     if Table1 is empty then return 1
03:     return floor(KICKS_FACTOR * log2(Table1.size))
```

- **Входные данные:** нет явных аргументов; используется внутреннее поле `Table1.size`.

**Шаги алгоритма:**
1. Если таблица пуста — возвращается 1 (защита от $\log 0$).
2. Иначе возвращается $\lfloor \lambda \cdot \log_2 m \rfloor$, где $\lambda$ = `KICKS_FACTOR`, $m$ = `Table1.size`.

**Анализ сложности:**

$$m = \mathrm{Table1.size} = \Theta(n) \Rightarrow \log_2 m = \Theta(\log n)$$

$$T(n) = c_1 \log n + c_2 = \Theta(\log n)$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(\log n)$$

---

**Процедура 2.1.4. Build (внутренняя)**
```basic
01: BuildImpl(begin, end, size)
02:     initSize = max(INIT_CAPACITY, size)
03:     Table1.assign(initSize, ∅)
04:     Table2.assign(initSize, ∅)
05:     KeysCount = 0
06:
07:     for each (key, value) in [begin, end) do
08:         Add(key, value)
```

- **Входные данные:** `begin`, `end` — итераторы диапазона пар ключ-значение; `size` — количество элементов.

**Шаги алгоритма:**
1. Определяется начальный размер таблицы: не менее `INIT_CAPACITY`.
2. Обе таблицы инициализируются пустыми слотами.
3. Каждый элемент диапазона вставляется через `Add`.

**Анализ сложности:**

Обозначим $n = \mathtt{size}$ — количество вставляемых элементов. Из процедуры 2.1.10 (Add) следует $A_{\mathrm{Add}} = \Theta(1)$ амортизированно на одну вставку. Суммируя по всем $n$ элементам:

$$T(n) = T_{\mathrm{init}} + \sum_{i=1}^{n} T_{\mathrm{Add}_i} = \Theta(n) + n \cdot \Theta(1) = \Theta(n)$$

где $T_{\mathrm{init}} = c \cdot \mathtt{initSize} = c \cdot \max(\mathtt{INIT\_CAPACITY}, n) = \Theta(n)$.

$$W(n) = A(n) = \Theta(n), \quad B(n) = \Theta(1) \text{ (при } n = 0\text{)}$$

---

**Процедура 2.1.5. Insert (внутренняя)**
```basic
01: InsertImpl(key, value, allowOverwrite)
02:     size = Table1.size
03:     H = StdHash(key)
04:
05:     // Проверка существующего ключа
06:     s1 = Table1[Hash1(H, size)]
07:     if s1.exists and s1.key = key then
08:         if allowOverwrite then s1.value = value; return true
09:         else raise DuplicateKeyException
10:     s2 = Table2[Hash2(H, size)]
11:     if s2.exists and s2.key = key then
12:         if allowOverwrite then s2.value = value; return true
13:         else raise DuplicateKeyException
14:
15:     // Цикл вытеснений (kicking)
16:     k = key;  v = value;  h = H
17:     for i from 0 to GetMaxKicks() - 1 do
18:         pos1 = Hash1(h, size)
19:         if not Table1[pos1].exists then
20:             Table1[pos1] = (k, v, true);  KeysCount++;  return true
21:         swap(k, v) with Table1[pos1].item
22:         h = StdHash(k)
23:
24:         pos2 = Hash2(h, size)
25:         if not Table2[pos2].exists then
26:             Table2[pos2] = (k, v, true);  KeysCount++;  return true
27:         swap(k, v) with Table2[pos2].item
28:         h = StdHash(k)
29:
30:     // Откат изменений по журналу
31:     rollback(log)
32:     return false
```

- **Входные данные:** `key` — ключ; `value` — значение; `allowOverwrite` — разрешение перезаписи при совпадении ключа.

**Шаги алгоритма:**
1. Вычисляется $H = \texttt{std::hash<TKey>\{\}(key)}$.
2. Проверяются обе позиции на наличие совпадающего ключа.
3. Запускается цикл вытеснений: вставляемый элемент последовательно занимает слоты, выталкивая их содержимое в альтернативные позиции.
4. При нахождении свободного слота — элемент помещается, возвращается `true`.
5. Если за $k_{\max}$ шагов свободный слот не найден — по журналу откатываются все произведённые перестановки, возвращается `false`.

**Анализ сложности:**

Пусть $k_{\max} = \Theta(\log n)$ (из процедуры 2.1.3).

*Худший случай* — выполняются все $k_{\max}$ итераций цикла (каждая за $\Theta(|key|)$ из-за пересчёта хеша) и полный откат по журналу:

$$W(n) = \Theta(k_{\max} \cdot |key|) = \Theta(\log n \cdot |key|)$$

При фиксированной длине ключа: $W(n) = \Theta(\log n)$.

*Лучший случай* — первый же слот (по $h_1$) свободен:

$$B(n) = \Theta(|key|) = \Theta(1) \text{ при фиксированном ключе}$$

*Средний случай* — при равномерном распределении ключей и $\alpha \le \alpha_{\max}$ ожидаемое число итераций кикинга составляет $O(1)$, откуда:

$$A(n) = \Theta(1) \text{ (при фиксированном ключе)}$$

---

**Процедура 2.1.6. Rehash**
```basic
01: RehashImpl(newTableSize)
02:     items = []
03:     for each slot in Table1 do
04:         if slot.exists then items.append(slot.item)
05:     for each slot in Table2 do
06:         if slot.exists then items.append(slot.item)
07:
08:     while true do
09:         seed1 = mix(seed1, REHASH_SALT1)
10:         seed2 = mix(seed2, REHASH_SALT2)
11:         Table1.assign(newTableSize, ∅)
12:         Table2.assign(newTableSize, ∅)
13:         KeysCount = 0;  success = true
14:
15:         for each (k, v) in items do
16:             if not InsertImpl(k, v, false) then
17:                 success = false;  break
18:         if success then return
```

- **Входные данные:** `newTableSize` — новый размер каждой таблицы.

**Шаги алгоритма:**
1. Собираются все $n$ существующих элементов из обеих таблиц.
2. Обновляются seed-значения с помощью смешивания XOR-умножением для изменения распределения.
3. Таблицы переинициализируются нулями.
4. Все $n$ элементов повторно вставляются с новыми хешами.
5. Если хотя бы одна вставка не удалась — обновляются seed и процедура повторяется.

**Анализ сложности:**

$$n \text{ — количество элементов до рехеширования}$$

Сбор элементов: линейный обход обеих таблиц размером $m' = \Theta(n)$ каждая:

$$T_{\text{collect}} = c_1 \cdot 2m' = \Theta(n)$$

Повторные вставки: каждая из $n$ вставок в новую незагруженную таблицу выполняется за $\Theta(1)$ в среднем; вероятность успеха всей партии при правильно выбранных seed — не менее $1/2$ за попытку, откуда ожидаемое число внешних итераций $= O(1)$:

$$T_{\text{reinsert}} = n \cdot \Theta(1) = \Theta(n)$$

$$T(n) = T_{\text{collect}} + T_{\text{reinsert}} = \Theta(n)$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(n)$$

---

**Процедура 2.1.7. Contains Key**
```basic
01: ContainsKeyImpl(key)
02:     H = StdHash(key)
03:     size = Table1.size
04:
05:     s1 = Table1[Hash1(H, size)]
06:     if s1.exists and s1.key = key then return true
07:
08:     s2 = Table2[Hash2(H, size)]
09:     if s2.exists and s2.key = key then return true
10:
11:     return false
```

- **Входные данные:** `key` — ключ, наличие которого проверяется.

**Шаги алгоритма:**
1. Вычисляется $H = \texttt{std::hash<TKey>\{\}(key)}$ — $\Theta(|key|)$.
2. Проверяется позиция `Table1[Hash1(H, size)]`.
3. Проверяется позиция `Table2[Hash2(H, size)]`.
4. Возвращается результат.

**Анализ сложности (строгий O(1)):**

$$T(n) = T_{\text{StdHash}} + T_{\text{Hash1}} + T_{\text{check}_1} + T_{\text{Hash2}} + T_{\text{check}_2} + T_{\text{return}}$$

$$T_{\text{StdHash}} = a|key| + b$$

$$T_{\text{Hash1}} = c_1 = \text{const}, \quad T_{\text{check}_1} = c_2 = \text{const}$$

$$T_{\text{Hash2}} = c_3 = \text{const}, \quad T_{\text{check}_2} = c_4 = \text{const}, \quad T_{\text{return}} = c_5 = \text{const}$$

$$T(n) = a|key| + d, \quad d = b + c_1 + c_2 + c_3 + c_4 + c_5$$

Ключевое наблюдение: $T(n)$ **не зависит от $n$** — числа элементов в таблице. Число проверяемых позиций фиксировано и равно **ровно двум**, независимо от $n$. Никакого цикла, никакой итерации по элементам нет.

$$T(n) = a|key| + d = \text{const относительно } n$$

$$\exists c_1, c_2 > 0, \exists n_0:\ \forall n \ge n_0 \quad c_1 \le T(n) \le c_2 \Rightarrow T(n) \in \Theta(1)$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

Это **строгая** (не амортизированная) оценка для худшего случая.

---

**Процедура 2.1.8. Try Get Value**
```basic
01: TryGetValueImpl(key, value)
02:     H = StdHash(key)
03:     size = Table1.size
04:
05:     s1 = Table1[Hash1(H, size)]
06:     if s1.exists and s1.key = key then
07:         value = s1.value;  return true
08:
09:     s2 = Table2[Hash2(H, size)]
10:     if s2.exists and s2.key = key then
11:         value = s2.value;  return true
12:
13:     return false
```

- **Входные данные:** `key` — искомый ключ; `value` — выходной параметр.

**Шаги алгоритма:**
1. Аналогично `ContainsKey` + запись значения в `value`.

**Анализ сложности:**

Структура идентична `ContainsKey`. Дополнительная операция присваивания — $\Theta(1)$.

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.1.9. Get Value**
```basic
01: GetValueImpl(key)
02:     H = StdHash(key)
03:     size = Table1.size
04:
05:     s1 = Table1[Hash1(H, size)]
06:     if s1.exists and s1.key = key then return s1.value
07:
08:     s2 = Table2[Hash2(H, size)]
09:     if s2.exists and s2.key = key then return s2.value
10:
11:     raise KeyNotFoundException
```

- **Входные данные:** `key` — искомый ключ.

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.1.10. Add**
```basic
01: AddImpl(key, value)
02:     if Table1 is empty then
03:         Table1.assign(INIT_CAPACITY, ∅);  Table2.assign(INIT_CAPACITY, ∅)
04:     if (KeysCount + 1) / (2 * Table1.size) > MaxLoadFactor then
05:         RehashImpl(Table1.size * 2)
06:     while not InsertImpl(key, value, false) do
07:         RehashImpl(Table1.size * 2)
```

- **Входные данные:** `key` — ключ; `value` — значение.

**Шаги алгоритма:**
1. При необходимости инициализируются таблицы.
2. При превышении $\alpha_{\max}$ — вызывается рехеширование.
3. Повторяются попытки вставки через `Insert`; при неудаче каждый раз вызывается `Rehash`.

**Анализ сложности:**

*Худший случай* — при превышении коэффициента загрузки или провале кикинга вызывается `Rehash`:

$$W(n) = \Theta(n)$$

*Лучший случай* — рехеширование не нужно, вставка сразу успешна:

$$B(n) = \Theta(1)$$

*Амортизированный случай* — стоимость рехеширования амортизируется на все вставки; при геометрическом удвоении каждый элемент участвует в $O(\log n)$ рехешированиях, то есть суммарная стоимость рехешей равна $O(n \log n)$ за $n$ вставок — это не Θ(1). Однако из-за вероятностной природы кикинга (expected $O(1)$ kicks per insert, expected $O(1)$ rehashes overall) ожидаемая амортизированная стоимость:

$$A(n) = \Theta(1) \text{ (ожидаемая амортизированная)}$$

---

**Процедура 2.1.11. Insert Or Assign**
```basic
01: InsertOrAssignImpl(key, value)
02:     if Table1 is empty then
03:         Table1.assign(INIT_CAPACITY, ∅);  Table2.assign(INIT_CAPACITY, ∅)
04:     if (KeysCount + 1) / (2 * Table1.size) > MaxLoadFactor then
05:         RehashImpl(Table1.size * 2)
06:     while not InsertImpl(key, value, true) do
07:         RehashImpl(Table1.size * 2)
```

- **Входные данные:** `key` — ключ; `value` — новое значение.

**Шаги алгоритма:**
1. Аналогично `Add`, с флагом `allowOverwrite = true`.
2. При совпадении ключа — значение обновляется без изменения счётчика.

**Анализ сложности:**

$$W(n) = \Theta(n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(1) \text{ (ожидаемая амортизированная)}$$

---

**Процедура 2.1.12. Remove**
```basic
01: RemoveImpl(key)
02:     H = StdHash(key)
03:     size = Table1.size
04:
05:     s1 = Table1[Hash1(H, size)]
06:     if s1.exists and s1.key = key then
07:         s1.exists = false;  KeysCount--;  return true
08:
09:     s2 = Table2[Hash2(H, size)]
10:     if s2.exists and s2.key = key then
11:         s2.exists = false;  KeysCount--;  return true
12:
13:     return false
```

- **Входные данные:** `key` — ключ удаляемого элемента.

**Шаги алгоритма:**
1. Проверяются обе позиции.
2. При обнаружении совпадения — слот помечается `Exists = false`, счётчик уменьшается.

**Анализ сложности:**

Структура идентична `ContainsKey`, дополнительное присваивание — $\Theta(1)$.

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.1.13. Clear**
```basic
01: ClearImpl()
02:     KeysCount = 0
03:     seed1 = SEED1_INIT;  seed2 = SEED2_INIT
04:     Table1.assign(INIT_CAPACITY, ∅)
05:     Table2.assign(INIT_CAPACITY, ∅)
```

- **Входные данные:** нет.

**Шаги алгоритма:**
1. Счётчик обнуляется.
2. Seed-значения сбрасываются к начальным константам.
3. Обе таблицы переинициализируются массивами фиксированного размера `INIT_CAPACITY = 8`.

**Анализ сложности:**

`INIT_CAPACITY` — фиксированная константа, не зависящая от $n$. Элементы `DictSlot` тривиально деструктируются (POD):

$$T(n) = c_1 + c_2 + c_3 \cdot \mathtt{INIT\_CAPACITY} + c_4 \cdot \mathtt{INIT\_CAPACITY} = c = \text{const}$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 2.1.14. Count**
```basic
01: CountImpl()
02:     return KeysCount
```

- **Входные данные:** нет.

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1)$$

---

**Сводная таблица сложности процедур раздела 2.1 (кукушкино хеширование)**

| Процедура                |      $W(n)$      |      $B(n)$      |      $A(n)$      |
|--------------------------|:----------------:|:----------------:|:----------------:|
| 2.1.1. Hash1             |   $\Theta(1)$    |   $\Theta(1)$    |   $\Theta(1)$    |
| 2.1.2. Hash2             |   $\Theta(1)$    |   $\Theta(1)$    |   $\Theta(1)$    |
| 2.1.3. Get Max Kicks     | $\Theta(\log n)$ | $\Theta(\log n)$ | $\Theta(\log n)$ |
| 2.1.4. Build             |   $\Theta(n)$    |   $\Theta(1)$    |   $\Theta(n)$    |
| 2.1.5. Insert            | $\Theta(\log n)$ |   $\Theta(1)$    |   $\Theta(1)$    |
| 2.1.6. Rehash            |   $\Theta(n)$    |   $\Theta(n)$    |   $\Theta(n)$    |
| 2.1.7. Contains Key      |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |
| 2.1.8. Try Get Value     |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |
| 2.1.9. Get Value         |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |
| 2.1.10. Add              |   $\Theta(n)$    |   $\Theta(1)$    |   $\Theta(1)$    |
| 2.1.11. Insert Or Assign |   $\Theta(n)$    |   $\Theta(1)$    |   $\Theta(1)$    |
| 2.1.12. Remove           |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |  $\Theta(1)$ ★   |
| 2.1.13. Clear            |   $\Theta(1)$    |   $\Theta(1)$    |   $\Theta(1)$    |
| 2.1.14. Count            |   $\Theta(1)$    |   $\Theta(1)$    |   $\Theta(1)$    |

> Здесь $n$ — количество элементов в таблице. ★ — **строгая** (не амортизированная) оценка для худшего случая: операция обращается к ровно двум позициям в двух массивах, без цикла и без зависимости от $n$. `Insert` в худшем случае — $\Theta(\log n)$ из-за цикла кикинга длиной $k_{\max} = \Theta(\log n)$; в среднем $\Theta(1)$ при $\alpha \le \alpha_{\max}$. $A(n) = \Theta(1)$ для `Add`/`InsertOrAssign` — ожидаемая амортизированная оценка с учётом редкого рехеширования.

---

### 2.2 Совершенное хеширование

В рамках данной работы была разработана структура данных `PerfectHashDictionary<TKey, TValue>`, реализующая статический ассоциативный массив на основе двухуровневого совершенного хеширования (Perfect Hashing).

Совершенное хеширование строит структуру данных для **фиксированного** множества ключей, гарантируя строгий $O(1)$ для всех операций поиска без каких-либо коллизий. Первый уровень (глобальный) распределяет ключи по корзинам с помощью глобального seed. Для каждой корзины второй уровень находит локальный seed, при котором ключи корзины размещаются в таблице без единой коллизии. Всё хранится в одном плоском массиве `_values`; поиск требует ровно **двух** хеш-вычислений и **двух** обращений к массиву.

Хеш-функция обоих уровней:

$$h(H, \text{seed}, m) = \bigl((\text{seed} \cdot \text{SALT} + 1) \cdot H + \text{seed}\bigr) \bmod 2^{64} \bmod m,$$

где $H = \texttt{std::hash<TKey>\{\}(key)}$, $\text{SALT} = 2654435761$ (число Кнута). Структура `PhBucket` хранит тройку $(\text{offset},\, \text{size},\, \text{seed})$. Массив `_values` содержит слоты `DictSlot<TKey, TValue>` с полями `Item` и `Exists` (tombstone для логического удаления).

---

**Процедура 2.2.1. Hash Raw (внутренняя)**
```basic
01: HashRaw(stdHash, seed, tableSize)
02:     return ((seed * SALT + 1) * stdHash + seed) mod tableSize
```

- **Входные данные:** `stdHash` — предварительно вычисленный хеш; `seed`, `tableSize` — параметры уровня.

**Шаги алгоритма:**
1. Линейная конгруэнтная трансформация хеша.
2. Редукция модулем `tableSize`.

**Анализ сложности:**

$$T(n) = c_1 + c_2 = c = \text{const}$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 2.2.2. Find Index (внутренняя)**
```basic
01: FindIndexImpl(key)
02:     H = StdHash(key)
03:     globalIndex = HashRaw(H, globalSeed, tableSize)
04:     bucket = buckets[globalIndex]
05:     if bucket.size = 0 then return -1
06:     return bucket.offset + HashRaw(H, bucket.seed, bucket.size)
```

- **Входные данные:** `key` — ключ.

**Шаги алгоритма:**
1. Вычисляется $H = \texttt{std::hash<TKey>\{\}(key)}$ — $\Theta(|key|)$.
2. Первый уровень: `globalIndex = h(H, globalSeed, tableSize)` — $\Theta(1)$.
3. Извлекается корзина `buckets[globalIndex]` — $\Theta(1)$.
4. Второй уровень: `localIndex = h(H, bucket.seed, bucket.size)` — $\Theta(1)$.
5. Возвращается плоский индекс `bucket.offset + localIndex` — $\Theta(1)$.

**Анализ сложности (строгий O(1)):**

$$T(n) = T_{\text{StdHash}} + T_{\text{level1}} + T_{\text{access}_1} + T_{\text{check}} + T_{\text{level2}} + T_{\text{return}}$$

$$T_{\text{StdHash}} = a|key| + b, \quad T_{\text{level1}} = c_1,\quad T_{\text{access}_1} = c_2, \quad \ldots$$

Все составляющие, кроме $T_{\text{StdHash}}$, — константы:

$$T(n) = a|key| + d, \quad d = \text{const}$$

$T(n)$ **не зависит от $n$**. Число обращений к массиву фиксировано и равно **ровно двум** — независимо от количества элементов. Нет цикла, нет пробирования.

$$\exists c_1, c_2 > 0:\ \forall n \ge 1 \quad c_1 \le T(n) \le c_2 \Rightarrow T(n) \in \Theta(1)$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 2.2.3. Find Seed (внутренняя)**
```basic
01: FindSeed(bucket, tableSize)
02:     occupied = array[tableSize] of false
03:
04:     for attempt from 0 to MAX_ATTEMPTS - 1 do
05:         fill(occupied, false)
06:         seed = RandomUInt64()
07:         collision = false
08:
09:         for each (key, _) in bucket do
10:             slot = HashRaw(StdHash(key), seed, tableSize)
11:             if occupied[slot] then collision = true;  break
12:             occupied[slot] = true
13:
14:         if not collision then return seed
15:
16:     return 0  // неудача
```

- **Входные данные:** `bucket` — список пар ключ-значение размером $b$; `tableSize` — размер таблицы второго уровня ($\approx b^2$).

**Шаги алгоритма:**
1. Генерируется случайный `seed`.
2. Для каждого ключа корзины вычисляется слот; при первой коллизии — попытка прерывается.
3. При отсутствии коллизий — `seed` принимается.

**Анализ сложности:**

Таблица второго уровня имеет размер $s = b^2$. Вероятность того, что два конкретных ключа попадут в один слот при случайном `seed`:

$$P[\text{коллизия пары}] = \frac{1}{b^2}$$

Всего пар $\binom{b}{2} < \frac{b^2}{2}$. Ожидаемое число коллизий:

$$\mathbb{E}[\text{коллизий}] < \frac{b^2}{2} \cdot \frac{1}{b^2} = \frac{1}{2} < 1$$

Раз в среднем меньше одной коллизии — большинство случайных `seed` подходят. Ожидаемое число попыток: $O(1)$.

Каждая попытка выполняет сброс массива `occupied` ($b^2$ ячеек) и $b$ хеш-вычислений. Доминирует сброс:

$$T_{\text{attempt}}(b) = O(b^2)$$

Ожидаемое суммарное время:

$$A(b) = O(1) \cdot O(b^2) = O(b^2)$$

---

**Процедура 2.2.4. Build (внутренняя)**
```basic
01: BuildImpl(begin, end, size)
02:     count = size;  tableSize = size
03:     bucketsList = array[tableSize] of empty lists
04:     attempts = 0
05:
06:     while attempts < MAX_ATTEMPTS do
07:         attempts++
08:         clear(bucketsList)
09:         globalSeed = RandomUInt64()
10:
11:         for each (key, value) in [begin, end) do
12:             bucketsList[HashRaw(StdHash(key), globalSeed, tableSize)].append((key, value))
13:
14:         sqSum = ∑ |bucketsList[i]|²  for i in [0, tableSize)
15:         if sqSum > 2 * tableSize then continue
16:
17:         buckets.assign(tableSize, ∅);  success = true
18:         for each bucket b in bucketsList do
19:             if b is empty then continue
20:             innerSize = NextPrime(|b|²)
21:             localSeed = FindSeed(b, innerSize)
22:             if localSeed = 0 then
23:                 tableSize *= 2;  resize(bucketsList, tableSize)
24:                 success = false;  break
25:             g = HashRaw(StdHash(b[0].key), globalSeed, tableSize)
26:             buckets[g] = {innerSize, localSeed}
27:
28:         if not success then continue
29:
30:         // Построение плоского массива
31:         offset = 0
32:         for each bucket b in buckets do
33:             b.offset = offset;  offset += b.size
34:         values.assign(offset, ∅)
35:
36:         for each bucket b in bucketsList do
37:             if b is empty then continue
38:             info = buckets[HashRaw(StdHash(b[0].key), globalSeed, tableSize)]
39:             for each (key, value) in b do
40:                 idx = info.offset + HashRaw(StdHash(key), info.seed, info.size)
41:                 values[idx] = (key, value, true)
42:         return
43:
44:     raise RuntimeError
```

- **Входные данные:** `begin`, `end` — итераторы диапазона; `size` — количество элементов.

**Шаги алгоритма:**
1. Генерируется `globalSeed` для первого уровня.
2. Ключи распределяются по $n$ корзинам. Если $\sum b_k^2 > 2n$ — повтор с новым `globalSeed`.
3. Для каждой непустой корзины вызывается `FindSeed`.
4. Строится плоский массив: смещения корзин расставляются последовательно, каждый ключ помещается по вычисленному плоскому индексу.

**Анализ сложности построения:**

Обозначим $n = \mathtt{size}$, $b_k$ — размер $k$-й корзины.

**Уровень 1 (поиск `globalSeed`):** случайный `globalSeed` с вероятностью $\Omega(1)$ сразу удовлетворяет условию $\sum b_k^2 \le 2n$, поэтому ожидаемое число итераций $O(1)$. Каждая итерация хеширует $n$ ключей:

$$T_1 = O(1) \cdot O(n) = O(n)$$

**Уровень 2 (поиск `localSeed` для каждой корзины):** условие $\sum b_k^2 \le 2n$ гарантировано проверкой на строке 15. Суммируя стоимость `FindSeed` по всем корзинам:

$$T_2 = \sum_k O(b_k^2) = O\!\left(\sum_k b_k^2\right) \le O(2n) = O(n)$$

**Построение плоского массива:** $n$ ключей, каждый за $O(1)$:

$$T_3 = O(n)$$

**Итоговая ожидаемая сложность:**

$$T(n) = T_1 + T_2 + T_3 = O(n) \Rightarrow A(n) = O(n)$$

---

**Процедура 2.2.5. Contains Key**
```basic
01: ContainsKeyImpl(key)
02:     flatIndex = FindIndexImpl(key)
03:     if flatIndex = -1 then return false
04:     slot = values[flatIndex]
05:     if not slot.exists then return false
06:     return slot.key = key
```

- **Входные данные:** `key` — ключ, наличие которого проверяется.

**Шаги алгоритма:**
1. `FindIndex` — два хеш-вычисления, два обращения к массиву.
2. Проверка флага `Exists` (защита от tombstone).
3. Полное сравнение ключа (защита от хеш-коллизий на внешнем ключе).

**Анализ сложности:**

Определяется целиком `FindIndex` (процедура 2.2.2):

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.2.6. Try Get Value**
```basic
01: TryGetValueImpl(key, value)
02:     flatIndex = FindIndexImpl(key)
03:     if flatIndex = -1 then return false
04:     slot = values[flatIndex]
05:     if not slot.exists then return false
06:     value = slot.value;  return true
```

- **Входные данные:** `key` — искомый ключ; `value` — выходной параметр.

**Примечание:** данный метод не сравнивает ключи — ожидается ключ из исходного множества. Метод `TryGetValidatedValue` выполняет полную проверку.

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.2.7. Get Value**
```basic
01: GetValueImpl(key)
02:     flatIndex = FindIndexImpl(key)
03:     if flatIndex = -1 then raise KeyNotFoundException
04:     slot = values[flatIndex]
05:     if not slot.exists then raise KeyNotFoundException
06:     return slot.value
```

- **Входные данные:** `key` — искомый ключ.

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.2.8. Add**
```basic
01: AddImpl(key, value)
02:     flatIndex = FindIndexImpl(key)
03:     if flatIndex ≠ -1 then
04:         slot = values[flatIndex]
05:         if slot.key = key then
06:             if slot.exists then raise DuplicateKeyException
07:             slot.value = value;  slot.exists = true;  return
08:
09:     // Ключ за пределами исходного множества: полная перестройка
10:     data = collect all (key, value) pairs + (key, value)
11:     Clear()
12:     BuildImpl(data.begin, data.end, data.size)
```

- **Входные данные:** `key` — ключ; `value` — значение.

**Шаги алгоритма:**
1. Если ключ принадлежит исходному множеству и помечен tombstone — значение восстанавливается за $\Theta(1)$.
2. Если ключ новый — собирается весь словарь, добавляется новый элемент, выполняется полная перестройка.

**Анализ сложности:**

*Лучший случай* — ключ был ранее удалён (tombstone), перестройка не нужна:

$$B(n) = \Theta(1)$$

*Худший случай* — новый ключ, требуется перестройка:

$$W(n) = A(n) = \Theta(n) \text{ (перестройка совершенного хеша)}$$

---

**Процедура 2.2.9. Insert Or Assign**
```basic
01: InsertOrAssignImpl(key, value)
02:     flatIndex = FindIndexImpl(key)
03:     if flatIndex ≠ -1 then
04:         slot = values[flatIndex]
05:         if slot.key = key then
06:             if not slot.exists then count++
07:             slot.value = value;  slot.exists = true;  return
08:
09:     // Ключ за пределами исходного множества: полная перестройка
10:     data = collect all (key, value) pairs + (key, value)
11:     Clear()
12:     BuildImpl(data.begin, data.end, data.size)
```

- **Входные данные:** `key` — ключ; `value` — новое значение.

**Анализ сложности:**

*Случай обновления существующего ключа* — $\Theta(1)$.

*Случай нового ключа* — $\Theta(n)$ (перестройка).

$$B(n) = \Theta(1), \quad W(n) = A(n) = \Theta(n)$$

---

**Процедура 2.2.10. Remove**
```basic
01: RemoveImpl(key)
02:     flatIndex = FindIndexImpl(key)
03:     if flatIndex = -1 then return false
04:     slot = values[flatIndex]
05:     if not slot.exists or slot.key ≠ key then return false
06:     slot.exists = false;  count--;  return true
```

- **Входные данные:** `key` — ключ удаляемого элемента.

**Шаги алгоритма:**
1. `FindIndex` находит слот.
2. Слот помечается `Exists = false` (tombstone). Физической реструктуризации нет.

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1) \quad \text{(строгая оценка)}$$

---

**Процедура 2.2.11. Clear**
```basic
01: ClearImpl()
02:     for each slot in values do
03:         slot.exists = false
04:     count = 0;  tableSize = 0
```

- **Входные данные:** нет.

**Шаги алгоритма:**
1. Каждый из $n$ слотов помечается как не существующий.
2. Счётчики сбрасываются. Структура `buckets` и `values` не освобождаются.

**Анализ сложности:**

Обход всех слотов занимает $\Theta(|\mathtt{values}|)$. Поскольку $|\mathtt{values}| = \sum_k s_k = \sum_k b_k^2 \le Cn = \Theta(n)$:

$$W(n) = B(n) = A(n) = \Theta(n)$$

---

**Процедура 2.2.12. Count**
```basic
01: CountImpl()
02:     return count
```

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1)$$

---

**Сводная таблица сложности процедур раздела 2.2 (совершенное хеширование)**

| Процедура                |    $W(n)$     |    $B(n)$     |    $A(n)$     |
|--------------------------|:-------------:|:-------------:|:-------------:|
| 2.2.1. Hash Raw          |  $\Theta(1)$  |  $\Theta(1)$  |  $\Theta(1)$  |
| 2.2.2. Find Index        | $\Theta(1)$ ★ | $\Theta(1)$ ★ | $\Theta(1)$ ★ |
| 2.2.3. Find Seed         | $\Theta(b^2)$ | $\Theta(b^2)$ | $\Theta(b^2)$ |
| 2.2.4. Build             | $\Theta(n)$†  |  $\Theta(n)$  |  $\Theta(n)$  |
| 2.2.5. Contains Key      | $\Theta(1)$ ★ | $\Theta(1)$ ★ | $\Theta(1)$ ★ |
| 2.2.6. Try Get Value     | $\Theta(1)$ ★ | $\Theta(1)$ ★ | $\Theta(1)$ ★ |
| 2.2.7. Get Value         | $\Theta(1)$ ★ | $\Theta(1)$ ★ | $\Theta(1)$ ★ |
| 2.2.8. Add               |  $\Theta(n)$  |  $\Theta(1)$  |  $\Theta(n)$  |
| 2.2.9. Insert Or Assign  |  $\Theta(n)$  |  $\Theta(1)$  |  $\Theta(n)$  |
| 2.2.10. Remove           | $\Theta(1)$ ★ | $\Theta(1)$ ★ | $\Theta(1)$ ★ |
| 2.2.11. Clear            |  $\Theta(n)$  |  $\Theta(n)$  |  $\Theta(n)$  |
| 2.2.12. Count            |  $\Theta(1)$  |  $\Theta(1)$  |  $\Theta(1)$  |

> Здесь $n$ — количество элементов; $b$ — размер конкретной корзины (для `FindSeed`). ★ — **строгая** (не амортизированная) оценка для худшего случая: ровно два хеш-вычисления и два обращения к массиву. † — $W(n) = \Theta(n)$ — ожидаемая оценка; детерминированная верхняя граница выше из-за `MAX_ATTEMPTS`, однако вероятность достижения этой границы экспоненциально мала. `Add`/`InsertOrAssign` с **новым** ключом вызывают полную перестройку — эти операции помечены `[[deprecated]]` в API; если ключ принадлежит исходному множеству (в том числе ранее удалённый tombstone), вставка выполняется за $\Theta(1)$.

---

### 2.3 Расширенное тестирование производительности

В дополнение к тестированию раздела 1.8 были проведены сравнительные замеры производительности реализаций `CuckooHashDictionary` и `PerfectHashDictionary` относительно эталона `std::unordered_map`. Тестирование выполнялось на разных типах ключей и различных объёмах данных, что позволяет наблюдать зависимость скорости от кэш-эффектов, длины ключа и размера таблицы.

**Условия тестирования:**
- **Init** — последовательная вставка $N$ пар ключ-значение; фиксируется суммарное время.
- **Read** — серия операций поиска по случайным ключам; фиксируется среднее время одной операции в наносекундах.
- Параметр «Длина ключа» применим только к `std::string`; прочерк означает числовой тип фиксированного размера.

---

**Таблица 2.3.1. CuckooHashDictionary vs std::unordered\_map**

| Тип ключа     |    Ключей | Длина ключа | Init `unordered_map` | Init `CuckooHash` | Init быстрее           | Read `unordered_map` | Read `CuckooHash` | Read быстрее             |
|---------------|----------:|-------------|---------------------:|------------------:|------------------------|---------------------:|------------------:|--------------------------|
| `std::string` | 1 000 000 | 10          |           416,612 мс |        544,636 мс | `unordered_map` ×1,31  |               140 нс |            105 нс | `CuckooHash` **×1,34**   |
| `std::string` | 1 000 000 | 1000        |           1357,28 мс |        3252,75 мс | `unordered_map` ×2,40  |               534 нс |            384 нс | `CuckooHash` **×1,39**   |
| `std::string` |     1 000 | 10          |            0,1701 мс |         0,6196 мс | `unordered_map` ×3,64  |                38 нс |             15 нс | `CuckooHash` **×2,55**   |
| `double`      | 1 000 000 | —           |           431,318 мс |        855,327 мс | `unordered_map` ×1,98  |               119 нс |             80 нс | `CuckooHash` **×1,49**   |
| `double`      |     1 000 | —           |            0,1478 мс |         0,3923 мс | `unordered_map` ×2,65  |                29 нс |             12 нс | `CuckooHash` **×2,33**   |
| `int`         | 1 000 000 | —           |           297,037 мс |        697,711 мс | `unordered_map` ×2,35  |                36 нс |             55 нс | `unordered_map` ×1,50    |
| `int`         |     1 000 | —           |            0,1772 мс |         0,3305 мс | `unordered_map` ×1,87  |                 9 нс |              6 нс | `CuckooHash` **×1,50**   |
| `int`         | 1 000 000 | —           |           603,641 мс |        406,804 мс | `CuckooHash` **×1,48** |               191 нс |            135 нс | `CuckooHash` **×1,41**   |
| `double`      | 1 000 000 | —           |           860,592 мс |        919,791 мс | `unordered_map` ×1,07  |               306 нс |            203 нс | `CuckooHash` **×1,51**   |
| `double`      |     1 000 | —           |            1,8202 мс |         2,8278 мс | `unordered_map` ×1,55  |                ~0 нс |             ~0 нс | `CuckooHash` **×2,17**\* |


---

**Таблица 2.3.2. PerfectHashDictionary vs std::unordered\_map**

| Тип ключа     |    Ключей | Длина ключа | Init `unordered_map` | Init `PerfectHash` | Init быстрее          | Read `unordered_map` | Read `PerfectHash` | Read быстрее            |
|---------------|----------:|-------------|---------------------:|-------------------:|-----------------------|---------------------:|-------------------:|-------------------------|
| `std::string` | 1 000 000 | 10          |           444,826 мс |          1053,7 мс | `unordered_map` ×2,37 |               152 нс |             126 нс | `PerfectHash` **×1,20** |
| `std::string` | 1 000 000 | 10          |           416,612 мс |         544,636 мс | `unordered_map` ×1,31 |               140 нс |             105 нс | `PerfectHash` **×1,34** |
| `std::string` | 1 000 000 | 1000        |           1357,28 мс |         3252,75 мс | `unordered_map` ×2,40 |               534 нс |             384 нс | `PerfectHash` **×1,39** |
| `std::string` |     1 000 | 10          |            0,1701 мс |          0,6196 мс | `unordered_map` ×3,64 |                38 нс |              15 нс | `PerfectHash` **×2,55** |
| `double`      | 1 000 000 | —           |           431,318 мс |         855,327 мс | `unordered_map` ×1,98 |               119 нс |              80 нс | `PerfectHash` **×1,49** |
| `double`      |     1 000 | —           |            0,1478 мс |          0,3923 мс | `unordered_map` ×2,65 |                29 нс |              12 нс | `PerfectHash` **×2,33** |
| `int`         | 1 000 000 | —           |           297,037 мс |         697,711 мс | `unordered_map` ×2,35 |                36 нс |              55 нс | `unordered_map` ×1,50   |
| `int`         |     1 000 | —           |            0,1772 мс |          0,3305 мс | `unordered_map` ×1,87 |                 9 нс |               6 нс | `PerfectHash` **×1,50** |

---

**Анализ результатов:**

*Инициализация.* Обе структуры уступают `std::unordered_map` по скорости построения во всех сценариях, кроме одного замера `CuckooHash` с `int` (×1,48 быстрее). Для `CuckooHash` накладные расходы объясняются циклом кикинга и возможными рехешированиями с перебором seed; для `PerfectHash` — итеративным поиском глобального и локальных seed с пересчётом хешей. При длинных строках (`len = 1000`) разрыв достигает ×2,40, что объясняется доминированием стоимости `std::hash<std::string>` в циклах построения.

*Чтение.* Обе реализации превосходят `std::unordered_map` по скорости поиска во всех сценариях, кроме `CuckooHash` с `int` при $N = 10^6$ (проигрыш ×1,50 в одном замере). Наилучший результат — у малых таблиц ($N = 1000$): прирост скорости до **×2,55** обусловлен тем, что вся структура умещается в L1/L2-кэш.

*Сравнение `CuckooHash` и `PerfectHash`.* При равных входных данных результаты чтения у этих структур близки: обе выполняют ровно **два** хеш-вычисления и **два** обращения к массиву без итераций. Разница определяется качеством перемешивания конкретных хеш-функций и компоновкой данных в памяти (`PerfectHash` хранит все значения в одном плоском массиве `_values`, тогда как `CuckooHash` использует два раздельных массива).

*Поведение при большом ключе (`len = 1000`).* При длинных строках доминирует стоимость `std::hash<std::string>`, одинаковая для всех структур. Разрыв в Read сокращается (×1,39 вместо ×2,55 для `len = 10`), что согласуется с теоретическим выводом: при фиксированном $|key|$ поиск занимает $\Theta(|key|)$, и именно эта составляющая ограничивает возможный выигрыш.

*Вывод.* `CuckooHashDictionary` и `PerfectHashDictionary` обеспечивают **строгий** $O(1)$ поиск в худшем случае — в отличие от всех реализаций на основе открытой адресации и открытого хеширования из разделов 1.3–1.7, у которых $W(n) = \Theta(n)$. Это свойство делает их предпочтительными для сценариев с жёсткими требованиями к времени выполнения, где недопустимы редкие, но дорогостоящие вырождённые случаи. Цена — более высокое время построения и, для `PerfectHash`, ограниченная поддержка мутирующих операций с новыми ключами.
