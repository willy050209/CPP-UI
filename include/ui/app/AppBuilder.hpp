/*
 * File: AppBuilder.hpp
 * Purpose: Declares the modern fluent AppBuilder and AppHost for dependency injection
 *          configuration and application host bootstrapping.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include "compat/Compat.hpp"
#include "Form.hpp"
#include "PlatformBackend.hpp"
#include "ui/ioc/ServiceCollection.hpp"
#include "ui/diagnostics/DiagnosticsOptions.hpp"

namespace cppui {
namespace app {

/// <summary>
/// 應用程式 Host 宿主 (AppHost)
/// 封裝已構建完成之 IoC 容器與應用程式運行調度。
/// </summary>
class AppHost {
public:
    /// <summary>
    /// 建構 AppHost 實例。
    /// </summary>
    /// <param name="services">已建構之服務提供者，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 services 為 nullptr 時擲出。</exception>
    explicit AppHost(std::shared_ptr<ioc::IServiceProvider> services);
    /// <summary>
    /// 解構 AppHost 實例。
    /// </summary>
    ~AppHost() = default;

    /// <summary>
    /// 取得 IoC 服務提供者。
    /// </summary>
    /// <returns>服務提供者智慧指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<ioc::IServiceProvider> Services() const noexcept {
        return m_services;
    }

    /// <summary>
    /// 從 IoC 容器解析指定 Form 型別並啟動應用程式；若未於容器中註冊則自動原地建構實例。
    /// </summary>
    /// <typeparam name="TForm">目標 Form 型別。</typeparam>
    /// <typeparam name="Args">表單建構子引數型別包。</typeparam>
    /// <param name="args">表單建構子引數。</param>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    /// <summary>
    /// 從 IoC 容器解析指定 Form 型別並啟動應用程式；若未於容器中註冊則嘗試以無引數建構實例。
    /// </summary>
    template<typename TForm, typename... Args,
             typename std::enable_if<(sizeof...(Args) == 0), int>::type = 0>
    int32_t Run() {
        std::shared_ptr<TForm> form = nullptr;
        if (m_services) {
            form = m_services->GetService<TForm>();
        }
        if (!form) {
            form = CreateDefaultForm<TForm>();
        }
        return RunForm(form);
    }

    /// <summary>
    /// 以傳入引數建構指定 Form 型別並啟動應用程式。
    /// </summary>
    template<typename TForm, typename Arg1, typename... RestArgs>
    int32_t Run(Arg1&& arg1, RestArgs&&... restArgs) {
        std::shared_ptr<TForm> form = CreateConstructedForm<TForm>(
            std::forward<Arg1>(arg1), std::forward<RestArgs>(restArgs)...
        );
        return RunForm(form);
    }

    /// <summary>
    /// 啟動指定表單實例。
    /// </summary>
    /// <param name="form">待啟動之主表單，不可為 nullptr。</param>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    /// <exception cref="std::invalid_argument">當 form 為 nullptr 時擲出。</exception>
    int32_t Run(std::shared_ptr<Form> form) {
        return RunForm(std::move(form));
    }

private:
    template<typename TForm>
    static typename std::enable_if<std::is_default_constructible<TForm>::value, std::shared_ptr<TForm>>::type
    CreateDefaultForm() {
        return std::make_shared<TForm>();
    }

    template<typename TForm>
    static typename std::enable_if<!std::is_default_constructible<TForm>::value, std::shared_ptr<TForm>>::type
    CreateDefaultForm() {
        return nullptr;
    }

    template<typename TForm, typename... Args>
    static typename std::enable_if<std::is_constructible<TForm, Args...>::value, std::shared_ptr<TForm>>::type
    CreateConstructedForm(Args&&... args) {
        return std::make_shared<TForm>(std::forward<Args>(args)...);
    }

    template<typename TForm, typename... Args>
    static typename std::enable_if<!std::is_constructible<TForm, Args...>::value, std::shared_ptr<TForm>>::type
    CreateConstructedForm(Args&&...) {
        return nullptr;
    }

    int32_t RunForm(std::shared_ptr<Form> form);

    std::shared_ptr<ioc::IServiceProvider> m_services;
};

/// <summary>
/// 現代化應用程式建構器 (AppBuilder)
/// 支援鏈式配置依賴服務與初始化 Host (對標 .NET Host.CreateApplicationBuilder)。
/// </summary>
class AppBuilder {
public:
    /// <summary>
    /// 預設建構 AppBuilder 實例。
    /// </summary>
    AppBuilder() = default;

    /// <summary>
    /// 解構 AppBuilder 實例。
    /// </summary>
    ~AppBuilder() = default;

    /// <summary>
    /// 取得服務註冊集合以進行依賴注入。
    /// </summary>
    /// <returns>ServiceCollection 引用。</returns>
    COMPAT_NODISCARD ioc::ServiceCollection& Services() noexcept {
        return m_services;
    }

    /// <summary>
    /// 設定渲染實作平台後端。
    /// </summary>
    /// <param name="backend">PlatformBackend 列舉值。</param>
    /// <returns>AppBuilder 引用支援鏈式調用。</returns>
    AppBuilder& UsePlatformBackend(PlatformBackend backend) noexcept;

    /// <summary>
    /// 切換為 Win32 原生/軟體渲染後端。
    /// </summary>
    /// <returns>AppBuilder 引用支援鏈式調用。</returns>
    AppBuilder& UseWin32() noexcept;

    /// <summary>
    /// 切換為 Skia 向量繪圖渲染後端。
    /// </summary>
    /// <returns>AppBuilder 引用支援鏈式調用。</returns>
    AppBuilder& UseSkia() noexcept;

    /// <summary>
    /// 配置全域執行期診斷選項 (Dirty Rect 高亮、FPS 計數器、Visual Tree Inspector)。
    /// </summary>
    /// <param name="options">診斷選項結構。</param>
    /// <returns>AppBuilder 引用支援鏈式調用。</returns>
    AppBuilder& WithDiagnostics(const diagnostics::DiagnosticsOptions& options) noexcept;

    /// <summary>
    /// 構建應用程式宿主。
    /// </summary>
    /// <returns>AppHost 實例。</returns>
    COMPAT_NODISCARD AppHost Build();

private:
    ioc::ServiceCollection m_services;
    PlatformBackend m_backend{PlatformBackend::Win32};
    diagnostics::DiagnosticsOptions m_diagnosticsOptions;
};

} // namespace app
} // namespace cppui
