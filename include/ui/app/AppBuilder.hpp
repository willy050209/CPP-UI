/*
 * File: AppBuilder.hpp
 * Purpose: Declares the modern fluent AppBuilder and AppHost for dependency injection
 *          configuration and application host bootstrapping.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include "compat/compat.hpp"
#include "Form.hpp"
#include "PlatformBackend.hpp"
#include "ui/ioc/ServiceCollection.hpp"

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
    ~AppHost() = default;

    /// <summary>
    /// 取得 IoC 服務提供者。
    /// </summary>
    /// <returns>服務提供者智慧指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<ioc::IServiceProvider> Services() const noexcept {
        return m_services;
    }

    /// <summary>
    /// 從 IoC 容器解析指定 Form 型別並啟動應用程式。
    /// </summary>
    /// <typeparam name="TForm">目標 Form 型別。</typeparam>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    template<typename TForm>
    int32_t Run() {
        auto form = m_services->GetRequiredService<TForm>();
        return RunForm(form);
    }

    /// <summary>
    /// 啟動指定表單。
    /// </summary>
    /// <param name="form">待啟動之主表單，不可為 nullptr。</param>
    /// <returns>應用程式退出代碼 (Exit Code)。</returns>
    /// <exception cref="std::invalid_argument">當 form 為 nullptr 時擲出。</exception>
    int32_t Run(std::shared_ptr<Form> form) {
        return RunForm(form);
    }

private:
    int32_t RunForm(std::shared_ptr<Form> form);

    std::shared_ptr<ioc::IServiceProvider> m_services;
};

/// <summary>
/// 現代化應用程式建構器 (AppBuilder)
/// 支援鏈式配置依賴服務與初始化 Host (對標 .NET Host.CreateApplicationBuilder)。
/// </summary>
class AppBuilder {
public:
    AppBuilder() = default;
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
    /// 構建應用程式宿主。
    /// </summary>
    /// <returns>AppHost 實例。</returns>
    COMPAT_NODISCARD AppHost Build();

private:
    ioc::ServiceCollection m_services;
    PlatformBackend m_backend{PlatformBackend::Win32};
};

} // namespace app
} // namespace cppui
