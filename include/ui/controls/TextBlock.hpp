#pragma once

#include <string>
#include "compat/compat.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 高效能文字排版與顯示元件 (TextBlock)
/// 橋接 M3 文字排版管線，支援多行折行、字型變更與精確邊界度量。
/// </summary>
class TextBlock : public core::UIElement {
public:
    TextBlock();
    explicit TextBlock(compat::string_view text);
    ~TextBlock() override = default;

    COMPAT_NODISCARD compat::string_view GetText() const noexcept { return m_text; }
    void SetText(compat::string_view text);

    COMPAT_NODISCARD const text::FontDescriptor& GetFont() const noexcept { return m_font; }
    void SetFont(const text::FontDescriptor& font);

    COMPAT_NODISCARD float GetFontSize() const noexcept { return m_font.GetSize(); }
    void SetFontSize(float size);

    COMPAT_NODISCARD compat::string_view GetFontFamily() const noexcept { return m_font.GetFamily(); }
    void SetFontFamily(compat::string_view family);

    COMPAT_NODISCARD text::FontWeight GetFontWeight() const noexcept { return m_font.GetWeight(); }
    void SetFontWeight(text::FontWeight weight);

    COMPAT_NODISCARD text::FontStyle GetFontStyle() const noexcept { return m_font.GetStyle(); }
    void SetFontStyle(text::FontStyle style);

    COMPAT_NODISCARD core::Color GetForeground() const noexcept { return m_foreground; }
    void SetForeground(core::Color foreground);

    COMPAT_NODISCARD text::TextWrapping GetTextWrapping() const noexcept { return m_wrapping; }
    void SetTextWrapping(text::TextWrapping wrapping);

    COMPAT_NODISCARD text::TextAlignment GetTextAlignment() const noexcept { return m_alignment; }
    void SetTextAlignment(text::TextAlignment alignment);

    COMPAT_NODISCARD text::TextTrimming GetTextTrimming() const noexcept { return m_trimming; }
    void SetTextTrimming(text::TextTrimming trimming);

    core::Size MeasureOverride(core::Size availableSize) override;
    void OnRender(rendering::DrawingContext& dc) override;

private:
    std::string           m_text;
    text::FontDescriptor  m_font;
    core::Color           m_foreground{ 0, 0, 0, 255 }; // 預設黑字
    text::TextWrapping    m_wrapping = text::TextWrapping::WordWrap;
    text::TextAlignment   m_alignment = text::TextAlignment::Left;
    text::TextTrimming    m_trimming = text::TextTrimming::None;
};

} // namespace controls
} // namespace cppui
