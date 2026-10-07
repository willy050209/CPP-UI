# PlatformWindowGeneric 類別 (cppui::pal::PlatformWindowGeneric)

跨平台純 C++ 記憶體通用視窗實作，專為無桌面環境 (Headless CI)、非 Windows 系統或自動化離線 UI 測試而設計。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::pal`](../namespaces/cppui_pal.md)
* **標頭檔**：`#include <ui/pal/PlatformWindowGeneric.hpp>`
* **模組**：`CPPUI PAL`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace pal {

class PlatformWindowGeneric : public IPlatformWindow {
public:
    explicit PlatformWindowGeneric(const WindowDesc& desc);
    ~PlatformWindowGeneric() override = default;

    // 視窗生命週期控制
    void Show() override;
    void Hide() override;
    void Close() override;
    COMPAT_NODISCARD bool IsClosed() const noexcept override;

    // 幾何與標題
    COMPAT_NODISCARD core::Size GetSize() const noexcept override;
    void SetSize(uint32_t width, uint32_t height) override;
    COMPAT_NODISCARD core::Size GetFramebufferSize() const noexcept override;
    COMPAT_NODISCARD float GetDpiScale() const noexcept override;
    void SetTitle(compat::string_view title) override;

    // 原生控制代碼 (回傳 this)
    COMPAT_NODISCARD void* GetNativeHandle() const noexcept override;

    // 事件通知回呼
    void SetEventCallback(EventCallback callback) override;

    // 訊息輪詢與等待
    void PollEvents() override;
    bool WaitEvents(int timeoutMs = -1) override;

    // 觸發視窗重繪請求
    void RequestRedraw() override;

    // 幀緩衝呈現
    void PresentFramebuffer(compat::span<const uint32_t> pixels, uint32_t width, uint32_t height) override;

    // 測試專用：手動注入事件
    void InjectEvent(const RawInputEvent& evt);

    // 測試與讀取專用：取得內部幀緩衝像素唯讀視圖
    COMPAT_NODISCARD compat::span<const uint32_t> GetFramebufferPixels() const noexcept;
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

`PlatformWindowGeneric` 是 CPPUI PAL 的純 C++ 視窗實作，具備以下特點：
1. **零外部依賴**：完全不依賴 Win32 API (`HWND`、`MSG`、GDI)、X11、Wayland 或 Cocoa。
2. **記憶體 Framebuffer 容器**：內部維護動態 `std::vector<uint32_t>` 幀緩衝區，在呼叫 `PresentFramebuffer()` 時接收渲染後的光柵化像素。
3. **事件注入機制 (`InjectEvent`)**：提供手動模擬注入滑鼠、鍵盤、DPI、視窗縮放等事件的功能，事件會存入內部事件隊列，並在 `PollEvents()` / `WaitEvents()` 時派發至已註冊的回呼函式。
4. **全平台 Headless CI 測試**：可在任何作業系統與 CI/CD 環境下無介面運行完整 UI 生命週期與測試斷言。

---

## 方法說明 (Method Specifications)

### `PlatformWindowGeneric(const WindowDesc& desc)`
依據傳入的 `WindowDesc` 初始化通用視窗的標題、幾何尺寸與可見性，並預先配置相對應大小的內部幀緩衝記憶體。

### `PresentFramebuffer(compat::span<const uint32_t> pixels, uint32_t width, uint32_t height)`
將渲染輸出結果拷貝保存至通用視窗的內部幀緩衝中，更新寬高尺寸。

### `InjectEvent(const RawInputEvent& evt)`
將指定的原生輸入事件放入內部待處理隊列中。

### `GetFramebufferPixels()`
```cpp
COMPAT_NODISCARD compat::span<const uint32_t> GetFramebufferPixels() const noexcept;
```
傳回當前內部幀緩衝像素的連續記憶體唯讀視圖。可用於測試斷言比對像素顏色、驗證繪圖輸出或將畫面寫入 PNG / BMP 檔案。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/PlatformWindowGeneric.hpp"
#include "ui/pal/RawInputEvent.hpp"

void TestGenericWindow() {
    using namespace cppui::pal;

    WindowDesc desc;
    desc.title = "Headless Test Window";
    desc.width = 400;
    desc.height = 300;

    auto window = PlatformWindowFactory::CreateGeneric(desc);

    bool clicked = false;
    window->SetEventCallback([&](const RawInputEvent& evt) {
        if (evt.type == RawInputEventType::MouseDown) {
            clicked = true;
        }
    });

    // 模擬滑鼠按下
    auto genericWindow = std::dynamic_pointer_cast<PlatformWindowGeneric>(window);
    genericWindow->InjectEvent(RawInputEvent::MakeMouseDown(100.0f, 150.0f, MouseButton::Left));
    genericWindow->PollEvents();

    compat::println("事件是否觸發: {}", clicked ? "是" : "否");
}
```

---

## 請參閱 (See Also)
* [`IPlatformWindow` 介面參考](pal_iplatformwindow.md)
* [`PlatformWindowFactory` 工廠類別](pal_platformwindowfactory.md)
* [`RawInputEvent` 原生事件](pal_rawinputevent.md)
* [`跨平台 PAL 架構指南`](../guides/cross_platform_architecture.md)
