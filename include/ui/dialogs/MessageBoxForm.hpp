/**
 * @file MessageBoxForm.hpp
 * @brief 純 CPPUI 主題渲染之模態訊息對話方塊 (MessageBoxForm)
 */

#pragma once

#include "compat/compat.hpp"
#include "ui/app/Form.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include <memory>
#include <string>

namespace cppui {
namespace controls {
class Button;
class TextBlock;
} // namespace controls

namespace layout {
class StackPanel;
} // namespace layout

namespace dialogs {

/// <summary>
/// 純 CPPUI 佈景主題模態訊息視窗 (MessageBoxForm)。
/// 遵循 ThemeManager 主題設定，具備深色/淺色模式切換與向量圖示。
/// </summary>
class MessageBoxForm : public app::Form {
public:
    /// <summary>
    /// 建構純 CPPUI 訊息方塊表單。
    /// </summary>
    /// <param name="text">提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <param name="buttons">MessageBoxButtons 按鈕組合列舉值。</param>
    /// <param name="icon">MessageBoxIcon 圖示列舉值。</param>
    /// <param name="defaultButton">MessageBoxDefaultButton 預設焦點按鈕。</param>
    MessageBoxForm(compat::string_view text,
                   compat::string_view caption,
                   MessageBoxButtons buttons = MessageBoxButtons::OK,
                   MessageBoxIcon icon = MessageBoxIcon::None,
                   MessageBoxDefaultButton defaultButton = MessageBoxDefaultButton::Button1);

    /// <summary>虛擬解構子。</summary>
    ~MessageBoxForm() override = default;

    /// <summary>
    /// 取得訊息文字字串。
    /// </summary>
    /// <returns>訊息文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetMessageText() const noexcept { return m_messageText; }

    /// <summary>
    /// 取得按鈕組合選項。
    /// </summary>
    /// <returns>MessageBoxButtons 列舉值。</returns>
    COMPAT_NODISCARD MessageBoxButtons GetButtons() const noexcept { return m_buttons; }

    /// <summary>
    /// 取得圖示類型。
    /// </summary>
    /// <returns>MessageBoxIcon 列舉值。</returns>
    COMPAT_NODISCARD MessageBoxIcon GetIcon() const noexcept { return m_icon; }

    /// <summary>
    /// 程式化模擬按鈕點擊 (主要用於單元測試與 Headless 驗證)。
    /// </summary>
    /// <param name="result">欲模擬觸發之 DialogResult 結果。</param>
    void SimulateButtonClick(app::DialogResult result);

protected:
    /// <summary>視窗初次載入生命週期回呼。</summary>
    void OnLoad() override;

    /// <summary>套用主題色彩變更。</summary>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void BuildUI();

    std::string m_messageText;
    std::string m_captionText;
    MessageBoxButtons m_buttons;
    MessageBoxIcon m_icon;
    MessageBoxDefaultButton m_defaultButton;

    std::shared_ptr<controls::TextBlock> m_textBlock;
    std::vector<std::shared_ptr<controls::Button>> m_actionButtons;
};

} // namespace dialogs
} // namespace cppui
