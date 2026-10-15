# ContextMenu 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ContextMenu.hpp`](../../../include/ui/controls/ContextMenu.hpp)
- **繼承關係**: 繼承自 [`controls::Popup`](controls_popup.md)

---

## 概述

`ContextMenu` 是右鍵上下文彈出選單控制項。內部封裝縱向項目清單與 [`Popup`](controls_popup.md) 浮層，支援 [`MenuItem`](controls_menuitem.md) 集合管理、滑鼠懸停導向、巢狀次選單 (Submenu) 展開、邊界自動鏡像翻轉與外部點擊關閉 (Light-Dismiss)。

---

## 核心成員方法

### 項目集合操作
- `void AddMenuItem(std::shared_ptr<MenuItem> item)`: 新增選單項目（不可為 `nullptr`）。
- `void RemoveMenuItem(const std::shared_ptr<MenuItem>& item)`: 移除指定之選單項目。
- `void ClearMenuItems()`: 清空選單內的所有項目。
- `COMPAT_NODISCARD size_t GetMenuItemCount() const noexcept`: 取得選單項目總數。
- `COMPAT_NODISCARD std::shared_ptr<MenuItem> GetMenuItem(size_t index) const`: 依索引取得選單項目。

### 彈出與顯示控制
- `void ShowAt(core::UIElement* target, core::Point offset)`: 於指定目標元件的相對位置彈出選單。
- `void Close()`: 關閉選單並自動收合其開啟的所有次級子選單。

---

## 使用範例

```cpp
#include "ui/controls/ContextMenu.hpp"
#include "ui/controls/MenuItem.hpp"

auto menu = std::make_shared<cppui::controls::ContextMenu>();

auto cutItem = std::make_shared<cppui::controls::MenuItem>("剪下");
cutItem->SetInputGestureText("Ctrl+X");
cutItem->OnClick([](cppui::controls::MenuItem&) {
    compat::println("執行剪下");
});

auto copyItem = std::make_shared<cppui::controls::MenuItem>("複製");
copyItem->SetInputGestureText("Ctrl+C");

menu->AddMenuItem(cutItem);
menu->AddMenuItem(copyItem);

// 於某個控制項上彈出
menu->ShowAt(myButton.get(), cppui::core::Point(0.0f, myButton->GetActualSize().height));
```
