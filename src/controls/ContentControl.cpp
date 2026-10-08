/*
 * File: ContentControl.cpp
 * Purpose: Implements ContentControl child hosting, measurement, and layout.
 */

#include "ui/controls/ContentControl.hpp"
#include <algorithm>
#include <stdexcept>

namespace cppui {
namespace controls {

ContentControl::ContentControl() = default;

ContentControl::ContentControl(compat::string_view text) {
    SetContent(text);
}

ContentControl::ContentControl(std::shared_ptr<core::UIElement> content) {
    SetContent(std::move(content));
}

void ContentControl::SetContent(std::shared_ptr<core::UIElement> content) {
    if (!content) {
        throw std::invalid_argument("ContentControl::SetContent: content cannot be null.");
    }
    if (m_content == content) return;

    if (m_content) {
        RemoveVisualChild(m_content);
    }

    m_content = std::move(content);

    if (m_content) {
        AddVisualChild(m_content);
    }

    InvalidateMeasure();
}

void ContentControl::SetContent(compat::string_view text) {
    auto textBlock = std::make_shared<TextBlock>(text);
    textBlock->SetForeground(m_foreground);
    textBlock->SetHitTestVisible(false); // 穿透至本控制項
    SetContent(std::move(textBlock));
}

compat::string_view ContentControl::GetContentText() const noexcept {
    if (auto* tb = dynamic_cast<TextBlock*>(m_content.get())) {
        return tb->GetText();
    }
    return {};
}

void ContentControl::SetContentText(compat::string_view text) {
    if (auto* tb = dynamic_cast<TextBlock*>(m_content.get())) {
        tb->SetText(text);
    } else {
        SetContent(text);
    }
}

compat::string_view ContentControl::GetText() const noexcept {
    return GetContentText();
}

void ContentControl::SetText(compat::string_view text) {
    SetContentText(text);
    Control::SetText(text);
}

core::Size ContentControl::MeasureOverride(core::Size availableSize) {
    core::Size deflated = DeflateSize(availableSize);
    core::Size contentDesired(0.0f, 0.0f);

    if (m_content) {
        m_content->Measure(deflated);
        contentDesired = m_content->GetDesiredSize();
    }

    return InflateSize(contentDesired);
}

void ContentControl::ArrangeOverride(core::Size finalSize) {
    if (!m_content) return;

    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float w = (std::max)(0.0f, finalSize.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
    float h = (std::max)(0.0f, finalSize.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    m_content->Arrange(core::Rect(left, top, w, h));
}

void ContentControl::OnRender(rendering::DrawingContext& dc) {
    RenderBackgroundAndBorder(dc, core::Rect(core::Point(0, 0), GetRenderBounds().GetSize()));
}

} // namespace controls
} // namespace cppui
