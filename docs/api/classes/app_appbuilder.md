# AppBuilder 與 AppHost 類別

- **命名空間**: `cppui::app`
- **標頭檔**: `include/ui/app/AppBuilder.hpp`

---

## 概述

`AppBuilder` 與 `AppHost` 為 CPPUI 應用程式提供現代化 Generic Host 宿主與依賴反轉（IoC）架構（對標 .NET `HostApplicationBuilder` 與 `IHost`）。

`AppBuilder` 負責收集並註冊應用程式生命週期內所需的各項服務與相依項目，呼叫 `Build()` 後產出不可變之 `AppHost` 宿主。`AppHost` 自動將服務容器對接至全域 `Application`，並能從容器中自動建構並啟動目標 `Form`。

---

## 類別宣告

```cpp
namespace cppui {
namespace app {

class AppHost {
public:
    explicit AppHost(std::shared_ptr<ioc::IServiceProvider> services);
    ~AppHost() = default;

    COMPAT_NODISCARD std::shared_ptr<ioc::IServiceProvider> Services() const noexcept;

    template<typename TForm>
    int Run();

    int Run(std::shared_ptr<Form> form);
};

class AppBuilder {
public:
    AppBuilder() = default;
    ~AppBuilder() = default;

    COMPAT_NODISCARD ioc::ServiceCollection& Services() noexcept;

    // 實作平台後端配置
    AppBuilder& UsePlatformBackend(PlatformBackend backend) noexcept;
    AppBuilder& UseWin32() noexcept;
    AppBuilder& UseSkia() noexcept;

    COMPAT_NODISCARD AppHost Build();
};

} // namespace app
} // namespace cppui
```

---

## 方法說明

### `AppBuilder::Services()`
- **回傳值**: `ioc::ServiceCollection&`。
- **說明**: 取得內部的服務註冊集合，以進行 Transient 或 Singleton 依賴項目註冊。

### `AppBuilder::UsePlatformBackend(backend)` / `UseWin32()` / `UseSkia()`
- **回傳值**: `AppBuilder&`（支援鏈式呼叫）。
- **說明**: 指定應用程式之繪圖渲染實作平台。預設為 `PlatformBackend::Win32`（Windows GDI / 軟體光柵緩衝區），可透過 `UseSkia()` 切換為 Google Skia 向量繪圖核心後端。

### `AppBuilder::Build()`
- **回傳值**: `AppHost`。
- **說明**: 封閉註冊表並建置出不可變的 `IServiceProvider`，設定全域渲染平台，產出可用於啟動應用程式的 `AppHost` 實例。

### `AppHost::Services()`
- **回傳值**: `std::shared_ptr<ioc::IServiceProvider>`。
- **說明**: 取得當前宿主所持有的依賴注入服務提供者。

### `AppHost::Run<TForm>()`
- **樣板參數**: `TForm`（必須為 `Form` 之衍生類別）。
- **回傳值**: `int`（結束代碼）。
- **說明**: 自服務容器中調用 `GetRequiredService<TForm>()` 解析出表單實例，並呼叫 `Application::Run` 啟動主訊息循環。

---

## 使用範例

```cpp
#include "ui/app/Application.hpp"
#include "ui/app/AppBuilder.hpp"
#include "MainForm.hpp"

int main() {
    auto builder = cppui::app::Application::CreateBuilder();

    // 註冊服務
    builder.Services().AddSingleton<IDatabase, SqlDatabase>();
    builder.Services().AddTransient<MainForm>([](cppui::ioc::IServiceProvider& sp) {
        return std::make_shared<MainForm>(sp.GetRequiredService<IDatabase>());
    });

    // 建置宿主並解析啟動
    auto app = builder.Build();
    return app.Run<MainForm>();
}
```
