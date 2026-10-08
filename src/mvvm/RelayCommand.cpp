#include "ui/mvvm/RelayCommand.hpp"
#include <utility>

namespace cppui {
namespace mvvm {

RelayCommand::RelayCommand(ExecuteFunc execute, CanExecuteFunc canExecute)
    : m_execute(std::move(execute)), m_canExecute(std::move(canExecute)) {}

RelayCommand::RelayCommand(std::function<void()> execute, std::function<bool()> canExecute) {
    if (execute) {
        m_execute = [execute](void*) { execute(); };
    }
    if (canExecute) {
        m_canExecute = [canExecute](void*) { return canExecute(); };
    }
}

bool RelayCommand::CanExecute(void* parameter) const {
    if (m_canExecute) {
        return m_canExecute(parameter);
    }
    return true;
}

void RelayCommand::Execute(void* parameter) {
    if (CanExecute(parameter) && m_execute) {
        m_execute(parameter);
    }
}

void RelayCommand::AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) {
    if (handler) {
        m_handlers.push_back(std::move(handler));
    }
}

void RelayCommand::NotifyCanExecuteChanged() {
    for (const auto& handler : m_handlers) {
        if (handler) {
            handler();
        }
    }
}

} // namespace mvvm
} // namespace cppui
