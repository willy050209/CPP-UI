/**
 * @file SaveFileDialog.cpp
 * @brief 實作儲存檔案對話方塊 (SaveFileDialog) 介面與雙後端派送
 */

#include "ui/dialogs/SaveFileDialog.hpp"
#include "ui/dialogs/FileDialogForm.hpp"
#include "Win32DialogService.hpp"

namespace cppui {
namespace dialogs {

SaveFileDialog::SaveFileDialog() {
    Reset();
}

void SaveFileDialog::Reset() {
    FileDialog::Reset();
    m_overwritePrompt = true;
    m_createPrompt = false;
}

app::DialogResult SaveFileDialog::ShowDialog(std::shared_ptr<app::Form> owner) {
    DialogBackendMode effectiveMode = (m_backendMode != DialogBackendMode::Auto)
                                    ? m_backendMode
                                    : DialogManager::GetDefaultBackendMode();

    if (effectiveMode == DialogBackendMode::Auto) {
        effectiveMode = Win32DialogService::IsAvailable()
                      ? DialogBackendMode::Native
                      : DialogBackendMode::BuiltIn;
    }

    if (effectiveMode == DialogBackendMode::Native && Win32DialogService::IsAvailable()) {
        return Win32DialogService::ShowSaveFileDialog(
            owner,
            m_title,
            m_initialDirectory,
            m_fileName,
            m_filter,
            m_filterIndex,
            m_defaultExt,
            m_overwritePrompt,
            m_checkPathExists,
            m_fileName
        );
    }

    // BuiltIn 或原生環境不可用時之純 CPPUI 表單
    auto form = std::make_shared<FileDialogForm>(
        FileDialogFormMode::Save,
        m_title,
        m_initialDirectory,
        m_fileName,
        m_filter,
        m_filterIndex,
        m_defaultExt,
        false, // multiselect 不適用於存檔
        m_overwritePrompt,
        false, // checkFileExists 存檔通常為新檔或覆寫
        m_checkPathExists
    );

    auto res = form->ShowDialog(owner);
    if (res == app::DialogResult::OK) {
        m_fileName = std::string(form->GetSelectedFileName());
    }
    return res;
}

} // namespace dialogs
} // namespace cppui
