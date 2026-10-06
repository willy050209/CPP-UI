# RunLoop 類別 (cppui::pal::RunLoop)

主執行緒低延遲事件循環管理器，負責原生作業系統訊息輪詢與渲染重繪回呼的節奏調度。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::pal`](../namespaces/cppui_pal.md)
* **標頭檔**：`#include <ui/pal/RunLoop.hpp>`
* **模組**：`CPPUI PAL`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace pal {

class RunLoop {
public:
    RunLoop() = default;
    ~RunLoop() = default;

    RunLoop(const RunLoop&) = delete;
    RunLoop& operator=(const RunLoop&) = delete;

    void AttachWindow(std::shared_ptr<IPlatformWindow> window);
    void SetRenderCallback(std::function<void()> callback);
    void RequestRedraw() noexcept;
    void Stop() noexcept;

    COMPAT_NODISCARD bool IsRunning() const noexcept;
    void Run();
};

} // namespace pal
} // namespace cppui
```

---

## 簡介 (Description)

`RunLoop` 負責管理主執行緒的生命週期循環。它協調三項核心任務：
1. **原生作業系統訊息分發**：定期或於事件到達時呼叫 `window->PollEvents()`。
2. **骯髒區域重繪觸發**：當元件狀態變更或接收到重繪請求 (`m_needsRedraw == true`) 時，呼叫使用者的 `RenderCallback`。
3. **無效 CPU 耗損防禦**：當視窗無需重繪且無使用者輸入時，透過 `window->WaitEvents(16)` 進入最多 16ms 的低延遲等待（約對應 60 FPS 間隔），將 CPU 使用率壓低至趨近於 0%。

---

## 方法說明 (Methods)

### `AttachWindow(std::shared_ptr<IPlatformWindow> window)`
將受控的原生視窗綁定至此事件循環。當該視窗被使用者關閉 (`IsClosed() == true`) 時，`RunLoop` 將自動退出。

### `SetRenderCallback(std::function<void()> callback)`
設定渲染重繪回呼函式。此函式在每一幀需要繪製時被調用。

### `RequestRedraw()`
```cpp
void RequestRedraw() noexcept;
```
請求下一幀執行畫面重繪。通常由 UI 佈局變更、滑鼠懸停效果、動畫計時器或原生視窗暴露出無效區域時觸發。

### `Stop()`
```cpp
void Stop() noexcept;
```
安全終止事件循環，使 `Run()` 返回。

### `IsRunning()`
```cpp
COMPAT_NODISCARD bool IsRunning() const noexcept;
```
檢查當前事件循環是否正在運行。

### `Run()`
```cpp
void Run();
```
進入主執行緒事件循環。該方法為**阻塞式呼叫**，直到視窗關閉或明確調用 `Stop()` 為止。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"

void RunApplication() {
    using namespace cppui::pal;

    WindowDesc desc;
    desc.title = "RunLoop 範例";
    auto window = PlatformWindowFactory::Create(desc);

    RunLoop loop;
    loop.AttachWindow(window);

    // 設定重繪回呼
    loop.SetRenderCallback([&]() {
        compat::println("幀刷新中...");
    });

    window->Show();
    // 進入事件循環
    loop.Run();
}
```

---

## 請參閱 (See Also)
* [`IPlatformWindow` 介面參考](pal_iplatformwindow.md)
* [`PlatformWindowFactory` 工廠類別](pal_platformwindowfactory.md)
* [快速入門指南](../guides/quickstart.md)
