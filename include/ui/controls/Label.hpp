/**
 * @file Label.hpp
 * @brief 標籤控制項，支援文字/任意內容呈現與目標控制項焦點關聯 (Target)
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/controls/ContentControl.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 標籤控制項 (Label)
/// 繼承自 ContentControl，提供純淨文字或內容標籤呈現，並支援目標控制項 (Target) 焦點導向關聯。
/// </summary>
class Label : public ContentControl {
public:
    /// <summary>
    /// 建構預設空白標籤控制項。
    /// </summary>
    Label();

    /// <summary>
    /// 建構帶有指定純文字內容之標籤控制項。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    explicit Label(compat::string_view text);

    /// <summary>
    /// 建構帶有自訂視覺節點內容之標籤控制項。
    /// </summary>
    /// <param name="content">視覺子節點指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 content 為 nullptr 時擲出。</exception>
    explicit Label(std::shared_ptr<core::UIElement> content);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Label() override = default;

    /// <summary>
    /// 取得標籤關聯之目標控制項 (Target)。
    /// </summary>
    /// <returns>目標控制項指標，若未設定則傳回 nullptr。</returns>
    COMPAT_NODISCARD Control* GetTarget() const noexcept { return m_target; }

    /// <summary>
    /// 設定標籤關聯之目標控制項 (Target)。
    /// 當使用者點擊此標籤時，將自動轉移輸入焦點至該目標控制項。
    /// </summary>
    /// <param name="target">目標控制項指標，可為 nullptr 以清除關聯。</param>
    void SetTarget(Control* target) noexcept { m_target = target; }

private:
    void InitializeLabel();

    Control* m_target = nullptr;
};

} // namespace controls
} // namespace cppui
