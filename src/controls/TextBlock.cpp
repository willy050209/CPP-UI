#include "ui/controls/TextBlock.hpp"

namespace cppui {
namespace controls {

TextBlock::TextBlock() : m_font("Segoe UI", 14.0f) {
}

TextBlock::TextBlock(compat::string_view text)
    : m_text(text.data(), text.size()), m_font("Segoe UI", 14.0f) {
}

void TextBlock::SetText(compat::string_view text) {
    if (m_text != text) {
        m_text.assign(text.data(), text.size());
        InvalidateMeasure();
    }
}

void TextBlock::SetFont(const text::FontDescriptor& font) {
    m_font = font;
    InvalidateMeasure();
}

void TextBlock::SetFontSize(float size) {
    if (m_font.GetSize() != size) {
        m_font.SetSize(size);
        InvalidateMeasure();
    }
}

void TextBlock::SetFontFamily(compat::string_view family) {
    if (m_font.GetFamily() != family) {
        m_font.SetFamily(family);
        InvalidateMeasure();
    }
}

void TextBlock::SetFontWeight(text::FontWeight weight) {
    if (m_font.GetWeight() != weight) {
        m_font.SetWeight(weight);
        InvalidateMeasure();
    }
}

void TextBlock::SetFontStyle(text::FontStyle style) {
    if (m_font.GetStyle() != style) {
        m_font.SetStyle(style);
        InvalidateMeasure();
    }
}

void TextBlock::SetForeground(core::Color foreground) {
    if (m_foreground != foreground) {
        m_foreground = foreground;
        InvalidateVisual();
    }
}

void TextBlock::SetTextWrapping(text::TextWrapping wrapping) {
    if (m_wrapping != wrapping) {
        m_wrapping = wrapping;
        InvalidateMeasure();
    }
}

void TextBlock::SetTextAlignment(text::TextAlignment alignment) {
    if (m_alignment != alignment) {
        m_alignment = alignment;
        InvalidateVisual();
    }
}

void TextBlock::SetTextTrimming(text::TextTrimming trimming) {
    if (m_trimming != trimming) {
        m_trimming = trimming;
        InvalidateMeasure();
    }
}

core::Size TextBlock::MeasureOverride(core::Size availableSize) {
    if (m_text.empty()) {
        return core::Size(0.0f, m_font.GetSize() * 1.2f);
    }

    float maxW = (m_wrapping == text::TextWrapping::NoWrap) ? 1e9f : availableSize.width;
    if (maxW <= 0.0f) {
        maxW = 1e9f;
    }

    return text::TextLayoutEngine::Get().MeasureText(m_text, m_font, maxW);
}

void TextBlock::OnRender(rendering::DrawingContext& dc) {
    if (m_text.empty() || m_foreground.a == 0) return;

    core::Rect bounds = GetVisualBounds();
    float layoutW = (bounds.width > 0.0f) ? bounds.width : 1e9f;
    float layoutH = (bounds.height > 0.0f) ? bounds.height : 1e9f;

    text::TextLayout layout = text::TextLayoutEngine::Get().CreateLayout(
        m_text,
        m_font,
        layoutW,
        layoutH,
        m_alignment,
        m_wrapping,
        m_trimming
    );

    if (layout.GetNativeHandle() != nullptr) {
        dc.DrawTextLayout(layout, core::Point(0.0f, 0.0f), m_foreground);
    } else {
        dc.DrawText(m_text, core::Point(0.0f, 0.0f), m_font, m_foreground);
    }
}

} // namespace controls
} // namespace cppui
