/*
 * File: AppBuilder.cpp
 * Purpose: Implements AppBuilder and AppHost for dependency injection setup and
 *          application execution with fail-fast validation.
 */

#include "ui/app/AppBuilder.hpp"
#include "ui/app/Application.hpp"
#include <stdexcept>
#include <utility>

namespace cppui {
namespace app {

AppHost::AppHost(std::shared_ptr<ioc::IServiceProvider> services)
    : m_services(std::move(services)) {
    if (!m_services) {
        throw std::invalid_argument("AppHost: services cannot be null.");
    }
    Application::SetServiceProvider(m_services);
}

int32_t AppHost::RunForm(std::shared_ptr<Form> form) {
    if (!form) {
        throw std::invalid_argument("AppHost::RunForm: form cannot be null.");
    }
    return Application::Run(form);
}

AppBuilder& AppBuilder::UsePlatformBackend(PlatformBackend backend) noexcept {
    m_backend = backend;
    return *this;
}

AppBuilder& AppBuilder::UseWin32() noexcept {
    m_backend = PlatformBackend::Win32;
    return *this;
}

AppBuilder& AppBuilder::UseSkia() noexcept {
    m_backend = PlatformBackend::Skia;
    return *this;
}

AppHost AppBuilder::Build() {
    Application::SetPlatformBackend(m_backend);
    auto provider = m_services.BuildServiceProvider();
    return AppHost(provider);
}

} // namespace app
} // namespace cppui
