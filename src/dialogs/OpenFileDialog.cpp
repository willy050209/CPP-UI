/**
 * @file OpenFileDialog.cpp
 * @brief 實作開啟檔案對話方塊 (OpenFileDialog) 介面與雙後端派送
 */

#include "ui/dialogs/OpenFileDialog.hpp"
#include "ui/dialogs/FileDialogForm.hpp"
#include "Win32DialogService.hpp"

namespace cppui {
namespace dialogs {

OpenFileDialog::OpenFileDialog() {
    Reset();
}

void OpenFileDialog::Reset() {
    FileDialog::Reset();
    m_checkFileExists = true;
    m_multiselect = false;
    m_showReadOnly = false;
    m_readOnlyChecked = false;
    m_fileNames.clear();
}

app::DialogResult OpenFileDialog::ShowDialog(std::shared_ptr<app::Form> owner) {
    DialogBackendMode effectiveMode = (m_backendMode != DialogBackendMode::Auto)
                                    ? m_backendMode
                                    : DialogManager::GetDefaultBackendMode();

    if (effectiveMode == DialogBackendMode::Auto) {
        effectiveMode = Win32DialogService::IsAvailable()
                      ? DialogBackendMode::Native
                      : DialogBackendMode::BuiltIn;
    }

    if (effectiveMode == DialogBackendMode::Native && Win32DialogService::IsAvailable()) {
        return Win32DialogService::ShowOpenFileDialog(
            owner,
            m_title,
            m_initialDirectory,
            m_filter,
            m_filterIndex,
            m_defaultExt,
            m_multiselect,
            m_checkFileExists,
            m_checkPathExists,
            m_fileName,
            m_fileNames
        );
    }

    // BuiltIn 或原生環境不可用時之純 CPPUI 表單
    auto form = std::make_shared<FileDialogForm>(
        FileDialogFormMode::Open,
        m_title,
        m_initialDirectory,
        m_fileName,
        m_filter,
        m_filterIndex,
        m_defaultExt,
        m_multiselect,
        false, // overwritePrompt 不適用於開檔
        m_checkFileExists,
        m_checkPathExists
    );

    auto res = form->ShowDialog(owner);
    if (res == app::DialogResult::OK) {
        m_fileName = std::string(form->GetSelectedFileName());
        m_fileNames = form->GetSelectedFileNames();
    }
    return res;
}

} // namespace dialogs
} // namespace cppui
