/**
 * @file ICommand.hpp
 * @brief 命令模式抽象介面，封裝 UI 觸發行為與執行能力狀態查詢
 */

#pragma once

#include "compat/Compat.hpp"
#include <functional>

namespace cppui {
namespace mvvm {

/// <summary>
/// 命令模式抽象介面 (ICommand)
/// 封裝 UI 行為邏輯與可執行性查詢 (對標 WPF/Avalonia ICommand)。
/// </summary>
class ICommand {
public:
    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    virtual ~ICommand() = default;

    /// <summary>
    /// 檢查命令當前是否可執行。
    /// </summary>
    /// <param name="parameter">命令執行參數指標，預設為 nullptr。</param>
    /// <returns>若可執行傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD virtual bool CanExecute(void* parameter = nullptr) const = 0;

    /// <summary>
    /// 執行命令動作。
    /// </summary>
    /// <param name="parameter">命令執行參數指標，預設為 nullptr。</param>
    virtual void Execute(void* parameter = nullptr) = 0;

    /// <summary>
    /// CanExecute 狀態變更回呼委派。
    /// </summary>
    using CanExecuteChangedHandler = std::function<void()>;

    /// <summary>
    /// 註冊 CanExecute 狀態變更監聽器。
    /// </summary>
    /// <param name="handler">狀態變更回呼函式。</param>
    virtual void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) = 0;

    /// <summary>
    /// 廣播通知 CanExecute 狀態已變更。
    /// </summary>
    virtual void NotifyCanExecuteChanged() = 0;
};

} // namespace mvvm
} // namespace cppui
