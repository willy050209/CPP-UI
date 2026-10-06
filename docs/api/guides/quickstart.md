# 快速入門指南 (Quickstart Guide)

本指南將引導您在 5 分鐘內使用 **CPPUI** 建立第一個原生視窗，並利用 DisplayList 與 DrawingContext 實現 60 FPS 流暢繪圖。

---

## 1. 建立視窗與事件循環 (Creating Window & RunLoop)

在 CPPUI 中，視窗生命週期由 [`PlatformWindowFactory`](../classes/pal_platformwindowfactory.md) 與 [`RunLoop`](../classes/pal_runloop.md) 共同管理：

```cpp
#include "compat/compat.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"
#include "src/rendering/backend/SoftwareRenderContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"

int main() {
    using namespace cppui::core;
    using namespace cppui::pal;
    using namespace cppui::rendering;

    // 1. 定義視窗屬性
    WindowDesc desc;
    desc.title = "CPPUI Quickstart App";
    desc.width = 800;
    desc.height = 600;

    // 2. 建立原生視窗
    auto window = PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println("無法建立原生視窗！");
        return -1;
    }

    // 3. 初始化軟體渲染後端
    SoftwareRenderContext renderContext;
    Size fbSize = window->GetFramebufferSize();
    renderContext.Init(window->GetNativeHandle(), 
                       static_cast<uint32_t>(fbSize.width), 
                       static_cast<uint32_t>(fbSize.height));

    // 4. 建立 DisplayList 與渲染器
    DisplayList displayList;
    DisplayListRenderer renderer;
    RunLoop loop;

    loop.AttachWindow(window);

    // 5. 設定每幀重繪回呼
    loop.SetRenderCallback([&]() {
        // A. 幀初清空命令串流
        displayList.Clear();

        // B. 錄製繪圖指令
        DrawingContext dc(displayList);

        // 繪製背景底色
        dc.DrawRectangle(Rect(0, 0, fbSize.width, fbSize.height), Colors::Black);

        // 繪製圓角卡片
        dc.DrawRoundedRectangle(Rect(50, 50, 400, 250), 16.0f, Colors::SteelBlue);

        // 使用局部剪裁繪製內容
        {
            DrawingContext::AutoClip clip(dc, Rect(70, 70, 360, 210));
            dc.DrawRectangle(Rect(80, 80, 200, 50), Colors::White);
            dc.DrawLine(Point(80, 150), Point(350, 150), Colors::Red, 3.0f);
        }

        // C. 開始渲染當前幀
        IRenderTarget* target = renderContext.BeginFrame();
        if (target) {
            renderer.Render(displayList, target);
            renderContext.EndFrame();
        }
    });

    // 6. 監聽視窗大小變更事件
    window->SetEventCallback([&](const RawInputEvent& e) {
        if (e.type == RawEventType::WindowResize) {
            renderContext.Resize(e.window.width, e.window.height);
            fbSize.width = static_cast<float>(e.window.width);
            fbSize.height = static_cast<float>(e.window.height);
            loop.RequestRedraw();
        }
    });

    // 7. 顯示視窗並啟動事件泵
    window->Show();
    compat::println("應用程式啟動中...");
    loop.Run();

    return 0;
}
```

---

## 2. 核心觀念解析 (Core Concepts)

1. **零直接 GPU 呼叫**：
   在 `RenderCallback` 中，您只與 `DrawingContext` 互動。所有繪圖操作都僅是打包成輕量二進位指令寫入 `DisplayList`，UI 邏輯無須關注光柵化細節。
2. **高分屏適配**：
   `window->GetFramebufferSize()` 提供畫布實際物理像素，而事件中提供邏輯座標與 DPI 縮放係數。
3. **無效 CPU 佔用消除**：
   `RunLoop` 內部會在空閒時自動呼叫 `WaitEvents` 進入最多 16ms 等待，避免無意義的死循環消耗 CPU。

---

## 3. 編譯與建置 (Building Your Code)

支援在 C++14、C++17、C++20、C++23 下直接編譯：

```powershell
# 使用 Clang++ (C++14)
clang++.exe -std=c++14 -O2 -I include -I include/compat main.cpp src\pal\win32\PlatformWindowWin32.cpp src\rendering\DisplayListRenderer.cpp src\rendering\backend\SoftwareRenderContext.cpp -lgdi32 -luser32 -o app.exe

# 使用 MSVC (C++14+)
cl.exe /O2 /EHsc /utf-8 /std:c++14 /I include /I include/compat main.cpp src\pal\win32\PlatformWindowWin32.cpp src\rendering\DisplayListRenderer.cpp src\rendering\backend\SoftwareRenderContext.cpp gdi32.lib user32.lib /Fe:app.exe
```

---

## 請參閱 (See Also)
* [DisplayList 二進位錄製深度解析](display_list_recording.md)
* [高分屏 DPI 與座標轉換指南](dpi_and_coordinates.md)
* [`IPlatformWindow` 介面參考](../classes/pal_iplatformwindow.md)
