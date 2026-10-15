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
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc = WindowDesc{});
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateGeneric(const WindowDesc& desc = WindowDesc{});
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateX11(const WindowDesc& desc = WindowDesc{});
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

`PlatformWindowFactory` 採用簡單工廠模式 (Simple Factory Pattern)，提供原生自適應建立入口 `Create()`、通用虛擬視窗入口 `CreateGeneric()` 與 Linux 原生視窗入口 `CreateX11()`：
* **`Create()`**：依據作業系統與執行期環境動態建立最佳原生視窗：
  * **Windows (`_WIN32`)**：實例化 `PlatformWindowWin32`（向 Windows 註冊視窗類別、啟用 Per-Monitor V2 高分屏 DPI 感知、64-bit 視窗指標映射、支援 GDI DIBits 呈現）。
  * **Linux (支援 X11)**：主動偵測 `DISPLAY` 環境變數。若變數存在且 X11 後端已編譯，自動實例化 [`PlatformWindowX11`](pal_platformwindowx11.md) 原生視窗；若處於無顯示卡/無 Display 之 Headless CI 環境，則自動安全回退至 [`PlatformWindowGeneric`](pal_platformwindowgeneric.md)。
  * **其他平台 (macOS/Headless)**：回退實例化 [`PlatformWindowGeneric`](pal_platformwindowgeneric.md)。
* **`CreateGeneric()`**：強制在任何作業系統上建立純 C++ 通用記憶體視窗 [`PlatformWindowGeneric`](pal_platformwindowgeneric.md)，專為 CI 無介面測試、離線渲染、單元測試而設計。
* **`CreateX11()`**：明確強制建立 Linux 原生 X11 實體視窗 [`PlatformWindowX11`](pal_platformwindowx11.md)（支援 WSLg 與實體 Linux 桌面）。若當前環境未編譯 X11 支援或無法連接 Display，將立即擲出 `std::runtime_error`。

---

## 靜態方法 (Static Methods)

### `Create(const WindowDesc& desc)`
依據傳入的視窗描述符建立最佳原生視窗實例。
```cpp
COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc = WindowDesc{});
```
* **參數**：
  * `desc`：[`WindowDesc`](pal_iplatformwindow.md#windowdesc-結構) 結構，指定視窗標題、寬高、可縮放性等初始屬性。寬高必須大於 0。
* **傳回值**：指向 [`IPlatformWindow`](pal_iplatformwindow.md) 介面的 `std::shared_ptr` 智慧指標。
* **例外**：若 `desc.width == 0` 或 `desc.height == 0`，立即擲出 `std::invalid_argument`。

### `CreateGeneric(const WindowDesc& desc)`
強制建立跨平台純記憶體通用視窗實例（無須作業系統 GUI 伺服器）。
```cpp
COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateGeneric(const WindowDesc& desc = WindowDesc{});
```
* **參數**：
  * `desc`：[`WindowDesc`](pal_iplatformwindow.md#windowdesc-結構) 結構。寬高必須大於 0。
* **傳回值**：指向 [`PlatformWindowGeneric`](pal_platformwindowgeneric.md) 實例的 `std::shared_ptr<IPlatformWindow>` 智慧指標。可用於 Headless 離線畫面渲染與模擬事件輸入。
* **例外**：若 `desc.width == 0` 或 `desc.height == 0`，立即擲出 `std::invalid_argument`。

### `CreateX11(const WindowDesc& desc)`
明確建立 Linux X11 原生實體視窗實例。
```cpp
COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateX11(const WindowDesc& desc = WindowDesc{});
```
* **參數**：
  * `desc`：[`WindowDesc`](pal_iplatformwindow.md#windowdesc-結構) 結構。寬高必須大於 0。
* **傳回值**：指向 [`PlatformWindowX11`](pal_platformwindowx11.md) 實例的 `std::shared_ptr<IPlatformWindow>` 智慧指標。
* **例外**：
  * 若 `desc.width == 0` 或 `desc.height == 0`，立即擲出 `std::invalid_argument`。
  * 若 X11 模組未啟用或無法連線至 `$DISPLAY`，立即擲出 `std::runtime_error`。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
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
