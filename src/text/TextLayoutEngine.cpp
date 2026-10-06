#include "ui/text/TextLayoutEngine.hpp"
#include "TextLayoutImpl.hpp"
#include <algorithm>

namespace cppui {
namespace text {

struct TextLayoutEngine::Impl {
    TextLayout CreateLayout(
        compat::string_view text,
        const FontDescriptor& font,
        float maxWidth,
        float maxHeight,
        TextAlignment alignment,
        TextWrapping wrapping,
        TextTrimming trimming
    ) const {
        auto layoutImpl = std::unique_ptr<TextLayout::Impl>(new TextLayout::Impl());
        layoutImpl->text = std::string(text.data(), text.size());
        layoutImpl->font = font;

#ifdef _WIN32
        IDWriteTextLayout* nativeLayout = DirectWriteTextEngine::Get().CreateNativeTextLayout(
            text, font, maxWidth, maxHeight, alignment, wrapping, trimming
        );

        if (nativeLayout) {
            layoutImpl->nativeLayout = nativeLayout;
            layoutImpl->wideText = DirectWriteTextEngine::Utf8ToUtf16(text);

            DWRITE_TEXT_METRICS tm;
            if (SUCCEEDED(nativeLayout->GetMetrics(&tm))) {
                layoutImpl->size = core::Size(tm.widthIncludingTrailingWhitespace, tm.height);
            }

            UINT32 lineCount = 0;
            nativeLayout->GetLineMetrics(nullptr, 0, &lineCount);
            if (lineCount > 0) {
                std::vector<DWRITE_LINE_METRICS> dwriteLines(lineCount);
                UINT32 actualLines = 0;
                nativeLayout->GetLineMetrics(dwriteLines.data(), lineCount, &actualLines);

                float currentY = 0.0f;
                size_t charOffset = 0;
                for (UINT32 i = 0; i < actualLines; ++i) {
                    TextLine line;
                    line.startIndex = charOffset;
                    line.length = dwriteLines[i].length;
                    line.bounds = core::Rect(0.0f, currentY, layoutImpl->size.width, dwriteLines[i].height);
                    line.baseline = dwriteLines[i].baseline;
                    line.isWrapped = (dwriteLines[i].isTrimmed != 0);

                    layoutImpl->lines.push_back(line);
                    currentY += dwriteLines[i].height;
                    charOffset += dwriteLines[i].length;
                }
            }
            return TextLayout(std::move(layoutImpl));
        }
#endif

        // 跨平台基礎備援估算
        float fontSize = font.GetSize();
        float estWidth = text.size() * (fontSize * 0.6f);
        float finalWidth = (maxWidth > 0.0f && maxWidth < 1e8f) ? std::min(estWidth, maxWidth) : estWidth;
        float finalHeight = fontSize * 1.3f;
        layoutImpl->size = core::Size(finalWidth, finalHeight);

        TextLine singleLine;
        singleLine.startIndex = 0;
        singleLine.length = text.size();
        singleLine.bounds = core::Rect(0.0f, 0.0f, finalWidth, finalHeight);
        singleLine.baseline = fontSize;
        singleLine.isWrapped = false;
        layoutImpl->lines.push_back(singleLine);

        return TextLayout(std::move(layoutImpl));
    }

    core::Size MeasureText(
        compat::string_view text,
        const FontDescriptor& font,
        float maxWidth
    ) const {
        if (text.empty()) {
            return core::Size(0.0f, font.GetSize() * 1.2f);
        }
        TextLayout layout = CreateLayout(text, font, maxWidth, 1e9f, TextAlignment::Left, TextWrapping::WordWrap, TextTrimming::None);
        return layout.GetSize();
    }
};

TextLayoutEngine& TextLayoutEngine::Get() {
    static TextLayoutEngine instance;
    return instance;
}

TextLayoutEngine::TextLayoutEngine() : m_impl(new Impl()) {}
TextLayoutEngine::~TextLayoutEngine() = default;

TextLayout TextLayoutEngine::CreateLayout(
    compat::string_view text,
    const FontDescriptor& font,
    float maxWidth,
    float maxHeight,
    TextAlignment alignment,
    TextWrapping wrapping,
    TextTrimming trimming
) const {
    if (m_impl) {
        return m_impl->CreateLayout(text, font, maxWidth, maxHeight, alignment, wrapping, trimming);
    }
    return TextLayout();
}

core::Size TextLayoutEngine::MeasureText(
    compat::string_view text,
    const FontDescriptor& font,
    float maxWidth
) const {
    if (m_impl) {
        return m_impl->MeasureText(text, font, maxWidth);
    }
    return core::Size();
}

} // namespace text
} // namespace cppui
