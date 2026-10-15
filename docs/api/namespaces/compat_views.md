# 視圖適配器與惰性管道 (compat::views)

定義於標頭檔 [`include/compat/Compat.hpp`](../../../include/compat/Compat.hpp)。  
所屬命名空間：`compat::views`。

---

## 1. 簡介 (Description)

視圖（View）是現代 C++ 中輕量、具備常數時間複製與移動複雜度（$O(1)$）且具備**惰性求值（Lazy Evaluation）**特性的 Range 觀察者物件。

`compat::views` 不僅提供完整的 **ISO C++20 核心視圖**，更領先業界實作了 **ISO C++23 `as_const`** 與 **ISO C++26 `concat` (P2542R8)、`cache_latest` (P3138R5)**，並完整支援 UNIX 管道運算子語法（`operator|`）。

### 核心優勢
* **零動態記憶體配置 (Zero Dynamic Allocation)**：視圖操作完全在棧上執行，不配置中繼 `std::vector`。
* **惰性求值 (Lazy Pipeline)**：只有在迭代器前進（`++it`）並解參考時，轉換與過濾邏輯才會即時計算。
* **高度可組合性 (Composability)**：藉由 `range_adaptor_closure`，任何視圖皆可透過 `|` 串接成高可讀性的資料處理管線。

---

## 2. 管道語法機制 (Pipeline Architecture: `operator|`)

所有的視圖適配器均支援管道運算子：

```cpp
#include "compat/Compat.hpp"

void ProcessNumbers() {
    using namespace compat::views;

    std::vector<int> numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // 篩選奇數 -> 計算平方 -> 僅截取前 3 個結果
    auto pipeline = numbers 
        | filter([](int n) { return n % 2 != 0; })
        | transform([](int n) { return n * n; })
        | take(3);

    for (int v : pipeline) {
        compat::print("{} ", v); // 輸出: 1 9 25 
    }
    compat::println("");
}
```

---

## 3. 核心視圖適配器目錄 (Views Catalog)

### 3.1 產生器與基礎工廠 (Generators & Factories)

| 視圖物件 | 類別樣板型別 | 說明與標準版本 |
| :--- | :--- | :--- |
| `compat::views::empty<T>` | `empty_view<T>` | 產生長度永遠為 0 的唯讀空視圖 (C++20)。滿足 `borrowed_range`。 |
| `compat::views::single(x)` | `single_view<T>` | 產生僅包含單一元素 `x` 的視圖 (C++20)。透過 `enable_borrowed_range` 約束生命週期。 |
| `compat::views::iota(first, [last])` | `iota_view<W, Bound>` | 產生從 `first` 遞增的數值或迭代器序列，可為有界或無窮序列 (C++20)。 |
| `compat::views::all(r)` | `all_t<R>` | 將可檢視容器轉化為視圖（左值轉為 `subrange`，右值轉為 `owning_view`）(C++20)。 |

### 3.2 轉換與過濾適配器 (Transformers & Filters)

| 視圖物件 | 類別樣板型別 | 說明與標準版本 |
| :--- | :--- | :--- |
| `compat::views::filter(pred)` | `filter_view<V, Pred>` | 依據謂詞 `pred` 惰性過濾元素。快取首個相符元素 (C++20)。 |
| `compat::views::transform(func)` | `transform_view<V, F>` | 依據轉換函式 `func` 惰性對每個元素進行投影轉換 (C++20)。 |
| `compat::views::take(n)` | `take_view<V>` | 取出前 `n` 個元素；若不足 `n` 個則取至結尾 (C++20)。 |
| `compat::views::take_while(pred)` | `take_while_view<V, Pred>` | 依據謂詞 `pred` 條件滿足時截取元素，直至首個不滿足處停止 (C++20)。 |
| `compat::views::drop(n)` | `drop_view<V>` | 略過前 `n` 個元素，從第 `n+1` 個元素開始遍歷 (C++20)。 |
| `compat::views::drop_while(pred)` | `drop_while_view<V, Pred>` | 依據謂詞 `pred` 條件滿足時跳過元素，自首個不滿足處開始遍歷 (C++20)。 |
| `compat::views::reverse` | `reverse_view<V>` | 將雙向 Range 反向逆序遍歷 (C++20)。 |

### 3.3 現代前沿視圖 (C++23 & C++26 Views)

| 視圖物件 | 類別樣板型別 | 規格草案與核心架構 |
| :--- | :--- | :--- |
| `compat::views::as_const` | `as_const_view<V>` | **ISO C++23 (P2278R4)**：將底層區間視為唯讀，解參考一律回傳 `const` 參考。 |
| `compat::views::cache_latest` | `cache_latest_view<V>` | **ISO C++26 (P3138R5)**：快取最後解參考之數值，具備嚴格的 `non-propagating-cache` 語意。 |
| `compat::views::concat(v1, v2, ...)` | `concat_view<Views...>` | **ISO C++26 (P2542R8 / N4984)**：零堆積多區間串接視圖。支援扁平化陣列快取前綴大小、雙向狀態機與 $O(1)$ 隨機存取跳轉階梯。 |

---

## 4. 生命週期與借用語意安全性 (Borrowing & Lifetime Safety)

> [!WARNING]
> **嚴禁視圖懸空 (Dangling Views)**：
> 視圖本身只持有指標或迭代器範圍，**不擁有底層元素的記憶體**。
> ```cpp
> // 錯誤示範：回傳借用臨時右值容器的視圖 -> 懸空指標 (Undefined Behavior)!
> auto BadFunction() {
>     return std::vector<int>{1, 2, 3} | compat::views::filter(...); // 危險！
> }
> 
> // 正確示範：底層容器必須在視圖生命週期內持續存活
> void GoodFunction(const std::vector<int>& data) {
>     auto view = data | compat::views::filter(...); // 安全！
>     for (int v : view) { ... }
> }
> ```

---

## 5. UI 實戰範例 (Practical UI Examples)

### 範例 1：使用 `views::iota` 動態生成網格佈局座標
```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

void GenerateGridCells() {
    using namespace cppui::core;
    using namespace cppui::views;

    // 產生索引 0 ~ 9，投影轉換為矩形邊界
    auto gridCells = iota(0, 10)
        | transform([](int idx) {
            float x = static_cast<float>((idx % 5) * 60);
            float y = static_cast<float>((idx / 5) * 60);
            return Rect(x, y, 50.0f, 50.0f);
        });

    for (const Rect& cell : gridCells) {
        compat::println("Cell: ({}, {}) - {}x{}", cell.x, cell.y, cell.width, cell.height);
    }
}
```

### 範例 2：ISO C++26 `views::concat` 串接多個圖元緩衝區
```cpp
#include "compat/Compat.hpp"
#include <vector>

void ProcessAllBuffers() {
    std::vector<int> headerOps = { 1, 2 };
    std::vector<int> bodyOps   = { 10, 20, 30 };
    std::vector<int> footerOps = { 99 };

    // 零記憶體拷貝，直接將三個容器串接為單一可遍歷視圖！
    auto allOps = compat::views::concat(headerOps, bodyOps, footerOps);

    for (int opId : allOps) {
        compat::print("{} ", opId); // 輸出: 1 2 10 20 30 99
    }
    compat::println("");
}
```

---

## 請參閱 (See Also)
* [`compat` 核心相容層總覽](compat.md)
* [`compat::ranges` 受約束範圍手冊](compat_ranges.md)
* [代碼規範與向下相容標準](../../specs/02_coding_standards_compat.md)
