#pragma once

#include "compat/compat.hpp"
#include "Form.hpp"
#include "PlatformBackend.hpp"
#include "Dispatcher.hpp"
#include "ui/ioc/IServiceProvider.hpp"
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cppui {
namespace app {

class AppBuilder;

/// <summary>
/// 應用程式終止策略 (對標 WPF ShutdownMode)
/// </summary>
enum class ShutdownMode {
    OnMainWindowClose, // 主表單關閉時終止應用程式 (WinForms 預設)
    OnLastWindowClose, // 所有開啟表單皆關閉時終止 (WPF 預設)
    OnExplicitShutdown // 僅在顯式呼叫 Application::Exit() 時終止
};

/// <summary>
/// 現代化應用程式靜態中樞 (Application)
/// 提供全域應用程式生命週期管理、訊息泵調度、全域狀態查詢與 IoC 依賴注入中繼。
/// 此類別為密封類別，不可繼承，所有功能皆透過靜態方法與屬性存取。
/// </summary>
class Application final {
public:
    Application() = delete;
    ~Application() = delete;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // --- 訊息泵循環與啟動 ---
    /// <summary>
    /// 啟動無主視窗之應用程式訊息循環
    /// </summary>
    static int Run();

    /// <summary>
    /// 接管 raw 指標並啟動主表單訊息循環 (自動安全轉換為 shared_ptr，防洩漏)
    /// 例如: Application::Run(new Form1());
    /// </summary>
    static int Run(Form* mainForm);

    /// <summary>
    /// 以智慧指標啟動主表單訊息循環
    /// </summary>
    static int Run(std::shared_ptr<Form> mainForm);

    /// <summary>
    /// 便捷樣板：原地建構指定 Form 型別並啟動訊息循環
    /// </summary>
    template<typename TForm, typename... Args>
    static int Run(Args&&... args) {
        auto form = std::make_shared<TForm>(std::forward<Args>(args)...);
        return Run(form);
    }

    // --- 訊息泵終止與退出 ---
    /// <summary>
    /// 請求退出應用程式訊息泵並關閉所有開啟表單
    /// </summary>
    static void Exit();

    /// <summary>
    /// 退出當前執行緒訊息泵
    /// </summary>
    static void ExitThread();

    /// <summary>
    /// 非阻塞處理當前訊息佇列中的待處理事件，維持 UI 即時響應
    /// </summary>
    static void DoEvents();

    // --- 狀態與表單查詢 ---
    COMPAT_NODISCARD static bool IsRunning() noexcept;
    COMPAT_NODISCARD static std::shared_ptr<Form> GetActiveForm() noexcept;
    COMPAT_NODISCARD static std::vector<std::shared_ptr<Form>> GetOpenForms();
    COMPAT_NODISCARD static ShutdownMode GetShutdownMode() noexcept;
    static void SetShutdownMode(ShutdownMode mode) noexcept;

    // --- 實作平台 / 渲染後端選擇 ---
    COMPAT_NODISCARD static PlatformBackend GetPlatformBackend() noexcept;
    static void SetPlatformBackend(PlatformBackend backend) noexcept;

    // --- 非同步事件調度與喚醒 (Dispatcher) ---
    /// <summary>
    /// 將非同步任務投遞至 UI 執行緒執行 (對標 WPF Dispatcher.InvokeAsync / Post)
    /// </summary>
    static void Post(std::function<void()> action);

    /// <summary>
    /// 檢查呼叫端是否運行於 UI 主執行緒
    /// </summary>
    COMPAT_NODISCARD static bool CheckAccess() noexcept;

    /// <summary>
    /// 取得全域 Dispatcher 調度器實例
    /// </summary>
    COMPAT_NODISCARD static Dispatcher& GetDispatcher() noexcept;

    /// <summary>
    /// 內部喚醒 UI 訊息迴圈 (支援跨執行緒安全調用)
    /// </summary>
    static void WakeUpUIThread();

    // --- 應用程式環境中繼資訊 ---
    COMPAT_NODISCARD static compat::string_view GetExecutablePath();
    COMPAT_NODISCARD static compat::string_view GetStartupPath();
    COMPAT_NODISCARD static compat::string_view GetProductName();
    static void SetProductName(compat::string_view name);
    COMPAT_NODISCARD static compat::string_view GetCompanyName();
    static void SetCompanyName(compat::string_view name);

    // --- 全域事件回呼 ---
    using ExitCallback = std::function<void()>;
    using IdleCallback = std::function<void()>;
    static void AddExitHandler(ExitCallback handler);
    static void AddIdleHandler(IdleCallback handler);

    // --- 依賴反轉 (IoC) 全域容器整合 ---
    static void SetServiceProvider(std::shared_ptr<ioc::IServiceProvider> provider);
    COMPAT_NODISCARD static std::shared_ptr<ioc::IServiceProvider> GetServices() noexcept;

    template<typename TService>
    COMPAT_NODISCARD static std::shared_ptr<TService> GetService() {
        auto sp = GetServices();
        return sp ? sp->GetService<TService>() : nullptr;
    }

    template<typename TService>
    COMPAT_NODISCARD static std::shared_ptr<TService> GetRequiredService() {
        auto sp = GetServices();
        if (!sp) {
            compat::println("[Application] 錯誤：未配置 IoC ServiceProvider！");
            return nullptr;
        }
        return sp->GetRequiredService<TService>();
    }

    // --- 表單註冊內部介面 (由 Form 生命週期調用) ---
    static void RegisterForm(std::shared_ptr<Form> form);
    static void UnregisterForm(Form* form);
    static void SetActiveForm(std::shared_ptr<Form> form);

    // --- 現代 Host 構建器入口 ---
    static AppBuilder CreateBuilder();
};

} // namespace app
} // namespace cppui
