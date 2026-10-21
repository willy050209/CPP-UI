# CPPUI 工程規範與核心開發原則 (RULES.md)

本文件依據 AI 工作流主控台 (Orchestrator) 規範與 CPPUI 專案規範 (`AGENTS.md`) 制定，所有 Worker 子代理必須嚴格遵守。

---

## 1. 架構與職責分離 (Separation of Concerns & Architecture)
1. **職責分離**：明確區分 Data Class / Struct (資料結構) 與 Logic (業務邏輯區塊)，避免混雜。
2. **純函數與靜態優先 (Pure Functions & Static)**：任何不存取物件實例狀態的邏輯強制宣告為 `static` 純函數，確保無副作用。
3. **I/O 隔離**：核心業務邏輯中嚴禁包含任何 I/O 操作（標準輸出使用 `compat::print` / `compat::println`，系統邊界處理）。
4. **不可變性 (Immutability)**：變數與傳入參數預設加上 `const`。
5. **Fail-Fast 原則**：公開 API 進入點必須第一時間校驗參數（Guard Clauses），指標或智能指標為空立即擲出 `std::invalid_argument`。

---

## 2. 精確型別與相容性規範 (compat.hpp & Types)
1. **固定寬度整數**：一律引入 `<cstdint>`，使用 `int32_t`, `uint32_t`, `int64_t`, `uint8_t` 或 `size_t`，嚴格禁止平台相依基本整數型別（除 `int main()` 外）。
2. **命名空間管理**：嚴禁全域 `using namespace`；嚴禁 C++17 巢狀命名空間 `namespace A::B`，必須顯式拆分。
3. **相容性巨集**：
   - C++14 合法：`COMPAT_CONSTEXPR_14`；C++17：`COMPAT_CONSTEXPR_17`；C++20：`COMPAT_CONSTEXPR_20`。
   - 標頭檔常數強制使用 `COMPAT_INLINE_VAR constexpr`。
   - 微架構熱點使用 `COMPAT_ALWAYS_INLINE`；純查詢/計算方法標註 `COMPAT_NODISCARD`。
   - 嚴禁呼叫 `std::clamp`，一律使用 `compat::clamp`。
   - 唯讀字串參數強制使用 `compat::string_view`；連續緩衝區強制使用 `compat::span<T>`。
4. **現代 Pimpl 慣用法**：公開類別 Pimpl 強制採用 `compat::propagate_const<std::unique_ptr<Impl>>` 確保深常數性，解構子必須在 `.cpp` 實作。

---

## 3. 註解規範 (Documentation Standards)
1. **XML 註解限定**：**僅對「類別 (Class/Struct)」與「函數/方法 (Function/Method)」**使用 XML 格式註解 (`///`)，包含 `<summary>`, `<param>`, `<returns>`, `<exception>`。
2. **程式內部註解**：類別與函數內部的執行邏輯**一律使用一般單行 (`//`) 或多行 (`/* */`) 註解**，禁止濫用 XML 標籤。
3. **標頭檔保護**：所有 Header File 開頭必須加上 `#pragma once`。

---

## 4. TDD 測試驅動開發與零警告原則
1. **TDD 循環**：先寫測試用例 (Red) → 實作程式碼使測試通過 (Green) → 重構優化 (Refactor)。
2. **全編譯器零警告**：維持 MSVC `/W4`、GCC/Clang 0 Errors / 0 Warnings。
