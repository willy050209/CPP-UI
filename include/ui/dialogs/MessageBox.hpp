/**
 * @file MessageBox.hpp
 * @brief 定義模態訊息對話方塊 (MessageBox) 外觀 API 與靜態呼叫方法
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/app/FormEventArgs.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include <memory>

namespace cppui {
namespace app {
class Form;
} // namespace app

namespace dialogs {

/// <summary>
/// 模態訊息對話方塊 (MessageBox)。
/// 提供顯示提示訊息、警告、錯誤與使用者確認等標準互動對話框功能。
/// </summary>
class MessageBox {
public:
    /// <summary>
    /// 顯示包含指定訊息文字之訊息方塊 (預設標題為空，單一 OK 按鈕)。
    /// </summary>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult Show(compat::string_view text);

    /// <summary>
    /// 顯示包含指定訊息文字與標題之訊息方塊 (預設單一 OK 按鈕)。
    /// </summary>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult Show(compat::string_view text, compat::string_view caption);

    /// <summary>
    /// 顯示包含指定訊息、標題與指定按鈕組合之訊息方塊。
    /// </summary>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <param name="buttons">MessageBoxButtons 按鈕組合列舉值。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons);

    /// <summary>
    /// 顯示包含指定訊息、標題、按鈕組合與圖示之訊息方塊。
    /// </summary>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <param name="buttons">MessageBoxButtons 按鈕組合列舉值。</param>
    /// <param name="icon">MessageBoxIcon 圖示列舉值。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons, MessageBoxIcon icon);

    /// <summary>
    /// 顯示包含指定訊息、標題、按鈕組合、圖示與預設焦點按鈕之訊息方塊。
    /// </summary>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <param name="buttons">MessageBoxButtons 按鈕組合列舉值。</param>
    /// <param name="icon">MessageBoxIcon 圖示列舉值。</param>
    /// <param name="defaultButton">MessageBoxDefaultButton 預設按鈕列舉值。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons, MessageBoxIcon icon,
                                  MessageBoxDefaultButton defaultButton);

    /// <summary>
    /// 顯示擁有指定主控表單之訊息方塊。
    /// </summary>
    /// <param name="owner">主控擁有者表單 shared_ptr (可為 nullptr)。</param>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <param name="buttons">MessageBoxButtons 按鈕組合列舉值。</param>
    /// <param name="icon">MessageBoxIcon 圖示列舉值。</param>
    /// <param name="defaultButton">MessageBoxDefaultButton 預設按鈕列舉值。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult Show(std::shared_ptr<app::Form> owner,
                                  compat::string_view text,
                                  compat::string_view caption = "",
                                  MessageBoxButtons buttons = MessageBoxButtons::OK,
                                  MessageBoxIcon icon = MessageBoxIcon::None,
                                  MessageBoxDefaultButton defaultButton = MessageBoxDefaultButton::Button1);

    /// <summary>
    /// 針對特定呼叫明確指定後端模式之完整簽章方法。
    /// </summary>
    /// <param name="mode">指定之後端模式 (Auto, Native, BuiltIn)。</param>
    /// <param name="owner">主控擁有者表單 shared_ptr (可為 nullptr)。</param>
    /// <param name="text">欲顯示之提示訊息文字視圖。</param>
    /// <param name="caption">視窗標題文字視圖。</param>
    /// <param name="buttons">MessageBoxButtons 按鈕組合列舉值。</param>
    /// <param name="icon">MessageBoxIcon 圖示列舉值。</param>
    /// <param name="defaultButton">MessageBoxDefaultButton 預設按鈕列舉值。</param>
    /// <returns>使用者操作之 DialogResult 結果。</returns>
    static app::DialogResult ShowWithMode(DialogBackendMode mode,
                                          std::shared_ptr<app::Form> owner,
                                          compat::string_view text,
                                          compat::string_view caption = "",
                                          MessageBoxButtons buttons = MessageBoxButtons::OK,
                                          MessageBoxIcon icon = MessageBoxIcon::None,
                                          MessageBoxDefaultButton defaultButton = MessageBoxDefaultButton::Button1);

    /// <summary>
    /// 設定 MessageBox 預設使用之後端模式。
    /// </summary>
    /// <param name="mode">DialogBackendMode 列舉值。</param>
    static void SetBackendMode(DialogBackendMode mode) noexcept;

    /// <summary>
    /// 取得 MessageBox 預設使用之後端模式。
    /// </summary>
    /// <returns>DialogBackendMode 列舉值。</returns>
    COMPAT_NODISCARD static DialogBackendMode GetBackendMode() noexcept;
};

} // namespace dialogs
} // namespace cppui
