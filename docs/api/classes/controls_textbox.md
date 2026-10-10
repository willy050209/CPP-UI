# TextBox 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/TextBox.hpp`](../../../include/ui/controls/TextBox.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`TextBox` 是單行文字編輯輸入控制項。支援滑鼠點擊焦點定位、字元打字輸入、Backspace 退格與 Delete 刪除（支援多位元組 UTF-8 安全字元解碼）、方向鍵左右游標移動、Home / End 快速定位、佔位提示文字（Placeholder）、文字超出邊界局部剪裁（Clip）、垂直垂直置中文字排版與獲焦外框光暈。

---

## 靜態路由事件

- `static const input::RoutedEvent TextBox::TextChangedEvent`: 當文字內容發生任何增刪修改時觸發（`RoutingStrategy::Bubble`）。

---

## 核心成員方法

### 文字與佔位提示
- `COMPAT_NODISCARD compat::string_view GetText() const noexcept`: 取得目前已輸入之文字視圖。
- `void SetText(compat::string_view text)`: 設定文字內容，自動更新游標位置並觸發排版與重繪。
- `void InsertText(compat::string_view text)`: 於目前插入游標索引處插入文字。
- `COMPAT_NODISCARD compat::string_view GetPlaceholder() const noexcept`: 取得目前佔位提示文字。
- `void SetPlaceholder(compat::string_view placeholder)`: 設定當文字為空時顯示的佔位提示。
- `COMPAT_NODISCARD core::Color GetPlaceholderColor() const noexcept / void SetPlaceholderColor(core::Color color)`: 取得或設定佔位提示文字色彩。

### 游標與字型
- `COMPAT_NODISCARD size_t GetCaretIndex() const noexcept / void SetCaretIndex(size_t index)`: 取得或設定目前插入游標字元索引。
- `COMPAT_NODISCARD float GetFontSize() const noexcept / void SetFontSize(float size)`: 取得或設定字體大小。
- `COMPAT_NODISCARD compat::string_view GetFontFamily() const noexcept / void SetFontFamily(compat::string_view family)`: 取得或設定字體系列。

### 唯讀與長度限制
- `COMPAT_NODISCARD bool IsReadOnly() const noexcept / void SetReadOnly(bool readOnly) noexcept`: 取得或設定文字輸入框是否為唯讀模式。
- `COMPAT_NODISCARD size_t GetMaxLength() const noexcept / void SetMaxLength(size_t maxLen)`: 取得或設定最大輸入字元長度限制（0 代表無長度上限）。

### 內容清除與文字選取
- `void Clear()`: 清空文字框內的所有文字，並將游標重置至開頭。
- `void SelectAll()`: 全選文字輸入框內的所有字元。
- `void Select(size_t start, size_t length)`: 設定選取範圍起點與字元長度。
- `COMPAT_NODISCARD size_t GetSelectionStart() const noexcept`: 取得目前選取區段之起始索引。
- `COMPAT_NODISCARD size_t GetSelectionLength() const noexcept`: 取得目前選取區段之字元長度。
- `COMPAT_NODISCARD std::string GetSelectedText() const`: 取得目前選取反白區段之文字副本。

### 密碼掩碼模式
- `COMPAT_NODISCARD bool IsPassword() const noexcept / void SetIsPassword(bool isPassword)`: 取得或設定是否啟用密碼遮罩隱藏模式。
- `COMPAT_NODISCARD char GetPasswordChar() const noexcept / void SetPasswordChar(char ch)`: 取得或設定密碼掩碼字元（預設為 `'*'`）。

### 事件監聽捷徑
- `void OnTextChanged(std::function<void(compat::string_view)> handler)`: 註冊文字內容變更監聽函式。

### 命中測試與繪製
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製背景與外框、設定內縮剪裁區域、繪製排版文字（或密碼遮罩符號）或佔位符，並在控制項獲焦時於字元邊界繪製垂直插入游標。

