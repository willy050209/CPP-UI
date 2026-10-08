/*
 * File: Button.hpp
 * Purpose: Declares Button control with click routing, states, and MVVM command execution.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include "compat/compat.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"

namespace cppui {
namespace mvvm {
class ICommand;
} // namespace mvvm

namespace controls {

class Button;

/// <summary>
/// 按鈕點擊路由事件參數 (ClickEventArgs)
/// </summary>
class ClickEventArgs : public input::RoutedEventArgs {
public:
    explicit ClickEventArgs(Button* source) noexcept;
};

using ClickEventHandler = std::function<void(Button&)>;

/// <summary>
/// 互動式按鈕控制項 (Button)
/// 實作完整的滑鼠懸停、按壓、獲焦霓虹外框與鍵盤空白鍵/Enter觸發狀態機，提供 Click 路由事件。
/// </summary>
class Button : public ContentControl {
public:
    static const input::RoutedEvent ClickEvent;

    Button();
    explicit Button(compat::string_view text);
    ~Button() override = default;

    COMPAT_NODISCARD bool IsPressed() const noexcept { return m_isPressed; }
    COMPAT_NODISCARD bool IsHovered() const noexcept { return m_isHovered; }
    COMPAT_NODISCARD bool IsEnabled() const noexcept { return m_isEnabled; }
    void SetEnabled(bool enabled);

    // --- 樣式調色盤配置 ---
    void SetHoverBackground(core::Color color);
    void SetPressedBackground(core::Color color);
    void SetFocusBorderBrush(core::Color color);

    // --- 事件綁定便利函式 ---
    void OnClick(ClickEventHandler handler);
    void OnClick(std::function<void()> handler);

    // --- MVVM 命令綁定 ---

    /// <summary>
    /// 綁定 MVVM 命令 (ICommand) 至按鈕。
    /// </summary>
    /// <param name="command">命令實例智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 command 為 nullptr 時擲出。</exception>
    void SetCommand(std::shared_ptr<mvvm::ICommand> command);

    COMPAT_NODISCARD std::shared_ptr<mvvm::ICommand> GetCommand() const noexcept { return m_command; }

    void OnRender(rendering::DrawingContext& dc) override;

protected:
    void RaiseClick();
    void UpdateEffectiveColors();

    bool        m_isPressed = false;
    bool        m_isHovered = false;
    bool        m_isEnabled = true;
    std::shared_ptr<mvvm::ICommand> m_command;

    core::Color m_normalBackground{ 45, 52, 70, 255 };
    core::Color m_hoverBackground{ 60, 75, 105, 255 };
    core::Color m_pressedBackground{ 32, 40, 58, 255 };
    core::Color m_disabledBackground{ 35, 38, 45, 255 };

    core::Color m_normalBorder{ 70, 80, 100, 255 };
    core::Color m_focusBorder{ 0, 160, 255, 255 }; // 獲焦時藍色霓虹框
};

} // namespace controls
} // namespace cppui
