#include "Dictorium/Dictorium.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace dtr;

// ── Test runner ───────────────────────────────────────────────────────────────

static int passed = 0;
static int failed = 0;

#define TEST(name) \
    void name(); \
    struct _Reg_##name { _Reg_##name() { \
        try { name(); \
            std::cout << "  [OK]  " #name "\n"; ++passed; \
        } catch (const std::exception& e) { \
            std::cout << "  [FAIL] " #name " -- " << e.what() << "\n"; ++failed; \
        } catch (...) { \
            std::cout << "  [FAIL] " #name " -- unknown exception\n"; ++failed; \
        } \
    }} _reg_##name; \
    void name()

#define ASSERT(expr) \
    if (!(expr)) throw std::runtime_error("ASSERT failed: " #expr)

// ── Basic operations ──────────────────────────────────────────────────────────

TEST(test_empty_dictionary) {
    AvlDictionary<int, int> d;
    ASSERT(d.Count() == 0);
    ASSERT(d.Height() == 0);
    ASSERT(!d.ContainsKey(1));
}

TEST(test_add_and_contains) {
    AvlDictionary<int, std::string> d;
    d.Add(10, "ten");
    d.Add(5,  "five");
    d.Add(15, "fifteen");
    ASSERT(d.Count() == 3);
    ASSERT(d.ContainsKey(10));
    ASSERT(d.ContainsKey(5));
    ASSERT(d.ContainsKey(15));
    ASSERT(!d.ContainsKey(99));
}

TEST(test_add_duplicate_throws) {
    AvlDictionary<int, int> d;
    d.Add(1, 100);
    try {
        d.Add(1, 200);
        ASSERT(false && "expected exception not thrown");
    } catch (const std::invalid_argument&) {}
    ASSERT(d.Count() == 1);
    ASSERT(d.GetValue(1) == 100);
}

TEST(test_get_value) {
    AvlDictionary<std::string, int> d = {{"a", 1}, {"b", 2}, {"c", 3}};
    ASSERT(d.GetValue("a") == 1);
    ASSERT(d.GetValue("b") == 2);
    ASSERT(d.GetValue("c") == 3);
}

TEST(test_get_value_missing_throws) {
    AvlDictionary<int, int> d;
    d.Add(1, 10);
    try {
        d.GetValue(99);
        ASSERT(false && "expected exception not thrown");
    } catch (const std::out_of_range&) {}
}

TEST(test_try_get_value) {
    AvlDictionary<int, double> d = {{1, 1.1}, {2, 2.2}};
    double v = 0;
    ASSERT(d.TryGetValue(1, v) && v == 1.1);
    ASSERT(d.TryGetValue(2, v) && v == 2.2);
    ASSERT(!d.TryGetValue(99, v));
}

TEST(test_insert_or_assign_update) {
    AvlDictionary<int, std::string> d;
    d.Add(1, "old");
    d.InsertOrAssign(1, "new");
    ASSERT(d.Count() == 1);
    ASSERT(d.GetValue(1) == "new");
}

TEST(test_insert_or_assign_new_key) {
    AvlDictionary<int, int> d;
    d.InsertOrAssign(42, 100);
    ASSERT(d.Count() == 1);
    ASSERT(d.GetValue(42) == 100);
}

TEST(test_remove_existing) {
    AvlDictionary<int, int> d = {{1,1},{2,2},{3,3}};
    ASSERT(d.Remove(2));
    ASSERT(d.Count() == 2);
    ASSERT(!d.ContainsKey(2));
}

TEST(test_remove_missing) {
    AvlDictionary<int, int> d = {{1,1}};
    ASSERT(!d.Remove(99));
    ASSERT(d.Count() == 1);
}

TEST(test_remove_leaf) {
    AvlDictionary<int, int> d = {{5,0},{3,0},{7,0}};
    ASSERT(d.Remove(3));
    ASSERT(!d.ContainsKey(3));
    ASSERT(d.ContainsKey(5));
    ASSERT(d.ContainsKey(7));
}

TEST(test_remove_node_with_two_children) {
    // Removing a node with two children -- replaced by in-order successor
    AvlDictionary<int, int> d = {{5,0},{3,0},{7,0},{6,0},{8,0}};
    ASSERT(d.Remove(5));
    ASSERT(!d.ContainsKey(5));
    ASSERT(d.ContainsKey(3));
    ASSERT(d.ContainsKey(6));
    ASSERT(d.ContainsKey(7));
    ASSERT(d.ContainsKey(8));
}

TEST(test_clear) {
    AvlDictionary<int, int> d = {{1,1},{2,2},{3,3}};
    d.Clear();
    ASSERT(d.Count() == 0);
    ASSERT(d.Height() == 0);
    ASSERT(!d.ContainsKey(1));
    // Re-insertion after clear must work
    d.Add(10, 10);
    ASSERT(d.Count() == 1);
}

TEST(test_initializer_list_duplicates_throw) {
    try {
        AvlDictionary<int, int> d = {{1,1},{2,2},{1,3}};
        ASSERT(false && "expected exception not thrown");
    } catch (const std::invalid_argument&) {}
}

// ── AVL-specific properties ───────────────────────────────────────────────────

TEST(test_height_single_element) {
    AvlDictionary<int, int> d;
    d.Add(1, 0);
    ASSERT(d.Height() == 1);
}

TEST(test_height_logarithmic_after_sorted_insert) {
    // Worst case for a naive BST: degenerates into a list.
    // AVL must keep height O(log n).
    AvlDictionary<int, int> d;
    const int N = 1000;
    for (int i = 0; i < N; ++i) d.Add(i, i);
    // For N=1000 AVL height <= 1.45 * log2(N) ~= 14.4
    const auto maxAllowedHeight = static_cast<size_t>(
        std::ceil(1.45 * std::log2(N + 1))
    );
    ASSERT(d.Height() <= maxAllowedHeight);
}

TEST(test_height_logarithmic_after_reverse_sorted_insert) {
    AvlDictionary<int, int> d;
    const int N = 1000;
    for (int i = N - 1; i >= 0; --i) d.Add(i, i);
    const auto maxAllowedHeight = static_cast<size_t>(
        std::ceil(1.45 * std::log2(N + 1))
    );
    ASSERT(d.Height() <= maxAllowedHeight);
}

TEST(test_height_correct_after_removal) {
    // After removing half the elements the tree must stay balanced
    AvlDictionary<int, int> d;
    const int N = 256;
    for (int i = 0; i < N; ++i) d.Add(i, i);
    for (int i = 0; i < N; i += 2) d.Remove(i);
    const auto maxAllowedHeight = static_cast<size_t>(
        std::ceil(1.45 * std::log2(N / 2 + 1))
    );
    ASSERT(d.Height() <= maxAllowedHeight);
}

// ── Iterator (in-order traversal) ────────────────────────────────────────────

TEST(test_iterator_empty) {
    AvlDictionary<int, int> d;
    ASSERT(d.begin() == d.end());
}

TEST(test_iterator_sorted_order) {
    AvlDictionary<int, int> d = {{5,0},{2,0},{8,0},{1,0},{3,0},{7,0},{9,0}};
    std::vector<int> keys;
    for (auto& [k, v] : d) keys.push_back(k);
    for (size_t i = 1; i < keys.size(); ++i)
        ASSERT(keys[i] > keys[i - 1]);
    ASSERT(keys.size() == 7);
}

TEST(test_iterator_sorted_after_sorted_insert) {
    // Ascending insert -- most aggressive case for a naive BST
    AvlDictionary<int, int> d;
    for (int i = 0; i < 100; ++i) d.Add(i, i * 2);
    int prev = -1;
    for (auto& [k, v] : d) {
        ASSERT(k > prev);
        ASSERT(v == k * 2);
        prev = k;
    }
}

TEST(test_iterator_values_correct) {
    AvlDictionary<int, int> d = {{3, 30}, {1, 10}, {2, 20}};
    for (auto& [k, v] : d)
        ASSERT(v == k * 10);
}

// ── WriteToStream / operator<< ────────────────────────────────────────────────

TEST(test_write_to_stream) {
    AvlDictionary<std::string, int> d = {{"a", 1}, {"b", 2}};
    std::ostringstream oss;
    oss << d;
    const std::string out = oss.str();
    // In-order output: "a" must appear before "b"
    ASSERT(out.find('"') != std::string::npos);
    ASSERT(out.find("\"a\"") < out.find("\"b\""));
}

// ── Stress test ───────────────────────────────────────────────────────────────

TEST(test_stress_insert_remove) {
    AvlDictionary<int, int> d;
    const int N = 2000;
    for (int i = 0; i < N; ++i) d.Add(i, i);
    ASSERT(d.Count() == static_cast<size_t>(N));
    for (int i = 0; i < N; i += 3) d.Remove(i);
    for (int i = 0; i < N; ++i) {
        if (i % 3 == 0) {
            ASSERT(!d.ContainsKey(i));
        } else {
            ASSERT(d.ContainsKey(i));
            ASSERT(d.GetValue(i) == i);
        }
    }
    // Iterator must yield keys in ascending order
    int prev = -1;
    for (auto& [k, v] : d) {
        ASSERT(k > prev);
        prev = k;
    }
}

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== AvlDictionary: correctness tests ===\n";

    std::cout << "\n--- Basic operations ---\n";
    _Reg_test_empty_dictionary{};
    _Reg_test_add_and_contains{};
    _Reg_test_add_duplicate_throws{};
    _Reg_test_get_value{};
    _Reg_test_get_value_missing_throws{};
    _Reg_test_try_get_value{};
    _Reg_test_insert_or_assign_update{};
    _Reg_test_insert_or_assign_new_key{};
    _Reg_test_remove_existing{};
    _Reg_test_remove_missing{};
    _Reg_test_remove_leaf{};
    _Reg_test_remove_node_with_two_children{};
    _Reg_test_clear{};
    _Reg_test_initializer_list_duplicates_throw{};

    std::cout << "\n--- AVL balance ---\n";
    _Reg_test_height_single_element{};
    _Reg_test_height_logarithmic_after_sorted_insert{};
    _Reg_test_height_logarithmic_after_reverse_sorted_insert{};
    _Reg_test_height_correct_after_removal{};

    std::cout << "\n--- Iterator ---\n";
    _Reg_test_iterator_empty{};
    _Reg_test_iterator_sorted_order{};
    _Reg_test_iterator_sorted_after_sorted_insert{};
    _Reg_test_iterator_values_correct{};

    std::cout << "\n--- WriteToStream ---\n";
    _Reg_test_write_to_stream{};

    std::cout << "\n--- Stress ---\n";
    _Reg_test_stress_insert_remove{};

    std::cout << "\n========================================\n";
    std::cout << "Passed: " << passed << "  |  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}