/*
 * File: Application.hpp
 * Purpose: Declares the central Application static hub for lifecycle management,
 *          message dispatching, thread marshalling, and IoC service resolution.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "compat/compat.hpp"
#include "Form.hpp"
#include "PlatformBackend.hpp"
#include "Dispatcher.hpp"
#include "ui/core/Exceptions.hpp"
#include "ui/ioc/IServiceProvider.hpp"

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
    /// 啟動無主視窗之應用程式訊息循環 (Headless 模式)。
    /// </summary>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    static int32_t Run();

    /// <summary>
    /// 接管 raw 指標並啟動主表單訊息循環 (自動安全轉換為 shared_ptr，防洩漏)。
    /// </summary>
    /// <param name="mainForm">主表單 raw 指標，嚴禁傳入 nullptr。</param>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    /// <exception cref="std::invalid_argument">當 mainForm 為 nullptr 時擲出。</exception>
    static int32_t Run(Form* mainForm);

    /// <summary>
    /// 以智慧指標啟動主表單訊息循環。
    /// </summary>
    /// <param name="mainForm">主表單智慧指標，嚴禁傳入 nullptr。</param>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    /// <exception cref="std::invalid_argument">當 mainForm 為 nullptr 時擲出。</exception>
    static int32_t Run(std::shared_ptr<Form> mainForm);

    /// <summary>
    /// 便捷樣板：原地建構指定 Form 型別並啟動訊息循環。
    /// </summary>
    /// <typeparam name="TForm">衍生自 Form 之表單型別。</typeparam>
    /// <param name="args">表單建構子引數。</param>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    template<typename TForm, typename... Args>
    static int32_t Run(Args&&... args) {
        auto form = std::make_shared<TForm>(std::forward<Args>(args)...);
        return Run(form);
    }

    // --- 訊息泵終止與退出 ---

    /// <summary>
    /// 請求退出應用程式訊息泵並關閉所有開啟表單。
    /// </summary>
    static void Exit();

    /// <summary>
    /// 退出當前執行緒訊息泵。
    /// </summary>
    static void ExitThread();

    /// <summary>
    /// 非阻塞處理當前訊息佇列中的待處理事件，維持 UI 即時響應。
    /// </summary>
    static void DoEvents();

    // --- 狀態與表單查詢 ---

    /// <summary>
    /// 查詢應用程式是否正運行於訊息循環中。
    /// </summary>
    /// <returns>若正運行中傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool IsRunning() noexcept;

    /// <summary>
    /// 取得當前處於焦點之作用中表單。
    /// </summary>
    /// <returns>作用中表單之 shared_ptr，若無則傳回 nullptr。</returns>
    COMPAT_NODISCARD static std::shared_ptr<Form> GetActiveForm() noexcept;

    /// <summary>
    /// 取得目前所有已開啟之表單列表。
    /// </summary>
    /// <returns>所有已開啟表單之 vector 清單。</returns>
    COMPAT_NODISCARD static std::vector<std::shared_ptr<Form>> GetOpenForms();

    /// <summary>
    /// 取得當前應用程式終止模式。
    /// </summary>
    /// <returns>ShutdownMode 列舉值。</returns>
    COMPAT_NODISCARD static ShutdownMode GetShutdownMode() noexcept;

    /// <summary>
    /// 設定應用程式終止模式。
    /// </summary>
    /// <param name="mode">ShutdownMode 列舉值。</param>
    static void SetShutdownMode(ShutdownMode mode) noexcept;

    // --- 實作平台 / 渲染後端選擇 ---

    /// <summary>
    /// 取得當前配置之圖形渲染與視窗平台後端。
    /// </summary>
    /// <returns>PlatformBackend 列舉值。</returns>
    COMPAT_NODISCARD static PlatformBackend GetPlatformBackend() noexcept;

    /// <summary>
    /// 設定圖形渲染與視窗平台後端 (Win32 / Skia)。
    /// </summary>
    /// <param name="backend">PlatformBackend 列舉值。</param>
    static void SetPlatformBackend(PlatformBackend backend) noexcept;

    // --- 非同步事件調度與喚醒 (Dispatcher) ---

    /// <summary>
    /// 將非同步任務投遞至 UI 執行緒執行 (對標 WPF Dispatcher.InvokeAsync / Post)。
    /// </summary>
    /// <param name="action">待排隊執行的任務回呼，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 action 為空時擲出。</exception>
    static void Post(std::function<void()> action);

    /// <summary>
    /// 檢查呼叫端是否運行於 UI 主執行緒。
    /// </summary>
    /// <returns>若處於 UI 主執行緒傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool CheckAccess() noexcept;

    /// <summary>
    /// 取得全域 Dispatcher 調度器實例。
    /// </summary>
    /// <returns>全域 Dispatcher 引用。</returns>
    COMPAT_NODISCARD static Dispatcher& GetDispatcher() noexcept;

    /// <summary>
    /// 內部喚醒 UI 訊息迴圈 (支援跨執行緒安全調用)。
    /// </summary>
    static void WakeUpUIThread();

    // --- 應用程式環境中繼資訊 ---

    /// <summary>
    /// 取得當前執行檔完整路徑。
    /// </summary>
    /// <returns>執行檔路徑字串視圖。</returns>
    COMPAT_NODISCARD static compat::string_view GetExecutablePath();

    /// <summary>
    /// 取得應用程式啟動目錄路徑。
    /// </summary>
    /// <returns>啟動目錄字串視圖。</returns>
    COMPAT_NODISCARD static compat::string_view GetStartupPath();

    /// <summary>
    /// 取得應用程式產品名稱。
    /// </summary>
    /// <returns>產品名稱字串視圖。</returns>
    COMPAT_NODISCARD static compat::string_view GetProductName();

    /// <summary>
    /// 設定應用程式產品名稱。
    /// </summary>
    /// <param name="name">產品名稱字串視圖。</param>
    static void SetProductName(compat::string_view name);

    /// <summary>
    /// 取得應用程式發行公司名稱。
    /// </summary>
    /// <returns>公司名稱字串視圖。</returns>
    COMPAT_NODISCARD static compat::string_view GetCompanyName();

    /// <summary>
    /// 設定應用程式發行公司名稱。
    /// </summary>
    /// <param name="name">公司名稱字串視圖。</param>
    static void SetCompanyName(compat::string_view name);

    // --- 全域事件回呼 ---
    using ExitCallback = std::function<void()>;
    using IdleCallback = std::function<void()>;

    /// <summary>
    /// 註冊應用程式結束事件回呼。
    /// </summary>
    /// <param name="handler">結束回呼函式。</param>
    static void AddExitHandler(ExitCallback handler);

    /// <summary>
    /// 註冊應用程式閒置事件回呼。
    /// </summary>
    /// <param name="handler">閒置回呼函式。</param>
    static void AddIdleHandler(IdleCallback handler);

    // --- 依賴反轉 (IoC) 全域容器整合 ---

    /// <summary>
    /// 配置全域 IoC 服務提供者。
    /// </summary>
    /// <param name="provider">服務提供者實例，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 provider 為 nullptr 時擲出。</exception>
    static void SetServiceProvider(std::shared_ptr<ioc::IServiceProvider> provider);

    /// <summary>
    /// 取得全域 IoC 服務提供者。
    /// </summary>
    /// <returns>服務提供者指標，若尚未配置傳回 nullptr。</returns>
    COMPAT_NODISCARD static std::shared_ptr<ioc::IServiceProvider> GetServices() noexcept;

    /// <summary>
    /// 從全域 IoC 容器解析選擇性服務。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <returns>服務實例智慧指標，若未註冊或未配置容器傳回 nullptr。</returns>
    template<typename TService>
    COMPAT_NODISCARD static std::shared_ptr<TService> GetService() {
        auto sp = GetServices();
        return sp ? sp->GetService<TService>() : nullptr;
    }

    /// <summary>
    /// 從全域 IoC 容器解析必要服務，若容器未配置或服務不存在立即擲出例外。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <returns>服務實例智慧指標。</returns>
    /// <exception cref="cppui::core::InvalidOperationException">當尚未配置 IoC ServiceProvider 時擲出。</exception>
    template<typename TService>
    COMPAT_NODISCARD static std::shared_ptr<TService> GetRequiredService() {
        auto sp = GetServices();
        if (!sp) {
            throw cppui::core::InvalidOperationException("Application::GetRequiredService: ServiceProvider is not configured.");
        }
        auto svc = sp->GetService<TService>();
        if (!svc) {
            throw cppui::core::InvalidOperationException("Application::GetRequiredService: Required service was not found in ServiceProvider.");
        }
        return svc;
    }

    // --- 表單註冊內部介面 (由 Form 生命週期調用) ---

    /// <summary>
    /// 註冊開啟中表單實例至全域清單。
    /// </summary>
    /// <param name="form">表單實例智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 form 為 nullptr 時擲出。</exception>
    static void RegisterForm(std::shared_ptr<Form> form);

    /// <summary>
    /// 從全域清單註銷已關閉或處置之表單。
    /// </summary>
    /// <param name="form">表單 raw 指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 form 為 nullptr 時擲出。</exception>
    static void UnregisterForm(Form* form);

    /// <summary>
    /// 設定當前作用中表單。
    /// </summary>
    /// <param name="form">表單實例智慧指標。</param>
    static void SetActiveForm(std::shared_ptr<Form> form);

    // --- 現代 Host 構建器入口 ---

    /// <summary>
    /// 建立應用程式建構器 AppBuilder 實例。
    /// </summary>
    /// <returns>AppBuilder 構建器物件。</returns>
    static AppBuilder CreateBuilder();
};

} // namespace app
} // namespace cppui
