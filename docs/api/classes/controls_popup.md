# Popup 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Popup.hpp`](../../../include/ui/controls/Popup.hpp)
- **繼承關係**: 繼承自 [`controls::ContentControl`](controls_contentcontrol.md) 並實作 [`input::ILightDismissable`](../input_routedevent.md)

---

## 概述

`Popup` 是頂層浮動彈出容器控制項。透過 [`OverlayLayer`](controls_overlaylayer.md) 於全視窗最頂層呈現內容，免受父容器之邊界裁切（Clipping）。具備邊界自動鏡像翻轉（Auto-flip）、多種停靠定位模式（`PlacementMode`）、錨定目標關聯與 Light-Dismiss（外部點擊或 ESC 鍵自動收合）支援。

---

## PlacementMode 列舉

| 列舉值 | 說明 |
| :--- | :--- |
| `PlacementMode::Bottom` | 預設：停靠於錨定目標正下方。 |
| `PlacementMode::Top` | 停靠於錨定目標正上方。 |
| `PlacementMode::Left` | 停靠於錨定目標左側。 |
| `PlacementMode::Right` | 停靠於錨定目標右側。 |
| `PlacementMode::Mouse` | 停靠於滑鼠游標所在座標。 |
| `PlacementMode::Custom` | 依據 `HorizontalOffset` 與 `VerticalOffset` 自訂定位。 |

---

## 核心成員方法

### 開啟與收合控制
- `COMPAT_NODISCARD bool IsOpen() const noexcept`: 查詢浮層是否處於開啟狀態。
- `void SetIsOpen(bool value)`: 設定開啟或關閉浮層。
- `COMPAT_NODISCARD bool StaysOpen() const noexcept`: 查詢是否鎖定常開。
- `void SetStaysOpen(bool value) noexcept`: 若設為 `false`，則點擊浮層外部或按下 ESC 鍵時自動關閉。
- `void OnOpened(PopupEventHandler handler)`: 註冊開啟後回呼。
- `void OnClosed(PopupEventHandler handler)`: 註冊關閉後回呼。

### 錨定目標與定位
- `COMPAT_NODISCARD core::UIElement* PlacementTarget() const noexcept`: 取得錨定目標元件。
- `void SetPlacementTarget(core::UIElement* target) noexcept`: 設定錨定目標元件。
- `COMPAT_NODISCARD PlacementMode GetPlacement() const noexcept`: 取得停靠定位模式。
- `void SetPlacement(PlacementMode mode) noexcept`: 設定停靠定位模式。
- `void SetHorizontalOffset(float offset) noexcept`: 設定 X 軸偏移。
- `void SetVerticalOffset(float offset) noexcept`: 設定 Y 軸偏移。
