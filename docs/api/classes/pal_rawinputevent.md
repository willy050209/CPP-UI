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

    // 靜態工廠方法
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point logicalPos, core::Point physicalPos, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point logicalPos, core::Point physicalPos, RawMouseButton btn, int clicks, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point logicalPos, core::Point physicalPos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point logicalPos, core::Point physicalPos, int deltaY, int deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeResize(uint32_t w, uint32_t h, float dpi, uint64_t ts = 0) noexcept;
    COMPAT_NODISCARD static RawInputEvent MakeClose(uint64_t ts = 0) noexcept;
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

作業系統底層的原生訊息格式各異（例如 Win32 的 `LPARAM` 封裝游標座標，Cocoa 的 `NSEvent`）。`RawInputEvent` 將其標準化為單一的緊湊 C++ 結構。

### 雙重座標系統設計 (Logical vs Physical)
在高分屏（如 150%、200% DPI）環境下，滑鼠事件同時攜帶：
* **`logicalPos` (DIP)**：裝置獨立邏輯座標（像素數除以 DPI 縮放係數），供 UI 元件進行版面佈局與邊界判定。
* **`physicalPos`**：視窗客戶區的真實物理像素座標，供底層光柵化像素定位或除錯。

---

## 列舉型別說明

### `RawEventType`
表示事件的具體種類：
* 滑鼠：`MouseMove`, `MouseDown`, `MouseUp`, `MouseWheel`, `MouseEnter`, `MouseLeave`。
* 鍵盤與文字：`KeyDown`, `KeyUp`, `TextInput`。
* 視窗：`WindowResize`, `WindowMove`, `WindowClose`, `WindowFocusGained`, `WindowFocusLost`, `DpiChanged`。

### `KeyModifiers` 與位元運算
修飾鍵支援位元運算子 `|`、`&` 與輔助函式 `HasModifier`：
```cpp
KeyModifiers mods = KeyModifiers::Control | KeyModifiers::Shift;
if (HasModifier(mods, KeyModifiers::Control)) {
    // 按下了 Ctrl
}
```

---

## 內部資料載體結構

### `RawMouseEventData`
```cpp
struct RawMouseEventData {
    core::Point    position;         // 邏輯座標 (DIP)
    core::Point    physicalPosition; // 物理像素座標
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
#include "compat/compat.hpp"
#include "ui/pal/RawInputEvent.hpp"

void HandleEvent(const cppui::pal::RawInputEvent& e) {
    using namespace cppui::pal;

    switch (e.type) {
        case RawEventType::MouseMove:
            compat::println("滑鼠移動至邏輯點 ({}, {})", 
                e.mouse.position.x, e.mouse.position.y);
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
* [`IPlatformWindow` 介面參考](pal_iplatformwindow.md)
* [高分屏 DPI 與座標轉換指南](../guides/dpi_and_coordinates.md)
* [`cppui::pal` 命名空間](../namespaces/cppui_pal.md)
