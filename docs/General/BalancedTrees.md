
## СОДЕРЖАНИЕ

1. [Глава 3. Сбалансированные деревья поиска](#глава-3-сбалансированные-деревья-поиска)
   - [3.1 Введение в сбалансированные деревья](#31-введение-в-сбалансированные-деревья)
   - [3.2 AVL-дерево](#32-avl-дерево)
   - [3.3 Красно-чёрное дерево](#33-красно-чёрное-дерево)
   - [3.4 Тестирование производительности](#34-тестирование-производительности)

---

## Примечание
```Под W(n) понимается худший случай, B(n) - лучший случай, A(n) - амортизационная оценка```
---

## ГЛАВА 3. СБАЛАНСИРОВАННЫЕ ДЕРЕВЬЯ ПОИСКА

### 3.1 Введение в сбалансированные деревья

Двоичное дерево поиска (Binary Search Tree, BST) — структура данных, поддерживающая словарные операции INSERT, SEARCH и DELETE. Для узла $x$ в BST выполняется инвариант: все ключи в левом поддереве строго меньше $x.key$, все ключи в правом поддереве строго больше $x.key$. Базовые операции выполняются за время $O(h)$, где $h$ — высота дерева. В случае несбалансированного дерева высота может достигать $n - 1$ (например, при последовательной вставке отсортированных ключей), что сводит поиск к линейному просмотру.

Самобалансирующееся дерево поиска поддерживает инвариант $h = O(\log n)$ при любой последовательности операций, обеспечивая гарантированную логарифмическую сложность. В рамках данной работы реализованы две такие структуры: `AvlDictionary<TKey, TValue>` и `RedBlackTree<TKey, TValue>`.

Внутреннее представление обеих структур основано на явном хранении дерева: каждый узел содержит пару $(key, value)$, указатели на потомков, а также поле балансировки. В `AvlDictionary` поле балансировки — это `unsigned char height`; в `RedBlackTree` — однобитовый флаг `bool color` и дополнительный указатель на родителя. Обе структуры реализуют итератор обхода in-order через явный стек без рекурсии.

---

### 3.2 AVL-дерево

AVL-дерево (Adelson-Velsky & Landis, 1962) — первое самобалансирующееся BST. Структура поддерживает следующий инвариант:

> **AVL-инвариант.** Для каждого узла $x$ фактор баланса $bf(x) = h(\text{left}(x)) - h(\text{right}(x))$ удовлетворяет $bf(x) \in \{-1, 0, 1\}$, где $h(\cdot)$ — высота поддерева в теоретической конвенции: высота пустого поддерева равна $-1$, одиночного узла — $0$.

**Теорема (высота AVL-дерева).** Высота AVL-дерева из $n \ge 1$ узлов удовлетворяет $h \le 1{,}44 \cdot \log_2 n$.

**Доказательство.** Пусть $N(h)$ — минимальное число узлов в AVL-дереве высоты $h$. Самое «тощее» AVL-дерево высоты $h$ имеет корень и два поддерева высот $h-1$ и $h-2$ (разница высот ровно 1, оба поддерева тоже минимальные). Отсюда рекуррентность:

$$N(0) = 1,\quad N(1) = 2,\quad N(h) = N(h-1) + N(h-2) + 1, \quad h \ge 2$$

Введём $M(h) = N(h) + 1$. Тогда $M(0) = 2,\; M(1) = 3,\; M(h) = M(h-1) + M(h-2)$ — рекуррентность чисел Фибоначчи. При стандартном соглашении $F(1)=1,\,F(2)=1,\,F(k)=F(k-1)+F(k-2)$ имеем $M(h) = F(h+3)$, откуда:

$$N(h) = F(h+3) - 1$$

Проверка: $N(0)=F(3)-1=2-1=1$ ✓, $N(1)=F(4)-1=3-1=2$ ✓, $N(2)=F(5)-1=5-1=4$ ✓.

По формуле Бине $F(k) = \dfrac{\varphi^k - \psi^k}{\sqrt{5}}$, где $\varphi = \dfrac{1+\sqrt{5}}{2} \approx 1{,}618$, $\psi = \dfrac{1-\sqrt{5}}{2} \approx -0{,}618$. Поскольку $|\psi| < 1$, при больших $k$: $F(k) > \dfrac{\varphi^k}{\sqrt{5}} - 1$.

Из $n \ge N(h) = F(h+3) - 1 > \dfrac{\varphi^{h+3}}{\sqrt{5}} - 2$ следует $n + 2 > \dfrac{\varphi^{h+3}}{\sqrt{5}}$, откуда:

$$h + 3 < \log_\varphi\!\bigl(\sqrt{5}\,(n+2)\bigr) = \frac{\log_2\!\bigl(\sqrt{5}\,(n+2)\bigr)}{\log_2 \varphi}$$

Так как $\log_2 \varphi \approx 0{,}6942$ и $\log_2\sqrt{5} \approx 1{,}161$:

$$h < \frac{\log_2 n + O(1)}{0{,}6942} - 3 \approx 1{,}4404\cdot\log_2 n + O(1)$$

При $n \ge 1$ константный член отрицателен, поэтому $h \le 1{,}44 \cdot \log_2 n$, то есть $h = O(\log n)$. $\square$

Более точная запись верхней границы:

$$h < 1{,}4404 \cdot \log_2(n+1) - 1{,}328$$

Реализован класс `AvlDictionary<TKey, TValue>` на основе узлов `detail::AvlNode<TKey, TValue>`. Каждый узел хранит `std::pair<TKey, TValue> data`, `AvlNode* left`, `AvlNode* right`, `unsigned char height`. Метод `_height` возвращает высоту поддерева с корнем в данном узле и используется всеми операциями балансировки.

**Процедура 3.2.1. Height**
```basic
01: HeightImpl(node)
02:     if node = NIL then
03:         return 0
04:     return node.height
```

- **Входные данные:** `node` — узел дерева или NIL.

**Шаги алгоритма:**
1. Если узел равен NIL — возвращается 0.
2. Иначе — возвращается сохранённое поле `node.height`.

**Анализ сложности:**

Алгоритм выполняет одну проверку и одно чтение поля — константное число операций, не зависящее от $n$:

$$T(n) = c = \text{const}$$

$$\exists\, c_1, c_2 > 0,\ \exists\, n_0 = 1:\ \forall n \ge n_0 \quad c_1 \le c \le c_2 \implies T(n) \in \Theta(1)$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

Метод `_fixHeight` пересчитывает поле `height` узла на основе высот его потомков. Вызывается на обратном пути рекурсии после каждой вставки, удаления и ротации.

**Процедура 3.2.2. Fix Height**
```basic
01: FixHeightImpl(node)
02:     hl = HeightImpl(node.left)
03:     hr = HeightImpl(node.right)
04:     node.height = max(hl, hr) + 1
```

- **Входные данные:** `node` — узел, высоту которого требуется пересчитать.

**Шаги алгоритма:**
1. Получаются высоты левого и правого поддеревьев — по $\Theta(1)$ каждая (процедура 3.2.1).
2. Поле `height` обновляется как $\max(h_l, h_r) + 1$.

**Анализ сложности:**

По процедуре 3.2.1: $T_{\text{HeightLeft}} = c_1,\; T_{\text{HeightRight}} = c_2$; операции $\max$ и присваивания — $c_3,\, c_4 = \text{const}$:

$$T(n) = c_1 + c_2 + c_3 + c_4 = c = \text{const}$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

Ротации — структурные преобразования, сохраняющие BST-инвариант и восстанавливающие AVL-инвариант. Каждая одиночная ротация переставляет три указателя и дважды вызывает `FixHeight`. Двойные ротации (LR, RL) составляются из двух одиночных.

**Процедура 3.2.3. Rotate Right**
```basic
01: RotateRightImpl(node)
02:     newNode = node.left
03:     node.left = newNode.right
04:     newNode.right = node
05:     FixHeightImpl(node)
06:     FixHeightImpl(newNode)
07:     return newNode
```

- **Входные данные:** `node` — корень разбалансированного поддерева ($bf(node) = 2$).

**Шаги алгоритма:**
1. `newNode` ← левый потомок `node`; становится новым корнем поддерева.
2. Правое поддерево `newNode` становится левым поддеревом `node`.
3. `node` становится правым потомком `newNode`.
4. Пересчитываются высоты сначала `node` (теперь ниже), затем `newNode` (новый корень).

**Корректность.** BST-инвариант сохраняется: если до ротации все ключи в левом поддереве `newNode` меньше `newNode.key`, ключи в правом поддереве `newNode` находятся между `newNode.key` и `node.key`, а ключи правого поддерева `node` больше `node.key`, то после ротации все эти отношения остаются в силе.

**Анализ сложности:**

Три присваивания указателей и два вызова `FixHeight` — все $\Theta(1)$ (процедура 3.2.2):

$$T(n) = 3 \cdot c_{\text{assign}} + 2 \cdot c_{\text{FixHeight}} = c = \text{const}$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

**Процедура 3.2.4. Rotate Left**
```basic
01: RotateLeftImpl(node)
02:     newNode = node.right
03:     node.right = newNode.left
04:     newNode.left = node
05:     FixHeightImpl(node)
06:     FixHeightImpl(newNode)
07:     return newNode
```

- **Входные данные:** `node` — корень разбалансированного поддерева ($bf(node) = -2$).

**Шаги алгоритма:** симметричны процедуре 3.2.3.

**Анализ сложности:**

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

Метод `_balance` пересчитывает высоту узла и при нарушении AVL-инварианта выполняет необходимую ротацию. Вызывается на каждом узле обратного пути рекурсии после вставки и удаления.

**Процедура 3.2.5. Balance**
```basic
01: BalanceImpl(node)
02:     FixHeightImpl(node)
03:     bf = HeightImpl(node.left) - HeightImpl(node.right)
04:
05:     if bf = 2 then
06:         if HeightImpl(node.left.left) < HeightImpl(node.left.right) then
07:             node.left = RotateLeftImpl(node.left)     // LR: двойной поворот
08:         return RotateRightImpl(node)                  // LL: одиночный поворот
09:
10:     if bf = -2 then
11:         if HeightImpl(node.right.right) < HeightImpl(node.right.left) then
12:             node.right = RotateRightImpl(node.right)  // RL: двойной поворот
13:         return RotateLeftImpl(node)                   // RR: одиночный поворот
14:
15:     return node
```

- **Входные данные:** `node` — узел, в котором после вставки или удаления мог нарушиться AVL-инвариант.

**Шаги алгоритма:**
1. Пересчитывается высота `node` через `FixHeight` ($\Theta(1)$).
2. Вычисляется $bf = h(\text{left}) - h(\text{right})$.
3. При $bf = 2$: если это случай LR — левая ротация левого потомка, затем правая ротация `node`. Иначе (LL) — только правая ротация `node`.
4. При $bf = -2$: симметрично — случаи RL и RR.
5. При $|bf| \le 1$ — узел возвращается без изменений.

**Замечание.** По конструкции алгоритмов вставки и удаления $|bf|$ не может превышать 2 на момент вызова `Balance`.

**Анализ сложности:**

`FixHeight` — $\Theta(1)$; вычисление $bf$ — два вызова `HeightImpl`, каждый $\Theta(1)$; каждая ротация — $\Theta(1)$ (процедуры 3.2.3–3.2.4); максимум две ротации:

$$T(n) = \Theta(1) + \Theta(1) + 2 \cdot \Theta(1) = \Theta(1)$$

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

Поиск реализован методами `ContainsKey`, `GetValue` и `TryGetValue`. Все три опираются на вспомогательный метод `_find`, выполняющий рекурсивный спуск по BST-инварианту.

**Процедура 3.2.6. Find**
```basic
01: FindImpl(node, key)
02:     if node = NIL then
03:         return NIL
04:     if node.key > key then
05:         return FindImpl(node.left, key)
06:     else if node.key < key then
07:         return FindImpl(node.right, key)
08:     else
09:         return node
```

- **Входные данные:** `node` — текущий узел; `key` — искомый ключ.

**Шаги алгоритма:**
1. Если узел NIL — ключ отсутствует, возвращается NIL.
2. Если $key < node.key$ — рекурсивный спуск в левое поддерево.
3. Если $key > node.key$ — рекурсивный спуск в правое поддерево.
4. Если $key = node.key$ — узел найден, возвращается.

**Анализ сложности:**

Пусть $n$ — число элементов в дереве, $h$ — его высота. На каждом уровне рекурсии выполняется $\Theta(1)$ работы. Глубина рекурсии не превышает $h$.

*Худший случай* — ключ отсутствует или находится на листе максимальной глубины; рекурсия проходит все $h$ уровней:

$$W(n) = \Theta(h)$$

По теореме о высоте AVL-дерева $h \le 1{,}44 \cdot \log_2 n$, поэтому $h = \Theta(\log n)$:

$$W(n) = \Theta(\log n)$$

*Лучший случай* — ключ совпадает с ключом корня:

$$B(n) = \Theta(1)$$

*Амортизированный случай* — операция read-only, каждый вызов независим и выполняется за $O(h)$; отложенной стоимости нет:

$$A(n) = W(n) = \Theta(\log n)$$

---

**Процедура 3.2.7. Contains Key**
```basic
01: ContainsKeyImpl(key)
02:     return FindImpl(Root, key) ≠ NIL
```

- **Входные данные:** `key` — ключ, наличие которого проверяется.

**Шаги алгоритма:**
1. Вызывается `FindImpl(Root, key)`.
2. Если результат не NIL — возвращается `true`, иначе — `false`.

**Анализ сложности:**

Стоимость определяется вызовом `FindImpl`:

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

**Процедура 3.2.8. Try Get Value**
```basic
01: TryGetValueImpl(key, value)
02:     node = FindImpl(Root, key)
03:     if node = NIL then
04:         return false
05:     value = node.data.second
06:     return true
```

- **Входные данные:** `key` — искомый ключ; `value` — выходной параметр.

**Шаги алгоритма:**
1. Вызывается `FindImpl(Root, key)`.
2. Если узел не найден — возвращается `false`.
3. Найденное значение записывается в `value`, возвращается `true`.

**Анализ сложности:**

Стоимость определяется вызовом `FindImpl`; все остальные операции — $\Theta(1)$:

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

**Процедура 3.2.9. Get Value**
```basic
01: GetValueImpl(key)
02:     node = FindImpl(Root, key)
03:     if node = NIL then
04:         raise KeyNotFoundException
05:     return node.data.second
```

- **Входные данные:** `key` — искомый ключ.

**Шаги алгоритма:**
1. Вызывается `FindImpl(Root, key)`.
2. Если узел не найден — генерируется исключение `KeyNotFoundException`.
3. Возвращается ссылка на значение.

**Анализ сложности:**

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

Вставка реализована методами `Add` и `InsertOrAssign`. Оба опираются на вспомогательный метод `_insert`, выполняющий рекурсивный спуск и вызов `Balance` на обратном пути.

**Процедура 3.2.10. Insert**
```basic
01: InsertImpl(node, key, value, inserted)
02:     if node = NIL then
03:         inserted = true
04:         return new Node{key, value, height = 1}
05:     if key < node.key then
06:         node.left = InsertImpl(node.left, key, value, inserted)
07:     else if key > node.key then
08:         node.right = InsertImpl(node.right, key, value, inserted)
09:     return BalanceImpl(node)
```

- **Входные данные:** `node` — текущий узел; `key`, `value` — вставляемая пара; `inserted` — выходной флаг.

**Шаги алгоритма:**
1. Если узел NIL — создаётся новый лист высотой 1, `inserted = true`.
2. По BST-инварианту выполняется рекурсивный спуск влево или вправо.
3. На обратном пути каждый узел балансируется через `BalanceImpl` ($\Theta(1)$, процедура 3.2.5).

**Анализ сложности:**

Пусть $d$ — глубина позиции вставки ($0 \le d \le h$). Спуск совершает ровно $d$ рекурсивных вызовов; обратный путь — ещё $d$ вызовов `BalanceImpl`. Работа на каждом уровне — $\Theta(1)$:

$$T(n) = d \cdot \Theta(1) + d \cdot \Theta(1) = \Theta(d)$$

*Худший случай* — позиция вставки на максимальной глубине $d = h$:

$$W(n) = \Theta(h) = \Theta(\log n)$$

*Лучший случай* — дерево пусто ($n = 0$, $d = 0$): создаётся корень за константное время:

$$B(n) = \Theta(1)$$

*Амортизированный случай* — каждая вставка независима, отложенной стоимости нет:

$$A(n) = \Theta(\log n)$$

---

**Процедура 3.2.11. Add**
```basic
01: AddImpl(key, value)
02:     if FindImpl(Root, key) ≠ NIL then
03:         raise DuplicateKeyException
04:     inserted = false
05:     Root = InsertImpl(Root, key, value, inserted)
06:     if inserted then
07:         Count = Count + 1
```

- **Входные данные:** `key` — ключ; `value` — значение.

**Шаги алгоритма:**
1. Проверяется отсутствие ключа через `FindImpl` — $W = \Theta(\log n)$.
2. Если ключ существует — генерируется `DuplicateKeyException`.
3. Иначе вызывается `InsertImpl`; при успехе счётчик увеличивается.

**Анализ сложности:**

`FindImpl` и `InsertImpl` выполняются последовательно, оба $W = \Theta(\log n)$:

$$W(n) = \Theta(\log n) + \Theta(\log n) = \Theta(\log n)$$

$$B(n) = \Theta(1) \quad \text{(пустое дерево: оба вызова завершаются за } \Theta(1)\text{)}$$

$$A(n) = \Theta(\log n)$$

---

**Процедура 3.2.12. Insert Or Assign**
```basic
01: InsertOrAssignImpl(key, value)
02:     node = FindImpl(Root, key)
03:     if node ≠ NIL then
04:         node.data.second = value
05:         return
06:     inserted = false
07:     Root = InsertImpl(Root, key, value, inserted)
08:     if inserted then
09:         Count = Count + 1
```

- **Входные данные:** `key` — ключ; `value` — значение.

**Шаги алгоритма:**
1. Ищется узел через `FindImpl`.
2. Если узел найден — значение обновляется на месте, $\Theta(1)$ после `FindImpl`.
3. Если не найден — вызывается `InsertImpl`.

**Анализ сложности:**

В обоих ветвях доминирует `FindImpl` или `InsertImpl`, оба $W = \Theta(\log n)$:

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

Удаление реализовано методом `Remove`, опирающимся на вспомогательные процедуры `_findMin`, `_removeMin` и `_remove`.

**Процедура 3.2.13. Remove**
```basic
01: RemoveImpl(node, key, removed)
02:     if node = NIL then
03:         removed = false
04:         return NIL
05:
06:     if key < node.key then
07:         node.left = RemoveImpl(node.left, key, removed)
08:     else if key > node.key then
09:         node.right = RemoveImpl(node.right, key, removed)
10:     else
11:         removed = true
12:         left = node.left
13:         right = node.right
14:         delete node
15:
16:         if right = NIL then
17:             return left
18:
19:         min = FindMinImpl(right)
20:         min.right = RemoveMinImpl(right)
21:         min.left = left
22:         return BalanceImpl(min)
23:
24:     return BalanceImpl(node)
```

- **Входные данные:** `node` — текущий узел; `key` — удаляемый ключ; `removed` — выходной флаг.

**Шаги алгоритма:**
1. Если узел NIL — ключ не найден, `removed = false`.
2. Рекурсивный спуск влево или вправо по BST-инварианту.
3. При нахождении узла: сохраняются потомки, узел освобождается. Если правое поддерево пусто — возвращается левое. Иначе — минимальный узел правого поддерева (`FindMin`) становится новым корнем: его правый потомок заменяется правым поддеревом без минимума (`RemoveMin`), левый — сохранённым левым поддеревом; результат балансируется.
4. На обратном пути каждый пройденный узел балансируется через `BalanceImpl`. В отличие от вставки, дисбаланс может распространяться вверх по всему пути до корня.

**Анализ сложности:**

`FindMinImpl` спускается по левым потомкам: $O(h)$ в худшем случае, $\Theta(1)$ в лучшем. `RemoveMinImpl` — аналогично $O(h)$. Спуск `RemoveImpl` — $d$ уровней ($d \le h$); обратный путь — $d$ вызовов `BalanceImpl`, каждый $\Theta(1)$:

$$T(n) = \Theta(d) + O(h) = O(h) = O(\log n)$$

*Худший случай* — удаляемый узел на максимальной глубине, правое поддерево непусто:

$$W(n) = \Theta(\log n)$$

*Лучший случай* — ключ отсутствует (первый же вызов возвращает NIL за $\Theta(1)$):

$$B(n) = \Theta(1)$$

*Амортизированный случай:*

$$A(n) = \Theta(\log n)$$

---

**Процедура 3.2.14. Clear**
```basic
01: ClearImpl(node)
02:     if node = NIL then return
03:     ClearImpl(node.left)
04:     ClearImpl(node.right)
05:     delete node
06:
07: ClearImpl(Root)
08: Root = NIL
09: Count = 0
```

- **Входные данные:** нет.

**Шаги алгоритма:**
1. Рекурсивный post-order обход — уничтожение каждого узла.
2. Корень обнуляется, счётчик сбрасывается в 0.

**Анализ сложности:**

Каждый из $n$ узлов посещается ровно один раз; на каждом — $\Theta(1)$ работы:

$$W(n) = B(n) = A(n) = \Theta(n)$$

---

**Процедура 3.2.15. Count**
```basic
01: CountImpl()
02:     return Count
```

**Анализ сложности:**

Возвращается значение кэшированного счётчика:

$$W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 3.2.16. Height**
```basic
01: HeightImpl()
02:     if Root = NIL then return 0
03:     return Root.height
```

**Анализ сложности:**

Читается поле `Root.height`, которое поддерживается актуальным процедурой `FixHeight` после каждой модификации:

$$W(n) = B(n) = A(n) = \Theta(1)$$

---

**Сводная таблица сложности процедур раздела 3.2 (AVL-дерево)**

| Процедура                  | $W(n)$ | $B(n)$ | $A(n)$ |
|----------------------------|:---:|:---:|:---:|
| 3.2.1. Height              | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |
| 3.2.2. Fix Height          | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |
| 3.2.3. Rotate Right        | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |
| 3.2.4. Rotate Left         | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |
| 3.2.5. Balance             | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |
| 3.2.6. Find                | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.7. Contains Key        | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.8. Try Get Value       | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.9. Get Value           | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.10. Insert             | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.11. Add                | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.12. Insert Or Assign   | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.13. Remove             | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.2.14. Clear              | $\Theta(n)$ | $\Theta(n)$ | $\Theta(n)$ |
| 3.2.15. Count              | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |
| 3.2.16. Height (публичный) | $\Theta(1)$ | $\Theta(1)$ | $\Theta(1)$ |

> Здесь $n$ — количество элементов в дереве. AVL-инвариант ($|bf| \le 1$) гарантирует $h \le 1{,}44 \cdot \log_2 n$, что обеспечивает $\Theta(\log n)$ для всех операций спуска. Каждый вызов `Balance` выполняет не более двух ротаций ($\Theta(1)$). При удалении дисбаланс может распространяться вверх по всему пути: `Balance` вызывается $O(\log n)$ раз, суммарная стоимость остаётся $\Theta(\log n)$.

---

### 3.3 Красно-чёрное дерево

Красно-чёрное дерево (Red-Black Tree, RBT) — самобалансирующееся BST, поддерживающее баланс через систему окраски узлов. Структура поддерживает следующие пять инвариантов:

> **RBT-инварианты (CLRS, гл. 13).**
> 1. Каждый узел окрашен в красный или чёрный цвет.
> 2. Корень — чёрный.
> 3. Все листья (sentinel `_nil`) — чёрные.
> 4. Потомки красного узла — чёрные (нет двух последовательных красных узлов на пути).
> 5. Все пути от данного узла до листьев содержат одинаковое количество чёрных узлов (чёрная высота $bh$).

**Теорема (высота красно-чёрного дерева).** Высота RBT из $n \ge 1$ внутренних узлов удовлетворяет $h \le 2 \cdot \log_2(n + 1)$.

**Доказательство.** Определим чёрную высоту $bh(v)$ узла $v$ как число чёрных узлов на любом пути от $v$ до листа, не считая сам $v$ (из инварианта 5 это значение одинаково для всех путей).

**Лемма.** Поддерево с корнем $v$ содержит не менее $2^{bh(v)} - 1$ внутренних узлов.

*Доказательство индукцией по высоте поддерева $v$.*

База ($h_v = 0$): $v$ — лист `_nil`, $bh(v) = 0$, число узлов $= 0 = 2^0 - 1$. ✓

Шаг: пусть $v$ — внутренний узел с потомками $l$ и $r$. Каждый из них имеет $bh \ge bh(v) - 1$: если потомок чёрный, его $bh = bh(v) - 1$; если красный — его $bh = bh(v)$ (сам не считается), но тогда все его потомки должны быть чёрными (инвариант 4), значит $bh(l), bh(r) \ge bh(v) - 1$ в любом случае. По индукционному предположению каждое поддерево содержит $\ge 2^{bh(v)-1} - 1$ узлов. Суммарно с корнем $v$:

$$2(2^{bh(v)-1} - 1) + 1 = 2^{bh(v)} - 1 \quad \checkmark$$

Применяем лемму к корню: $n \ge 2^{bh(\text{root})} - 1$.

Из инварианта 4: на любом пути от корня до листа нет двух подряд идущих красных узлов, поэтому не менее половины узлов пути (длиной $h$) — чёрные, откуда $bh(\text{root}) \ge \lceil h/2 \rceil \ge h/2$.

Подставляем: $n \ge 2^{h/2} - 1$, то есть $n + 1 \ge 2^{h/2}$. Логарифмируя:

$$\frac{h}{2} \le \log_2(n+1) \implies h \le 2\log_2(n+1) \quad \square$$

Реализован класс `RedBlackTree<TKey, TValue>` на основе узлов `detail::RedBlackNode<TKey, TValue>`. Каждый узел хранит `std::pair<TKey, TValue> data`, `RedBlackNode* parent`, `RedBlackNode* left`, `RedBlackNode* right`, `bool color` (`true` = красный, `false` = чёрный). Используется единый sentinel-узел `_nil` (чёрный) вместо `nullptr`: это упрощает граничные случаи в ротациях и `_fixRemove`.

**Процедура 3.3.1. Rotate Left**
```basic
01: RotateLeftImpl(x)
02:     y = x.right
03:     x.right = y.left
04:     if y.left ≠ _nil then
05:         y.left.parent = x
06:     y.parent = x.parent
07:     if x.parent = NIL then
08:         Root = y
09:     else if x = x.parent.left then
10:         x.parent.left = y
11:     else
12:         x.parent.right = y
13:     y.left = x
14:     x.parent = y
15:     return y
```

- **Входные данные:** `x` — узел, вокруг которого выполняется левая ротация.

**Шаги алгоритма:**
1. `y` — правый потомок `x`; становится новым корнем поддерева.
2. Левое поддерево `y` становится правым поддеревом `x`; если оно не `_nil`, обновляется его `parent`.
3. `y.parent` устанавливается равным родителю `x`; родитель перенаправляется на `y`.
4. `x` становится левым потомком `y`.

**Анализ сложности:**

Все операции — присваивания указателей и сравнения, каждая $\Theta(1)$:

$$T(n) = c = \text{const} \implies W(n) = B(n) = A(n) = \Theta(1)$$

---

**Процедура 3.3.2. Rotate Right**
```basic
01: RotateRightImpl(y)
02:     x = y.left
03:     y.left = x.right
04:     if x.right ≠ _nil then
05:         x.right.parent = y
06:     x.parent = y.parent
07:     if y.parent = NIL then
08:         Root = x
09:     else if y = y.parent.right then
10:         y.parent.right = x
11:     else
12:         y.parent.left = x
13:     x.right = y
14:     y.parent = x
15:     return x
```

- **Входные данные:** `y` — узел, вокруг которого выполняется правая ротация.

**Шаги алгоритма:** симметричны процедуре 3.3.1.

**Анализ сложности:**

$$\Rightarrow W(n) = B(n) = A(n) = \Theta(1)$$

---

Поиск реализован вспомогательным методом `_find`, выполняющим рекурсивный спуск по BST-инварианту с проверкой против sentinel `_nil`.

**Процедура 3.3.3. Find**
```basic
01: FindImpl(node, key)
02:     if node = _nil then
03:         return NIL
04:     if key < node.key then
05:         return FindImpl(node.left, key)
06:     else if key > node.key then
07:         return FindImpl(node.right, key)
08:     else
09:         return node
```

- **Входные данные:** `node` — текущий узел; `key` — искомый ключ.

**Шаги алгоритма:**
1. Если узел равен `_nil` — ключ отсутствует, возвращается NIL.
2. Рекурсивный спуск влево или вправо по BST-инварианту.
3. При совпадении ключа — возвращается узел.

**Анализ сложности:**

Глубина рекурсии не превышает $h$; на каждом уровне — $\Theta(1)$ работы.

*Худший случай* — ключ отсутствует или находится на максимальной глубине:

$$W(n) = \Theta(h) = \Theta(\log n)$$

поскольку $h \le 2\log_2(n+1)$, значит $h = \Theta(\log n)$.

*Лучший случай* — ключ совпадает с ключом корня:

$$B(n) = \Theta(1)$$

*Амортизированный случай* — read-only операция, отложенной стоимости нет:

$$A(n) = W(n) = \Theta(\log n)$$

---

**Процедура 3.3.4. Contains Key**
```basic
01: ContainsKeyImpl(key)
02:     return FindImpl(Root, key) ≠ NIL
```

**Анализ сложности:**

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

**Процедура 3.3.5. Try Get Value**
```basic
01: TryGetValueImpl(key, value)
02:     node = FindImpl(Root, key)
03:     if node = NIL then return false
04:     value = node.data.second
05:     return true
```

**Анализ сложности:**

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

**Процедура 3.3.6. Get Value**
```basic
01: GetValueImpl(key)
02:     node = FindImpl(Root, key)
03:     if node = NIL then raise KeyNotFoundException
04:     return node.data.second
```

**Анализ сложности:**

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

Вставка реализована методами `Add` и `InsertOrAssign`, опирающимися на `_bstInsert` и `_fixInsert`.

**Процедура 3.3.7. BST Insert**
```basic
01: BstInsertImpl(key, value)
02:     z = new Node{key, value, color = RED, left = _nil, right = _nil, parent = NIL}
03:     y = NIL; x = Root
04:     while x ≠ _nil do
05:         y = x
06:         if z.key < x.key then x = x.left
07:         else x = x.right
08:     z.parent = y
09:     if y = NIL then Root = z
10:     else if z.key < y.key then y.left = z
11:     else y.right = z
12:     if z.parent = NIL then z.color = BLACK; return   // дерево было пустым
13:     if z.parent.color = BLACK then return            // инварианты не нарушены
14:     FixInsertImpl(z)
```

- **Входные данные:** `key` — ключ; `value` — значение.

**Шаги алгоритма:**
1. Создаётся красный узел `z` с потомками `_nil`.
2. Итеративный спуск находит позицию вставки; `y` — будущий родитель `z`.
3. `z` прикрепляется к `y` как левый или правый потомок.
4. Если дерево было пустым — `z` становится чёрным корнем (инвариант 2); возврат.
5. Если родитель чёрный — инварианты не нарушены; возврат.
6. Если родитель красный — нарушен инвариант 4; запускается `FixInsertImpl`.

**Анализ сложности:**

Итеративный спуск — ровно $d$ итераций ($d \le h$), $\Theta(1)$ на итерацию:

$$T_{\text{спуск}}(n) = \Theta(d) \le \Theta(h) = \Theta(\log n)$$

*Худший случай:* $W(n) = \Theta(\log n)$

*Лучший случай* — дерево пусто ($d = 0$): создание и установка корня — $\Theta(1)$: $B(n) = \Theta(1)$

---

**Процедура 3.3.8. Fix Insert**
```basic
01: FixInsertImpl(z)
02:     while z.parent ≠ NIL and z.parent.color = RED do
03:         if z.parent = z.parent.parent.left then
04:             uncle = z.parent.parent.right
05:             if uncle.color = RED then              // Случай 1: перекраска
06:                 z.parent.color = BLACK
07:                 uncle.color = BLACK
08:                 z.parent.parent.color = RED
09:                 z = z.parent.parent
10:             else
11:                 if z = z.parent.right then         // Случай 2: LR → LL
12:                     z = z.parent
13:                     RotateLeftImpl(z)
14:                 z.parent.color = BLACK             // Случай 3: одна ротация
15:                 z.parent.parent.color = RED
16:                 RotateRightImpl(z.parent.parent)
17:         // симметричный блок для правого поддерева
18:     Root.color = BLACK
```

- **Входные данные:** `z` — вставленный красный узел с красным родителем.

**Шаги алгоритма:**
1. Случай 1 (дядя красный): перекрашиваются родитель и дядя в чёрный, дед — в красный; `z` поднимается к деду. Структура не меняется. Цикл продолжается.
2. Случай 2 (дядя чёрный, `z` — внутренний потомок): одна ротация переводит в случай 3.
3. Случай 3 (дядя чёрный, `z` — внешний потомок): перекраска родителя и деда, одна ротация деда. Цикл завершается.

**Анализ сложности.**

Случай 1 поднимает `z` на 2 уровня вверх за $\Theta(1)$ перекрасок. Так как $h \le 2\log_2(n+1)$, число повторений случая 1 не превышает $\lfloor h/2 \rfloor \le \log_2(n+1)$. Случаи 2 и 3 выполняют суммарно не более 2 ротаций и завершают цикл:

$$T_{\text{FixInsert}}(n) \le \log_2(n+1) \cdot \Theta(1) + 2 \cdot \Theta(1) = \Theta(\log n)$$

*Худший случай* — случай 1 повторяется $\lfloor h/2 \rfloor$ раз: $W(n) = \Theta(\log n)$

*Лучший случай* — сразу случай 3 (одна итерация): $B(n) = \Theta(1)$

*Амортизированный случай:* $A(n) = \Theta(\log n)$

---

**Процедура 3.3.9. Add**
```basic
01: AddImpl(key, value)
02:     if FindImpl(Root, key) ≠ NIL then
03:         raise DuplicateKeyException
04:     BstInsertImpl(key, value)
05:     Count = Count + 1
```

**Анализ сложности:**

`FindImpl` и `BstInsertImpl` (включая `FixInsertImpl`) последовательны, оба $W = \Theta(\log n)$:

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

**Процедура 3.3.10. Insert Or Assign**
```basic
01: InsertOrAssignImpl(key, value)
02:     node = FindImpl(Root, key)
03:     if node ≠ NIL then
04:         node.data.second = value
05:         return
06:     BstInsertImpl(key, value)
07:     Count = Count + 1
```

**Анализ сложности:**

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

Удаление реализовано методом `Remove`, опирающимся на `_bstRemove` и `_fixRemove`.

**Процедура 3.3.11. BST Remove**
```basic
01: BstRemoveImpl(key)
02:     z = FindImpl(Root, key)
03:     if z = NIL then return false
04:
05:     y = z; yOriginalColor = y.color; x = NIL
06:     if z.left = _nil then
07:         x = z.right; TransplantImpl(z, z.right)
08:     else if z.right = _nil then
09:         x = z.left; TransplantImpl(z, z.left)
10:     else
11:         y = MinimumImpl(z.right); yOriginalColor = y.color; x = y.right
12:         if y.parent = z then x.parent = y
13:         else
14:             TransplantImpl(y, y.right)
15:             y.right = z.right; y.right.parent = y
16:         TransplantImpl(z, y)
17:         y.left = z.left; y.left.parent = y; y.color = z.color
18:
19:     delete z
20:     if yOriginalColor = BLACK then
21:         FixRemoveImpl(x)
22:     return true
```

- **Входные данные:** `key` — удаляемый ключ.

**Шаги алгоритма:**
1. Ищется узел `z` через `FindImpl`. Если не найден — возвращается `false`.
2. Если `z` имеет не более одного непустого потомка — `Transplant` заменяет `z` этим потомком.
3. Если `z` имеет двух непустых потомков — `y` = минимум правого поддерева (`MinimumImpl`). `y` пересаживается на место `z` с сохранением цвета `z`.
4. Если исходный цвет `y` был чёрным — чёрная высота нарушена; запускается `FixRemoveImpl(x)`.

**Анализ сложности:**

`FindImpl` — $O(\log n)$; `MinimumImpl` — $O(h) = O(\log n)$ в худшем, $\Theta(1)$ в лучшем; `Transplant` — $\Theta(1)$:

*Худший случай:* $W(n) = \Theta(\log n)$

*Лучший случай* — ключ отсутствует, `FindImpl` завершается за $\Theta(1)$ (пустое дерево): $B(n) = \Theta(1)$

---

**Процедура 3.3.12. Fix Remove**
```basic
01: FixRemoveImpl(x)
02:     while x ≠ Root and x.color = BLACK do
03:         if x = x.parent.left then
04:             s = x.parent.right
05:             if s.color = RED then                          // Случай 1
06:                 s.color = BLACK; x.parent.color = RED
07:                 RotateLeftImpl(x.parent); s = x.parent.right
08:             if s.left.color = BLACK and s.right.color = BLACK then  // Случай 2
09:                 s.color = RED; x = x.parent
10:             else
11:                 if s.right.color = BLACK then              // Случай 3
12:                     s.left.color = BLACK; s.color = RED
13:                     RotateRightImpl(s); s = x.parent.right
14:                 s.color = x.parent.color                  // Случай 4
15:                 x.parent.color = BLACK; s.right.color = BLACK
16:                 RotateLeftImpl(x.parent); x = Root
17:         // симметричный блок для правого поддерева
18:     x.color = BLACK
```

- **Входные данные:** `x` — узел, принявший «лишний чёрный» (double-black) после удаления чёрного узла.

**Шаги алгоритма:**
1. Случай 1 (брат `s` красный): перекраска и ротация переводят к чёрному брату (случай 2, 3 или 4). Цикл не завершается.
2. Случай 2 (оба потомка `s` чёрные): перекраска `s` в красный, `x` поднимается к родителю. Цикл продолжается.
3. Случай 3 (правый потомок `s` чёрный, левый красный): перекраска и ротация переводят в случай 4. Цикл не завершается.
4. Случай 4 (правый потомок `s` красный): перекраска и ротация разрешают double-black; `x = Root`, цикл завершается.

**Анализ сложности.**

Случай 2 — единственный, повторяющий цикл без ротации: поднимает `x` на 1 уровень вверх. Максимальное число повторений — $h$. Случаи 1, 3, 4 выполняют суммарно не более 3 ротаций и завершают цикл:

$$T_{\text{FixRemove}}(n) \le h \cdot \Theta(1) + 3 \cdot \Theta(1) = \Theta(h) = \Theta(\log n)$$

*Худший случай* — случай 2 повторяется $h$ раз: $W(n) = \Theta(\log n)$

*Лучший случай* — сразу случай 4: $B(n) = \Theta(1)$

*Амортизированный случай:* $A(n) = \Theta(\log n)$

---

**Процедура 3.3.13. Remove**
```basic
01: RemoveImpl(key)
02:     removed = BstRemoveImpl(key)
03:     if removed then Count = Count - 1
04:     return removed
```

**Анализ сложности:**

$$W(n) = \Theta(\log n), \quad B(n) = \Theta(1), \quad A(n) = \Theta(\log n)$$

---

**Процедура 3.3.14. Clear**
```basic
01: ClearImpl(node)
02:     if node = _nil then return
03:     ClearImpl(node.left)
04:     ClearImpl(node.right)
05:     delete node
06:
07: ClearImpl(Root)
08: Root = _nil
09: Count = 0
```

**Анализ сложности:**

Post-order обход посещает каждый из $n$ внутренних узлов ровно один раз; sentinel `_nil` не удаляется:

$$W(n) = B(n) = A(n) = \Theta(n)$$

---

**Процедура 3.3.15. Count**
```basic
01: CountImpl()
02:     return Count
```

**Анализ сложности:**

$$W(n) = B(n) = A(n) = \Theta(1)$$

---

**Сводная таблица сложности процедур раздела 3.3 (красно-чёрное дерево)**

| Процедура                           |      $W(n)$      |   $B(n)$    |      $A(n)$      |
|-------------------------------------|:----------------:|:-----------:|:----------------:|
| 3.3.1. Rotate Left                  |   $\Theta(1)$    | $\Theta(1)$ |   $\Theta(1)$    |
| 3.3.2. Rotate Right                 |   $\Theta(1)$    | $\Theta(1)$ |   $\Theta(1)$    |
| 3.3.3. Find                         | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.4. Contains Key                 | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.5. Try Get Value                | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.6. Get Value                    | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.7. BST Insert                   | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.8. Fix Insert                   | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.9. Add                          | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.10. Insert Or Assign            | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.11. BST Remove                  | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.12. Fix Remove                  | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.13. Remove                      | $\Theta(\log n)$ | $\Theta(1)$ | $\Theta(\log n)$ |
| 3.3.14. Clear                       |   $\Theta(n)$    | $\Theta(n)$ |   $\Theta(n)$    |
| 3.3.15. Count                       |   $\Theta(1)$    | $\Theta(1)$ |   $\Theta(1)$    |
| 3.3.16. Height  |   $\Theta(1)$    | $\Theta(1)$ |   $\Theta(1)$    |

> Здесь $n$ — количество внутренних узлов дерева. RBT-инварианты гарантируют $h \le 2\log_2(n+1)$, что обеспечивает $\Theta(\log n)$ для всех операций спуска. `FixInsert` выполняет не более 2 ротаций; `FixRemove` — не более 3 ротаций. `Height` в текущей реализации выполняет полный обход за $\Theta(n)$.

---

### 3.4 Тестирование производительности

Для оценки практической эффективности разработанных реализаций было проведено сравнительное тестирование производительности. В качестве эталонной реализации использовался стандартный контейнер `std::map` из стандартной библиотеки C++, реализованный на основе красно-чёрного дерева. Каждая тестируемая структура данных сравнивалась с эталоном в двух сценариях.

**Условия тестирования:**
- **Инициализация (Init):** последовательная вставка $N$ пар ключ–значение; фиксируется суммарное время заполнения структуры.
- **Чтение (Read):** серия операций поиска по ключам; фиксируется среднее время одной операции в наносекундах.

---

**Таблица 3.4.1. AvlDictionary vs std::map ($N = 200\,000$, типы ключей: `int`, `long long`, `string`, `BigKey` (64 байта), MinGW GCC 15.2, Windows)**

| Операция | `std::map` | `AvlDictionary` | Преимущество |
|---|:---:|:---:|:---:|
| Инициализация | ~285 мс | ~318 мс | `std::map` в 1,11× быстрее |
| Чтение | ~1 020 нс/оп | ~590 нс/оп | `AvlDictionary` в **1,73×** быстрее |
| ContainsKey | ~1 005 нс/оп | ~605 нс/оп | `AvlDictionary` в **1,66×** быстрее |
| Insert (один элемент) | ~1 097 нс/оп | ~1 367 нс/оп | `std::map` в 1,25× быстрее |
| Итерация | ~165 нс/оп | ~229 нс/оп | `std::map` в 1,39× быстрее |
| Высота дерева | ~31 уровень (эмп.) | ~21 уровень (эмп.) | AVL на ~32% ниже |

---

**Таблица 3.4.2. RedBlackTree vs std::map ($N = 1\,000\,000$)**

| Операция | `std::map` | `RedBlackTree` | Преимущество |
|---|:---:|:---:|:---:|
| Инициализация | ~2 643 мс | ~3 365 мс | `std::map` в 1,27× быстрее |
| Чтение | ~2 380 нс/оп | ~2 219 нс/оп | `RedBlackTree` в **1,07×** быстрее |
---

**Анализ результатов:**

*Инициализация.* Обе реализации уступают `std::map` по скорости вставки. `AvlDictionary` медленнее в 1,11×: при каждой вставке выполняется $\Theta(\log n)$ вызовов `FixHeight` и `Balance` на обратном пути рекурсии. `RedBlackTree` медленнее в 1,27×: `FixInsert` выполняет до $O(\log n)$ перекрасок и до 2 ротаций; дополнительные затраты связаны с обслуживанием `parent`-указателей при каждом связывании.

*Чтение.* `AvlDictionary` превосходит `std::map` в ~1,7×. Эмпирическая высота AVL-дерева (~21 уровень) меньше эмпирической высоты красно-чёрного дерева в `std::map` (~31 уровень), что снижает число сравнений при каждом поиске. `RedBlackTree` превосходит `std::map` в 1,07×: обе реализации — красно-чёрные деревья со схожими высотами; небольшое преимущество объясняется различиями в константных множителях реализаций.

*Итерация.* `AvlDictionary` уступает `std::map` в 1,39×. AVL-итератор использует явный `std::stack` для in-order обхода; накладные расходы на стек превышают затраты `std::map`, использующего `parent`-указатели для перехода к следующему узлу без стека.

*Общий вывод.* `AvlDictionary` предпочтителен для read-heavy нагрузок: более строгий инвариант высоты обеспечивает выигрыш до ~1,7× при поиске. `RedBlackTree` и `std::map` предпочтительны для write-heavy нагрузок: не более 2 ротаций при вставке и не более 3 ротаций при удалении против $O(\log n)$ потенциальных ротаций при удалении в AVL.
