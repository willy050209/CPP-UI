#include "ui/app/AppBuilder.hpp"
#include "ui/app/Application.hpp"
#include <utility>

namespace cppui {
namespace app {

AppHost::AppHost(std::shared_ptr<ioc::IServiceProvider> services)
    : m_services(std::move(services)) {
    Application::SetServiceProvider(m_services);
}

int AppHost::RunForm(std::shared_ptr<Form> form) {
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
