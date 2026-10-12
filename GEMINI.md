# CPPUI 專案 Agent 開發規範 (Project Rules)

## 1. 架構與函數式範式 (Functional Paradigm & Architecture) [Strict]
1. **貫徹 SoC 原則 (Separation of Concerns)**：將大型或複雜系統拆分為彼此獨立的模組或區塊，每個區塊僅負責單一關注點。
2. **純函數與靜態優先 (Pure Functions & Static)**：業務邏輯應盡可能實作為純函數。回傳值僅依賴傳入參數，**絕對不可修改外部狀態 (No Side Effects)**，並強制標示為 `static`。
3. **I/O 隔離 (I/O Segregation)**：核心業務邏輯中**嚴禁**包含任何 I/O 操作（如 `print`、檔案讀寫、網路請求）。所有 I/O 必須推延至系統邊界（如 `main.cpp` 或專屬 I/O 層）處理。
4. **不可變性 (Immutability)**：變數與傳入參數預設必須加上 `const`。若需改變資料，請回傳修改後的新副本，而非就地修改 (In-place mutation)。
5. **純函數與靜態方法 (Pure Functions & Static Methods)**：任何不需存取物件實例狀態（即程式碼中沒有使用到 `this`）的輔助方法或業務邏輯，**強制**宣告為 `static`。純函數確保無副作用，利於編譯器最佳化與單元測試。
6. **標準輸出**：使用 `compat::print` / `compat::println` 作為標準終端輸出。

## 2. 精確的型別定義 (Fixed-width Integers & Type Safety) [Strict]
1. **禁止使用平台依賴型別**：嚴格禁止使用 `int`、`long`、`short` 等基本整數型別（除了 C++ 標準規範規定的 `int main()` 進入點之外）。
2. **固定寬度整數**：一律引入 `<cstdint>`，並強制使用 `int32_t`、`uint32_t`、`int64_t`、`uint8_t` 等固定寬度型別，或使用 `size_t` 處理容器大小與索引，確保跨平台一致性。

## 3. 命名空間管理 (Namespace Management) [Strict]
1. **嚴禁全域 using namespace**：嚴格禁止在全域範圍使用 `using namespace`。
2. **顯式命名空間前綴**：一律顯式加上命名空間前綴（如 `std::string`、`cppui::core::Point`）。僅允許在函式或區域範圍內使用 namespace alias。
3. **嚴禁 C++17 巢狀命名空間**：嚴禁 `namespace A::B { ... }`，強制拆分寫作：
   ```cpp
   namespace A {
   namespace B {
       // ...
   } // namespace B
   } // namespace A
   ```

## 4. API 設計與參數驗證 (Fail-Fast Principle)
1. **Fail-Fast 參數檢查**：針對所有公開的 API 介面，必須在函數的最開頭檢查所有傳入參數。
2. **指標非空防護**：若參數為指標或智能指標等不允許為空的型態，**必須立即擲出例外** (`std::invalid_argument`)。

## 5. 安全性與防禦性編程 (Safety & Defensive Programming)
1. **記憶體安全 (RAII)**：嚴格遵守 RAII。禁止使用 raw `new` 和 `delete`，一律使用 `std::make_unique` 或 `std::make_shared`。
2. **現代化視圖優先 (Modern Views & Borrowing)**：
   - 唯讀字串參數強制使用 `compat::string_view`，嚴禁 `const std::string&` 或 `const char* + size_t`。
   - 連續記憶體緩衝區強制使用 `compat::span<T>` / `compat::span<const T>`，嚴禁 `const T*, size_t` 或硬綁定 `const std::vector<T>&`。
   - 集合處理優先採用 `compat::views::*` 搭配 `operator|`（零拷貝惰性評估）。
   - 嚴禁回傳或持有指向臨時右值容器的懸空視圖。
3. **錯誤處理雙軌制**：
   - 契約破壞（傳入 Null / 非法引數）：使用 **Exceptions**（傳入 Null / 非法引數使用 `std::invalid_argument` / `std::runtime_error`；其餘領域異常繼承 `cppui::core::Exception`）。
   - 預期內業務失敗：使用 **`compat::expected`** 或 `compat::optional`。
4. **巨集與相容性規範 (compat.hpp)**：
   - C++14 合法之函式：使用 `COMPAT_CONSTEXPR_14`；C++17 使用 `COMPAT_CONSTEXPR_17`；C++20 使用 `COMPAT_CONSTEXPR_20`。
   - 標頭檔常數強制使用 `COMPAT_INLINE_VAR constexpr`。
   - 微架構熱點使用 `COMPAT_ALWAYS_INLINE`；純查詢/計算方法標註 `COMPAT_NODISCARD`。
   - 嚴禁呼叫 `std::clamp`，一律使用 `compat::clamp`。
5. **現代 Pimpl 慣用法與深常數性 (Modern Pimpl & Deep Constness) [Strict]**：
   - **深常數性指標強制 (Deep Constness)**：凡公開介面類別（如複雜資源、多執行緒鎖、平台代碼、繪圖管線等）採用 Pimpl 模式時，**強制使用 `compat::propagate_const<std::unique_ptr<Impl>>`**。嚴格禁止使用原生 `std::unique_ptr<Impl>` 或裸指針，防止淺常數性（Shallow Constness）漏洞導致 `const` 成員方法能意外修改 `Impl` 狀態。
   - **完整型別與生命週期規範 (Complete Type in .cpp)**：解構子 `~ClassName()`、移動建構子與移動賦值運算子必須在 `.hpp` 中宣告，並在 `.cpp`（`Impl` 定義為完整型別的編譯單元）中使用 `= default` 或明確實作。嚴禁在標頭檔 inline 實作解構子，避免觸發不完整型別（Incomplete Type）刪除之未定義行為或編譯錯誤。
   - **編譯防火牆與 ABI 穩定性 (Compilation Firewall & ABI Stability)**：公開標頭檔嚴禁引入私有第三方、平台相依或巨型內部標頭檔（如 `<shared_mutex>`、渲染後端具體類別、Win32/X11 控制代碼等），所有內部成員變數與私有輔助方法一律封裝於 `.cpp` 中的 `Impl` 結構內。確保對外匯出類別的記憶體佈局與大小鎖定為單一指標（8 位元組），防止內部演進破壞 ABI 向下相容性。

## 6. 文件與註解規範 (Documentation Guidelines)
1. **XML 註解限定**：**只有「類別 (Class)」與「函數/方法 (Method)」**必須使用 XML 格式的註解 (`///`) 說明：
   - `<summary>`: 簡述功能與目的。
   - `<param name="...">`: 說明傳入參數的意義與限制。
   - `<returns>`: 說明回傳值與可能的狀態。
   - `<exception cref="...">`: 清楚列出該 API 會提早擲出的例外。
2. **一般註解**：類別與函數之外的程式碼元素（如區域變數、內部邏輯等），一律使用一般單行 (`//`) 或多行 (`/* */`) 註解，避免過度標記導致版面雜亂。
3. **檔案頂部精要解說**：標頭檔與實作檔最上方必須加入多行註解說明該檔案的功能。
4. **標頭檔前置指令**：所有 Header File 必須在開頭加上 `#pragma once`。

## 7. 測試與除錯策略 (Testing Strategy)
1. 單元測試輕量且全面，涵蓋 Happy Path 與 Edge Cases。
2. **必須包含對 API 傳入 Null 或非法參數是否正確擲出例外的測試**。

## 8. 零警告與零錯誤品質標準 (Zero Warnings & Errors Policy) [Strict]
1. **全編譯器零警告**：程式碼在 MSVC (`/W4`)、GCC (`-Wall -Wextra -Wpedantic`) 以及 Clang 嚴格編譯選項下，必須維持 **0 Errors / 0 Warnings**。
2. **Release / NDEBUG 模式相容**：僅在 `assert` 中使用的變數必須加上 `(void)` 轉型，避免在 Release 模式下觸發未使用的變數警告。
3. **即時消除**：任何新加入或修改的程式碼若引發編譯警告，必須立即修復並消除，嚴禁提交任何帶有編譯警告的程式碼。
