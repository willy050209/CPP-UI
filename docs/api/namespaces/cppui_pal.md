# cppui::pal 命名空間 (Namespace)

`cppui::pal` 命名空間代表 **平台抽象層 (Platform Abstraction Layer, PAL)**，負責封裝作業系統底層的原生視窗系統、事件泵與輸入裝置。

---

## 簡介 (Description)

CPPUI 追求極致的跨平台性與高內聚架構。UI 邏輯、佈局系統與繪圖管線**完全不直接相依任何特定作業系統的 API（例如 Win32 API、Cocoa 或 Wayland）**。所有平台互動皆透過 `cppui::pal` 介面隔離。

### 核心職責
1. **視窗生命週期管理**：建立原生視窗、調整大小、顯示/隱藏、最小化/關閉、設定視窗標題與取得原生控制代碼 (`HWND`, `NSWindow` 等)。
2. **高分屏與 DPI 抽象**：自動偵測 Windows Per-Monitor V2 高分屏 DPI 變化，回傳實時 DPI 縮放係數 (`GetDpiScale()`)。
3. **主執行緒低延遲事件循環 (RunLoop)**：驅動非阻塞/可等待訊息泵 (`PollEvents` / `WaitEvents`)，協同 VSync 重繪調度，防止 CPU 空轉並維持 60~120 FPS 流暢度。
4. **統一原生輸入事件模型**：將 Windows `WM_MOUSEMOVE`、`WM_KEYDOWN` 或 macOS Cocoa 事件轉換為跨平台的 [`RawInputEvent`](../classes/pal_rawinputevent.md)。

---

## 類別與介面索引 (Classes & Interfaces)

| 型別 (Type) | 類別形態 | 簡介與職責 |
| :--- | :--- | :--- |
| [`IPlatformWindow`](../classes/pal_iplatformwindow.md) | 純虛擬抽象介面 | 跨平台原生視窗的抽象介面，定義視窗操作、DPI 查詢與事件派發合約。 |
| [`PlatformWindowFactory`](../classes/pal_platformwindowfactory.md) | 工廠類別 | 根據編譯期與執行期作業系統環境建立最佳原生視窗實例（Windows 實例化 Win32，Linux 實例化 X11 或 Generic）。 |
| [`PlatformWindowX11`](../classes/pal_platformwindowx11.md) | 實體實作類別 | Linux X11 / WSLg 原生實體視窗實作，支援 Xlib 事件循環、雙緩衝 XPutImage 繪圖與 Self-pipe 異步喚醒。 |
| [`PlatformWindowGeneric`](../classes/pal_platformwindowgeneric.md) | 實體實作類別 | 跨平台純記憶體通用虛擬視窗，專為 Headless CI、離線繪圖與單元測試設計。 |
| [`RunLoop`](../classes/pal_runloop.md) | 實體控制類別 | 主執行緒低延遲事件循環，負責協調原生訊息處理與畫面刷新回呼 (`RenderCallback`)。 |

---

## 結構與列舉索引 (Structs & Enums)

| 型別 (Type) | 分類 | 簡介與職責 |
| :--- | :--- | :--- |
| [`RawInputEvent`](../classes/pal_rawinputevent.md) | 結構體 | 平台原生輸入事件之統一共用體結構，包含事件型別、時間戳與修飾鍵。 |
| [`WindowDesc`](../classes/pal_iplatformwindow.md#windowdesc-結構) | 結構體 | 視窗建立參數描述符（標題、尺寸、可縮放性、可見度、裝飾框）。 |
| `RawEventType` | 列舉 | 原生事件類型列舉（`MouseMove`, `MouseDown`, `KeyDown`, `WindowResize`, `DpiChanged` 等）。 |
| `RawMouseButton` | 列舉 | 滑鼠按鍵列舉（`Left`, `Right`, `Middle`, `XButton1`, `XButton2`）。 |
| `KeyModifiers` | 列舉旗標 | 鍵盤修飾鍵位元旗標（`Control`, `Shift`, `Alt`, `Meta`, `CapsLock`），支援 `|` 與 `&` 運算。 |

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"

int main() {
    using namespace cppui::pal;

    // 1. 定義視窗參數
    WindowDesc desc;
    desc.title = "CPPUI PAL 範例視窗";
    desc.width = 1024;
    desc.height = 768;

    // 2. 透過工廠建立原生視窗
    auto window = PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println("建立視窗失敗！");
        return -1;
    }

    // 3. 註冊輸入事件回呼
    window->SetEventCallback([](const RawInputEvent& event) {
        if (event.type == RawEventType::MouseMove) {
            compat::println("滑鼠邏輯座標: ({}, {})", 
                event.mouse.position.x, event.mouse.position.y);
        }
    });

    // 4. 啟動事件循環
    RunLoop loop;
    loop.AttachWindow(window);
    loop.SetRenderCallback([&]() {
        // 每幀重繪邏輯
    });

    window->Show();
    loop.Run();

    return 0;
}
```

---

## 請參閱 (See Also)
* [CPPUI API 首頁](../README.md)
* [`IPlatformWindow` 介面參考](../classes/pal_iplatformwindow.md)
* [`RunLoop` 類別參考](../classes/pal_runloop.md)
* [`RawInputEvent` 結構參考](../classes/pal_rawinputevent.md)
* [快速入門指南](../guides/quickstart.md)
