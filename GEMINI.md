# CPPUI 專案 Agent 開發規範 (Project Rules)

## C++ 向下相容性與巨集使用規範 (compat.hpp)

1. **`constexpr` 規範**：
   - C++14 合法之函式/方法（包含分支、區域變數）：**強制使用 `COMPAT_CONSTEXPR_14`**。
   - C++17 合法之函式：**強制使用 `COMPAT_CONSTEXPR_17`**。
   - C++20 合法之函式：**強制使用 `COMPAT_CONSTEXPR_20`**。

2. **`inline` 與常數宣告規範**：
   - 標頭檔全域/命名空間常數：**強制使用 `COMPAT_INLINE_VAR constexpr`**（嚴禁裸寫 `inline constexpr`）。
   - 效能關鍵微架構路徑：**強制使用 `COMPAT_ALWAYS_INLINE`**。
   - 純查詢/計算方法：**優先標註 `COMPAT_NODISCARD`**。

3. **`static` 與私有輔助規範**：
   - 檔案內部輔助函式一律使用 `static` 或匿名命名空間。

4. **命名空間巢狀規範**：
   - **嚴禁**使用 C++17 巢狀命名空間 `namespace A::B { ... }`。
   - **強制**拆分寫作：
     ```cpp
     namespace A {
     namespace B {
         // ...
     } // namespace B
     } // namespace A
     ```

5. **演算法規範**：
   - **嚴禁**呼叫 C++17 `std::clamp`，一律使用 `compat::clamp`。
