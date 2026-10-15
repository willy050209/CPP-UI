/**
 * @file FileDialog.hpp
 * @brief 定義通用檔案對話框抽象基底類別 (FileDialog) 與共通屬性
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/app/FormEventArgs.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace cppui {
namespace app {
class Form;
} // namespace app

namespace dialogs {

/// <summary>
/// 通用對話框抽象基底類別 (CommonDialog)。
/// </summary>
class CommonDialog {
public:
    virtual ~CommonDialog() = default;

    /// <summary>
    /// 以模態方式顯示對話方塊。
    /// </summary>
    /// <param name="owner">可選之主控擁有者表單 shared_ptr。</param>
    /// <returns>操作結果 DialogResult。</returns>
    virtual app::DialogResult ShowDialog(std::shared_ptr<app::Form> owner = nullptr) = 0;

    /// <summary>
    /// 重設對話框所有屬性為預設值。
    /// </summary>
    virtual void Reset() = 0;
};

/// <summary>
/// 檔案對話框通用基底類別 (FileDialog)。
/// 封裝篩選器、起始目錄、預設副檔名、檔案檢查與後端模式等通用屬性。
/// </summary>
class FileDialog : public CommonDialog {
public:
    FileDialog();
    ~FileDialog() override = default;

    // --- 屬性存取介面 ---

    /// <summary>取得對話框標題字串視圖。</summary>
    COMPAT_NODISCARD compat::string_view GetTitle() const noexcept { return m_title; }

    /// <summary>設定對話框標題。</summary>
    /// <param name="title">標題文字視圖。</param>
    void SetTitle(compat::string_view title);

    /// <summary>取得初始目錄字串視圖。</summary>
    COMPAT_NODISCARD compat::string_view GetInitialDirectory() const noexcept { return m_initialDirectory; }

    /// <summary>設定初始目錄。</summary>
    /// <param name="dir">目錄路徑文字視圖。</param>
    void SetInitialDirectory(compat::string_view dir);

    /// <summary>取得選取之檔案名稱或路徑字串視圖。</summary>
    COMPAT_NODISCARD compat::string_view GetFileName() const noexcept { return m_fileName; }

    /// <summary>設定檔案名稱或初始路徑。</summary>
    /// <param name="name">檔案名稱文字視圖。</param>
    void SetFileName(compat::string_view name);

    /// <summary>取得檔案篩選字串視圖。</summary>
    COMPAT_NODISCARD compat::string_view GetFilter() const noexcept { return m_filter; }

    /// <summary>設定檔案篩選字串 (例如 "Text Files (*.txt)|*.txt|All Files (*.*)|*.*")。</summary>
    /// <param name="filter">篩選器格式字串視圖。</param>
    void SetFilter(compat::string_view filter);

    /// <summary>取得目前選定之篩選條件索引 (1-based)。</summary>
    COMPAT_NODISCARD uint32_t GetFilterIndex() const noexcept { return m_filterIndex; }

    /// <summary>設定目前選定之篩選條件索引 (1-based)。</summary>
    /// <param name="index">1-based 索引數值。</param>
    void SetFilterIndex(uint32_t index) noexcept { m_filterIndex = (index == 0) ? 1 : index; }

    /// <summary>取得預設副檔名字串視圖。</summary>
    COMPAT_NODISCARD compat::string_view GetDefaultExt() const noexcept { return m_defaultExt; }

    /// <summary>設定預設副檔名。</summary>
    /// <param name="ext">副檔名字串視圖 (可包含或不包含開頭點號)。</param>
    void SetDefaultExt(compat::string_view ext);

    /// <summary>取得是否檢查檔案必須存在。</summary>
    COMPAT_NODISCARD bool GetCheckFileExists() const noexcept { return m_checkFileExists; }

    /// <summary>設定是否檢查檔案必須存在。</summary>
    /// <param name="check">true 檢查，false 略過。</param>
    void SetCheckFileExists(bool check) noexcept { m_checkFileExists = check; }

    /// <summary>取得是否檢查路徑必須存在。</summary>
    COMPAT_NODISCARD bool GetCheckPathExists() const noexcept { return m_checkPathExists; }

    /// <summary>設定是否檢查路徑必須存在。</summary>
    /// <param name="check">true 檢查，false 略過。</param>
    void SetCheckPathExists(bool check) noexcept { m_checkPathExists = check; }

    /// <summary>取得關閉對話框時是否復原當前工作目錄。</summary>
    COMPAT_NODISCARD bool GetRestoreDirectory() const noexcept { return m_restoreDirectory; }

    /// <summary>設定關閉對話框時是否復原當前工作目錄。</summary>
    /// <param name="restore">true 復原，false 不復原。</param>
    void SetRestoreDirectory(bool restore) noexcept { m_restoreDirectory = restore; }

    /// <summary>取得對話框後端模式。</summary>
    COMPAT_NODISCARD DialogBackendMode GetBackendMode() const noexcept { return m_backendMode; }

    /// <summary>設定對話框後端模式。</summary>
    /// <param name="mode">DialogBackendMode 列舉值。</param>
    void SetBackendMode(DialogBackendMode mode) noexcept { m_backendMode = mode; }

    /// <summary>重設對話框為初始預設值。</summary>
    void Reset() override;

protected:
    std::string m_title;
    std::string m_initialDirectory;
    std::string m_fileName;
    std::string m_filter;
    uint32_t m_filterIndex = 1;
    std::string m_defaultExt;
    bool m_checkFileExists = false;
    bool m_checkPathExists = true;
    bool m_restoreDirectory = false;
    DialogBackendMode m_backendMode = DialogBackendMode::Auto;
};

} // namespace dialogs
} // namespace cppui
