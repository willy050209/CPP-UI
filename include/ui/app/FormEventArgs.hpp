/**
 * @file FormEventArgs.hpp
 * @brief 表單生命週期事件參數及狀態列舉定義 (DialogResult, WindowState, FormClosingEventArgs)
 */

#pragma once

#include "compat/compat.hpp"

namespace cppui {
namespace app {

/// <summary>
/// 對話框回傳結果代碼 (對標 .NET DialogResult)
/// </summary>
enum class DialogResult {
    None,
    OK,
    Cancel,
    Abort,
    Retry,
    Ignore,
    Yes,
    No
};

/// <summary>
/// 視窗啟動位置策略 (對標 .NET FormStartPosition)
/// </summary>
enum class FormStartPosition {
    Manual,
    CenterScreen,
    WindowsDefaultLocation
};

/// <summary>
/// 視窗顯示狀態 (WindowState)
/// </summary>
enum class WindowState {
    Normal,
    Minimized,
    Maximized
};

/// <summary>
/// 視窗即將關閉事件參數 (FormClosingEventArgs)
/// 允許透過設定 Cancel = true 取消關閉操作。
/// </summary>
class FormClosingEventArgs {
public:
    bool Cancel = false;
    bool CloseReasonIsUser = true;
};

/// <summary>
/// 視窗關閉完成事件參數 (FormClosedEventArgs)
/// </summary>
class FormClosedEventArgs {
public:
    bool CloseReasonIsUser = true;
};

} // namespace app
} // namespace cppui
