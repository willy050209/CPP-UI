/**
 * @file RelayCommand.hpp
 * @brief 輕量 Lambda 委派命令實作，支援以可呼叫物件定義 Execute 與 CanExecute
 */

#pragma once

#include "compat/Compat.hpp"
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
    /// <summary>
    /// 具參數之執行回呼委派。
    /// </summary>
    using ExecuteFunc = std::function<void(void*)>;

    /// <summary>
    /// 具參數之可執行性判斷委派。
    /// </summary>
    using CanExecuteFunc = std::function<bool(void*)>;

    /// <summary>
    /// 建構具參數的委派命令。
    /// </summary>
    /// <param name="execute">執行回呼函式。</param>
    /// <param name="canExecute">可執行性判斷函式，預設為 nullptr (即永遠為 true)。</param>
    explicit RelayCommand(ExecuteFunc execute, CanExecuteFunc canExecute = nullptr);

    /// <summary>
    /// 建構無參數的委派命令。
    /// </summary>
    /// <param name="execute">無參數執行回呼函式。</param>
    /// <param name="canExecute">無參數判斷函式，預設為 nullptr (即永遠為 true)。</param>
    explicit RelayCommand(std::function<void()> execute, std::function<bool()> canExecute = nullptr);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~RelayCommand() override = default;

    /// <summary>
    /// 檢查命令當前是否可執行。
    /// </summary>
    /// <param name="parameter">執行參數指標。</param>
    /// <returns>若可執行傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool CanExecute(void* parameter = nullptr) const override;

    /// <summary>
    /// 執行命令。
    /// </summary>
    /// <param name="parameter">執行參數指標。</param>
    void Execute(void* parameter = nullptr) override;

    /// <summary>
    /// 註冊 CanExecute 狀態變更監聽器。
    /// </summary>
    /// <param name="handler">狀態變更回呼函式。</param>
    void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) override;

    /// <summary>
    /// 廣播通知 CanExecute 狀態已變更。
    /// </summary>
    void NotifyCanExecuteChanged() override;

    /// <summary>
    /// 便捷靜態工廠方法：原地建構 RelayCommand 並回傳 shared_ptr。
    /// </summary>
    /// <typeparam name="Args">轉發建構引數型別。</typeparam>
    /// <param name="args">建構子引數。</param>
    /// <returns>RelayCommand 的 shared_ptr 實例。</returns>
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
/// 便捷命名空間工廠函數：建構具備參數的 RelayCommand。
/// </summary>
/// <param name="execute">執行回呼函式。</param>
/// <param name="canExecute">可執行性判斷函式。</param>
/// <returns>RelayCommand 的 shared_ptr 實例。</returns>
inline std::shared_ptr<RelayCommand> MakeCommand(RelayCommand::ExecuteFunc execute, RelayCommand::CanExecuteFunc canExecute = nullptr) {
    return std::make_shared<RelayCommand>(std::move(execute), std::move(canExecute));
}

/// <summary>
/// 便捷命名空間工廠函數：建構無參數的 RelayCommand。
/// </summary>
/// <param name="execute">無參數執行回呼函式。</param>
/// <param name="canExecute">無參數判斷函式。</param>
/// <returns>RelayCommand 的 shared_ptr 實例。</returns>
inline std::shared_ptr<RelayCommand> MakeCommand(std::function<void()> execute, std::function<bool()> canExecute = nullptr) {
    return std::make_shared<RelayCommand>(std::move(execute), std::move(canExecute));
}

} // namespace mvvm
} // namespace cppui
