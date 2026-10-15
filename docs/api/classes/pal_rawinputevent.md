# RawInputEvent 與列舉結構 (cppui::pal::RawInputEvent)

定義跨平台統一的原生輸入事件模型，包含滑鼠、鍵盤、視窗幾何與 DPI 變更事件。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::pal`](../namespaces/cppui_pal.md)
* **標頭檔**：`#include <ui/pal/RawInputEvent.hpp>`
* **模組**：`CPPUI PAL`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace pal {

enum class RawEventType : uint16_t {
    None = 0,
    MouseMove, MouseDown, MouseUp, MouseWheel, MouseEnter, MouseLeave,
    KeyDown, KeyUp, TextInput,
    WindowResize, WindowMove, WindowClose, WindowFocusGained, WindowFocusLost, DpiChanged
};

enum class RawMouseButton : uint8_t {
    None = 0, Left = 1, Right = 2, Middle = 3, XButton1 = 4, XButton2 = 5
};

enum class KeyModifiers : uint8_t {
    None = 0, Control = 1 << 0, Shift = 1 << 1, Alt = 1 << 2, Meta = 1 << 3, CapsLock = 1 << 4
};

struct RawInputEvent {
    RawEventType  type = RawEventType::None;
    uint64_t      timestampMs = 0;
    KeyModifiers  modifiers = KeyModifiers::None;

    union {
        RawMouseEventData  mouse;
        RawKeyEventData    key;
        RawTextInputData   text;
        RawWindowEventData window;
    };

    // 靜態工廠方法 (支援 clientPos, physicalPos, logicalPos)
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point clientPos, core::Point physicalPos, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point pos, KeyModifiers mods, uint64_t ts = 0) noexcept;

    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, RawMouseButton btn, int clicks, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point clientPos, core::Point physicalPos, RawMouseButton btn, int clicks, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point pos, RawMouseButton btn, int clicks, KeyModifiers mods, uint64_t ts = 0) noexcept;

    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point clientPos, core::Point physicalPos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point pos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept;

    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, int deltaY, int deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point clientPos, core::Point physicalPos, int deltaY, int deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point pos, int deltaY, int deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept;

    COMPAT_NODISCARD static RawInputEvent MakeResize(uint32_t w, uint32_t h, float dpi, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeClose(uint64_t ts = 0) noexcept;
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

作業系統底層的原生訊息格式各異（例如 Win32 的 `LPARAM` 封裝游標座標，Cocoa 的 `NSEvent`）。`RawInputEvent` 將其標準化為單一的緊湊 C++ 結構。

### 三重座標空間設計 (Client Layout vs Physical vs Logical)
在高分屏（如 125%、150%、200% DPI）環境下，滑鼠事件嚴格分離空間定義：
* **`position` (客戶區排版座標)**：相對於視窗客戶區的排版像素坐標，與視覺樹 [`Visual`](core_visual.md) 的佈局邊界 1:1 精確對齊，保證命中測試 0 像素偏差。
* **`physicalPosition`**：相對於視窗客戶區的原生物理像素坐標，供底層光柵化緩衝區定位或調試。
* **`logicalPosition` (DIP)**：透過 [`CoordinateTransform`](core_coordinatetransform.md) 換算之設備無關邏輯坐標（物理像素除以 `dpiScale`）。

---

## 內部資料載體結構

### `RawMouseEventData`
```cpp
struct RawMouseEventData {
    core::Point    position;         // 客戶區排版座標 (與 Visual 佈局對齊)
    core::Point    physicalPosition; // 物理像素座標 (Physical Pixels)
    core::Point    logicalPosition;  // 邏輯坐標 (DIP: Device Independent Pixels)
    RawMouseButton button;           // 觸發按鍵
    int32_t        wheelDeltaX;      // 水平滾輪量
    int32_t        wheelDeltaY;      // 垂直滾輪量 (正值向上，負值向下)
    int32_t        clickCount;       // 單擊 (1), 雙擊 (2)
};
```

### `RawWindowEventData`
```cpp
struct RawWindowEventData {
    uint32_t width;    // 客戶區物理像素寬度
    uint32_t height;   // 客戶區物理像素高度
    float    dpiScale; // DPI 縮放係數 (如 1.0, 1.25, 1.5, 2.0)
};
```

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/pal/RawInputEvent.hpp"

void HandleEvent(const cppui::pal::RawInputEvent& e) {
    using namespace cppui::pal;

    switch (e.type) {
        case RawEventType::MouseMove:
            compat::println("排版坐標 ({}, {}) | 邏輯 DIP ({}, {})", 
                e.mouse.position.x, e.mouse.position.y,
                e.mouse.logicalPosition.x, e.mouse.logicalPosition.y);
            break;

        case RawEventType::MouseDown:
            if (e.mouse.button == RawMouseButton::Left) {
                if (e.mouse.clickCount == 2) {
                    compat::println("滑鼠左鍵雙擊！");
                }
            }
            break;

        case RawEventType::DpiChanged:
            compat::println("DPI 縮放改變為: {}%", 
                static_cast<int>(e.window.dpiScale * 100));
            break;

        default:
            break;
    }
}
```

---

## 請參閱 (See Also)
* [`CoordinateTransform` 類別參考](core_coordinatetransform.md)
* [`IPlatformWindow` 介面參考](pal_iplatformwindow.md)
* [高分屏 DPI 與座標轉換指南](../guides/dpi_and_coordinates.md)
* [`cppui::pal` 命名空間](../namespaces/cppui_pal.md)
