/*
 * File: Button.cpp
 * Purpose: Implements Button interaction state machine, routing, and command execution.
 */

#include "ui/controls/Button.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/mvvm/ICommand.hpp"
#include "ui/app/Form.hpp"
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
            std::shared_ptr<Button> self;
            try {
                self = std::dynamic_pointer_cast<Button>(shared_from_this());
            } catch (...) {}

            input::InputManager::Get().ReleasePointerCapture(this);
            m_isPressed = false;

            // 檢查放開游標時是否仍位於按鈕局部邊界內
            core::Point localPos = e.GetPosition(this);
            core::Size btnSize = GetRenderBounds().GetSize();
            if (btnSize.IsEmpty()) btnSize = GetVisualBounds().GetSize();
            core::Rect bounds(core::Point(0, 0), btnSize);
            bool isInside = bounds.Contains(localPos);

            InvalidateVisual();
            e.SetHandled(true);

            if (isInside) {
                RaiseClick();
            }
        }
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        if (!m_enabled) return;
        // Space 或 Return/Enter 觸發點擊
        if (e.GetKey() == input::Key::Space || e.GetKey() == input::Key::Enter) {
            std::shared_ptr<Button> self;
            try {
                self = std::dynamic_pointer_cast<Button>(shared_from_this());
            } catch (...) {}

            e.SetHandled(true);
            RaiseClick();
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

void Button::SetBackground(core::Color bg) {
    Control::SetBackground(bg);
    m_normalBackground = bg;
    auto clampCh = [](int32_t val) -> uint8_t {
        return static_cast<uint8_t>(compat::clamp(val, 0, 255));
    };
    m_hoverBackground = core::Color(
        clampCh(static_cast<int32_t>(bg.r) + 20),
        clampCh(static_cast<int32_t>(bg.g) + 20),
        clampCh(static_cast<int32_t>(bg.b) + 20),
        bg.a
    );
    m_pressedBackground = core::Color(
        clampCh(static_cast<int32_t>(bg.r) - 20),
        clampCh(static_cast<int32_t>(bg.g) - 20),
        clampCh(static_cast<int32_t>(bg.b) - 20),
        bg.a
    );
    InvalidateVisual();
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
    if (m_dialogResult != app::DialogResult::None) {
        if (auto* form = FindForm()) {
            form->SetDialogResult(m_dialogResult);
            form->Close();
        }
    }
}

void Button::SetHorizontalContentAlignment(layout::HorizontalAlignment alignment) {
    if (m_horizontalContentAlignment != alignment) {
        m_horizontalContentAlignment = alignment;
        InvalidateArrange();
    }
}

void Button::SetVerticalContentAlignment(layout::VerticalAlignment alignment) {
    if (m_verticalContentAlignment != alignment) {
        m_verticalContentAlignment = alignment;
        InvalidateArrange();
    }
}

void Button::ArrangeOverride(core::Size finalSize) {
    if (!m_content) return;

    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float availW = (std::max)(0.0f, finalSize.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
    float availH = (std::max)(0.0f, finalSize.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    core::Size contentDesired = m_content->GetDesiredSize();

    // 高度保護機制：若按鈕明確高度大於文字高度，但預設內距過大導致 availH < contentDesired.height，
    // 動態擴展可用高度以容納文字，並於按鈕邊框內垂直置中，避免文字被壓縮截斷
    float totalBorderV = m_borderThickness.Vertical();
    if (finalSize.height > totalBorderV) {
        float innerH = finalSize.height - totalBorderV;
        if (contentDesired.height <= innerH && availH < contentDesired.height) {
            availH = contentDesired.height;
            top = m_borderThickness.top + (innerH - contentDesired.height) * 0.5f;
        }
    }

    float contentW = (std::min)(availW, contentDesired.width);
    float contentH = (std::min)(availH, contentDesired.height);

    float targetX = left;
    switch (m_horizontalContentAlignment) {
        case layout::HorizontalAlignment::Left:
            targetX = left;
            break;
        case layout::HorizontalAlignment::Center:
            targetX = left + (std::max)(0.0f, availW - contentW) * 0.5f;
            break;
        case layout::HorizontalAlignment::Right:
            targetX = left + (std::max)(0.0f, availW - contentW);
            break;
        case layout::HorizontalAlignment::Stretch:
            targetX = left;
            contentW = availW;
            break;
    }

    float targetY = top;
    switch (m_verticalContentAlignment) {
        case layout::VerticalAlignment::Top:
            targetY = top;
            break;
        case layout::VerticalAlignment::Center:
            targetY = top + (std::max)(0.0f, availH - contentH) * 0.5f;
            break;
        case layout::VerticalAlignment::Bottom:
            targetY = top + (std::max)(0.0f, availH - contentH);
            break;
        case layout::VerticalAlignment::Stretch:
            targetY = top;
            contentH = availH;
            break;
    }

    m_content->Arrange(core::Rect(targetX, targetY, contentW, contentH));
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
