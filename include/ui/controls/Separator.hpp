/**
 * @file Separator.hpp
 * @brief 分隔線控制項，支援水平與垂直方向之細線呈現
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/layout/LayoutTypes.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 分隔線控制項 (Separator)
/// 用於在選單、工具列或容器中區隔不同群組之控制項，支援水平與垂直方向。
/// </summary>
class Separator : public Control {
public:
    /// <summary>
    /// 建構預設水平分隔線控制項 (厚度為 1.0f)。
    /// </summary>
    Separator();

    /// <summary>
    /// 建構指定方向之分隔線控制項。
    /// </summary>
    /// <param name="orientation">分隔線方向 (Horizontal 或 Vertical)。</param>
    explicit Separator(layout::Orientation orientation);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Separator() override = default;

    /// <summary>
    /// 取得分隔線方向。
    /// </summary>
    /// <returns>方向列舉值。</returns>
    COMPAT_NODISCARD layout::Orientation GetOrientation() const noexcept { return m_orientation; }

    /// <summary>
    /// 設定分隔線方向。
    /// </summary>
    /// <param name="orientation">方向列舉值。</param>
    void SetOrientation(layout::Orientation orientation);

    /// <summary>
    /// 取得分隔線線條厚度 (像素)。
    /// </summary>
    /// <returns>厚度數值。</returns>
    COMPAT_NODISCARD float GetThickness() const noexcept { return m_thickness; }

    /// <summary>
    /// 設定分隔線線條厚度。
    /// </summary>
    /// <param name="thickness">厚度像素值，必須大於等於 0.0f。</param>
    /// <exception cref="std::invalid_argument">當 thickness 小於 0.0f 時擲出。</exception>
    void SetThickness(float thickness);

    /// <summary>
    /// 取得分隔線繪製顏色。
    /// </summary>
    /// <returns>顏色物件。</returns>
    COMPAT_NODISCARD core::Color GetLineColor() const noexcept { return m_lineColor; }

    /// <summary>
    /// 設定分隔線繪製顏色。
    /// </summary>
    /// <param name="color">欲設定之顏色。</param>
    void SetLineColor(core::Color color);

    /// <summary>
    /// 排版測量階段，依方向與厚度回傳所需尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 控制項繪製階段，繪製水平或垂直分隔線。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

private:
    layout::Orientation m_orientation = layout::Orientation::Horizontal;
    float m_thickness = 1.0f;
    core::Color m_lineColor;
    bool m_hasExplicitLineColor = false;
};

} // namespace controls
} // namespace cppui
