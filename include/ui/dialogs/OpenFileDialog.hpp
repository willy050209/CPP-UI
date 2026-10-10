/**
 * @file OpenFileDialog.hpp
 * @brief 定義開啟檔案對話方塊 (OpenFileDialog) 介面與屬性
 */

#pragma once

#include "ui/dialogs/FileDialog.hpp"
#include <vector>

namespace cppui {
namespace dialogs {

/// <summary>
/// 開啟檔案對話方塊 (OpenFileDialog)。
/// 支援單選或多選檔案、唯讀選項勾選、檔案存在性驗證與雙後端派送。
/// </summary>
class OpenFileDialog : public FileDialog {
public:
    OpenFileDialog();
    ~OpenFileDialog() override = default;

    /// <summary>取得是否允許多選檔案。</summary>
    COMPAT_NODISCARD bool GetMultiselect() const noexcept { return m_multiselect; }

    /// <summary>設定是否允許多選檔案。</summary>
    /// <param name="multiselect">true 允許多選，false 僅單選。</param>
    void SetMultiselect(bool multiselect) noexcept { m_multiselect = multiselect; }

    /// <summary>取得已選取之所有檔案路徑清單。</summary>
    COMPAT_NODISCARD const std::vector<std::string>& GetFileNames() const noexcept { return m_fileNames; }

    /// <summary>取得是否顯示「唯讀」核取方塊。</summary>
    COMPAT_NODISCARD bool GetShowReadOnly() const noexcept { return m_showReadOnly; }

    /// <summary>設定是否顯示「唯讀」核取方塊。</summary>
    /// <param name="show">true 顯示，false 隱藏。</param>
    void SetShowReadOnly(bool show) noexcept { m_showReadOnly = show; }

    /// <summary>取得「唯讀」核取方塊是否被勾選。</summary>
    COMPAT_NODISCARD bool GetReadOnlyChecked() const noexcept { return m_readOnlyChecked; }

    /// <summary>設定「唯讀」核取方塊勾選狀態。</summary>
    /// <param name="check">true 勾選，false 取消勾選。</param>
    void SetReadOnlyChecked(bool check) noexcept { m_readOnlyChecked = check; }

    /// <summary>
    /// 顯示強制回應開檔對話框。
    /// </summary>
    /// <param name="owner">可選之主控擁有者表單 shared_ptr。</param>
    /// <returns>操作結果 DialogResult。</returns>
    app::DialogResult ShowDialog(std::shared_ptr<app::Form> owner = nullptr) override;

    /// <summary>重設對話框為預設值。</summary>
    void Reset() override;

private:
    bool m_multiselect = false;
    bool m_showReadOnly = false;
    bool m_readOnlyChecked = false;
    std::vector<std::string> m_fileNames;
};

} // namespace dialogs
} // namespace cppui
