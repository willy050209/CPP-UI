/**
 * @file Label.cpp
 * @brief 標籤控制項實作
 */

#include "ui/controls/Label.hpp"
#include "ui/input/PointerEventArgs.hpp"

namespace cppui {
namespace controls {

Label::Label() : ContentControl() {
    InitializeLabel();
}

Label::Label(compat::string_view text) : ContentControl(text) {
    InitializeLabel();
}

Label::Label(std::shared_ptr<core::UIElement> content) : ContentControl(content) {
    InitializeLabel();
}

void Label::InitializeLabel() {
    SetBackground(core::Color(0, 0, 0, 0));
    SetBorderThickness(core::Thickness(0.0f));
    SetPadding(core::Thickness(2.0f, 2.0f, 2.0f, 2.0f));
    SetFocusable(false);
    SetTabStop(false);
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (m_target != nullptr && m_target->IsEnabled()) {
            m_target->Focus();
            e.SetHandled(true);
        }
    });
}

} // namespace controls
} // namespace cppui
