/*
 * File: Border.hpp
 * Purpose: Declares Border, a container element that draws a background, border,
 *          and padding around a single child element.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include "compat/compat.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 單一內容裝飾框容器 (Border)
/// 提供背景填色、邊框筆刷、邊框粗細、圓角半徑與內部留白 (Padding)。
/// </summary>
class Border : public core::UIElement {
public:
    Border() = default;
    ~Border() override = default;

    /// <summary>
    /// 取得當前承載之子元件 raw 指標。
    /// </summary>
    /// <returns>子元件指標，若無則傳回 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetChild() const noexcept { return m_child.get(); }

    /// <summary>
    /// 設定內部裝飾之子元件。
    /// </summary>
    /// <param name="child">子元件智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 child 為 nullptr 時擲出。</exception>
    void SetChild(std::shared_ptr<core::UIElement> child);

    /// <summary>
    /// 取得背景填色色彩。
    /// </summary>
    /// <returns>背景色彩。</returns>
    COMPAT_NODISCARD core::Color GetBackground() const noexcept { return m_background; }

    /// <summary>
    /// 設定背景填色色彩。
    /// </summary>
    /// <param name="background">背景色彩。</param>
    void SetBackground(core::Color background);

    /// <summary>
    /// 取得邊框筆刷色彩。
    /// </summary>
    /// <returns>邊框筆刷色彩。</returns>
    COMPAT_NODISCARD core::Color GetBorderBrush() const noexcept { return m_borderBrush; }

    /// <summary>
    /// 設定邊框筆刷色彩。
    /// </summary>
    /// <param name="borderBrush">邊框筆刷色彩。</param>
    void SetBorderBrush(core::Color borderBrush);

    /// <summary>
    /// 取得邊框厚度。
    /// </summary>
    /// <returns>Thickness 物件。</returns>
    COMPAT_NODISCARD core::Thickness GetBorderThickness() const noexcept { return m_borderThickness; }

    /// <summary>
    /// 設定邊框厚度。
    /// </summary>
    /// <param name="thickness">邊框厚度。</param>
    void SetBorderThickness(const core::Thickness& thickness);

    /// <summary>
    /// 取得內部留白厚度 (Padding)。
    /// </summary>
    /// <returns>Thickness 物件。</returns>
    COMPAT_NODISCARD core::Thickness GetPadding() const noexcept { return m_padding; }

    /// <summary>
    /// 設定內部留白厚度。
    /// </summary>
    /// <param name="padding">內部留白厚度。</param>
    void SetPadding(const core::Thickness& padding);

    /// <summary>
    /// 取得圓角半徑。
    /// </summary>
    /// <returns>圓角半徑像素值。</returns>
    COMPAT_NODISCARD float GetCornerRadius() const noexcept { return m_cornerRadius; }

    /// <summary>
    /// 設定圓角半徑。
    /// </summary>
    /// <param name="cornerRadius">圓角半徑像素值。</param>
    void SetCornerRadius(float cornerRadius);

    /// <summary>
    /// 覆寫尺寸度量，將子元件度量尺寸疊加邊框與內距厚度。
    /// </summary>
    /// <param name="availableSize">可用尺寸。</param>
    /// <returns>容器期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫尺寸排列，扣除邊框與內距後佈局內部子節點。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製邊框容器背景與圓角框線。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

private:
    std::shared_ptr<core::UIElement> m_child = nullptr;
    core::Color                      m_background = core::Colors::Transparent;
    core::Color                      m_borderBrush = core::Colors::Transparent;
    core::Thickness                  m_borderThickness{ 0.0f, 0.0f, 0.0f, 0.0f };
    core::Thickness                  m_padding{ 0.0f, 0.0f, 0.0f, 0.0f };
    float                            m_cornerRadius = 0.0f;
};

} // namespace layout
} // namespace cppui
