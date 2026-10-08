#pragma once

#include "compat/compat.hpp"
#include <functional>

namespace cppui {
namespace mvvm {

/// <summary>
/// 命令模式抽象介面 (ICommand)
/// 封裝 UI 行為邏輯與可執行性查詢 (對標 WPF/Avalonia ICommand)。
/// </summary>
class ICommand {
public:
    virtual ~ICommand() = default;

    /// <summary>
    /// 檢查命令當前是否可執行
    /// </summary>
    COMPAT_NODISCARD virtual bool CanExecute(void* parameter = nullptr) const = 0;

    /// <summary>
    /// 執行命令動作
    /// </summary>
    virtual void Execute(void* parameter = nullptr) = 0;

    using CanExecuteChangedHandler = std::function<void()>;

    /// <summary>
    /// 註冊 CanExecute 狀態變更監聽器
    /// </summary>
    virtual void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) = 0;

    /// <summary>
    /// 廣播通知 CanExecute 狀態已變更
    /// </summary>
    virtual void NotifyCanExecuteChanged() = 0;
};

} // namespace mvvm
} // namespace cppui
