/**
 * @file SaveFileDialog.hpp
 * @brief 定義儲存檔案對話方塊 (SaveFileDialog) 介面與屬性
 */

#pragma once

#include "ui/dialogs/FileDialog.hpp"

namespace cppui {
namespace dialogs {

/// <summary>
/// 儲存檔案對話方塊 (SaveFileDialog)。
/// 支援覆寫提示、新建提示、預設副檔名自動補齊與雙後端派送。
/// </summary>
class SaveFileDialog : public FileDialog {
public:
    SaveFileDialog();
    ~SaveFileDialog() override = default;

    /// <summary>取得若檔案已存在時是否顯示覆寫提示訊息。</summary>
    COMPAT_NODISCARD bool GetOverwritePrompt() const noexcept { return m_overwritePrompt; }

    /// <summary>設定若檔案已存在時是否顯示覆寫提示訊息。</summary>
    /// <param name="prompt">true 提示確認，false 直接覆寫。</param>
    void SetOverwritePrompt(bool prompt) noexcept { m_overwritePrompt = prompt; }

    /// <summary>取得若檔案不存在時是否提示建立新檔案。</summary>
    COMPAT_NODISCARD bool GetCreatePrompt() const noexcept { return m_createPrompt; }

    /// <summary>設定若檔案不存在時是否提示建立新檔案。</summary>
    /// <param name="prompt">true 提示確認，false 不提示。</param>
    void SetCreatePrompt(bool prompt) noexcept { m_createPrompt = prompt; }

    /// <summary>
    /// 顯示強制回應存檔對話框。
    /// </summary>
    /// <param name="owner">可選之主控擁有者表單 shared_ptr。</param>
    /// <returns>操作結果 DialogResult。</returns>
    app::DialogResult ShowDialog(std::shared_ptr<app::Form> owner = nullptr) override;

    /// <summary>重設對話框為預設值。</summary>
    void Reset() override;

private:
    bool m_overwritePrompt = true;
    bool m_createPrompt = false;
};

} // namespace dialogs
} // namespace cppui
