# Application 類別 (cppui::app::Application)

提供全域靜態方法與屬性以管理應用程式生命週期，包含訊息泵循環、視窗登記、事件調度與依賴注入中繼（對標 .NET `System.Windows.Forms.Application`）。此類別為密封類別 (`final`)，不可被繼承。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::app`](../namespaces/cppui_app.md)
* **標頭檔**：`#include <ui/app/Application.hpp>`
* **模組**：`CPPUI Application Framework`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace app {

enum class ShutdownMode {
    OnMainWindowClose, // 主表單關閉時終止應用程式 (WinForms 預設)
    OnLastWindowClose, // 所有開啟表單皆關閉時終止 (WPF 預設)
    OnExplicitShutdown // 僅在顯式呼叫 Application::Exit() 時終止
};

enum class PlatformBackend {
    Win32, // 原生 Win32 GDI / 軟體光柵緩衝區渲染後端
    Skia   // Google Skia 向量繪圖渲染後端
};

class Application final {
public:
    Application() = delete;
    ~Application() = delete;

    // 啟動應用程式訊息泵
    static int Run();
    static int Run(Form* mainForm);
    static int Run(std::shared_ptr<Form> mainForm);

    template<typename TForm, typename... Args>
    static int Run(Args&&... args);

    // 終止與退出
    static void Exit();
    static void ExitThread();

    // 處理當前訊息佇列
    static void DoEvents();

    // 狀態查詢
    COMPAT_NODISCARD static bool IsRunning() noexcept;
    COMPAT_NODISCARD static std::shared_ptr<Form> GetActiveForm() noexcept;
    COMPAT_NODISCARD static std::vector<std::shared_ptr<Form>> GetOpenForms();
    COMPAT_NODISCARD static ShutdownMode GetShutdownMode() noexcept;
    static void SetShutdownMode(ShutdownMode mode) noexcept;

    // 實作平台 / 渲染後端選擇
    COMPAT_NODISCARD static PlatformBackend GetPlatformBackend() noexcept;
    static void SetPlatformBackend(PlatformBackend backend) noexcept;

    // 非同步事件調度與喚醒 (Dispatcher)
    static void Post(std::function<void()> action);
    COMPAT_NODISCARD static bool CheckAccess() noexcept;
    COMPAT_NODISCARD static Dispatcher& GetDispatcher() noexcept;
    static void WakeUpUIThread();

    // 應用程式環境中繼資訊
    COMPAT_NODISCARD static compat::string_view GetExecutablePath();
    COMPAT_NODISCARD static compat::string_view GetStartupPath();
    COMPAT_NODISCARD static compat::string_view GetProductName();
    static void SetProductName(compat::string_view name);
    COMPAT_NODISCARD static compat::string_view GetCompanyName();
    static void SetCompanyName(compat::string_view name);

    // 全域事件回呼
    using ExitCallback = std::function<void()>;
    using IdleCallback = std::function<void()>;
    static void AddExitHandler(ExitCallback handler);
    static void AddIdleHandler(IdleCallback handler);

    // 依賴反轉 (IoC) 全域容器
    static void SetServiceProvider(std::shared_ptr<ioc::IServiceProvider> provider);
    COMPAT_NODISCARD static std::shared_ptr<ioc::IServiceProvider> GetServices() noexcept;

    template<typename TService>
    COMPAT_NODISCARD static std::shared_ptr<TService> GetService();

    template<typename TService>
    COMPAT_NODISCARD static std::shared_ptr<TService> GetRequiredService();

    // 現代 Host 構建器入口
    static AppBuilder CreateBuilder();
};

} // namespace app
} // namespace cppui
```

---

## 簡介 (Description)

`Application` 是 CPPUI 桌面應用的核心靜態中樞：
1. **極速進入點**：提供 `Application::Run(new Form1())`，內部自動以 `std::shared_ptr<Form>` 安全接管表單所有權，防範記憶體洩漏。
2. **訊息循環封裝**：整合底層 PAL 訊息循環，自動分派事件至開啟的表單中，並依據 `ShutdownMode` 自動偵測終止條件。
3. **DoEvents 支援**：在長耗時任務中調用 `Application::DoEvents()` 可立即處理排隊的視窗訊息與繪圖請求，防止介面凍結。
4. **IoC 整合**：內建全域 `IServiceProvider` 存取介面，或透過 `Application::CreateBuilder()` 使用現代 Generic Host 風格進行服務與表單注入。

---

## 靜態方法說明 (Static Methods)

### `Run(Form* mainForm)` / `Run(std::shared_ptr<Form> mainForm)`
```cpp
static int Run(Form* mainForm);
static int Run(std::shared_ptr<Form> mainForm);
```
啟動主執行緒訊息泵並顯示指定的主表單。當主表單關閉時（在 `OnMainWindowClose` 模式下），訊息循環自動退出並清理所有開啟視窗。

### `DoEvents()`
```cpp
static void DoEvents();
```
非阻塞處理當前作業系統訊息佇列中所有待處理事件與非同步 Dispatcher 任務。

### `Post(std::function<void()> action)`
```cpp
static void Post(std::function<void()> action);
```
將非同步回呼操作投遞至 UI 主執行緒任務佇列（呼叫底層 Dispatcher），並立即喚醒處於等待狀態的 UI 訊息循環。

### `CheckAccess()`
```cpp
COMPAT_NODISCARD static bool CheckAccess() noexcept;
```
判定當前呼叫端執行緒是否為 UI 主執行緒。

### `SetPlatformBackend(PlatformBackend backend)` / `GetPlatformBackend()`
```cpp
static void SetPlatformBackend(PlatformBackend backend) noexcept;
COMPAT_NODISCARD static PlatformBackend GetPlatformBackend() noexcept;
```
取得或設定應用程式渲染實作平台（`PlatformBackend::Win32` 或 `PlatformBackend::Skia`）。表單在建立時將依據此全域配置初始化對應之渲染環境。

### `Exit()`
```cpp
static void Exit();
```
請求終止訊息泵，安全調用所有開啟表單的 `Close()` 與生命週期事件，並觸發 `ApplicationExit` 回呼。

### `CreateBuilder()`
```cpp
static AppBuilder CreateBuilder();
```
建立現代化應用程式建構器，支援在啟動前以鏈式語法向 `ServiceCollection` 註冊依賴服務與配置渲染平台。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"

class MyMainForm : public cppui::app::Form {
public:
    MyMainForm() {
        SetTitle("我的主視窗");
        SetFormSize(800, 600);
    }
};

int main() {
    // 極簡單行啟動主視窗
    return cppui::app::Application::Run(new MyMainForm());
}
```

---

## 請參閱 (See Also)
* [`Dispatcher` 任務調度器](app_dispatcher.md)
* [`Form` 視窗基底類別](app_form.md)
* [`AppBuilder` 應用程式建構器](app_form.md#appbuilder-與-apphost)
* [`ServiceCollection` 服務註冊集合](ioc_servicecollection.md)
* [`現代 MVVM 與 IoC 架構指南`](../guides/mvvm_and_ioc_architecture.md)
