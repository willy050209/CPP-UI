# CPPUI Agent 開發規範與契約總則 (Rules Overview)

歡迎進入 CPPUI 專案。本工作目錄之 `.gemini/` 資料夾包含專案的核心契約規範與過往常見問題知識庫。
所有 Agent 在執行任務、編寫程式碼與執行終端機命令時，必須遵照以下指引：

---

## 1. 核心文檔導引

- [**contracts.md**](file:///D:/P/CPP/CPPUI/.gemini/contracts.md)：
  - **Windows PowerShell 5.1 終端機契約**（嚴禁 `&&`/`||`，副檔名 `.exe`，環境變數 `$env:VAR`）。
  - **C++ 相容性巨集契約 (`compat.hpp`)**（`COMPAT_CONSTEXPR_*`, `COMPAT_NODISCARD`, 拆分命名空間, `compat::clamp`）。
  - **控制台輸出契約**（**優先使用 `compat::print` / `compat::println` 家族**，專案程式碼與測試嚴禁裸寫 `std::cout` / `printf`）。
  - **架構分層契約**（UI 與平台解耦，坐標空間隔離與 `CoordinateTransform`）。
- [**common_issues.md**](file:///D:/P/CPP/CPPUI/.gemini/common_issues.md)：
  - 專案歷次 5 大核心問題根因與防範：
    1. High-DPI 坐標系偏離與點擊失準 (High-DPI Hit-Test Mismatch)
    2. DirectWrite 高解析度 DPI 膨脹 (DirectWrite DPI Inflation)
    3. DirectWrite COM 原生物件跨棧釋放閃退 (DirectWrite COM Lifetime & Dangling Pointers)
    4. 文字光柵化負向邊界筆畫截斷 (Glyph Overhang Compensation)
    5. Windows PowerShell 5.1 腳本陷阱 (PowerShell 5.1 Pitfalls)

---

## 2. 契約執行優先級

當進行各項任務時，規則適用優先順序如下：
1. **使用者即時指令與特定契約要求**（最高優先級）。
2. **`.gemini/contracts.md`**：終端機執行、C++ 相容性、`compat::print` 規範與架構邊界。
3. **`.gemini/common_issues.md`**：避坑手冊與歷史架構決策不變量。
4. **專案規格書與設計文件 (`docs/specs/`)**。

請隨時參照上述文件，確保每次提交與變更均維持最高品質與向下相容性。
