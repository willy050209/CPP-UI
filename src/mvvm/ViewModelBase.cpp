#include "ui/mvvm/ViewModelBase.hpp"
#include <utility>

namespace cppui {
namespace mvvm {

void ViewModelBase::AddPropertyChangedHandler(PropertyChangedCallback handler) {
    if (handler) {
        m_handlers.push_back(std::move(handler));
    }
}

void ViewModelBase::NotifyPropertyChanged(compat::string_view propertyName) {
    for (const auto& handler : m_handlers) {
        if (handler) {
            handler(propertyName);
        }
    }
}

} // namespace mvvm
} // namespace cppui
