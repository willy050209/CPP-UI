# 受約束範圍與演算法 (compat::ranges)

定義於標頭檔 [`include/compat/Compat.hpp`](../../../include/compat/Compat.hpp)。  
所屬命名空間：`compat::ranges`。

---

## 1. 簡介 (Description)

`compat::ranges` 完全對齊 **ISO C++20 / C++23 Ranges 標準**，徹底克服了傳統 STL 迭代器雙參數 (`begin`, `end`) 容易錯配與缺少概念約束的缺陷。

### 核心革新
1. **Range 全區間重載**：演算法可直接接受完整容器物件（如 `ranges::sort(vec)`），無須手寫 `vec.begin(), vec.end()`。
2. **Niebloid 演算法物件**：所有演算法皆實作為 Niebloid 函式物件，徹底禁用引數相依查找 (ADL)，杜絕跨命名空間符號衝突。
3. **投影機制 (Projections)**：所有演算法原生支援投影引數（可傳遞成員變數指標或 Callable），省去撰寫自訂 Predicate 的冗贅代碼。
4. **懸空迭代器安全防護 (`dangling`)**：當對臨時右值容器呼叫尋找演算法時，回傳型別自動變更為 `dangling` 標籤，杜絕懸空迭代器未定義行為。

---

## 2. 自訂點物件 (Customization Point Objects, CPO)

CPO 是一組特殊的函式物件，保證安全統一存取容器或陣列的邊界與尺寸，優先呼叫成員函式，次選相容的自由函式：

| CPO 物件 | 簽章摘要 | 說明 |
| :--- | :--- | :--- |
| `compat::ranges::begin(r)` | `auto begin(R&& r)` | 取得指向區間首元素的迭代器。支援常規陣列與 STL 容器。 |
| `compat::ranges::end(r)` | `auto end(R&& r)` | 取得指向區間尾部的哨兵 (Sentinel) 或迭代器。 |
| `compat::ranges::cbegin(r)` | `auto cbegin(R&& r)` | 取得指向常數唯讀首元素的迭代器。 |
| `compat::ranges::cend(r)` | `auto cend(R&& r)` | 取得指向常數唯讀尾部的哨兵。 |
| `compat::ranges::size(r)` | `auto size(R&& r)` | 取得區間元素總數。$O(1)$ 複雜度。 |
| `compat::ranges::ssize(r)` | `auto ssize(R&& r)` | 取得有號整數格式的長度 (`ptrdiff_t`)，避免與無號整數比較之警報。 |
| `compat::ranges::empty(r)` | `bool empty(R&& r)` | 判斷區間是否為空。 |
| `compat::ranges::data(r)` | `auto data(R&& r)` | 取得指向連續記憶體區塊底層陣列的原始指標。 |

---

## 3. 受約束演算法家族 (Constrained Range Algorithms)

`compat::ranges` 提供 40+ 個現代演算法，全數支援**投影 (Projections)** 與**全區間傳遞**：

### 常用演算法速查表

| 演算法 | 傳統 STL 對應 | 現代 Ranges 呼叫範例 |
| :--- | :--- | :--- |
| `ranges::sort` | `std::sort` | `ranges::sort(items, {}, &Item::priority)` (支援以成員變數投影排序) |
| `ranges::find` | `std::find` | `ranges::find(items, targetId, &Item::id)` |
| `ranges::find_if` | `std::find_if` | `ranges::find_if(elements, &UIElement::IsVisible)` |
| `ranges::copy` | `std::copy` | `ranges::copy(source, dest_iter)` (回傳 `in_out_result`) |
| `ranges::for_each` | `std::for_each` | `ranges::for_each(elements, [](auto& e) { e.Render(); })` |
| `ranges::all_of` | `std::all_of` | `ranges::all_of(elements, &UIElement::IsEnabled)` |
| `ranges::any_of` | `std::any_of` | `ranges::any_of(elements, &UIElement::IsFocused)` |

---

## 4. 投影機制與實戰範例 (Projections in Action)

傳統 C++ 排序結構體時必須撰寫自訂 Lambda，而在 `compat::ranges` 中，可以直接傳入成員指標或投影函式：

```cpp
#include "compat/Compat.hpp"
#include <vector>

struct Widget {
    int id;
    int zIndex;
    compat::string_view name;
};

void SortWidgetsByZIndex() {
    std::vector<Widget> widgets = {
        { 1, 10, "ModalDialog" },
        { 2,  0, "Background" },
        { 3,  5, "Sidebar" }
    };

    // 1. 使用成員指標投影，依 zIndex 進行升冪排序！
    compat::ranges::sort(widgets, {}, &Widget::zIndex);

    // 2. 搜尋特定 ID 的元件
    auto it = compat::ranges::find(widgets, 1, &Widget::id);
    if (it != widgets.end()) {
        compat::println("找到頂層元件: {}", it->name);
    }

    // 輸出排序結果
    for (const auto& w : widgets) {
        compat::println("Z={}: {}", w.zIndex, w.name);
    }
}
```

---

## 5. C++23 `ranges::to` 容器轉化器

支援將任何管線視圖結果一鍵轉換為指定實體 STL 容器：

```cpp
#include "compat/Compat.hpp"
#include <vector>

void ConvertViewToVector() {
    using namespace compat::views;

    // 將惰性管線運算結果物化 (Materialize) 為 std::vector<int>
    std::vector<int> evenSquares = iota(1, 10)
        | filter([](int n) { return n % 2 == 0; })
        | transform([](int n) { return n * n; })
        | compat::ranges::to<std::vector<int>>();
}
```

---

## 請參閱 (See Also)
* [`compat` 核心相容層總覽](compat.md)
* [`compat::views` 視圖適配器手冊](compat_views.md)
* [代碼規範與向下相容標準](../../specs/02_coding_standards_compat.md)
