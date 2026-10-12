# 快速入門指南 (Quickstart Guide)

本指南將引導您在 3 分鐘內使用 **CPPUI** 建立第一個現代化桌面視窗應用程式。我們將利用高階封裝的 [`Application`](../classes/app_application.md)、[`Form`](../classes/app_form.md) 以及聲明式 **Fluent RAD DSL**，以極致優雅、型別安全且零樣板的現代 C++ 語法建構跨平台視窗。

---

## 1. 成果預覽 (Preview)

完成本指南後，您將運行出如下圖所示的現代化深色風格桌面視窗：

![CPPUI 快速入門表單預覽](../../images/quickstart_preview.png)

---

## 2. 核心架構理念 (Architecture Concept)

在 CPPUI 中，您無需手動處理 Win32 `HWND`、訊息泵 (`MSG`)、雙緩衝繪圖或繁瑣的視窗縮放重繪邏輯。整個桌面應用程式建構於三個核心基石之上：

1. **[`Application`](../classes/app_application.md)**：全域應用程式生命週期中樞，封裝跨平台原生事件泵、執行緒調度器 ([`Dispatcher`](../classes/app_dispatcher.md)) 與 IoC 服務容器。
2. **[`Form`](../classes/app_form.md)**：現代化視窗基底類別，內建雙階段佈局管線 (Measure/Arrange)、樣式主題以及 DisplayList 向量渲染。
3. **[Fluent RAD DSL](fluent_rad_dsl_and_signals.md)**：現代 C++ 宣告式 UI 排版語法，提供如 `VStack`、`HStack`、`Label`、`Button`、`Border` 等鏈式屬性設定器。

---

## 3. 步驟一：定義視窗表單 (HelloWorldForm.hpp)

首先建立視窗表單的標頭檔。繼承自 `cppui::app::Form`：

```cpp
/*
 * File: HelloWorldForm.hpp
 * Purpose: 宣告 HelloWorldForm 視窗表單類別
 */

#pragma once

#include "ui/app/Form.hpp"

/// <summary>
/// 快速入門 Hello World 桌面表單
/// </summary>
class HelloWorldForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 HelloWorldForm 並初始化視窗介面。
    /// </summary>
    HelloWorldForm();

    ~HelloWorldForm() override = default;
};
```

---

## 4. 步驟二：使用 Fluent DSL 配置介面 (HelloWorldForm.cpp)

在實作檔中，於建構函式內設定視窗標題、寬高，並透過 `SetContent` 與 Fluent DSL 建構視覺元件樹：

```cpp
/*
 * File: HelloWorldForm.cpp
 * Purpose: 使用 Fluent RAD DSL 佈局 Hello World 視窗介面
 */

#include "HelloWorldForm.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/graphics/Colors.hpp"

HelloWorldForm::HelloWorldForm() {
    using namespace cppui::dsl;
    using namespace cppui::core;

    // 1. 設定視窗基本外觀與初始客戶區大小
    SetTitle("CPPUI - Hello World");
    SetFormSize(480, 260);

    // 2. 宣告式建立介面階層 (Declarative UI Layout)
    SetContent(
        VStack({
            // 標題文字
            Label("Hello, CPPUI World!")
                .FontSize(24.0f)
                .Foreground(Colors::White),

            // 副標題簡介
            Label("極簡、高效、現代化的跨平台 C++ UI 框架。")
                .FontSize(14.0f)
                .Foreground(Color(160, 160, 160, 255)),

            // 互動按鈕
            Button("關閉視窗")
                .FontSize(14.0f)
                .Padding(Thickness(16.0f, 8.0f, 16.0f, 8.0f))
                .Background(Color(45, 110, 210, 255))
                .Foreground(Colors::White)
                .CornerRadius(6.0f)
                .OnClick([this]() {
                    Close(); // 點擊按鈕關閉視窗
                })
        })
        .Padding(Thickness(32.0f))
        .Spacing(16.0f)
    );
}
```

---

## 5. 步驟三：應用程式進入點 (main.cpp)

透過靜態樣板方法 `cppui::app::Application::Run<T>()` 啟動應用程式。它會自動實例化表單、建立平台原生視窗、啟動 60 FPS 訊息泵，並在主表單關閉時優雅釋放所有系統資源：

```cpp
/*
 * File: main.cpp
 * Purpose: CPPUI 應用程式進入點
 */

#include "ui/app/Application.hpp"
#include "HelloWorldForm.hpp"

int main() {
    return cppui::app::Application::Run<HelloWorldForm>();
}
```

---

## 6. 步驟四：CMake 配置與專案建置

在您的專案 `CMakeLists.txt` 中引入 `cppui_core` 庫：

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyFirstApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入 CPPUI 標頭與核心庫
add_executable(my_first_app
    main.cpp
    HelloWorldForm.hpp
    HelloWorldForm.cpp
)

target_link_libraries(my_first_app PRIVATE cppui_core)
```

### 建置與執行命令

在 Windows (PowerShell) 環境下執行：

```powershell
# 1. 產生專案建置檔案
cmake -B build -S .

# 2. 編譯執行檔
cmake --build build --config Debug

# 3. 執行應用程式
.\build\bin\Debug\my_first_app.exe
```

---

## 7. 深入解析 (Key Highlights)

- **自動記憶體管理 (RAII)**：所有 UI 元件內部均以智慧指標託管，您無需手動調用 `new` 或 `delete`。
- **高 DPI 支援**：CPPUI 自動偵測每台螢幕的 DPI 縮放因子，所有向量圖形與字型均以邏輯像素度量，縮放自動清晰無鋸齒。
- **現代深色主題**：表單預設啟用現代沉浸式深色主題，並支援即時動態更換調色盤。

---

## 8. 下一步推薦 (Next Steps)

恭喜您完成了第一個 CPPUI 應用程式！現在您已經掌握了視窗生命週期與宣告式排版的基本概念，接著讓我們從零開始動手打造一個功能齊全、支援即時輸入驗證與動態健康狀態分析的實用桌面應用：

👉 **[建立我的第一個 BMI 計算機程式 (Tutorial: BMI Calculator)](tutorial_bmi_calculator.md)** —— 學習輸入方塊 (`TextBox`)、即時數值解析 (`compat::parse`)、按鈕事件處理與動態色彩狀態回饋。
👉 **[2D 圖形繪圖與影像濾鏡教學 (Tutorial: Graphics & Drawing)](tutorial_graphics_drawing.md)** —— 探索 GDI+ 風格 2D 向量繪圖 (`Graphics`)、漸層畫刷、幾何路徑與高效能灰階影像濾鏡 (`ImageFilters`)。
