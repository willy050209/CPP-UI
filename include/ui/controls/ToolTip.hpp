/*
 * File: ToolTip.hpp
 * Purpose: Declares ToolTip visual control.
 */

#pragma once

#include <string>
#include <memory>
#include "compat/Compat.hpp"
#include "ui/controls/ContentControl.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 輕量懸停提示控制項 (ToolTip)
/// 顯示輔助說明文字或自訂視覺內容之浮動提示標籤。
/// </summary>
class ToolTip : public ContentControl {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    ToolTip();

    /// <summary>
    /// 建構純文字提示實例。
    /// </summary>
    /// <param name="text">提示字串視圖。</param>
    explicit ToolTip(compat::string_view text);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~ToolTip() override = default;

    /// <summary>
    /// 套用主題樣式。
    /// </summary>
    /// <param name="theme">Theme 常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;
};

} // namespace controls
} // namespace cppui
