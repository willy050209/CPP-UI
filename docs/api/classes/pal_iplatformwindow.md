# IPlatformWindow 介面 (cppui::pal::IPlatformWindow)

跨平台抽象視窗介面 (Platform Abstraction Layer, PAL)，提供作業系統原生視窗生命週期、幾何狀態、DPI 縮放與事件回呼的統一合約。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::pal`](../namespaces/cppui_pal.md)
* **標頭檔**：`#include <ui/pal/IPlatformWindow.hpp>`
* **模組**：`CPPUI PAL`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace pal {

struct WindowDesc {
    compat::string_view title = "CPPUI Window";
    uint32_t width = 800;
    uint32_t height = 600;
    bool resizable = true;
    bool visible = true;
    bool decorated = true;
};

class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;

    // 視窗生命週期控制
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Close() = 0;
    COMPAT_NODISCARD virtual bool IsClosed() const noexcept = 0;

    // 幾何與標題
    COMPAT_NODISCARD virtual core::Size GetSize() const noexcept = 0;
    virtual void SetSize(uint32_t width, uint32_t height) = 0;
    COMPAT_NODISCARD virtual core::Size GetFramebufferSize() const noexcept = 0;
    COMPAT_NODISCARD virtual float GetDpiScale() const noexcept = 0;
    virtual void SetTitle(compat::string_view title) = 0;

    // 原生控制代碼 (例如 Win32 HWND, Cocoa NSWindow, Wayland wl_surface)
    COMPAT_NODISCARD virtual void* GetNativeHandle() const noexcept = 0;

    // 事件通知回呼
    using EventCallback = std::function<void(const RawInputEvent&)>;
    virtual void SetEventCallback(EventCallback callback) = 0;

    // 訊息輪詢與等待
    virtual void PollEvents() = 0;
    virtual bool WaitEvents(int timeoutMs = -1) = 0;

    // 喚醒等待狀態之原生訊息迴圈 (支援跨執行緒調用)
    virtual void WakeUp() = 0;

    // 觸發視窗重繪請求
    virtual void RequestRedraw() = 0;

    // 幀緩衝呈現 (跨平台軟體渲染呈現到視窗畫面)
    virtual void PresentFramebuffer(compat::span<const uint32_t> pixels, uint32_t width, uint32_t height) = 0;
    virtual void PresentFramebuffer(const uint32_t* pixels, uint32_t width, uint32_t height) {
        if (pixels && width > 0 && height > 0) {
            PresentFramebuffer(compat::span<const uint32_t>(pixels, static_cast<size_t>(width) * height), width, height);
        }
    }
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

`IPlatformWindow` 是 CPPUI PAL 的核心純虛擬介面。它將不同作業系統（Windows Win32, Linux X11/Wayland, macOS Cocoa）原生視窗的 API 細節完全封裝，使上層 UI 與渲染管線能夠以一致的方式取得視窗資訊、接收輸入並排程繪圖。

使用者通常不直接建構此介面的實作，而是透過 [`PlatformWindowFactory::Create`](pal_platformwindowfactory.md) 取得 `std::shared_ptr<IPlatformWindow>`。

---

## 輔助結構：`WindowDesc`

用於建立視窗時的參數配置：

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `title` | `compat::string_view` | `"CPPUI Window"` | 視窗標題列顯示的文字。 |
| `width` | `uint32_t` | `800` | 視窗客戶區的初始邏輯寬度 (DIP)。 |
| `height` | `uint32_t` | `600` | 視窗客戶區的初始邏輯高度 (DIP)。 |
| `resizable` | `bool` | `true` | 是否允許使用者拖拉邊框調整大小。 |
| `visible` | `bool` | `true` | 視窗建立完成後是否立即顯示。 |
| `decorated` | `bool` | `true` | 是否具備作業系統標準外框與標題列（設為 `false` 建立無邊框視窗）。 |

---

## 方法說明 (Method Specifications)

### `Show()` / `Hide()` / `Close()`
* `Show()`：將視窗顯示於螢幕上並帶至前台。
* `Hide()`：隱藏視窗。
* `Close()`：關閉視窗並釋放原生資源。

### `IsClosed()`
```cpp
COMPAT_NODISCARD virtual bool IsClosed() const noexcept = 0;
```
* **傳回值**：若視窗已被使用者或程式碼關閉，則傳回 `true`；此狀態為 `RunLoop` 終止訊息循環的主要依據。

### `GetSize()` / `GetFramebufferSize()`
```cpp
COMPAT_NODISCARD virtual core::Size GetSize() const noexcept = 0;
COMPAT_NODISCARD virtual core::Size GetFramebufferSize() const noexcept = 0;
```
* **備註**：`GetSize()` 回傳**邏輯尺寸 (DIP)**；`GetFramebufferSize()` 回傳**物理像素尺寸**。在 200% 縮放高分屏下，若 `GetSize()` 為 `(800, 600)`，則 `GetFramebufferSize()` 為 `(1600, 1200)`。

### `GetDpiScale()`
```cpp
COMPAT_NODISCARD virtual float GetDpiScale() const noexcept = 0;
```
* **傳回值**：當前顯示器的 DPI 縮放係數（如 `1.0f` 代表 100% 96 DPI；`1.5f` 代表 150% 144 DPI）。

### `SetEventCallback(EventCallback callback)`
註冊平台原生輸入事件監聽器。當滑鼠移動、按鍵按下、視窗大小改變或 DPI 變化時，將觸發此回呼函式並傳入 [`RawInputEvent`](pal_rawinputevent.md)。

### `PollEvents()` / `WaitEvents(int timeoutMs)`
* `PollEvents()`：非阻塞輪詢原生作業系統訊息隊列，立即分發所有待處理訊息。
* `WaitEvents(int timeoutMs)`：低延遲阻塞等待訊息。若有訊息到達或超過 `timeoutMs` 毫秒（預設 `-1` 為無限等待）則喚醒。

### `WakeUp()`
```cpp
virtual void WakeUp() = 0;
```
* **說明**：
  支援跨執行緒安全呼叫，立即喚醒處於 `WaitEvents` 低功耗阻塞等待狀態的原生視窗訊息循環。在 Win32 平台下透過 `PostMessageW(m_hwnd, WM_APP_DISPATCH, 0, 0)` 喚醒 `MsgWaitForMultipleObjectsEx`；在跨平台通用端透過 `std::condition_variable::notify_all()` 立即解除休眠，使非同步派發任務（Dispatcher）得以及時在主執行緒消費執行。

### `RequestRedraw()`
通知視窗系統與 `RunLoop` 在下一幀觸發重繪回呼。

### `PresentFramebuffer(compat::span<const uint32_t> pixels, uint32_t width, uint32_t height)`
```cpp
virtual void PresentFramebuffer(compat::span<const uint32_t> pixels, uint32_t width, uint32_t height) = 0;
virtual void PresentFramebuffer(const uint32_t* pixels, uint32_t width, uint32_t height);
```
* **參數**：
  * `pixels`：唯讀 32-bit ARGB/BGRA 像素緩衝區視圖（型別為 `compat::span<const uint32_t>`）。
  * `width`：像素緩衝區的水平寬度（像素）。
  * `height`：像素緩衝區的垂直高度（像素）。
* **說明**：
  跨平台軟體渲染輸出核心函式。將 CPU 記憶體中光柵化的像素資料直接呈現到平台原生視窗（例如在 Win32 下使用 `StretchDIBits`，在通用視窗 `PlatformWindowGeneric` 下儲存至內部像素緩衝區，在 Linux 下送交 XImage/wl_shm）。此方法使上層繪圖管線與 Demo 完全解耦作業系統專屬的 Device Context (DC) 與 GDI API。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"

void ExampleWindow() {
    using namespace cppui::pal;

    WindowDesc desc;
    desc.title = "自訂標題視窗";
    desc.width = 1280;
    desc.height = 720;

    auto window = PlatformWindowFactory::Create(desc);
    if (!window) return;

    compat::println("DPI 縮放: {}%", static_cast<int>(window->GetDpiScale() * 100));
    window->Show();
}
```

---

## 請參閱 (See Also)
* [`PlatformWindowFactory` 工廠類別](pal_platformwindowfactory.md)
* [`RunLoop` 事件循環](pal_runloop.md)
* [`RawInputEvent` 原生事件](pal_rawinputevent.md)
* [`cppui::pal` 命名空間](../namespaces/cppui_pal.md)
