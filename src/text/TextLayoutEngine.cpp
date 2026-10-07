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
        auto layoutImpl = std::make_shared<TextLayout::Impl>();
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

        // 跨平台純 C++ 文字排版與多行換行備援實作
        float fontSize = font.GetSize();
        float charW = fontSize * 0.6f;
        float lineH = fontSize * 1.3f;

        if (text.empty()) {
            layoutImpl->size = core::Size(0.0f, lineH);
            TextLine emptyLine;
            emptyLine.startIndex = 0;
            emptyLine.length = 0;
            emptyLine.bounds = core::Rect(0.0f, 0.0f, 0.0f, lineH);
            emptyLine.baseline = fontSize;
            emptyLine.isWrapped = false;
            layoutImpl->lines.push_back(emptyLine);
            return TextLayout(std::move(layoutImpl));
        }

        float maxObservedWidth = 0.0f;
        float currentY = 0.0f;
        size_t currentOffset = 0;

        // 依據換行符號及 maxWidth 分割行
        while (currentOffset < text.size()) {
            size_t nextNewline = text.find('\n', currentOffset);
            size_t segEnd = (nextNewline != compat::string_view::npos) ? nextNewline : text.size();
            size_t segLen = segEnd - currentOffset;

            // 判斷是否需要自動折行
            float segWidth = segLen * charW;
            if (wrapping != TextWrapping::NoWrap && maxWidth > 0.0f && maxWidth < 1e8f && segWidth > maxWidth) {
                size_t charsPerLine = std::max(size_t(1), static_cast<size_t>(maxWidth / charW));
                size_t subOffset = currentOffset;
                while (subOffset < segEnd) {
                    size_t chunkLen = std::min(charsPerLine, segEnd - subOffset);
                    float chunkW = chunkLen * charW;
                    maxObservedWidth = std::max(maxObservedWidth, chunkW);

                    TextLine line;
                    line.startIndex = subOffset;
                    line.length = chunkLen;
                    line.bounds = core::Rect(0.0f, currentY, chunkW, lineH);
                    line.baseline = fontSize;
                    line.isWrapped = (subOffset + chunkLen < segEnd);

                    layoutImpl->lines.push_back(line);
                    currentY += lineH;
                    subOffset += chunkLen;
                }
            } else {
                maxObservedWidth = std::max(maxObservedWidth, segWidth);

                TextLine line;
                line.startIndex = currentOffset;
                line.length = segLen;
                line.bounds = core::Rect(0.0f, currentY, segWidth, lineH);
                line.baseline = fontSize;
                line.isWrapped = false;

                layoutImpl->lines.push_back(line);
                currentY += lineH;
            }

            if (nextNewline != compat::string_view::npos) {
                currentOffset = nextNewline + 1;
            } else {
                break;
            }
        }

        layoutImpl->size = core::Size(maxObservedWidth, currentY);
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
