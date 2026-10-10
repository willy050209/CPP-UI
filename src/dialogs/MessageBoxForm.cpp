/**
 * @file MessageBoxForm.cpp
 * @brief 實作純 CPPUI 主題渲染之模態訊息視窗 (MessageBoxForm)
 */

#include "ui/dialogs/MessageBoxForm.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/theme/ThemeManager.hpp"
#include <stdexcept>

namespace cppui {
namespace dialogs {

MessageBoxForm::MessageBoxForm(compat::string_view text,
                               compat::string_view caption,
                               MessageBoxButtons buttons,
                               MessageBoxIcon icon,
                               MessageBoxDefaultButton defaultButton)
    : m_messageText(text)
    , m_captionText(caption.empty() ? "訊息" : std::string(caption))
    , m_buttons(buttons)
    , m_icon(icon)
    , m_defaultButton(defaultButton) {
    SetTitle(m_captionText);
    SetFormSize(460, 190);
    SetStartPosition(app::FormStartPosition::CenterScreen);

    BuildUI();
}

void MessageBoxForm::OnLoad() {
    app::Form::OnLoad();
    ApplyTheme(theme::ThemeManager::Get().GetCurrentTheme());
}

void MessageBoxForm::SimulateButtonClick(app::DialogResult result) {
    SetDialogResult(result);
    Close();
}

void MessageBoxForm::BuildUI() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    const auto& colors = theme.Colors();
    const auto& metrics = theme.Metrics();

    auto rootBorder = std::make_shared<layout::Border>();
    rootBorder->SetBackground(colors.windowBackground);
    rootBorder->SetBorderBrush(colors.surfaceBorder);
    rootBorder->SetBorderThickness(metrics.borderThicknessDefault);
    rootBorder->SetCornerRadius(metrics.cornerRadiusMedium);
    rootBorder->SetPadding(core::Thickness(16.0f, 16.0f, 16.0f, 16.0f));

    auto mainStack = std::make_shared<layout::StackPanel>(layout::Orientation::Vertical);
    mainStack->SetSpacing(16.0f);

    // 上層訊息與圖示區塊
    auto bodyStack = std::make_shared<layout::StackPanel>(layout::Orientation::Horizontal);
    bodyStack->SetSpacing(14.0f);

    // 圖示文字塊
    if (m_icon != MessageBoxIcon::None) {
        auto iconBlock = std::make_shared<controls::TextBlock>();
        iconBlock->SetFontSize(24.0f);

        switch (m_icon) {
        case MessageBoxIcon::Information:
            iconBlock->SetText("[i]");
            iconBlock->SetForeground(colors.info);
            break;
        case MessageBoxIcon::Warning:
            iconBlock->SetText("[!]");
            iconBlock->SetForeground(colors.warning);
            break;
        case MessageBoxIcon::Error:
            iconBlock->SetText("[X]");
            iconBlock->SetForeground(colors.error);
            break;
        case MessageBoxIcon::Question:
            iconBlock->SetText("[?]");
            iconBlock->SetForeground(colors.accentPrimary);
            break;
        default:
            break;
        }
        bodyStack->AddChild(iconBlock);
    }

    // 提示文字區塊
    m_textBlock = std::make_shared<controls::TextBlock>(m_messageText);
    m_textBlock->SetForeground(colors.textPrimary);
    m_textBlock->SetFontSize(14.0f);
    bodyStack->AddChild(m_textBlock);

    mainStack->AddChild(bodyStack);

    // 底部按鈕排版區塊 (水平靠右)
    auto buttonRow = std::make_shared<layout::StackPanel>(layout::Orientation::Horizontal);
    buttonRow->SetSpacing(10.0f);

    auto addButton = [this, &buttonRow, colors](compat::string_view label, app::DialogResult res, bool isPrimary) {
        auto btn = std::make_shared<controls::Button>(label);
        btn->SetWidth(80.0f);
        btn->SetHeight(32.0f);
        if (isPrimary) {
            btn->SetBackground(colors.accentPrimary);
            btn->SetHoverBackground(colors.accentHover);
            btn->SetPressedBackground(colors.accentPressed);
        } else {
            btn->SetBackground(colors.controlBackground);
            btn->SetHoverBackground(colors.controlHoverBackground);
            btn->SetPressedBackground(colors.controlPressedBackground);
        }
        btn->OnClick([this, res]() {
            SetDialogResult(res);
            Close();
        });
        m_actionButtons.push_back(btn);
        buttonRow->AddChild(btn);
    };

    switch (m_buttons) {
    case MessageBoxButtons::OK:
        addButton("確定", app::DialogResult::OK, true);
        break;
    case MessageBoxButtons::OKCancel:
        addButton("確定", app::DialogResult::OK, m_defaultButton == MessageBoxDefaultButton::Button1);
        addButton("取消", app::DialogResult::Cancel, m_defaultButton == MessageBoxDefaultButton::Button2);
        break;
    case MessageBoxButtons::YesNo:
        addButton("是", app::DialogResult::Yes, m_defaultButton == MessageBoxDefaultButton::Button1);
        addButton("否", app::DialogResult::No, m_defaultButton == MessageBoxDefaultButton::Button2);
        break;
    case MessageBoxButtons::YesNoCancel:
        addButton("是", app::DialogResult::Yes, m_defaultButton == MessageBoxDefaultButton::Button1);
        addButton("否", app::DialogResult::No, m_defaultButton == MessageBoxDefaultButton::Button2);
        addButton("取消", app::DialogResult::Cancel, m_defaultButton == MessageBoxDefaultButton::Button3);
        break;
    case MessageBoxButtons::RetryCancel:
        addButton("重試", app::DialogResult::Retry, m_defaultButton == MessageBoxDefaultButton::Button1);
        addButton("取消", app::DialogResult::Cancel, m_defaultButton == MessageBoxDefaultButton::Button2);
        break;
    case MessageBoxButtons::AbortRetryIgnore:
        addButton("中止", app::DialogResult::Abort, m_defaultButton == MessageBoxDefaultButton::Button1);
        addButton("重試", app::DialogResult::Retry, m_defaultButton == MessageBoxDefaultButton::Button2);
        addButton("略過", app::DialogResult::Ignore, m_defaultButton == MessageBoxDefaultButton::Button3);
        break;
    }

    mainStack->AddChild(buttonRow);
    rootBorder->SetChild(mainStack);

    SetContent(rootBorder);
}

void MessageBoxForm::ApplyTheme(const theme::Theme& theme) {
    app::Form::ApplyTheme(theme);
    const auto& colors = theme.Colors();

    if (auto border = std::dynamic_pointer_cast<layout::Border>(GetContent())) {
        border->SetBackground(colors.windowBackground);
        border->SetBorderBrush(colors.surfaceBorder);
    }
    if (m_textBlock) {
        m_textBlock->SetForeground(colors.textPrimary);
    }
}

} // namespace dialogs
} // namespace cppui
