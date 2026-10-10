/**
 * @file MessageBox.cpp
 * @brief 實作模態訊息方塊 (MessageBox) 靜態介面與雙後端派送
 */

#include "ui/dialogs/MessageBox.hpp"
#include "ui/dialogs/MessageBoxForm.hpp"
#include "Win32DialogService.hpp"

namespace cppui {
namespace dialogs {

namespace {
DialogBackendMode s_msgBoxBackendMode = DialogBackendMode::Auto;
} // namespace

app::DialogResult MessageBox::Show(compat::string_view text) {
    return Show(nullptr, text, "", MessageBoxButtons::OK, MessageBoxIcon::None, MessageBoxDefaultButton::Button1);
}

app::DialogResult MessageBox::Show(compat::string_view text, compat::string_view caption) {
    return Show(nullptr, text, caption, MessageBoxButtons::OK, MessageBoxIcon::None, MessageBoxDefaultButton::Button1);
}

app::DialogResult MessageBox::Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons) {
    return Show(nullptr, text, caption, buttons, MessageBoxIcon::None, MessageBoxDefaultButton::Button1);
}

app::DialogResult MessageBox::Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons, MessageBoxIcon icon) {
    return Show(nullptr, text, caption, buttons, icon, MessageBoxDefaultButton::Button1);
}

app::DialogResult MessageBox::Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons, MessageBoxIcon icon,
                                  MessageBoxDefaultButton defaultButton) {
    return Show(nullptr, text, caption, buttons, icon, defaultButton);
}

app::DialogResult MessageBox::Show(std::shared_ptr<app::Form> owner,
                                  compat::string_view text,
                                  compat::string_view caption,
                                  MessageBoxButtons buttons,
                                  MessageBoxIcon icon,
                                  MessageBoxDefaultButton defaultButton) {
    DialogBackendMode mode = (s_msgBoxBackendMode != DialogBackendMode::Auto)
                           ? s_msgBoxBackendMode
                           : DialogManager::GetDefaultBackendMode();
    return ShowWithMode(mode, owner, text, caption, buttons, icon, defaultButton);
}

app::DialogResult MessageBox::ShowWithMode(DialogBackendMode mode,
                                          std::shared_ptr<app::Form> owner,
                                          compat::string_view text,
                                          compat::string_view caption,
                                          MessageBoxButtons buttons,
                                          MessageBoxIcon icon,
                                          MessageBoxDefaultButton defaultButton) {
    DialogBackendMode effectiveMode = mode;
    if (effectiveMode == DialogBackendMode::Auto) {
        effectiveMode = Win32DialogService::IsAvailable()
                      ? DialogBackendMode::Native
                      : DialogBackendMode::BuiltIn;
    }

    if (effectiveMode == DialogBackendMode::Native && Win32DialogService::IsAvailable()) {
        return Win32DialogService::ShowMessageBox(owner, text, caption, buttons, icon, defaultButton);
    }

    // BuiltIn 或原生不支援時的退回處理
    auto form = std::make_shared<MessageBoxForm>(text, caption, buttons, icon, defaultButton);
    return form->ShowDialog(owner);
}

void MessageBox::SetBackendMode(DialogBackendMode mode) noexcept {
    s_msgBoxBackendMode = mode;
}

DialogBackendMode MessageBox::GetBackendMode() noexcept {
    return s_msgBoxBackendMode;
}

} // namespace dialogs
} // namespace cppui
