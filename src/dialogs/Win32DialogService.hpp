/**
 * @file Win32DialogService.hpp
 * @brief 定義 Win32 原生 COM 對話框服務介面
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/app/FormEventArgs.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include <memory>
#include <string>
#include <vector>

namespace cppui {
namespace app {
class Form;
} // namespace app

namespace dialogs {

/// <summary>
/// Win32 原生 COM 對話框服務類別 (Win32DialogService)。
/// 封裝 IFileOpenDialog, IFileSaveDialog 與 MessageBoxW 原生呼叫。
/// </summary>
class Win32DialogService {
public:
    /// <summary>
    /// 檢查當前平台與環境是否支援原生 Win32 對話框。
    /// </summary>
    /// <returns>若支援傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool IsAvailable() noexcept;

    /// <summary>
    /// 顯示原生 Windows MessageBox。
    /// </summary>
    static app::DialogResult ShowMessageBox(
        std::shared_ptr<app::Form> owner,
        compat::string_view text,
        compat::string_view caption,
        MessageBoxButtons buttons,
        MessageBoxIcon icon,
        MessageBoxDefaultButton defaultButton);

    /// <summary>
    /// 顯示原生 Windows IFileOpenDialog。
    /// </summary>
    static app::DialogResult ShowOpenFileDialog(
        std::shared_ptr<app::Form> owner,
        compat::string_view title,
        compat::string_view initialDir,
        compat::string_view filter,
        uint32_t filterIndex,
        compat::string_view defaultExt,
        bool multiselect,
        bool checkFileExists,
        bool checkPathExists,
        std::string& outSelectedFile,
        std::vector<std::string>& outSelectedFiles);

    /// <summary>
    /// 顯示原生 Windows IFileSaveDialog。
    /// </summary>
    static app::DialogResult ShowSaveFileDialog(
        std::shared_ptr<app::Form> owner,
        compat::string_view title,
        compat::string_view initialDir,
        compat::string_view fileName,
        compat::string_view filter,
        uint32_t filterIndex,
        compat::string_view defaultExt,
        bool overwritePrompt,
        bool checkPathExists,
        std::string& outSelectedFile);
};

} // namespace dialogs
} // namespace cppui
