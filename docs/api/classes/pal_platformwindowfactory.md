# PlatformWindowFactory 類別 (cppui::pal::PlatformWindowFactory)

負責依據當前執行環境與作業系統，動態建立最佳原生視窗實例的工廠類別。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::pal`](../namespaces/cppui_pal.md)
* **標頭檔**：`#include <ui/pal/PlatformWindowFactory.hpp>`
* **模組**：`CPPUI PAL`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace pal {

class PlatformWindowFactory {
public:
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc);
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

`PlatformWindowFactory` 採用簡單工廠模式 (Simple Factory Pattern)，提供單一靜態建立入口 `Create()`。它內部依據作業系統條件編譯巨集（如 `_WIN32`、`__APPLE__`、`__linux__`）實例化對應的視窗實作類別：
* **Windows**：實例化 `PlatformWindowWin32`（內部向 Windows 註冊視窗類別、啟用 Per-Monitor V2 高分屏 DPI 感知，並建立 64-bit 視窗指標映射）。
* **未來平台**：將無縫對接 SDL3 / Wayland / Cocoa。

---

## 靜態方法 (Static Methods)

### `Create(const WindowDesc& desc)`
依據傳入的視窗描述符建立原生視窗實例。
```cpp
COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc);
```
* **參數**：
  * `desc`：[`WindowDesc`](pal_iplatformwindow.md#windowdesc-結構) 結構，指定視窗標題、寬高、可縮放性等初始屬性。
* **傳回值**：指向 [`IPlatformWindow`](pal_iplatformwindow.md) 介面的 `std::shared_ptr` 智慧指標；若平台不支援或建立失敗則回傳 `nullptr`。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"

void CreateAppWindow() {
    using namespace cppui::pal;

    WindowDesc desc;
    desc.title = "主應用程式視窗";
    desc.width = 1024;
    desc.height = 768;

    auto window = PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println("錯誤：無法建立原生視窗！");
        return;
    }

    compat::println("成功建立視窗，原生控制代碼指標: {}", window->GetNativeHandle());
}
```

---

## 請參閱 (See Also)
* [`IPlatformWindow` 介面參考](pal_iplatformwindow.md)
* [`RunLoop` 事件循環](pal_runloop.md)
* [`cppui::pal` 命名空間](../namespaces/cppui_pal.md)
