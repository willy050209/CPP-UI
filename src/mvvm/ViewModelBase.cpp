#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/app/Application.hpp"
#include <utility>
#include <string>

namespace cppui {
namespace mvvm {

void ViewModelBase::AddPropertyChangedHandler(PropertyChangedCallback handler) {
    if (handler) {
        m_handlers.push_back(std::move(handler));
    }
}

void ViewModelBase::NotifyPropertyChanged(compat::string_view propertyName) {
    if (!app::Application::CheckAccess()) {
        std::string prop = std::string(propertyName.data(), propertyName.size());
        app::Application::Post([this, prop]() {
            NotifyPropertyChanged(prop);
        });
        return;
    }

    for (const auto& handler : m_handlers) {
        if (handler) {
            handler(propertyName);
        }
    }
}

} // namespace mvvm
} // namespace cppui
