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

AppHost AppBuilder::Build() {
    auto provider = m_services.BuildServiceProvider();
    return AppHost(provider);
}

} // namespace app
} // namespace cppui
