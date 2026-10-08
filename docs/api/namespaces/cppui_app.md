# cppui::app 命名空間

- **標頭檔目錄**: `include/ui/app/`

---

## 概述

`cppui::app` 命名空間提供 CPPUI 框架的現代化應用程式生命週期管理、訊息調度、表單視窗控制器以及 Generic Host 應用程式構建器。此層級封裝底層平台訊息泵與視窗事件，為應用程式提供如 .NET `System.Windows.Forms.Application`、`System.Windows.Forms.Form` 與現代 `HostApplicationBuilder` 般簡潔、直覺且具備依賴注入支持的開發體驗。

---

## 包含型別與類別

| 型別名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| [`Application`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_application.md) | `Application.hpp` | 密封型靜態應用程式中樞，負責主訊息循環調度、全域狀態管理與 IoC 依賴反轉容器對接。 |
| [`Form`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_form.md) | `Form.hpp` | 現代桌面頂層視窗控制器，封裝底層平台視窗、雙階段排版與完整生命週期事件。 |
| [`AppBuilder`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_appbuilder.md) | `AppBuilder.hpp` | 應用程式 Host 構建器，提供鏈式服務配置與依賴注入容器註冊。 |
| [`AppHost`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_appbuilder.md) | `AppBuilder.hpp` | 構建完成之應用程式宿主，負責解析目標 Form 並啟動主循環。 |
| [`FormClosingEventArgs`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_formeventargs.md) | `FormEventArgs.hpp` | 表單即將關閉事件參數，支援藉由 `Cancel = true` 攔截取消關閉行為。 |
| [`FormClosedEventArgs`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_formeventargs.md) | `FormEventArgs.hpp` | 表單完成關閉事件參數。 |
| [`ShutdownMode`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_application.md#shutdownmode) | `Application.hpp` | 應用程式終止策略列舉 (`OnMainWindowClose`, `OnLastWindowClose`, `OnExplicitShutdown`)。 |
| [`DialogResult`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_formeventargs.md) | `FormEventArgs.hpp` | 模態對話方塊回傳結果列舉 (`None`, `OK`, `Cancel`, `Yes`, `No` 等)。 |
| [`WindowState`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_formeventargs.md) | `FormEventArgs.hpp` | 視窗呈現狀態列舉 (`Normal`, `Minimized`, `Maximized`)。 |
| [`FormStartPosition`](file:///D:/P/CPP/CPPUI/docs/api/classes/app_formeventargs.md) | `FormEventArgs.hpp` | 視窗初始呈現位置列舉 (`CenterScreen`, `Manual`, `WindowsDefault`)。 |

---

## 典型使用模式

### 1. 直覺極簡啟動
```cpp
#include "ui/app/Application.hpp"
#include "MyForm.hpp"

int main() {
    return cppui::app::Application::Run(new MyForm());
}
```

### 2. 現代 Generic Host 與 IoC 注入啟動
```cpp
#include "ui/app/Application.hpp"
#include "ui/app/AppBuilder.hpp"
#include "MyForm.hpp"

int main() {
    auto builder = cppui::app::Application::CreateBuilder();
    builder.Services().AddSingleton<IMyService, MyServiceImpl>();
    builder.Services().AddTransient<MyForm>();

    auto app = builder.Build();
    return app.Run<MyForm>();
}
```
