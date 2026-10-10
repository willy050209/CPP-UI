/*
 * File: TextLayoutEngine.cpp
 * Purpose: Implements high-level text layout formatting, line wrapping, and metrics.
 */

#include "ui/text/TextLayoutEngine.hpp"
#include "TextLayoutImpl.hpp"
#include "FallbackFontMetrics.hpp"
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

        // 跨平台純 C++ 文字排版與多行換行備援實作 (與 FallbackFontHelper 絕對對齊)
        (void)maxHeight;
        (void)trimming;
        auto metrics = FallbackFontHelper::ComputeMetrics(font.GetSize());
        float lineH = metrics.lineHeight;

        if (text.empty()) {
            layoutImpl->size = core::Size(0.0f, lineH);
            TextLine emptyLine;
            emptyLine.startIndex = 0;
            emptyLine.length = 0;
            emptyLine.bounds = core::Rect(0.0f, 0.0f, 0.0f, lineH);
            emptyLine.baseline = metrics.baseline;
            emptyLine.isWrapped = false;
            layoutImpl->lines.push_back(emptyLine);
            return TextLayout(std::move(layoutImpl));
        }

        // 計算 UTF-8 子字串寬度之輔助 lambda
        auto CalcSubstrWidth = [&](size_t start, size_t end) noexcept -> float {
            float w = 0.0f;
            size_t p = start;
            while (p < end) {
                size_t bytes = 0;
                uint32_t cp = FallbackFontHelper::DecodeUtf8(text.data() + p, end - p, bytes);
                w += FallbackFontHelper::IsFullwidth(cp) ? (metrics.charWidth * 2.0f) : metrics.charWidth;
                p += bytes;
            }
            return w;
        };

        float maxObservedWidth = 0.0f;
        float currentY = 0.0f;
        size_t currentOffset = 0;

        // 依據換行符號及 maxWidth 分割行
        while (currentOffset < text.size()) {
            size_t nextNewline = text.find('\n', currentOffset);
            size_t segEnd = (nextNewline != compat::string_view::npos) ? nextNewline : text.size();

            float segWidth = CalcSubstrWidth(currentOffset, segEnd);

            // 判斷是否需要自動折行
            if (wrapping != TextWrapping::NoWrap && maxWidth > 0.0f && maxWidth < 1e8f && segWidth > maxWidth) {
                size_t subOffset = currentOffset;
                while (subOffset < segEnd) {
                    size_t chunkEnd = subOffset;
                    float currentChunkW = 0.0f;
                    while (chunkEnd < segEnd) {
                        size_t bytes = 0;
                        uint32_t cp = FallbackFontHelper::DecodeUtf8(text.data() + chunkEnd, segEnd - chunkEnd, bytes);
                        float charAdvance = FallbackFontHelper::IsFullwidth(cp) ? (metrics.charWidth * 2.0f) : metrics.charWidth;
                        if (chunkEnd > subOffset && currentChunkW + charAdvance > maxWidth) {
                            break;
                        }
                        currentChunkW += charAdvance;
                        chunkEnd += bytes;
                    }

                    size_t chunkLen = chunkEnd - subOffset;
                    float chunkW = currentChunkW;

                    float availW = (maxWidth > 0.0f && maxWidth < 1e8f) ? maxWidth : chunkW;
                    float lineX = 0.0f;
                    if (alignment == TextAlignment::Center && availW > chunkW) {
                        lineX = (availW - chunkW) * 0.5f;
                    } else if (alignment == TextAlignment::Right && availW > chunkW) {
                        lineX = (availW - chunkW);
                    }

                    maxObservedWidth = (std::max)(maxObservedWidth, lineX + chunkW);

                    TextLine line;
                    line.startIndex = subOffset;
                    line.length = chunkLen;
                    line.bounds = core::Rect(lineX, currentY, chunkW, lineH);
                    line.baseline = metrics.baseline;
                    line.isWrapped = (chunkEnd < segEnd);

                    layoutImpl->lines.push_back(line);
                    currentY += lineH;
                    subOffset = chunkEnd;
                }
            } else {
                float availW = (maxWidth > 0.0f && maxWidth < 1e8f) ? maxWidth : segWidth;
                float lineX = 0.0f;
                if (alignment == TextAlignment::Center && availW > segWidth) {
                    lineX = (availW - segWidth) * 0.5f;
                } else if (alignment == TextAlignment::Right && availW > segWidth) {
                    lineX = (availW - segWidth);
                }

                maxObservedWidth = (std::max)(maxObservedWidth, lineX + segWidth);

                TextLine line;
                line.startIndex = currentOffset;
                line.length = segEnd - currentOffset;
                line.bounds = core::Rect(lineX, currentY, segWidth, lineH);
                line.baseline = metrics.baseline;
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

TextLayoutEngine::TextLayoutEngine() : m_impl(std::make_unique<Impl>()) {}
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
