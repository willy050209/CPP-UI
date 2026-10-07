# CoordinateTransform 類別 (cppui::core::CoordinateTransform)

封裝物理像素（Physical Pixels）與設備無關邏輯像素（Logical DIPs）間的雙向幾何轉換，提供型別安全、嚴格遵循 `compat.hpp` 契約的點、尺寸、矩形、外邊距與純量換算介面。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::core`](../namespaces/cppui_core.md)
* **標頭檔**：`#include <ui/core/CoordinateTransform.hpp>`
* **模組**：`CPPUI Core`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`
* **巨集標註**：`COMPAT_CONSTEXPR_14`, `COMPAT_NODISCARD`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace core {

class CoordinateTransform {
public:
    COMPAT_CONSTEXPR_14 explicit CoordinateTransform(float dpiScale = 1.0f) noexcept;

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetDpiScale() const noexcept;
    COMPAT_CONSTEXPR_14 void SetDpiScale(float dpiScale) noexcept;

    // 物理像素 -> 邏輯 DIP
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point ToLogical(Point physicalPoint) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size ToLogical(Size physicalSize) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect ToLogical(Rect physicalRect) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Thickness ToLogical(Thickness physicalThickness) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float ToLogical(float physicalValue) const noexcept;

    // 邏輯 DIP -> 物理像素
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point ToPhysical(Point logicalPoint) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size ToPhysical(Size logicalSize) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect ToPhysical(Rect logicalRect) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Thickness ToPhysical(Thickness logicalThickness) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float ToPhysical(float logicalValue) const noexcept;

    // 靜態無狀態輔助方法
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Point PhysicalToLogical(Point pt, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Point LogicalToPhysical(Point pt, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Size PhysicalToLogical(Size sz, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Size LogicalToPhysical(Size sz, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Rect PhysicalToLogical(Rect r, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Rect LogicalToPhysical(Rect r, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Thickness PhysicalToLogical(Thickness t, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Thickness LogicalToPhysical(Thickness t, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 float PhysicalToLogical(float val, float scale) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 float LogicalToPhysical(float val, float scale) noexcept;
};

} // namespace core
} // namespace cppui
```

---

## 簡介 (Description)

在現代高解析度顯示裝置（如 125%、150%、200% 縮放螢幕）上，物理像素與設備無關邏輯像素（DIP，以 96 DPI 為基準）之間存在縮放比例。
`CoordinateTransform` 提供集中的幾何轉換機制，徹底消除散落於視窗回呼、輸入派發與繪圖管線中的手動除法與乘法，防止浮點除零與點擊命中錯位。

### 核心設計原則
1. **數值穩定性與邊界安全**：
   - 當傳入的 `dpiScale <= 0.0f` 時，自動安全回退為 `1.0f`，防止浮點除零崩潰（Divide-by-zero）。
2. **往返換算恆等式**：
   - 對於任意幾何結構 $X$ 與縮放係數 $S > 0$，保證：
     $$\text{ToPhysical}(\text{ToLogical}(X)) \equiv X$$
3. **無狀態靜態支援**：
   - 提供 `PhysicalToLogical` 與 `LogicalToPhysical` 靜態輔助函式，支援臨時數值運算無需建立類別實例。
4. **C++14 常數表達式支援**：
   - 全方法均標註 `COMPAT_CONSTEXPR_14`，支援在編譯期進行幾何常數求解。

---

## 成員方法說明

### 建構式
```cpp
COMPAT_CONSTEXPR_14 explicit CoordinateTransform(float dpiScale = 1.0f) noexcept;
```
指定初始 DPI 縮放係數。若 `dpiScale <= 0.0f` 則自動回退為 `1.0f`。

### `GetDpiScale` / `SetDpiScale`
* `float GetDpiScale() const noexcept`：取得當前縮放係數（如 1.0f, 1.25f, 1.5f, 2.0f）。
* `void SetDpiScale(float dpiScale) noexcept`：更新縮放係數，小於等於 0 時安全回退為 1.0f。

### `ToLogical` (物理像素 -> 邏輯 DIP)
將物理像素空間的幾何物件換算為設備無關邏輯像素（除以 `dpiScale`）：
* `Point ToLogical(Point physicalPoint) const noexcept`
* `Size ToLogical(Size physicalSize) const noexcept`
* `Rect ToLogical(Rect physicalRect) const noexcept`
* `Thickness ToLogical(Thickness physicalThickness) const noexcept`
* `float ToLogical(float physicalValue) const noexcept`

### `ToPhysical` (邏輯 DIP -> 物理像素)
將邏輯 DIP 空間的幾何物件換算為物理像素（乘以 `dpiScale`）：
* `Point ToPhysical(Point logicalPoint) const noexcept`
* `Size ToPhysical(Size logicalSize) const noexcept`
* `Rect ToPhysical(Rect logicalRect) const noexcept`
* `Thickness ToPhysical(Thickness logicalThickness) const noexcept`
* `float ToPhysical(float logicalValue) const noexcept`

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/core/CoordinateTransform.hpp"

void DemonstrateCoordinateTransform() {
    using namespace cppui::core;

    // 模擬 150% DPI 螢幕環境
    CoordinateTransform transform(1.5f);

    // 視窗實體像素座標 (36, 120) 轉換為邏輯 DIP
    Point physicalPos(36.0f, 120.0f);
    Point logicalPos = transform.ToLogical(physicalPos);
    compat::println("邏輯 DIP 座標: ({}, {})", logicalPos.x, logicalPos.y); // (24, 80)

    // 邏輯排版邊界 (10, 20, 100, 50) 轉換為物理光柵化邊界
    Rect logicalBounds(10.0f, 20.0f, 100.0f, 50.0f);
    Rect physicalBounds = transform.ToPhysical(logicalBounds);
    compat::println("物理邊界: X={}, Y={}, W={}, H={}",
        physicalBounds.x, physicalBounds.y,
        physicalBounds.width, physicalBounds.height); // (15, 30, 150, 75)

    // 靜態方法快速換算
    float physicalWidth = CoordinateTransform::LogicalToPhysical(200.0f, 1.5f);
    compat::println("物理寬度: {}", physicalWidth); // 300.0
}
```

---

## 請參閱 (See Also)
* [`Point` 結構參考](core_point.md)
* [`Rect` 結構參考](core_rect.md)
* [`RawInputEvent` 結構參考](pal_rawinputevent.md)
* [高分屏 DPI 與座標轉換指南](../guides/dpi_and_coordinates.md)
