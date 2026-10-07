# PointerEventArgs 類別 (cppui::input::PointerEventArgs)

封裝滑鼠與觸控指標相關之路由事件參數，攜帶相對於目標節點與視窗的幾何座標、點擊按鍵、修飾鍵旗標、雙擊計數與滾輪偏移量。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/PointerEventArgs.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

class PointerEventArgs : public RoutedEventArgs {
public:
    PointerEventArgs(const RoutedEvent& routedEvent,
                     core::UIElement*   source,
                     core::Point        windowPosition,
                     pal::RawMouseButton button = pal::RawMouseButton::None,
                     pal::KeyModifiers  modifiers = pal::KeyModifiers::None,
                     int32_t            clickCount = 1,
                     int32_t            wheelDeltaX = 0,
                     int32_t            wheelDeltaY = 0) noexcept;

    // 取得相對於視窗客戶區的座標
    COMPAT_NODISCARD core::Point GetWindowPosition() const noexcept;

    // 取得相對於特定元件局部座標系的座標
    COMPAT_NODISCARD core::Point GetPosition(const core::UIElement* relativeTo) const noexcept;

    // 按鍵狀態查詢
    COMPAT_NODISCARD pal::RawMouseButton GetButton() const noexcept;
    COMPAT_NODISCARD bool IsLeftButton() const noexcept;
    COMPAT_NODISCARD bool IsRightButton() const noexcept;
    COMPAT_NODISCARD bool IsMiddleButton() const noexcept;

    // 修飾鍵查詢
    COMPAT_NODISCARD pal::KeyModifiers GetModifiers() const noexcept;
    COMPAT_NODISCARD bool HasModifier(pal::KeyModifiers mod) const noexcept;

    // 單擊 (1) 或雙擊 (2)
    COMPAT_NODISCARD int32_t GetClickCount() const noexcept;

    // 滾輪偏移量
    COMPAT_NODISCARD int32_t GetWheelDeltaX() const noexcept;
    COMPAT_NODISCARD int32_t GetWheelDeltaY() const noexcept;
};

using PointerEventHandler = std::function<void(PointerEventArgs&)>;

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

`PointerEventArgs` 為指標操作（包含滑鼠移動、左中右鍵按下/放開、滾輪滾動）的專用參數類別。

### 局部座標轉換 (`GetPosition`)
- 傳入 `relativeTo` 元件指標時，`PointerEventArgs` 會透過視覺樹的累計位移矩陣，自動將視窗客戶區座標轉換為該元件的內部局部幾何座標 $(0, 0) \sim (W, H)$。
- 方便控制項在自身內部計算圖表拖曳點、滑桿 Thumb 位置或畫布選取矩形。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/core/UIElement.hpp"

void SetupButtonInteraction(cppui::core::UIElement& button) {
    using namespace cppui::input;

    button.OnPointerPressed([](PointerEventArgs& e) {
        if (e.IsLeftButton()) {
            if (e.GetClickCount() == 2) {
                compat::println("按鈕被雙擊！");
            } else {
                core::Point localPos = e.GetPosition(e.GetSource());
                compat::println("按鈕單擊，局部座標: ({}, {})", localPos.x, localPos.y);
            }
        }
    });

    button.OnPointerWheel([](PointerEventArgs& e) {
        compat::println("滾輪垂直滾動: {}", e.GetWheelDeltaY());
    });
}
```

---

## 請參閱 (See Also)
* [`RoutedEventArgs` 基底參考](input_routedeventargs.md)
* [`RawInputEvent` 結構參考](pal_rawinputevent.md)
* [`UIElement` 類別參考](core_uielement.md)
