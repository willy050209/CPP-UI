# Control 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Control.hpp`](../../../include/ui/controls/Control.hpp)
- **繼承關係**: 繼承自 [`core::UIElement`](core_uielement.md)

---

## 概述

`Control` 是 CPPUI 中所有具備樣式、外觀邊界、事件互動與繪圖能力之 UI 控制項的統一基底類別。
本類別完整對標 **Windows Forms (`System.Windows.Forms.Control`)** 與 **WPF (`System.Windows.Controls.Control`)** 核心架構規格，提供識別名稱與標籤、文字協定、狀態與可用性、排版約束（`Anchor` / `Dock`）、幾何與客戶區邊界、字型環境屬性繼承（Ambient Font）、滑鼠游標、親緣樹與 Z-Order 調度、佈局掛起與批次計算、滑鼠捕獲、以及 GDI+ 風格 `CreateGraphics` 與 `OnPaint` 繪圖互操作介面。

---

## 列舉定義

### `AnchorStyles`
位元遮罩列舉，控制父容器大小變更時控制項邊界與容器邊緣的錨定行為：
- `None = 0`: 不進行錨定。
- `Top = 1`: 錨定至上緣。
- `Bottom = 2`: 錨定至下緣。
- `Left = 4`: 錨定至左緣。
- `Right = 8`: 錨定至右緣。
支援標準位元運算子 `|`, `&`, `^`, `~`。

### `DockStyle`
控制項在父容器內的貼齊停靠模式：
- `None = 0`: 不停靠。
- `Top = 1`: 停靠於頂端邊緣。
- `Bottom = 2`: 停靠於底端邊緣。
- `Left = 3`: 停靠於左側邊緣。
- `Right = 4`: 停靠於右側邊緣。
- `Fill = 5`: 填滿剩餘內部區域。

### `CursorType`
滑鼠懸停於此控制項時之游標樣式：
- `Default`, `Arrow`, `IBeam`, `Wait`, `Cross`, `Hand`, `No`, `SizeAll`, `SizeNS`, `SizeWE`, `SizeNESW`, `SizeNWSE`, `UpArrow`。

---

## 核心 API 群組

### 1. 識別與資料附加 (Identity & Tag)
- `compat::string_view GetName() const noexcept` / `void SetName(compat::string_view name)`: 取得或設定控制項識別名稱。
- `std::shared_ptr<void> GetTag() const noexcept` / `void SetTag(std::shared_ptr<void> tag)`: 取得或附加自訂資料指標。
- `template<typename T> std::shared_ptr<T> GetTag() const`: 樣板方法，型別安全地轉型並取得自訂資料。

### 2. 文字協定與多型同步 (Text Protocol)
- `virtual compat::string_view GetText() const noexcept`: 取得控制項文字標題，可被 `ContentControl`、`TextBox` 等子類別多型覆寫。
- `virtual void SetText(compat::string_view text)`: 設定控制項文字內容，自動觸發 `OnTextChangedCore` 與文字變更事件。

### 3. 可用狀態與可見性 (State & Availability)
- `virtual bool IsEnabled() const noexcept` / `virtual void SetEnabled(bool enabled)`: 取得或設定控制項可用狀態。停用時無法獲焦且自動刷新外觀。
- `bool IsVisible() const noexcept` / `void SetVisible(bool visible)`: 取得或設定控制項可見性（同步管理視覺樹與排版 `Visibility` 狀態）。
- `void Show()` / `void Hide()`: 快捷顯示與隱藏函式。

### 4. 幾何座標、邊界與客戶區 (Bounds & Client Area)
- `float GetLeft() const noexcept` / `void SetLeft(float left)`: X 軸座標。
- `float GetTop() const noexcept` / `void SetTop(float top)`: Y 軸座標。
- `float GetRight() const noexcept`: 右邊界座標 (`Left + Width`)。
- `float GetBottom() const noexcept`: 下邊界座標 (`Top + Height`)。
- `core::Point GetLocation() const noexcept` / `void SetLocation(core::Point loc)` / `void SetLocation(float x, float y)`: 取得或設定位置。
- `core::Size GetSize() const noexcept` / `void SetSize(core::Size sz)` / `void SetSize(float w, float h)`: 取得或設定尺寸。
- `core::Rect GetBounds() const noexcept` / `void SetBounds(float x, float y, float w, float h)` / `void SetBounds(const core::Rect& bounds)`: 取得或設定邊界矩形。
- `core::Rect GetClientRectangle() const noexcept`: 扣除內距與邊框後之局部客戶區矩形（以 `(0, 0)` 為原點）。
- `core::Size GetClientSize() const noexcept`: 扣除內距與邊框後之客戶區尺寸。

### 5. 排版約束 (Anchor & Dock)
- `AnchorStyles GetAnchor() const noexcept` / `void SetAnchor(AnchorStyles anchor)`: 取得或設定邊界錨定樣式。
- `DockStyle GetDock() const noexcept` / `void SetDock(DockStyle dock)`: 取得或設定邊界停靠模式。

### 6. 字型與環境屬性繼承 (Font & Ambient Font)
- `compat::string_view GetFontFamily() const noexcept` / `void SetFontFamily(compat::string_view family)`: 字型家族名稱（未設定時自動沿視覺樹向上繼承，預設為 `"Segoe UI"`）。
- `float GetFontSize() const noexcept` / `void SetFontSize(float size)`: 字型大小像素（未設定時向上繼承，預設為 `14.0f`）。
- `text::FontWeight GetFontWeight() const noexcept` / `void SetFontWeight(text::FontWeight weight)`: 字重。
- `text::FontStyle GetFontStyle() const noexcept` / `void SetFontStyle(text::FontStyle style)`: 傾斜度。
- `text::FontDescriptor GetFont() const` / `void SetFont(const text::FontDescriptor& font)`: 完整有效字型描述項。
- `bool HasExplicitFont() const noexcept`: 檢查本元件是否具備自訂顯式字型設定。

### 7. 滑鼠游標與外觀 (Cursor & Colors)
- `CursorType GetCursor() const noexcept` / `void SetCursor(CursorType cursor)`: 滑鼠游標樣式。
- `core::Color GetBackColor() const noexcept` / `void SetBackColor(core::Color c)`: WinForms 風格背景色別名。
- `core::Color GetForeColor() const noexcept` / `void SetForeColor(core::Color c)`: WinForms 風格前景色別名。

### 8. 視覺樹與親緣導航 (Hierarchy Navigation)
- `Control* GetParent() const noexcept`: 取得最近之父 Control 指標。
- `app::Form* FindForm() const noexcept`: 沿視覺樹向上搜尋承載本控制項之最頂層視窗表單。
- `Control* GetTopLevelControl() const noexcept`: 取得頂層根控制項。
- `bool Contains(const Control* control) const noexcept`: 檢查目標是否為自身之子控制項。
- `std::vector<Control*> GetChildControls() const`: 取得所有直接子控制項指標陣列。

### 9. Z-Order 圖層堆疊控制 (Z-Order Management)
- `void BringToFront()`: 將本控制項移至父容器子節點的最頂層（最前景）。
- `void SendToBack()`: 將本控制項移至父容器子節點的最底層（最背景）。

### 10. 焦點與滑鼠捕獲 (Focus & Pointer Capture)
- `bool Focus()` / `void Select()`: 使控制項請求輸入焦點。
- `bool ContainsFocus() const noexcept`: 檢查自身或任何子節點是否擁有鍵盤焦點。
- `bool CaptureMouse()`: 請求捕獲全域滑鼠指標訊息。
- `void ReleaseMouseCapture()`: 釋放滑鼠指標捕獲。
- `bool HasMouseCapture() const noexcept`: 檢查是否正捕獲滑鼠。

### 11. 佈局生命週期管理 (Layout Lifecycle)
- `void SuspendLayout() noexcept`: 暫停排版計算，計數加一。
- `void ResumeLayout(bool performLayout = true)`: 恢復排版計算；計數歸零時選擇性立即重新排版。
- `void PerformLayout()`: 強制立即觸發度量與排列。
- `bool IsLayoutSuspended() const noexcept`: 檢查當前是否處於掛起狀態。

### 12. 重繪與即時更新 (Repaint & Refresh)
- `void Invalidate()`: 標記整片區域為髒區並請求重繪。
- `void Invalidate(const core::Rect& rect)`: 標記局部髒區。
- `void Update()`: 即時更新渲染管線。
- `void Refresh()`: 強制立即重繪 (`Invalidate() + Update()`)。

### 13. 圖形互操作與 GDI+ 繪圖 (Graphics Interoperability)
- `std::unique_ptr<graphics::Graphics> CreateGraphics()`: 建立關聯於客戶區尺寸的獨立 Graphics 繪圖實例。
- `virtual void OnPaint(graphics::Graphics& g)`: GDI+ 風格自訂繪圖虛擬點，衍生控制項可直接覆寫此函式繪圖。
- `void OnRender(rendering::DrawingContext& dc) override`: 執行標準背景邊框繪圖並將 DrawingContext 轉譯為 Graphics 調用 `OnPaint` 與 Paint 回呼。

### 14. 座標轉換 (Coordinate Transformation)
- `core::Rect RectangleToScreen(const core::Rect& clientRect) const`: 客戶區矩形轉全域螢幕矩形。
- `core::Rect RectangleToClient(const core::Rect& screenRect) const`: 全域螢幕矩形轉客戶區局部矩形。

### 15. 事件監聽器 (Event Listeners)
- `OnClick(std::function<void()>)`: 點擊事件。
- `OnDoubleClick(std::function<void()>)`: 雙擊事件。
- `OnTextChanged(std::function<void(compat::string_view)>)`: 文字變更事件。
- `OnEnabledChanged(std::function<void(bool)>)`: 可用性變更事件。
- `OnVisibleChanged(std::function<void(bool)>)`: 可見性變更事件。
- `OnSizeChanged(std::function<void(core::Size)>)`: 尺寸變更事件。
- `OnLocationChanged(std::function<void(core::Point)>)`: 位置變更事件。
- `OnMouseEnter(std::function<void()>)` / `OnMouseLeave(std::function<void()>)`: 滑鼠進入/離開事件。
- `OnPaintHandler(std::function<void(graphics::Graphics&)>)`: 自訂 Paint 繪圖回呼。
