#pragma once

#include "compat/compat.hpp"
#include "ICommand.hpp"
#include <functional>
#include <vector>

namespace cppui {
namespace mvvm {

/// <summary>
/// 輕量 Lambda 委派命令實作 (RelayCommand)
/// 允許直接以 Lambda 函式定義 Execute 與 CanExecute。
/// </summary>
class RelayCommand : public ICommand {
public:
    using ExecuteFunc = std::function<void(void*)>;
    using CanExecuteFunc = std::function<bool(void*)>;

    explicit RelayCommand(ExecuteFunc execute, CanExecuteFunc canExecute = nullptr);
    explicit RelayCommand(std::function<void()> execute, std::function<bool()> canExecute = nullptr);
    ~RelayCommand() override = default;

    COMPAT_NODISCARD bool CanExecute(void* parameter = nullptr) const override;
    void Execute(void* parameter = nullptr) override;
    void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) override;
    void NotifyCanExecuteChanged() override;

private:
    ExecuteFunc m_execute;
    CanExecuteFunc m_canExecute;
    std::vector<CanExecuteChangedHandler> m_handlers;
};

} // namespace mvvm
} // namespace cppui
