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

    /// <summary>
    /// 便捷靜態工廠方法：原地建構 RelayCommand 並回傳 shared_ptr
    /// </summary>
    template<typename... Args>
    static std::shared_ptr<RelayCommand> Create(Args&&... args) {
        return std::make_shared<RelayCommand>(std::forward<Args>(args)...);
    }

private:
    ExecuteFunc m_execute;
    CanExecuteFunc m_canExecute;
    std::vector<CanExecuteChangedHandler> m_handlers;
};

/// <summary>
/// 便捷命名空間工廠函數：建構具備參數的 RelayCommand
/// </summary>
inline std::shared_ptr<RelayCommand> MakeCommand(RelayCommand::ExecuteFunc execute, RelayCommand::CanExecuteFunc canExecute = nullptr) {
    return std::make_shared<RelayCommand>(std::move(execute), std::move(canExecute));
}

/// <summary>
/// 便捷命名空間工廠函數：建構無參數的 RelayCommand
/// </summary>
inline std::shared_ptr<RelayCommand> MakeCommand(std::function<void()> execute, std::function<bool()> canExecute = nullptr) {
    return std::make_shared<RelayCommand>(std::move(execute), std::move(canExecute));
}

} // namespace mvvm
} // namespace cppui
