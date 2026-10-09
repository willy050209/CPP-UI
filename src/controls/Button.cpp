/*
 * File: Button.cpp
 * Purpose: Implements Button interaction state machine, routing, and command execution.
 */

#include "ui/controls/Button.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/mvvm/ICommand.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <stdexcept>

namespace cppui {
namespace controls {

const input::RoutedEvent Button::ClickEvent(100, "Click", input::RoutingStrategy::Bubble);

ClickEventArgs::ClickEventArgs(Button* source) noexcept
    : input::RoutedEventArgs(Button::ClickEvent, source) {}

Button::Button() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    m_normalBackground = theme.Colors().controlBackground;
    m_hoverBackground = theme.Colors().controlHoverBackground;
    m_pressedBackground = theme.Colors().controlPressedBackground;
    m_disabledBackground = theme.Colors().controlDisabledBackground;
    m_normalBorder = theme.Colors().controlBorder;
    m_focusBorder = theme.Colors().controlFocusedBorder;

    SetFocusable(true);
    SetTabStop(true);
    SetPadding(theme.Metrics().paddingButton);
    SetBorderThickness(theme.Metrics().borderThicknessDefault);
    SetCornerRadius(theme.Metrics().cornerRadiusSmall);
    SetBackground(m_normalBackground);
    SetBorderBrush(m_normalBorder);
    SetForeground(theme.Colors().textPrimary);
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
    m_hasExplicitForeground = false;

    OnPointerEntered([this](input::PointerEventArgs&) {
        if (!m_enabled) return;
        m_isHovered = true;
        InvalidateVisual();
    });

    OnPointerExited([this](input::PointerEventArgs&) {
        if (!m_enabled) return;
        m_isHovered = false;
        m_isPressed = false;
        InvalidateVisual();
    });

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!m_enabled) return;
        if (e.IsLeftButton()) {
            m_isPressed = true;
            input::InputManager::Get().CapturePointer(this);
            InvalidateVisual();
            e.SetHandled(true);
        }
    });

    OnPointerReleased([this](input::PointerEventArgs& e) {
        if (!m_enabled) return;
        if (e.IsLeftButton() && m_isPressed) {
            input::InputManager::Get().ReleasePointerCapture(this);
            m_isPressed = false;

            // 檢查放開游標時是否仍位於按鈕局部邊界內
            core::Point localPos = e.GetPosition(this);
            core::Size btnSize = GetRenderBounds().GetSize();
            if (btnSize.IsEmpty()) btnSize = GetVisualBounds().GetSize();
            core::Rect bounds(core::Point(0, 0), btnSize);
            if (bounds.Contains(localPos)) {
                RaiseClick();
            }

            InvalidateVisual();
            e.SetHandled(true);
        }
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        if (!m_enabled) return;
        // Space (0x20) 或 Return (0x0D) 觸發點擊
        if (e.GetVirtualKey() == 0x20 || e.GetVirtualKey() == 0x0D) {
            RaiseClick();
            e.SetHandled(true);
        }
    });

    OnGotFocus([this](input::FocusEventArgs&) {
        InvalidateVisual();
    });

    OnLostFocus([this](input::FocusEventArgs&) {
        InvalidateVisual();
    });
}

Button::Button(compat::string_view text) : Button() {
    SetContent(text);
}

void Button::SetEnabled(bool enabled) {
    if (m_enabled != enabled) {
        Control::SetEnabled(enabled);
    }
}

void Button::SetHoverBackground(core::Color color) {
    m_hoverBackground = color;
    InvalidateVisual();
}

void Button::SetPressedBackground(core::Color color) {
    m_pressedBackground = color;
    InvalidateVisual();
}

void Button::SetFocusBorderBrush(core::Color color) {
    m_focusBorder = color;
    InvalidateVisual();
}

void Button::OnClick(ClickEventHandler handler) {
    AddHandler(ClickEvent, [handler](input::RoutedEventArgs& e) {
        if (auto* btn = dynamic_cast<Button*>(e.GetSource())) {
            handler(*btn);
        }
    });
}

void Button::OnClick(std::function<void()> handler) {
    AddHandler(ClickEvent, [handler](input::RoutedEventArgs&) {
        handler();
    });
}

void Button::SetCommand(std::shared_ptr<mvvm::ICommand> command) {
    if (!command) {
        throw std::invalid_argument("Button::SetCommand: command cannot be null.");
    }
    m_command = command;
    m_command->AddCanExecuteChangedHandler([this]() {
        if (m_command) {
            SetEnabled(m_command->CanExecute());
        }
    });
    SetEnabled(m_command->CanExecute());
}

void Button::RaiseClick() {
    ClickEventArgs args(this);
    RaiseEvent(args);
    if (m_command && m_command->CanExecute()) {
        m_command->Execute();
    }
}

void Button::OnRender(rendering::DrawingContext& dc) {
    core::Size sz = GetRenderBounds().GetSize();
    if (sz.IsEmpty()) sz = GetVisualBounds().GetSize();
    core::Rect bounds(core::Point(0, 0), sz);
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    core::Color bg = m_normalBackground;
    core::Color border = m_normalBorder;

    if (!m_enabled) {
        bg = m_disabledBackground;
    } else if (m_isPressed) {
        bg = m_pressedBackground;
    } else if (m_isHovered) {
        bg = m_hoverBackground;
    }

    if (m_enabled && IsFocused()) {
        border = m_focusBorder;
    }

    // 繪製背景與外框
    if (m_cornerRadius > 0.0f) {
        dc.DrawRoundedRectangle(bounds, m_cornerRadius, bg);
    } else {
        dc.DrawRectangle(bounds, bg);
    }

    if (border.a > 0 && m_borderThickness.top > 0.0f) {
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), border, m_borderThickness.top);
        dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), border, m_borderThickness.bottom);
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), border, m_borderThickness.left);
        dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), border, m_borderThickness.right);
    }
}

void Button::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    if (!m_hasExplicitBackground) {
        m_normalBackground = theme.Colors().controlBackground;
        m_hoverBackground = theme.Colors().controlHoverBackground;
        m_pressedBackground = theme.Colors().controlPressedBackground;
        m_disabledBackground = theme.Colors().controlDisabledBackground;
        m_background = m_normalBackground;
    }
    if (!m_hasExplicitBorder) {
        m_normalBorder = theme.Colors().controlBorder;
        m_focusBorder = theme.Colors().controlFocusedBorder;
        m_borderBrush = m_normalBorder;
    }
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
