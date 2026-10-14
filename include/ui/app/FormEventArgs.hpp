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
/// 視窗內容自適應模式 (對標 WPF SizeToContent)。
/// </summary>
enum class SizeToContent : uint32_t {
    Manual = 0,         // 固定視窗尺寸，由 SetWidth/SetHeight 或 SetFormSize 指定
    Width = 1,          // 寬度依內容自適應，高度保持手動
    Height = 2,         // 高度依內容自適應，寬度保持手動
    WidthAndHeight = 3  // 寬度與高度皆依內容自適應 (AutoSize)
};

/// <summary>
/// 視窗自適應縮放約束模式 (對標 WinForms AutoSizeMode)。
/// </summary>
enum class AutoSizeMode : uint32_t {
    GrowAndShrink = 0, // 可隨內容增大或縮小
    GrowOnly = 1       // 僅可隨內容增大，不小於初始指定之尺寸
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
