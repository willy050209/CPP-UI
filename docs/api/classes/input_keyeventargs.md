# KeyEventArgs 與 TextInputEventArgs (cppui::input)

封裝鍵盤實體按鍵（KeyDown/KeyUp）與高階文字輸入（TextInput/IME）的路由事件參數。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/KeyEventArgs.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

/// <summary>
/// 實體鍵盤事件參數 (KeyDown / KeyUp)
/// </summary>
class KeyEventArgs : public RoutedEventArgs {
public:
    KeyEventArgs(const RoutedEvent& routedEvent,
                 core::UIElement*   source,
                 uint32_t           virtualKey,
                 uint32_t           scanCode,
                 bool               isRepeat,
                 pal::KeyModifiers  modifiers) noexcept;

    COMPAT_NODISCARD uint32_t GetVirtualKey() const noexcept;
    COMPAT_NODISCARD uint32_t GetScanCode() const noexcept;
    COMPAT_NODISCARD bool IsRepeat() const noexcept;
    COMPAT_NODISCARD pal::KeyModifiers GetModifiers() const noexcept;
    COMPAT_NODISCARD bool HasModifier(pal::KeyModifiers mod) const noexcept;
};

/// <summary>
/// 文字輸入事件參數 (TextInput)
/// </summary>
class TextInputEventArgs : public RoutedEventArgs {
public:
    TextInputEventArgs(const RoutedEvent& routedEvent,
                       core::UIElement*   source,
                       uint32_t           codepoint) noexcept;

    TextInputEventArgs(const RoutedEvent&  routedEvent,
                       core::UIElement*    source,
                       compat::string_view text) noexcept;

    COMPAT_NODISCARD uint32_t GetCodepoint() const noexcept;
    COMPAT_NODISCARD compat::string_view GetText() const noexcept;
};

using KeyEventHandler       = std::function<void(KeyEventArgs&)>;
using TextInputEventHandler = std::function<void(TextInputEventArgs&)>;

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

在桌面作業系統中，鍵盤操作區分為**底層按鍵信號 (Key Down/Up)** 與**合成文字字元 (Text Input)**：
1. **`KeyEventArgs`**：
   - 表示實體按鍵的按下或釋放，攜帶 Windows 虛擬鍵碼（`VK_*`，如 `VK_RETURN`, `VK_BACK`, `VK_TAB`, `VK_ESCAPE`）、硬體掃描碼（ScanCode）與長按重複旗標（`IsRepeat`）。
   - 適用於快捷鍵監聽、游標導航控制（方向鍵、Home、End）。
2. **`TextInputEventArgs`**：
   - 經過輸入法（IME）或按鍵映射後所合成的具體 Unicode 字元（UTF-32 Codepoint）或 UTF-8 字串片段。
   - 支援中文字、日語假名、特殊符號與重音字符輸入。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/core/UIElement.hpp"

void SetupInputBox(cppui::core::UIElement& inputBox) {
    using namespace cppui::input;

    // 監聽退格鍵與方向鍵
    inputBox.OnKeyDown([](KeyEventArgs& e) {
        if (e.GetVirtualKey() == 0x08) { // VK_BACK (Backspace)
            compat::println("按下退格鍵刪除字元！");
            e.SetHandled(true);
        } else if (e.GetVirtualKey() == 0x0D) { // VK_RETURN (Enter)
            compat::println("按下 Enter 提交表單！");
            e.SetHandled(true);
        }
    });

    // 監聽具體輸入字串
    inputBox.OnTextInput([](TextInputEventArgs& e) {
        compat::println("輸入文字: {}", e.GetText());
    });
}
```

---

## 請參閱 (See Also)
* [`RoutedEventArgs` 基底參考](input_routedeventargs.md)
* [`FocusManager` 類別參考](input_focusmanager.md)
* [`UIElement` 類別參考](core_uielement.md)
