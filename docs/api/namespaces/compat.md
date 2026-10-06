# compat 命名空間與相容性契約 (Namespace & Macro Contracts)

`compat` 命名空間與 [`compat.hpp`](file:///D:/P/CPP/CPPUI/include/compat/compat.hpp) 提供 CPPUI 的 **零依賴現代 C++ 向下相容墊片層 (Shim Layer)** 與微架構編譯器最佳化契約。

---

## 簡介 (Description)

為了讓 CPPUI 能夠跨越現代與工業級老舊環境（從 C++14、C++17 到 C++20、C++23），專案引入自研單標頭相容庫 `compat.hpp`：
* **新版本編譯器 (C++20/23)**：自動 `using` 原生 `std::` 標準庫（例如 `std::string_view`, `std::optional`, `std::expected`），達到零開銷與全域編譯最佳化。
* **舊版本編譯器 (C++14/17)**：自動切換至內部完整驗證之 Polyfill 實作，確保相同 API 寫法能在 C++14 下無縫通過編譯。

---

## 核心型別映射 (Type Mapping Table)

| 相容型別 | 對應標準概念 | 適用場景與優勢 |
| :--- | :--- | :--- |
| `compat::string_view` | `std::string_view` (C++17) | 零拷貝唯讀字串視圖，廣泛用於視窗標題、文字繪製與字串參數。 |
| `compat::optional<T>` | `std::optional<T>` (C++17) | 安全可空值容器，用於命中測試、屬性查詢等可能無傳回值的場景。 |
| `compat::expected<T, E>` | `std::expected<T, E>` (C++23) | 型別安全之錯誤處理機制，適用於檔案加載、字型剖析，消除異常開銷。 |
| `compat::span<T>` | `std::span<T>` (C++20) | 連續記憶體切片視圖，用於指標陣列、頂點資料、字元代碼流。 |
| `compat::clamp(v, lo, hi)` | `std::clamp` (C++17) | 數值區間安全夾取，相容 C++14，避免溢出。 |
| `compat::println(...)` | `std::println` (C++23) | 跨平台 UTF-8 友善終端輸出，徹底杜絕 Windows 繁體中文終端亂碼。 |
| `compat::print(...)` | `std::print` (C++23) | 格式化終端輸出，不換行。 |

---

## 公開巨集契約 (Public Macro Contracts)

為保證跨 MSVC、Clang++、G++ 三大編譯器時零報錯與零擴展警告（如 `-Wc++17-extensions`、`error C2429`、`error C7525`），全專案遵循以下公開巨集：

### 1. `constexpr` 相容巨集
* `COMPAT_CONSTEXPR_14`：在 C++14+ 展開為 `constexpr`。用於包含分支、區域變數或迴圈的結構建構式與成員方法。
* `COMPAT_CONSTEXPR_17`：在 C++17+ 展開為 `constexpr`。用於 constexpr lambda、if constexpr。
* `COMPAT_CONSTEXPR_20`：在 C++20+ 展開為 `constexpr`。

### 2. `inline` 與常數宣告巨集
* `COMPAT_INLINE_VAR`：
  - 在 C++17+ 自動展開為 `inline`。
  - 在 C++14/11 自動降級為 `static`。
  - **強制用於標頭檔全域常數宣告**（例如 `COMPAT_INLINE_VAR constexpr Color Transparent = ...;`），杜絕 ODR 衝突與 C7525 報錯。
* `COMPAT_ALWAYS_INLINE`：
  - 微架構關鍵熱點強制內聯。
  - MSVC 展開為 `__forceinline`，GCC/Clang 展開為 `__attribute__((always_inline)) inline`。

### 3. 編譯器提示與屬性巨集
* `COMPAT_NODISCARD`：在支援的標準下展開為 `[[nodiscard]]`。純運算與狀態查詢方法強制標記。
* `COMPAT_LIKELY(expr)`：分支預測提示，標記大概率發生的條件分支。
* `COMPAT_UNLIKELY(expr)`：分支預測提示，標記極少發生的例外分支（如緩衝區溢位擴展）。
* `COMPAT_UNREACHABLE()`：提示編譯器該執行路徑不可達，協助微架構死碼消除。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"

// 1. 常數定義
COMPAT_INLINE_VAR constexpr int MaxBufferSlots = 1024;

// 2. 跨版本相容函式
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 inline int ClampValue(int val, int minVal, int maxVal) {
    return compat::clamp(val, minVal, maxVal);
}

void ExampleCompatUsage() {
    compat::string_view text = "Hello CPPUI";
    compat::optional<int> maybeVal = 42;

    if (COMPAT_LIKELY(maybeVal.has_value())) {
        compat::println("字串: {}, 數值: {}", text, maybeVal.value());
    }
}
```

---

## 請參閱 (See Also)
* [CPPUI API 首頁](../README.md)
* [代碼規範與向下相容標準](../../specs/02_coding_standards_compat.md)
