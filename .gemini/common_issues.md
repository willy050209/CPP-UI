# CPPUI 常見問題與避坑知識庫 (Common Issues Knowledge Base)

本文件收錄 CPPUI 專案歷次開發過程中出現的 **5 大核心高頻問題**，詳細記錄其問題現象、根本原因剖析、架構不變量與長效防範方案。所有參與本專案的 Agent 與工程師在進行相關模組修改時必須查閱並嚴格遵守。

---

## 1. High-DPI 坐標系偏離與點擊失準 (High-DPI Hit-Test Mismatch)

### 1.1 現象 (Symptom)
- 在 Windows 系統顯示縮放設定為非 100%（例如 125%、150%、200%）的螢幕上：
  - 滑鼠游標懸停於按鈕上方時，無法觸發 Hover 高亮邊框，或者游標移到按鈕外側某個偏移位置時按鈕反而高亮。
  - 點擊按鈕沒有反應，或者必須點擊偏離按鈕一段距離的空白區域才會觸發 Click 事件。
  - 此問題在 100% 顯示縮放的環境下**完全無法察覺**，具備高度隱蔽性。

### 1.2 根本原因 (Root Cause)
1. **視窗客戶區與排版空間一致**：
   - 應用程式（如 `m5_input_demo`）建立視窗大小為 $W \times H$（如 $1180 \times 780$ 客戶區像素）。
   - 佈局系統（`LayoutManager::UpdateLayout`）以該客戶區尺寸進行 Arrange，按鈕排版在客戶區排版坐標 $(36, 120)$。
   - 光柵化緩衝區（`SoftwareRenderTarget`）大小為 $1180 \times 780$，`StretchDIBits` 以 $1:1$ 比例輸出至視窗客戶區。視覺上按鈕位於視窗客戶區的 $(36, 120)$。
2. **舊 PAL 層過早且錯誤地進行縮放換算**：
   - 舊版 `PlatformWindowWin32.cpp` 在收到 `WM_MOUSEMOVE` 或 `WM_LBUTTONDOWN` 時，取得原生坐標 $(36, 120)$ 後，直接執行了 `x / m_dpiScale`（在 150% 縮放下除以 1.5），算出 $(24, 80)$ 並填入 `mouse.position`。
3. **HitTest 比對脫節**：
   - `InputManager` 調用 `root.HitTest(mouse.position)`，將傳入的 $(24, 80)$ 與排版在 $(36, 120)$ 的按鈕邊界比對，判定點落在按鈕外部，導致點擊命中完全落空！

### 1.3 治本架構方案 (Solution & Invariant)
- **空間語意分離**：
  - **視窗客戶區排版空間 (Client Layout Space)**：與視覺樹 (`Visual`)、佈局容器同構，單位為客戶區像素。
  - **物理像素空間 (Physical Pixels)**：視窗實際硬體呈現像素。
  - **邏輯 DIP 空間 (Logical DIP)**：以 96 DPI 為基準的無關設備像素。
- **引入專屬 `CoordinateTransform` 類別**：
  - 所有的空間換算交由 `cppui::core::CoordinateTransform`。
  - `RawMouseEventData` 嚴格定義：
    - `position`：始終為**視窗客戶區排版坐標**（與視覺樹直接對齊，供 `HitTest` 與路由事件使用）。
    - `physicalPosition`：物理像素坐標。
    - `logicalPosition`：由 `m_coordTransform.ToLogical()` 計算出的邏輯 DIP 坐標。
- **嚴禁行為**：嚴禁在 PAL 事件回呼中私自除以 `m_dpiScale` 污染 `mouse.position`。

---

## 2. DirectWrite 高解析度 DPI 膨脹導致字卡溢出 (DirectWrite DPI Inflation)

### 2.1 現象 (Symptom)
- 在 150% 或更高 DPI 環境下，UI 元件內的文字異常巨大，文字溢出卡片、按鈕或邊界框，甚至遮擋鄰近元件。

### 2.2 根本原因 (Root Cause)
- DirectWrite API（如 `CreateTextFormat`）原生接受的字型大小（`fontSize`）是以 DIP (1/96 英吋) 為單位。
- 若在上層排版測量或字型定義階段已經乘以 `dpiScale`（例如 $14 \times 1.5 = 21$），然後在傳遞給 DirectWrite 時又設定了 RenderTarget 的 DPI 縮放係數（再次放大 $1.5$ 倍），文字實際光柵化尺寸會變成 $14 \times 1.5 \times 1.5 = 31.5$（二次放大 $\text{DPI}^2$），導致文字幾何暴增。

### 2.3 治本架構方案 (Solution & Invariant)
- **單一職責縮放原則**：
  - UI 樣式系統中定義的 `FontSize` 一律為**邏輯點數 (Logical DIP)**。
  - 在全流程渲染管線中，確定單一縮放層級：若 RenderTarget 或後端已套用整體畫布 DPI 縮放矩陣，則 TextFormat 必須傳遞未縮放的邏輯字級；若採用純軟體手動光柵化，則僅在最終產生 GlyphRun 或像素點陣時換算一次物理像素。
  - 嚴禁在 Layout 測量、Format 建立與 Render 繪製三處重複乘算 DPI。

---

## 3. DirectWrite COM 原生物件跨棧釋放閃退 (DirectWrite COM Lifetime & Dangling Pointers)

### 3.1 現象 (Symptom)
- 程式在快速重繪、文字變更、視窗縮放或退出時，拋出記憶體存取違規（Access Violation `0xC0000005`）。

### 3.2 根本原因 (Root Cause)
- Windows DirectWrite 核心介面（`IDWriteFactory`, `IDWriteTextFormat`, `IDWriteTextLayout`, `IDWriteFontCollection`）為 Win32 COM 元件，內部依賴嚴格的引用計數管理。
- 若直接使用裸指標（`IDWriteTextFormat*`），在物件複製或解構時容易發生 Double-Free 或懸空指標（Dangling Pointer）。此外，`TextLayout` 內部依賴於其建立時的 `TextFormat` 與 `Factory`，若父工廠先於子版面配置釋放，解構子版面配置時將引發崩潰。

### 3.3 治本架構方案 (Solution & Invariant)
- **RAII 與智慧指標**：
  - 專案內部一律使用具備引用計數管理的 COM 封裝（如 `Microsoft::WRL::ComPtr<T>` 或具備自動 `Release()` 的 RAII 包裝器）。
  - 生命週期拓撲由長至短依序為：`IDWriteFactory`（全域/長期存活） $\rightarrow$ `IDWriteTextFormat`（樣式快取） $\rightarrow$ `IDWriteTextLayout`（單次測量/重繪）。
  - 嚴禁跨越類別邊界傳遞未增長引用計數的裸 COM 指標。

---

## 4. 文字光柵化負向邊界筆畫截斷 (Glyph Overhang Compensation)

### 4.1 現象 (Symptom)
- 渲染斜體字（Italic）、具有外延飾線（Serif）或特殊西文字母（如 'f', 'j', 'g', 'p'）時，字母左右或上下外緣被硬生生切掉一條黑線或留白。

### 4.2 根本原因 (Root Cause)
- DirectWrite 的 `IDWriteTextLayout::GetMetrics` 返回的 `DWRITE_TEXT_METRICS` 僅代表黑體基準排版方塊（Layout Bounds）。
- 斜體傾斜筆畫或特殊字形會延伸超出這個方塊。DirectWrite 提供了 `IDWriteTextLayout::GetOverhangMetrics` 來回報延伸量（`left`, `top`, `right`, `bottom` overhang）。
- 若光柵化緩衝區或繪圖裁剪矩形（Clip Rect）僅嚴格設定為 `metrics.width` 與 `metrics.height`，超出排版方塊的延伸筆畫就會被裁剪截斷。

### 4.3 治本架構方案 (Solution & Invariant)
- 在計算文字光柵化點陣圖大小或繪圖邊界時，調用 `GetOverhangMetrics`：
  $$\text{RenderWidth} = \text{metrics.width} + \text{overhang.left} + \text{overhang.right}$$
  $$\text{RenderHeight} = \text{metrics.height} + \text{overhang.top} + \text{overhang.bottom}$$
- 光柵化時提供邊界安全補償 padding，並在繪圖原點加入對應 offset，徹底消除文字截斷。

---

## 5. Windows PowerShell 5.1 腳本陷阱 (PowerShell 5.1 Pitfalls)

### 5.1 現象 (Symptom)
- CI 矩陣測試或本地執行指令時：
  - 出現語法錯誤 `The token '&&' is not a valid statement separator in this version`。
  - 當搜尋結果或輸出只有 1 個元素時，呼叫 `.Count` 報錯或回傳空，導致迴圈崩潰。
  - 包含引號的路徑在傳遞給編譯器時反斜線被吞掉或字串未閉合。
  - 指令呼叫 `curl` 報錯參數無效。

### 5.2 根本原因 (Root Cause)
1. **PowerShell 5.1 缺少現代語法**：不支持 POSIX 鏈接符號 `&&` 與 `||`。
2. **純量與陣列自動解包 (Scalar Unwrapping)**：PowerShell 在管線輸出 1 個物件時會自動將其退化為純量物件（非陣列），若輸出 0 個物件則為 `$null`。在未啟用 strict 模式下存取屬性容易引發例外。
3. **別名污染**：`curl`, `wget`, `sc`, `where` 均為內建 Cmdlet 別名。
4. **反斜線轉義誤解**：PowerShell 的轉義符號為反引號 `` ` ``，若在字串結尾寫 `\"` 會將引號轉義導致未閉合。

### 5.3 治本防範方案 (Solution & Invariant)
- **鏈接替換**：一律使用 `; if ($?) { ... }` 代替 `&&`。
- **強制作為陣列處理**：涉及集合變數宣告或結果接收一律使用 `@(...)` 包裹，例如 `@(Get-ChildItem ...)`。
- **二進位加 `.exe`**：外部指令一律使用 `curl.exe`, `where.exe`, `git.exe`。
- **路徑正斜線優先**：在引號傳遞參數時優先使用正斜線 `/`（如 `-Iinclude/ui`），避免反斜線引號衝突。
