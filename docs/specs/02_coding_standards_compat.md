# CPPUI 設計規範 02：代碼規範、核心契約與向下相容標準 (Coding Standards, Core Contracts & compat.hpp)

## 1. 核心相容性哲學

現代 C++ 提供了豐富的語法與標準庫功能（如 C++17 `std::string_view`、`std::optional`，C++20 `std::format`、`std::span`，C++23 `std::expected`），但在許多工業環境與既有編譯器上，無法保證支援最新標準。

CPPUI 專案全面引入自行研發的 [`compat.hpp`](../../include/compat/compat.hpp) 作為基礎相容層：
1. **在新版編譯器（如 C++20/23）下**：`compat::` 自動無縫 alias 到原生 `std::` 實作，享有零開銷（Zero-Overhead）與極致編譯器最佳化。
2. **在舊版編譯器（如 C++14/17）下**：`compat::` 自動提供完整的 Polyfill 退回實作，確保相同語法能無痛通過編譯。

---

## 2. 核心架構契約與 C++ 規範 (Core Directives & C++ Standards)

### 2.1 架構與函數式範式 (Functional Paradigm & Architecture) [Strict]
- **貫徹 SoC 原則 (Separation of Concerns)**：將大型或複雜的程式／系統拆分成多個彼此獨立的模組或區塊，讓「每個區塊只負責處理一件事情或一個關注點」。
- **純函數與靜態優先 (Pure Functions & Static)**：業務邏輯應盡可能實作為純函數。函數的回傳值應僅依賴於傳入的參數，且**絕對不可修改外部狀態 (No Side Effects)**，並強制標示為 `static`。
- **I/O 隔離 (I/O Segregation)**：核心業務邏輯中**嚴禁**包含任何 I/O 操作（如輸出列印、檔案讀寫、網路請求）。所有的 I/O 必須推延至系統邊界（如 `main.cpp` 或專屬的 I/O 層）處理。
- **不可變性 (Immutability)**：變數與傳入參數預設必須加上 `const`。若需改變資料，請回傳修改後的新副本，而非就地修改 (In-place mutation)。
- **純函數與靜態方法 (Pure Functions & Static Methods)**：任何不需存取物件實例狀態（即程式碼中沒有使用到 `this`）的輔助方法或業務邏輯，**強制**宣告為 `static`。
- **標準輸出格式**：在系統邊界或除錯層需要終端輸出時，一律使用 `compat::print` / `compat::println`。

### 2.2 精確的型別定義 (Fixed-width Integers & Type Safety) [Strict]
- **禁止使用平台依賴型別**：嚴格禁止在程式碼中使用 `int`、`long`、`short` 等寬度依賴編譯器與平台的整數型別（唯一的例外是 ISO C++ 標準規範的進入點 `int main(...)`）。
- **固定寬度整數**：一律引入 `<cstdint>`，並強制使用 `int32_t`、`uint32_t`、`int64_t`、`uint8_t` 等固定寬度型別，或使用 `size_t` 處理容器大小與索引，確保跨平台一致性。

### 2.3 命名空間管理 (Namespace Management) [Strict]
- **嚴格禁止**在全域範圍使用 `using namespace ...;`（包含所有標頭檔、實做檔與單元測試檔）。
- 一律顯式加上命名空間前綴（如 `std::string`, `cppui::core::Size`）。僅允許在函數或區域範圍內使用 namespace alias。

### 2.4 API 設計與參數驗證 (Fail-Fast Principle)
- 針對所有公開的 API 介面，必須在函數的最開頭檢查所有傳入的參數。
- 若參數為指標或智能指標等不允許為空的型態，**必須立即擲出例外**（標準契約錯誤擲出 `std::invalid_argument`，內部狀態錯誤擲出 `cppui::core::InvalidOperationException`）。

### 2.5 安全性與防禦性編程 (Safety & Defensive Programming)
- **記憶體安全 (RAII)**：嚴格遵守 RAII。禁止使用裸 `new` 和 `delete`，一律使用 `std::make_unique` 或 `std::make_shared`。
- **錯誤處理雙軌制**：
  - 契約破壞（傳入 Null、無效引數）：使用 **Exceptions**（`std::invalid_argument` 等）。
  - 預期內的業務錯誤或管線查詢：使用 **`compat::expected<T, E>`** 或 `compat::optional<T>`。
- **編譯期檢查與現代屬性**：積極使用 `COMPAT_NODISCARD`（純查詢方法）。盡量使用 `constexpr` 讓運算提前至編譯期。廣泛應用 `compat::ranges` 與 `compat::views`。

### 2.6 現代 Pimpl 慣用法與編譯防火牆規範 (Modern Pimpl & Compilation Firewall Standard) [Strict]
- **深常數性指標強制 (Deep Constness)**：凡公開介面類別（如複雜資源、多執行緒鎖、平台代碼、繪圖管線等）採用 Pimpl 模式時，**強制使用 `compat::propagate_const<std::unique_ptr<Impl>>`**。嚴格禁止使用原生 `std::unique_ptr<Impl>` 或裸指針，防止淺常數性（Shallow Constness）漏洞導致 `const` 成員方法能意外修改 `Impl` 狀態。
- **完整型別與生命週期規範 (Complete Type in .cpp)**：解構子 `~ClassName()`、移動建構子與移動賦值運算子必須在 `.hpp` 中宣告，並在 `.cpp`（`Impl` 定義為完整型別的編譯單元）中使用 `= default` 或明確實作。嚴禁在標頭檔 inline 實作解構子，避免觸發不完整型別（Incomplete Type）刪除之未定義行為或編譯錯誤。
- **編譯防火牆與 ABI 穩定性 (Compilation Firewall & ABI Stability)**：公開標頭檔嚴禁引入私有第三方、平台相依或巨型內部標頭檔（如 `<shared_mutex>`、渲染後端具體類別、Win32/X11 控制代碼等），所有內部成員變數與私有輔助方法一律封裝於 `.cpp` 中的 `Impl` 結構內。確保對外匯出類別的記憶體佈局與大小鎖定為單一指標（8 位元組），防止內部演進破壞 ABI 向下相容性。

---

## 3. 強制性相容型別與現代化視圖使用規範 (Modern Views & Types)

在 `cppui` 命名空間內編寫所有公共頭文件與核心邏輯時，**一律遵循以下型別映射與現代化視圖規範**：

### 3.1 型別映射標準表

| 標準庫概念 | 禁用寫法 | 強制使用寫法 | 說明 |
| :--- | :--- | :--- | :--- |
| **平台相依整數**| `int`, `long`, `short`, `unsigned int` | `int32_t`, `uint32_t`, `size_t` | 消除 32/64 位元平台與資料模型歧異。 |
| **字串視圖** | `const char*` + `size_t` / `const std::string&` | `compat::string_view` | 避免多餘字串拷貝，唯讀字串參數首選。 |
| **陣列切片視圖**| `const T*` + `size_t` / `const std::vector<T>&` | `compat::span<T>` / `compat::span<const T>` | 連續記憶體緩衝區（如指令流、字符 Glyphs）首選。 |
| **Pimpl 實作指標**| `std::unique_ptr<Impl>` / `Impl*` | `compat::propagate_const<std::unique_ptr<Impl>>` | 消除淺常數性漏洞，保證深常數傳播、ABI 穩定性與編譯防火牆。 |
| **集合惰性管線**| 手寫暫存 `std::vector` / 繁複迴圈 | `compat::views::*` 搭配 `operator\|` | 零分配惰性過濾與投影（`filter`, `transform`, `take`）。 |
| **可空值語意** | 裸指標 `nullptr` 代表無值 | `compat::optional<T>` | 清楚表述數值可能不存在（如文字命中測試結果）。 |
| **型別安全聯合**| `union` 或 `void*` | `compat::variant<Ts...>` | 用於多型事件參數、屬性值包裝。 |
| **錯誤返回** | C 風格錯誤代碼 / 拋出異常 | `compat::expected<T, E>` | 適用於可預期的管線失敗，不產生異常棧開銷。 |
| **字串格式化** | `sprintf` / `std::stringstream` | `compat::format(...)` | 型別安全且高效的文字格式化。 |
| **終端除錯輸出**| `printf` / `std::cout` | `compat::print(...)` / `compat::println(...)` | 跨平台 UTF-8 友善終端輸出。 |

### 3.2 現代化視圖 (Views) 優先契約
1. **字串視圖優先**：
   - 所有函式與方法的唯讀字串參數，**一律使用 `compat::string_view`**。
   - 嚴禁使用 `const std::string&` 作為唯讀參數型別（除非該函式內部必須轉存所有權且使用移動語意）。
   - 與底層 Win32 C-API 互動需傳遞 NUL 結尾字串時，封裝於內部私有實作轉換，公開介面維持 `compat::string_view`。
2. **緩衝區切片視圖優先 (`compat::span`)**：
   - 所有接受連續記憶體區塊（如頂點、繪圖操作流、字符、像素區段）的函式，**強制使用 `compat::span<T>` 或 `compat::span<const T>`**。
   - 嚴禁使用老舊的雙參數裸指標加長度 (`const T* ptr, size_t count`)，杜絕越界與安全漏洞。
   - 避免將參數型別硬綁定為特定容器（如 `const std::vector<T>&`），改用 `compat::span<const T>` 即可同時無縫接收 `std::vector`、`std::array`、原生陣列或連續平坦記憶體池。
3. **資料流轉換優先走惰性管線 (`compat::views::*`)**：
   - 處理元件樹、命中清單或圖元集合的過濾、投影與截取時，優先採用 `compat::views::*` 搭配管道運算子 `|`（如 `compat::views::filter`, `transform`, `take`, `as_const`）。
   - 實現 $O(1)$ 複製複雜度與零堆積配置，嚴禁手寫建立臨時 `std::vector` 進行中繼儲存。
4. **生命週期安全原則 (Borrowing & Lifetime Safety)**：
   - 視圖（`string_view`, `span`, `subrange`, 視圖適配器）為不具備所有權的「借用視圖」（Borrowed View）。
   - 嚴禁持有或回傳指向臨時右值物件（Rvalue Container）的懸空視圖。

---

## 4. 註解與文檔規範 (Documentation Guidelines)

1. **檔案精要解說**：
   - 在標頭檔與實作檔最上方加入多行註解（`/* ... */`），扼要說明該檔案之核心責任與架構定位。
   - 標頭檔必須在開頭加上 `#pragma once`。
2. **XML 註解限定 (XML Comments)**：
   - **只有「類別 (Class)」與「函數/方法 (Method)」**必須使用 XML 格式的註解 (`///`) 進行說明：
     - `<summary>`: 簡述功能與目的。
     - `<param name="...">`: 說明傳入參數的意義與限制。
     - `<returns>`: 說明回傳值與可能的狀態。
     - `<exception cref="...">`: 清楚列出該 API 會提早擲出的例外。
3. **一般註解 (Standard Comments)**：
   - 除了類別與函數之外的程式碼元素（如區域變數、內部邏輯說明等），一律使用一般單行 (`//`) 或多行 (`/* */`) 註解即可，避免過度標記導致版面雜亂。

---

## 5. 測試與除錯策略 (Testing & Verification Strategy)

1. **高覆蓋率與輕量測試**：
   - 核心邏輯皆偏好純函數，單元測試必須極度輕量且全面，涵蓋 Happy Path 與 Edge Cases。
2. **負向例外測試 (Negative Exception Tests) [Strict]**：
   - 所有公開 API 必須包含對傳入 Null、空值或非法參數時是否正確擲出例外（如 `std::invalid_argument`、`cppui::core::InvalidOperationException`）的斷言測試。
3. **矩陣編譯驗證**：
   - 任何規格變更與重構均須通過 `tests/run_matrix_suite.ps1` 跨編譯器矩陣驗證（MSVC, Clang++, G++ x C++14, C++17, C++20, C++23 共 168+ 項測試全數 PASS）。

---

## 6. 代碼風格與命名約定 (Naming Conventions)

1. **命名空間 (Namespaces)**：
   - 頂層：`cppui`
   - 子模組：`cppui::core`, `cppui::pal`, `cppui::rendering`, `cppui::text`, `cppui::layout`, `cppui::input`, `cppui::controls`, `cppui::app`, `cppui::mvvm`
   - 私有細節：`cppui::detail`
   - 嚴禁巢狀命名空間 `namespace A::B`，必須拆分：
     ```cpp
     namespace cppui {
     namespace core {
         // ...
     } // namespace core
     } // namespace cppui
     ```
2. **型別命名 (Types)**：
   - 類別、結構、列舉：大駝峰 `PascalCase`（例如 `DrawingContext`, `DisplayList`, `VisualTree`）。
   - 介面：以 `I` 開頭的大駝峰（例如 `IPlatformWindow`, `IRenderer`）。
   - 列舉成員：大駝峰 `PascalCase` 或強型別列舉 `enum class Alignment { Left, Center, Right };`。
3. **函式與方法 (Functions & Methods)**：
   - 公共方法與 Fluent API：大駝峰 `PascalCase`（例如 `Measure()`, `Arrange()`, `InvalidateVisual()`, `Background(Color)`）。
   - 私有/內部輔助函式：小駝峰 `camelCase`（例如 `calculateDesiredSize()`, `rebuildDirtyRegions()`）。
4. **變數與成員 (Variables & Fields)**：
   - 類別成員變數：必須加上 `m_` 前綴（例如 `m_bounds`, `m_parent`, `m_isDirty`）。
   - 區域變數與參數：小駝峰 `camelCase`（例如 `availableSize`, `finalRect`）。
   - 常數與靜態唯讀：大駝峰或全大寫蛇形（例如 `DefaultFontSize`, `MAX_ARENA_BLOCK_SIZE`）。

---

## 7. 跨標準巨集與微架構最佳化規範 (Macros & Micro-Architecture)

為了確保在 C++14、C++17、C++20、C++23 下零語法報錯、零擴展警告（`-Wc++17-extensions`、`error C2429`、`error C7525`），專案強制遵循以下巨集契約：

### 7.1 `constexpr` 規範
- 函式/方法具備 C++14 合法分支或局部變數時：**強制使用 `COMPAT_CONSTEXPR_14`**。
- C++17 合法語法（如 lambda / if constexpr）：**使用 `COMPAT_CONSTEXPR_17`**。
- C++20 合法語法：**使用 `COMPAT_CONSTEXPR_20`**。
- 純數值常數或 POD 結構成員：若編譯期可計算，使用 `constexpr`。

### 7.2 `inline` 與常數宣告規範
- **全域/命名空間常數**：嚴禁裸寫 `inline constexpr`。**強制使用 `COMPAT_INLINE_VAR constexpr`**（在 C++17+ 展開為 `inline`；在 C++14/11 降級為 `static`，徹底根絕 ODR 碰撞與 C7525 錯誤）。
- **關鍵小函式/內聯**：
  - 微架構熱點：**強制使用 `COMPAT_ALWAYS_INLINE`**。
  - 一般內聯函式：使用 `inline`，且若有回傳值建議搭配 `COMPAT_NODISCARD`。

### 7.3 跨標準演算法與微架構提示
- **數值夾取**：禁止呼叫 C++17 的 `std::clamp`，**一律使用 `compat::clamp`**。
- **分支預測**：高頻核心路徑使用 `COMPAT_LIKELY(expr)` / `COMPAT_UNLIKELY(expr)`。
- **不可達程式碼**：使用 `COMPAT_UNREACHABLE()`。
- **傳回值檢查**：所有狀態查詢與純計算函式強制標記 `COMPAT_NODISCARD`。
