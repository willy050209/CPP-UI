/**
 * @file CheckBox.hpp
 * @brief 核取方塊控制項，支援勾選狀態切換、勾選標記繪製與狀態變更路由事件
 */

#pragma once

#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/Button.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 核取方塊控制項 (CheckBox)
/// 具備打勾狀態切換、勾選標記向量繪製、自訂標籤與 Checked / Unchecked 路由事件。
/// </summary>
class CheckBox : public Button {
public:
    static const input::RoutedEvent CheckedEvent;
    static const input::RoutedEvent UncheckedEvent;

    /// <summary>
    /// 建構空白核取方塊控制項。
    /// </summary>
    CheckBox();

    /// <summary>
    /// 建構帶有標籤文字之核取方塊控制項。
    /// </summary>
    /// <param name="label">標籤文字視圖。</param>
    explicit CheckBox(compat::string_view label);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~CheckBox() override = default;

    /// <summary>
    /// 檢查當前是否處於勾選狀態。
    /// </summary>
    /// <returns>若已勾選回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsChecked() const noexcept { return m_isChecked; }

    /// <summary>
    /// 設定核取方塊之勾選狀態，並視情況觸發 Checked 或 Unchecked 事件。
    /// </summary>
    /// <param name="checked">true 為勾選，false 為取消勾選。</param>
    void SetChecked(bool checked);

    /// <summary>
    /// 反轉目前勾選狀態。
    /// </summary>
    void Toggle();

    /// <summary>
    /// 註冊勾選狀態變更回呼函式。
    /// </summary>
    /// <param name="handler">狀態變更回呼函式，接受目前勾選狀態。</param>
    void OnCheckedChanged(std::function<void(bool)> handler);

    /// <summary>
    /// 覆寫尺寸度量，計算方塊與文字之綜合期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排列階段，佈局方塊與文字相對位置。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製核取方塊外框、勾選狀態標記與文字。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

private:
    bool m_isChecked = false;
    float m_boxSize = 18.0f;
    float m_boxSpacing = 8.0f;

    core::Color m_checkColor{ 0, 160, 255, 255 }; // 勾選時藍色填色
};

} // namespace controls
} // namespace cppui
