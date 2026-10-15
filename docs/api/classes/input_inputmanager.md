# InputManager 類別 (cppui::input::InputManager)

輸入管線核心分發中樞單例（Singleton），負責接收來自 PAL 層的原生 [`RawInputEvent`](pal_rawinputevent.md)，執行命中測試與路由轉換，並維護游標懸停追蹤（Hover Tracking）與指標捕獲（Pointer Capture）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/InputManager.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

class InputManager {
public:
    static InputManager& Get() noexcept;

    // --- 核心分發入口：接收底層視窗原始事件並轉發至視覺樹 ---
    void ProcessRawEvent(core::UIElement& root, const pal::RawInputEvent& rawEvent);

    // --- 游標懸停追蹤 (Hover Tracking) ---
    COMPAT_NODISCARD core::UIElement* GetHoveredElement() const noexcept;

    // --- 指標捕獲管理 (Pointer Capture) ---
    bool CapturePointer(core::UIElement* element);
    void ReleasePointerCapture(core::UIElement* element = nullptr);
    COMPAT_NODISCARD core::UIElement* GetCapturedElement() const noexcept;

    // --- 狀態重設 ---
    void ResetState();

    // --- 元件解構保護 ---
    void NotifyElementDestroyed(core::UIElement* element) noexcept;

    // --- 內部子管線分流 ---
    void ProcessMouseMove(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessMouseDown(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessMouseUp(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessMouseWheel(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessKeyDown(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods);
    void ProcessKeyUp(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods);
    void ProcessTextInput(core::UIElement& root, const pal::RawTextInputData& textData);
};

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

`InputManager` 是連接底層作業系統視窗原生事件與高階 UI 視覺樹的樞紐：

### 1. 原始事件轉譯與分流 (`ProcessRawEvent`)
- 接收 `RawInputEvent`，依據事件型別分流至 `ProcessMouseMove`, `ProcessMouseDown`, `ProcessKeyDown` 等專門管線。
- 支援 High-DPI 客戶區幾何空間，`mouseData.position` 與視覺樹佈局邊界 1:1 精確對齊，確保命中測試（`Visual::HitTest`）0 像素偏離。

### 2. 懸停追蹤機制 (Hover Tracking)
- 在每次滑鼠移動時，`InputManager` 調用 `root.HitTest(position)` 找出當前位於游標下方的元件。
- 若命中元件與 `m_hoveredElement` 不同：
  - 向舊元件派發直接路由事件 `PointerExited`。
  - 向新元件派發直接路由事件 `PointerEntered`。
  - 更新當前懸停狀態。

### 3. 指標捕獲語意 (Pointer Capture)
- 當使用者按下按鈕或開始拖曳滑桿時，控制項可呼叫 `CapturePointer(this)`。
- 一旦指標被捕獲：
  - 即便使用者將滑鼠快速移出該控制項邊界甚至視窗外部，所有的 `MouseMove` 與 `MouseUp` 事件依然強制派發給該捕獲元件。
  - 當放開滑鼠時，元件可呼叫 `ReleasePointerCapture()` 解除捕獲。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/core/UIElement.hpp"

void SetupWindowInputPipeline(cppui::pal::IPlatformWindow& window, 
                              cppui::core::UIElement& rootElement) {
    using namespace cppui::input;

    // 將底層視窗回呼接駁至 InputManager
    window.SetEventCallback([&](const cppui::pal::RawInputEvent& e) {
        InputManager::Get().ProcessRawEvent(rootElement, e);
        window.RequestRedraw();
    });
}
```

---

## 請參閱 (See Also)
* [`FocusManager` 類別參考](input_focusmanager.md)
* [`EventRoute` 類別參考](input_eventroute.md)
* [`PointerEventArgs` 類別參考](input_pointereventargs.md)
* [輸入管線與路由事件指南](../guides/input_and_routed_events.md)
