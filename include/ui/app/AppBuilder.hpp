#pragma once

#include "compat/compat.hpp"
#include "Form.hpp"
#include "PlatformBackend.hpp"
#include "ui/ioc/ServiceCollection.hpp"
#include <memory>

namespace cppui {
namespace app {

/// <summary>
/// 應用程式 Host 宿主 (AppHost)
/// 封裝已構建完成之 IoC 容器與應用程式運行調度。
/// </summary>
class AppHost {
public:
    explicit AppHost(std::shared_ptr<ioc::IServiceProvider> services);
    ~AppHost() = default;

    COMPAT_NODISCARD std::shared_ptr<ioc::IServiceProvider> Services() const noexcept {
        return m_services;
    }

    /// <summary>
    /// 從 IoC 容器解析指定 Form 型別並啟動應用程式
    /// </summary>
    template<typename TForm>
    int Run() {
        auto form = m_services->GetRequiredService<TForm>();
        return RunForm(form);
    }

    /// <summary>
    /// 啟動指定表單
    /// </summary>
    int Run(std::shared_ptr<Form> form) {
        return RunForm(form);
    }

private:
    int RunForm(std::shared_ptr<Form> form);

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
    /// 取得服務註冊集合以進行依賴注入
    /// </summary>
    COMPAT_NODISCARD ioc::ServiceCollection& Services() noexcept {
        return m_services;
    }

    /// <summary>
    /// 設定渲染實作平台後端
    /// </summary>
    AppBuilder& UsePlatformBackend(PlatformBackend backend) noexcept;

    /// <summary>
    /// 切換為 Win32 原生/軟體渲染後端
    /// </summary>
    AppBuilder& UseWin32() noexcept;

    /// <summary>
    /// 切換為 Skia 向量繪圖渲染後端
    /// </summary>
    AppBuilder& UseSkia() noexcept;

    /// <summary>
    /// 構建應用程式宿主
    /// </summary>
    COMPAT_NODISCARD AppHost Build();

private:
    ioc::ServiceCollection m_services;
    PlatformBackend m_backend{PlatformBackend::Win32};
};

} // namespace app
} // namespace cppui
