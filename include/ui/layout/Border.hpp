#pragma once

#include <memory>
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

    COMPAT_NODISCARD core::UIElement* GetChild() const noexcept { return m_child.get(); }
    void SetChild(std::shared_ptr<core::UIElement> child);

    COMPAT_NODISCARD core::Color GetBackground() const noexcept { return m_background; }
    void SetBackground(core::Color background);

    COMPAT_NODISCARD core::Color GetBorderBrush() const noexcept { return m_borderBrush; }
    void SetBorderBrush(core::Color borderBrush);

    COMPAT_NODISCARD core::Thickness GetBorderThickness() const noexcept { return m_borderThickness; }
    void SetBorderThickness(const core::Thickness& thickness);

    COMPAT_NODISCARD core::Thickness GetPadding() const noexcept { return m_padding; }
    void SetPadding(const core::Thickness& padding);

    COMPAT_NODISCARD float GetCornerRadius() const noexcept { return m_cornerRadius; }
    void SetCornerRadius(float cornerRadius);

    core::Size MeasureOverride(core::Size availableSize) override;
    void ArrangeOverride(core::Size finalSize) override;
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
