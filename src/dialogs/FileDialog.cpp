/**
 * @file FileDialog.cpp
 * @brief 實作檔案對話框抽象基底類別 (FileDialog) 通用屬性與重設行為
 */

#include "ui/dialogs/FileDialog.hpp"

namespace cppui {
namespace dialogs {

FileDialog::FileDialog() {
    Reset();
}

void FileDialog::SetTitle(compat::string_view title) {
    m_title = std::string(title);
}

void FileDialog::SetInitialDirectory(compat::string_view dir) {
    m_initialDirectory = std::string(dir);
}

void FileDialog::SetFileName(compat::string_view name) {
    m_fileName = std::string(name);
}

void FileDialog::SetFilter(compat::string_view filter) {
    m_filter = std::string(filter);
}

void FileDialog::SetDefaultExt(compat::string_view ext) {
    m_defaultExt = std::string(ext);
}

void FileDialog::Reset() {
    m_title.clear();
    m_initialDirectory.clear();
    m_fileName.clear();
    m_filter.clear();
    m_filterIndex = 1;
    m_defaultExt.clear();
    m_checkFileExists = false;
    m_checkPathExists = true;
    m_restoreDirectory = false;
    m_backendMode = DialogBackendMode::Auto;
}

} // namespace dialogs
} // namespace cppui
