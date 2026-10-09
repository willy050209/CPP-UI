#include "ui/controls/TextBlock.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"

namespace cppui {
namespace controls {

TextBlock::TextBlock() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    m_font = text::FontDescriptor(theme.Typography().fontFamily, theme.Typography().fontSizeBody);
    m_foreground = theme.Colors().textPrimary;
}

TextBlock::TextBlock(compat::string_view text)
    : TextBlock() {
    m_text.assign(text.data(), text.size());
}

void TextBlock::SetText(compat::string_view text) {
    if (m_text != text) {
        m_text.assign(text.data(), text.size());
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetFont(const text::FontDescriptor& font) {
    m_hasExplicitFont = true;
    if (m_font != font) {
        m_font = font;
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetFontSize(float size) {
    m_hasExplicitFont = true;
    if (m_font.GetSize() != size) {
        m_font.SetSize(size);
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetFontFamily(compat::string_view family) {
    m_hasExplicitFont = true;
    if (m_font.GetFamily() != family) {
        m_font.SetFamily(family);
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetFontWeight(text::FontWeight weight) {
    m_hasExplicitFont = true;
    if (m_font.GetWeight() != weight) {
        m_font.SetWeight(weight);
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetFontStyle(text::FontStyle style) {
    m_hasExplicitFont = true;
    if (m_font.GetStyle() != style) {
        m_font.SetStyle(style);
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetForeground(core::Color foreground) {
    m_hasExplicitForeground = true;
    if (m_foreground != foreground) {
        m_foreground = foreground;
        InvalidateVisual();
    }
}

void TextBlock::SetTextWrapping(text::TextWrapping wrapping) {
    if (m_wrapping != wrapping) {
        m_wrapping = wrapping;
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::SetTextAlignment(text::TextAlignment alignment) {
    if (m_alignment != alignment) {
        m_alignment = alignment;
        m_isLayoutDirty = true;
        InvalidateVisual();
    }
}

void TextBlock::SetTextTrimming(text::TextTrimming trimming) {
    if (m_trimming != trimming) {
        m_trimming = trimming;
        m_isLayoutDirty = true;
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

    m_textLayout = text::TextLayoutEngine::Get().CreateLayout(
        m_text,
        m_font,
        maxW,
        1e9f,
        m_alignment,
        m_wrapping,
        m_trimming
    );
    m_isLayoutDirty = false;
    return m_textLayout.GetSize();
}

void TextBlock::OnRender(rendering::DrawingContext& dc) {
    if (m_text.empty() || m_foreground.a == 0) return;

    core::Rect bounds = GetVisualBounds();
    float layoutW = (bounds.width > 0.0f) ? bounds.width : 1e9f;
    float layoutH = (bounds.height > 0.0f) ? bounds.height : 1e9f;

    if (m_isLayoutDirty || m_textLayout.GetLineCount() == 0) {
        m_textLayout = text::TextLayoutEngine::Get().CreateLayout(
            m_text,
            m_font,
            layoutW,
            layoutH,
            m_alignment,
            m_wrapping,
            m_trimming
        );
        m_isLayoutDirty = false;
    }

    if (m_textLayout.GetNativeHandle() != nullptr) {
        dc.DrawTextLayout(m_textLayout, core::Point(0.0f, 0.0f), m_foreground);
    } else {
        dc.DrawText(m_text, core::Point(0.0f, 0.0f), m_font, m_foreground);
    }
}

void TextBlock::ApplyTheme(const theme::Theme& theme) {
    if (!m_hasExplicitForeground) {
        m_foreground = theme.Colors().textPrimary;
    }
    if (!m_hasExplicitFont) {
        m_font.SetFamily(theme.Typography().fontFamily);
        m_font.SetSize(theme.Typography().fontSizeBody);
        m_isLayoutDirty = true;
        InvalidateMeasure();
    }
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
