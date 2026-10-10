/**
 * @file FileDialogForm.hpp
 * @brief 純 CPPUI 主題渲染之檔案選擇對話方塊視窗 (FileDialogForm)
 */

#pragma once

#include "compat/compat.hpp"
#include "ui/app/Form.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace cppui {
namespace controls {
class Button;
class TextBox;
class TextBlock;
} // namespace controls

namespace layout {
class StackPanel;
class Border;
} // namespace layout

namespace dialogs {

/// <summary>
/// 檔案對話框操作類型。
/// </summary>
enum class FileDialogFormMode : uint8_t {
    Open,
    Save
};

/// <summary>
/// 純 CPPUI 佈景主題檔案選擇視窗 (FileDialogForm)。
/// 實作目錄瀏覽、萬用字元過濾、深淺主題切換與程式化模擬介面。
/// </summary>
class FileDialogForm : public app::Form {
public:
    /// <summary>
    /// 建構純 CPPUI 檔案選擇表單。
    /// </summary>
    /// <param name="mode">FileDialogFormMode 操作模式 (Open 或 Save)。</param>
    /// <param name="title">對話框標題文字視圖。</param>
    /// <param name="initialDir">起始目錄路徑文字視圖。</param>
    /// <param name="fileName">初始檔案名稱文字視圖。</param>
    /// <param name="filter">檔案篩選器字串視圖。</param>
    /// <param name="filterIndex">初始篩選器 1-based 索引。</param>
    /// <param name="defaultExt">預設副檔名字串視圖。</param>
    /// <param name="multiselect">是否允許多選。</param>
    /// <param name="overwritePrompt">若為存檔且檔案存在時是否提示覆寫。</param>
    /// <param name="checkFileExists">是否檢查檔案必須存在。</param>
    /// <param name="checkPathExists">是否檢查目錄路徑必須存在。</param>
    FileDialogForm(FileDialogFormMode mode,
                   compat::string_view title,
                   compat::string_view initialDir,
                   compat::string_view fileName,
                   compat::string_view filter,
                   uint32_t filterIndex,
                   compat::string_view defaultExt,
                   bool multiselect = false,
                   bool overwritePrompt = true,
                   bool checkFileExists = false,
                   bool checkPathExists = true);

    /// <summary>虛擬解構子。</summary>
    ~FileDialogForm() override = default;

    /// <summary>取得選取之主要檔案完整路徑字串視圖。</summary>
    COMPAT_NODISCARD compat::string_view GetSelectedFileName() const noexcept { return m_selectedFileName; }

    /// <summary>取得選取之所有檔案完整路徑清單引用。</summary>
    COMPAT_NODISCARD const std::vector<std::string>& GetSelectedFileNames() const noexcept { return m_selectedFileNames; }

    /// <summary>取得當前瀏覽之目錄路徑。</summary>
    COMPAT_NODISCARD const std::filesystem::path& GetCurrentDirectory() const noexcept { return m_currentDir; }

    /// <summary>取得是否允許多選。</summary>
    COMPAT_NODISCARD bool GetMultiselect() const noexcept { return m_multiselect; }

    /// <summary>取得是否檢查目錄路徑必須存在。</summary>
    COMPAT_NODISCARD bool GetCheckPathExists() const noexcept { return m_checkPathExists; }

    /// <summary>
    /// 切換當前目錄並重新整理清單。
    /// </summary>
    /// <param name="path">欲切換之目錄路徑。</param>
    void NavigateTo(const std::filesystem::path& path);

    /// <summary>
    /// 重新載入當前目錄之檔案清單。
    /// </summary>
    void RefreshDirectory();

    /// <summary>
    /// 程式化設定選定檔案並模擬觸發確認動作 (主要用於單元測試與 Headless 驗證)。
    /// </summary>
    /// <param name="fileName">欲選取之檔案名稱或路徑。</param>
    /// <returns>操作結果 DialogResult。</returns>
    app::DialogResult SimulateConfirm(compat::string_view fileName);

    /// <summary>
    /// 程式化模擬點擊取消。
    /// </summary>
    void SimulateCancel();

protected:
    /// <summary>表單載入生命週期掛鉤。</summary>
    void OnLoad() override;

    /// <summary>套用主題色彩變更。</summary>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void BuildUI();
    void OnConfirmAction();
    void PopulateDirectoryItems();

    FileDialogFormMode m_mode;
    std::filesystem::path m_currentDir;
    std::string m_selectedFileName;
    std::vector<std::string> m_selectedFileNames;
    std::string m_defaultExt;
    bool m_multiselect = false;
    bool m_overwritePrompt = true;
    bool m_checkFileExists = false;
    bool m_checkPathExists = true;

    std::vector<FileFilterItem> m_filterItems;
    size_t m_currentFilterIndex = 0;

    // UI 元件參照
    std::shared_ptr<controls::TextBox> m_pathBox;
    std::shared_ptr<controls::TextBox> m_fileNameBox;
    std::shared_ptr<layout::StackPanel> m_fileListPanel;
    std::shared_ptr<controls::Button> m_actionButton;
    std::shared_ptr<controls::Button> m_cancelButton;
    std::shared_ptr<controls::Button> m_upButton;
    std::shared_ptr<controls::Button> m_refreshButton;
    std::shared_ptr<controls::Button> m_filterButton;
};

} // namespace dialogs
} // namespace cppui
