/**
 * @file Canvas.hpp
 * @brief 絕對定位版面容器，支援子元件 X/Y (Left/Top) 附加屬性配置
 */

#pragma once

#include <unordered_map>
#include <memory>
#include "compat/Compat.hpp"
#include "ui/layout/Panel.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 絕對定位版面容器 (Canvas)
/// 允許透過 Canvas::SetLeft、Canvas::SetTop 等附加屬性精確設定子元件於畫布中的絕對位置。
/// </summary>
class Canvas : public Panel {
public:
    /// <summary>
    /// 建構畫布版面容器。
    /// </summary>
    Canvas() = default;

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Canvas() override = default;

    /// <summary>
    /// 設定元件於畫布中的 Left 附加屬性。
    /// </summary>
    /// <param name="element">目標 UI 元件引用。</param>
    /// <param name="left">X 軸偏移像素值。</param>
    static void SetLeft(core::UIElement& element, float left);

    /// <summary>
    /// 取得元件於畫布中的 Left 附加屬性。
    /// </summary>
    /// <param name="element">目標 UI 元件常數引用。</param>
    /// <returns>X 軸偏移像素值。</returns>
    COMPAT_NODISCARD static float GetLeft(const core::UIElement& element);

    /// <summary>
    /// 設定元件於畫布中的 Top 附加屬性。
    /// </summary>
    /// <param name="element">目標 UI 元件引用。</param>
    /// <param name="top">Y 軸偏移像素值。</param>
    static void SetTop(core::UIElement& element, float top);

    /// <summary>
    /// 取得元件於畫布中的 Top 附加屬性。
    /// </summary>
    /// <param name="element">目標 UI 元件常數引用。</param>
    /// <returns>Y 軸偏移像素值。</returns>
    COMPAT_NODISCARD static float GetTop(const core::UIElement& element);

    /// <summary>
    /// 設定元件於畫布中的 Left 附加屬性 (shared_ptr 多載)。
    /// </summary>
    /// <param name="element">目標 UI 元件智慧指標。</param>
    /// <param name="left">X 軸偏移像素值。</param>
    static void SetLeft(const std::shared_ptr<core::UIElement>& element, float left);

    /// <summary>
    /// 取得元件於畫布中的 Left 附加屬性 (shared_ptr 多載)。
    /// </summary>
    /// <param name="element">目標 UI 元件智慧指標。</param>
    /// <returns>X 軸偏移像素值。</returns>
    COMPAT_NODISCARD static float GetLeft(const std::shared_ptr<core::UIElement>& element);

    /// <summary>
    /// 設定元件於畫布中的 Top 附加屬性 (shared_ptr 多載)。
    /// </summary>
    /// <param name="element">目標 UI 元件智慧指標。</param>
    /// <param name="top">Y 軸偏移像素值。</param>
    static void SetTop(const std::shared_ptr<core::UIElement>& element, float top);

    /// <summary>
    /// 取得元件於畫布中的 Top 附加屬性 (shared_ptr 多載)。
    /// </summary>
    /// <param name="element">目標 UI 元件智慧指標。</param>
    /// <returns>Y 軸偏移像素值。</returns>
    COMPAT_NODISCARD static float GetTop(const std::shared_ptr<core::UIElement>& element);

    /// <summary>
    /// 覆寫排版度量，畫布依據子元件期望尺寸進行度量。
    /// </summary>
    /// <param name="availableSize">可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排版排列，依據 Left/Top 附加屬性將子元件配置至絕對座標。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

private:
    static std::unordered_map<const core::UIElement*, float> s_leftProperties;
    static std::unordered_map<const core::UIElement*, float> s_topProperties;
};

} // namespace layout
} // namespace cppui
