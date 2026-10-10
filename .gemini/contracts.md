# CPPUI 專案核心契約與開發規範 (Project Contracts)

本文件定義 CPPUI 專案所有開發、終端機執行、C++ 相容性、控制台日誌以及架構分層的強制契約。
所有 Agent 與開發者必須嚴格遵守，任何偏離均視為違規。

---

## 1. Windows PowerShell 5.1 終端機執行契約 (Terminal Execution Rules)

本機環境運行 **Windows 11**，預設命令解譯器為 **Windows PowerShell 5.1 (Desktop Edition)**（未安裝 PowerShell Core `pwsh`）。
必須嚴格遵守以下指令撰寫規範，以防止語法崩潰與指令衝突：

### 1.1 嚴禁 `&&` 與 `||` 管線串接
- 在 Windows PowerShell 5.1 中，`&&` 與 `||` 為非法語彙，會觸發致命語法錯誤：
  `The token '&&' is not a valid statement separator in this version`。
- **必要作法**：
  - 優先拆分為獨立的指令呼叫。
  - 單行依序執行一律使用分號 `;`：
    ```powershell
    step1; step2
    ```
  - 條件相依執行（等同 `&&`）強制使用 `$?` 判斷：
    ```powershell
    step1; if ($?) { step2 }
    ```
  - 失敗備援執行（等同 `||`）強制使用 `-not $?` 判斷：
    ```powershell
    step1; if (-not $?) { fallback }
    ```

### 1.2 外部執行檔必須帶副檔名 `.exe` (別名衝突保護)
PowerShell 5.1 內建將多個 Linux 風格指令名稱綁定至內部 Cmdlet，若不加副檔名會導致參數解析失敗或非預期行為：
- **`curl`**：內建別名至 `Invoke-WebRequest`。**必須寫作 `curl.exe`**。
- **`wget`**：內建別名至 `Invoke-WebRequest`。**必須寫作 `curl.exe` 或 `Invoke-WebRequest`**。
- **`sc`**：內建別名至 `Set-Content`。服務管理**必須寫作 `sc.exe`**。
- **`where`**：內建別名至 `Where-Object`。路徑尋找**必須寫作 `where.exe`**（嚴禁寫 `which`）。
- **`git` / `powershell`**：建議一律寫作 `git.exe` 與 `powershell.exe`。

### 1.3 環境變數必須使用 `$env:VAR_NAME`
- 嚴禁 Unix 風格 `$NAME`（會被展開為未設定的空 PowerShell 區域變數）。
- 嚴禁 CMD 風格 `%NAME%`（會導致語法錯誤或字面值解析）。
- **正確語法**：`$env:USERPROFILE`, `$env:PATH`, `$env:TEMP`。
- **設定變數**：`$env:MY_VAR = "value"`。

### 1.4 嚴禁行尾 `&` 作為背景執行
- PowerShell 中 `&` 為**呼叫運算子 (Call Operator)**（如 `& "C:\tool.exe"`），不是背景執行運算子。
- 行尾放置 `&` 會觸發語法錯誤。背景執行請依賴工具內建參數或 `Start-Process` / `Start-Job`。

### 1.5 跳脫字元與引號規範
- PowerShell 的跳脫字元是**反引號** (`` ` ``)，**不是反斜線** (`\`)。
- 單引號 (`'...'`) 為純字面值（不展開任何變數或表達式）。
- 雙引號 (`"..."`) 允許變數插入 (`$var`) 與指令替換 (`$()`)。

---

## 2. C++ 向下相容性與巨集規範 (compat.hpp)

為確保專案能在 C++14、C++17、C++20 與 C++23 等多標準及 MSVC、Clang++、G++ 三大編譯器間無縫編譯，所有程式碼必須遵循以下相容性巨集契約：

### 2.1 `constexpr` 規範
- C++14 合法之函式/方法（包含條件分支、迴圈、區域變數）：**強制使用 `COMPAT_CONSTEXPR_14`**。
- C++17 合法之函式：**強制使用 `COMPAT_CONSTEXPR_17`**。
- C++20 合法之函式：**強制使用 `COMPAT_CONSTEXPR_20`**。

### 2.2 `inline` 與常數宣告規範
- 標頭檔全域/命名空間常數：**強制使用 `COMPAT_INLINE_VAR constexpr`**（嚴禁裸寫 `inline constexpr`）。
- 效能關鍵微架構路徑：**強制使用 `COMPAT_ALWAYS_INLINE`**。
- 純查詢/計算方法：**優先標註 `COMPAT_NODISCARD`**。

### 2.3 `static` 與私有輔助規範
- 檔案內部輔助函式一律使用 `static` 或匿名命名空間，避免符號外溢與多重定義。

### 2.4 命名空間巢狀規範
- **嚴禁使用 C++17 巢狀命名空間 `namespace A::B { ... }`**。
- **強制拆分寫作**：
  ```cpp
  namespace A {
  namespace B {
      // 實作內容
  } // namespace B
  } // namespace A
  ```

### 2.5 演算法規範
- **嚴禁直接呼叫 C++17 `std::clamp`**，一律使用 `compat::clamp`。

### 2.6 現代化視圖與借用語意優先規範 (Modern Views & Borrowing)
- **字串視圖首選**：所有唯讀字串參數**強制使用 `compat::string_view`**；嚴禁使用 `const std::string&` 或 `const char* + size_t`。
- **緩衝區切片首選**：連續記憶體區塊（頂點、指令流、字符、像素）**強制使用 `compat::span<T>` / `compat::span<const T>`**；嚴禁使用裸指標加長度 (`const T*, size_t`) 或硬綁定 `const std::vector<T>&`。
- **集合處理優先走管線**：集合篩選、投影與截取優先採用 `compat::views::*` 搭配 `operator|`（如 `filter`, `transform`, `take`, `as_const`），實現零拷貝惰性評估，嚴禁手寫中繼 `std::vector`。
- **生命週期保護**：視圖僅為不具所有權之借用觀察者，嚴禁回傳或持有指向臨時右值容器（Rvalue Container）的懸空視圖。

---

## 3. 控制台診斷輸出契約 (Console Output & Logging)

- **核心原則：優先使用 `compat::print` / `compat::println` 家族**。
- 專案所有核心模組、測試案例、示範程式中的控制台日誌輸出，**強制使用 `compat::print` 或 `compat::println`**。
- **嚴禁在程式碼中裸寫 `std::cout` 或 `printf`**，確保跨平台跨編譯器字元編碼（UTF-8）與標準庫相容性。

---

## 4. 架構分層與職責隔離契約 (Architecture & Layering Rules)

依據 `docs/specs/` 核心規範：

### 4.1 UI 與平台完全解耦
- UI 核心邏輯（Layout, Visual, UIElement, RoutedEvents, Controls）**絕對禁止直接包含或呼叫 OS 原生視窗 API**（如 `<windows.h>`, X11, Cocoa）。
- 一切平台視窗建立、訊息循環、事件轉換由 PAL（`IPlatformWindow`, `PlatformFactory`, `RawInputEvent`）負責。

### 4.2 坐標空間隔離與轉換契約
- **排版空間 (Client Layout Coordinates)**：視覺樹 `Visual`、`UIElement::m_visualBounds` 與佈局容器一律基於視窗客戶區排版坐標系。
- **物理像素 (Physical Pixels)**：底層視窗客戶區點陣與光柵化緩衝區（如 `SoftwareRenderTarget`）大小。
- **設備無關邏輯像素 (Logical DIP)**：以 96 DPI 為基準（$1.0\times$ 縮放）之設備無關單位。
- **坐標轉換契約**：
  - 物理與邏輯間的幾何轉換必須由獨立類別 `cppui::core::CoordinateTransform` 集中負責。
  - 嚴禁在業務邏輯或事件回呼中散落任意除以或乘以 DPI 縮放係數的行為。
  - `RawMouseEventData` 嚴格區分：
    - `position`：視窗客戶區排版坐標（與視覺樹佈局對齊）。
    - `physicalPosition`：物理像素坐標。
    - `logicalPosition`：邏輯 DIP 坐標。

---

## 5. 零編譯警告與零錯誤契約 (Zero Compiler Warnings & Errors) [Strict]

1. **零警告容忍 (Zero Warnings Policy)**：所有建置與編譯（MSVC `/W4`、GCC `-Wall -Wextra -Wpedantic`、Clang `-Wall -Wextra -Wpedantic`）必須達成 **0 Errors / 0 Warnings**。
2. **預防非預期警告**：
   - **Release / NDEBUG 未使用變數防護**：若變數僅在 `assert(...)` 斷言中使用，在 Release / NDEBUG 模式下斷言被移除時會觸發 `-Wunused-variable` 或 `-Wunused-but-set-variable`，必須在斷言後顯式加上 `(void)variable;`。
   - **基底類別初值列**：繼承 `std::enable_shared_from_this<T>` 或其他基底類別之自訂拷貝/移動建構子，初值列必須顯式初始化基底類別。
   - **未使用參數標註**：公開介面或回呼實作中因簽章限制而未使用的參數，應註解參數名稱（例如 `Size /*availableSize*/`）或加上 `(void)param;`。
   - **型別轉換精度截斷**：避免平台無關型別與整數/浮點數間的隱式截斷警告，一律顯式進行靜態轉型（`static_cast<T>`）。
3. **編譯驗證強制令**：任何程式碼修改後，必須於本地或 WSL 環境重新編譯，徹底確認輸出為 **0 warnings, 0 errors**。
