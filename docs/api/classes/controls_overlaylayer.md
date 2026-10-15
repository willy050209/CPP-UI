# OverlayLayer 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/OverlayLayer.hpp`](../../../include/ui/controls/OverlayLayer.hpp)
- **繼承關係**: 繼承自 [`layout::Panel`](../layout_panel.md)

---

## 概述

`OverlayLayer` 是視窗最頂層的懸浮容器面板。自動掛載於 `Form` 根視覺節點的最頂部，用於承載 [`Popup`](controls_popup.md)、[`ContextMenu`](controls_contextmenu.md)、[`ToolTip`](controls_tooltip.md) 與下拉清單等浮動控制項。具備點擊穿透機制：只有點擊到內部開啟的浮動元素時才會命中，未命中時則穿透至下層常規視窗內容。

---

## 核心成員方法

### 浮動子元件管理
- `void AddFloatingElement(std::shared_ptr<core::UIElement> element)`: 掛載浮動子元件（不可為 `nullptr`）。
- `void RemoveFloatingElement(const std::shared_ptr<core::UIElement>& element)`: 移除浮動子元件。
- `COMPAT_NODISCARD size_t GetFloatingElementCount() const noexcept`: 取得目前承載之浮動子元件數量。

### 命中測試與穿透
- `core::Visual* HitTest(core::Point localPoint) override`: 覆寫命中測試，僅在點擊落入可見浮動子元件時傳回該節點，否則傳回 `nullptr` 實現非干擾點擊穿透。

### 靜態祖先搜尋
- `static OverlayLayer* GetOverlayLayer(core::UIElement* element)`: 沿著視覺樹向上搜尋所屬表單之 `OverlayLayer` 實例。
