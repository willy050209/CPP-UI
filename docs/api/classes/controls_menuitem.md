# MenuItem 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/MenuItem.hpp`](../../../include/ui/controls/MenuItem.hpp)
- **繼承關係**: 繼承自 [`controls::ContentControl`](controls_contentcontrol.md)

---

## 概述

`MenuItem` 是選單項目控制項，用於主選單列或 [`ContextMenu`](controls_contextmenu.md)。支援標題文字、右側快捷鍵提示字串（如 `"Ctrl+S"`）、MVVM [`ICommand`](../mvvm_icommand.md) 命令綁定、點擊事件回呼與巢狀次級選單 (Submenu) 階層級聯展開。

---

## 核心成員方法

### 標題與快捷鍵手勢
- `COMPAT_NODISCARD compat::string_view GetHeader() const noexcept`: 取得選單項目標題文字。
- `void SetHeader(compat::string_view header)`: 設定選單項目標題文字。
- `COMPAT_NODISCARD compat::string_view GetInputGestureText() const noexcept`: 取得快捷鍵手勢字串（如 `"Ctrl+C"`）。
- `void SetInputGestureText(compat::string_view text)`: 設定快捷鍵手勢提示文字。

### 命令綁定與回呼
- `COMPAT_NODISCARD std::shared_ptr<mvvm::ICommand> GetCommand() const noexcept`: 取得繫結之 MVVM 命令。
- `void SetCommand(std::shared_ptr<mvvm::ICommand> command)`: 繫結 MVVM 命令；點擊時自動檢查 `CanExecute` 並調用 `Execute`。
- `void OnClick(ClickHandler handler)`: 註冊項目點擊回呼常式 `void(MenuItem&)`.

### 巢狀次級選單 (Submenu)
- `void AddSubmenuItem(std::shared_ptr<MenuItem> item)`: 加入子選單項目，自動呈現向右開展箭頭標誌。
- `COMPAT_NODISCARD bool HasSubmenu() const noexcept`: 查詢是否包含子選單。
- `COMPAT_NODISCARD bool IsSubmenuOpen() const noexcept`: 查詢次級選單目前是否展開。
- `void SetIsSubmenuOpen(bool open)`: 程式化開展或關閉次級選單。
