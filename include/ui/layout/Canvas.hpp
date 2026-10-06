#pragma once

#include <unordered_map>
#include <memory>
#include "compat/compat.hpp"
#include "ui/layout/Panel.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 絕對定位版面容器 (Canvas)
/// 允許透過 Canvas::SetLeft、Canvas::SetTop 等附加屬性精確設定子元件於畫布中的絕對位置。
/// </summary>
class Canvas : public Panel {
public:
    Canvas() = default;
    ~Canvas() override = default;

    static void SetLeft(core::UIElement& element, float left);
    COMPAT_NODISCARD static float GetLeft(const core::UIElement& element);

    static void SetTop(core::UIElement& element, float top);
    COMPAT_NODISCARD static float GetTop(const core::UIElement& element);

    static void SetLeft(const std::shared_ptr<core::UIElement>& element, float left);
    COMPAT_NODISCARD static float GetLeft(const std::shared_ptr<core::UIElement>& element);

    static void SetTop(const std::shared_ptr<core::UIElement>& element, float top);
    COMPAT_NODISCARD static float GetTop(const std::shared_ptr<core::UIElement>& element);

    core::Size MeasureOverride(core::Size availableSize) override;
    void ArrangeOverride(core::Size finalSize) override;

private:
    static std::unordered_map<const core::UIElement*, float> s_leftProperties;
    static std::unordered_map<const core::UIElement*, float> s_topProperties;
};

} // namespace layout
} // namespace cppui
