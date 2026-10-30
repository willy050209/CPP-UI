#pragma once

#include "ui/text/TextLayout.hpp"
#include "FallbackFontMetrics.hpp"
#ifdef _WIN32
#include "DirectWriteTextEngine.hpp"
#endif
#include <string>
#include <vector>
#include <algorithm>

namespace cppui {
namespace text {

struct TextLayout::Impl {
    std::string           text;
    FontDescriptor        font;
    core::Size            size;
    std::vector<TextLine> lines;
#ifdef _WIN32
    IDWriteTextLayout*    nativeLayout = nullptr;
    std::wstring          wideText;
#endif

    Impl() = default;

    ~Impl() {
#ifdef _WIN32
        if (nativeLayout) {
            nativeLayout->Release();
            nativeLayout = nullptr;
        }
#endif
    }

    TextHitTestResult HitTestPoint(core::Point pt) const {
        TextHitTestResult res;
#ifdef _WIN32
        if (nativeLayout) {
            BOOL isTrailing = FALSE;
            BOOL isInside = FALSE;
            DWRITE_HIT_TEST_METRICS htm;

            HRESULT hr = nativeLayout->HitTestPoint(pt.x, pt.y, &isTrailing, &isInside, &htm);
            if (SUCCEEDED(hr)) {
                res.isTrailingHit = (isTrailing == TRUE);
                res.isInside = (isInside == TRUE);
                res.textPosition = static_cast<size_t>(htm.textPosition);
                float cursorX = htm.left + (res.isTrailingHit ? htm.width : 0.0f);
                res.cursorRect = core::Rect(cursorX, htm.top, 2.0f, htm.height);
                return res;
            }
        }
#endif
        // 純 C++ 跨平台等寬模型備援命中測試 (與 FallbackFontHelper 絕對對齊)
        auto metrics = FallbackFontHelper::ComputeMetrics(font.GetSize());
        float lineH = metrics.lineHeight;

        if (lines.empty() || text.empty()) {
            res.textPosition = 0;
            res.isInside = false;
            res.isTrailingHit = false;
            res.cursorRect = core::Rect(0.0f, 0.0f, 2.0f, lineH);
            return res;
        }

        // 查找對應行
        const TextLine* targetLine = &lines.front();
        for (const auto& l : lines) {
            if (pt.y >= l.bounds.Top() && pt.y < l.bounds.Bottom()) {
                targetLine = &l;
                break;
            }
            if (pt.y >= l.bounds.Bottom()) {
                targetLine = &l;
            }
        }

        float relX = pt.x - targetLine->bounds.Left();
        if (relX <= 0.0f) {
            res.textPosition = targetLine->startIndex;
            res.isInside = (pt.y >= targetLine->bounds.Top() && pt.y <= targetLine->bounds.Bottom());
            res.isTrailingHit = false;
            res.cursorRect = core::Rect(targetLine->bounds.Left(), targetLine->bounds.Top(), 2.0f, targetLine->bounds.height);
            return res;
        }

        size_t curByte = targetLine->startIndex;
        size_t lineEndByte = targetLine->startIndex + targetLine->length;
        float currentX = 0.0f;
        size_t hitPos = curByte;
        bool isInside = (relX <= targetLine->bounds.width && pt.y >= targetLine->bounds.Top() && pt.y <= targetLine->bounds.Bottom());

        while (curByte < lineEndByte) {
            size_t bytes = 0;
            uint32_t cp = FallbackFontHelper::DecodeUtf8(text.data() + curByte, lineEndByte - curByte, bytes);
            float charAdvance = FallbackFontHelper::IsFullwidth(cp) ? (metrics.charWidth * 2.0f) : metrics.charWidth;
            if (relX < currentX + charAdvance * 0.5f) {
                hitPos = curByte;
                break;
            }
            currentX += charAdvance;
            curByte += bytes;
            hitPos = curByte;
        }

        res.textPosition = hitPos;
        res.isInside = isInside;
        res.isTrailingHit = false;
        float cursorX = targetLine->bounds.Left() + currentX;
        res.cursorRect = core::Rect(cursorX, targetLine->bounds.Top(), 2.0f, targetLine->bounds.height);
        return res;
    }

    core::Rect HitTestTextPosition(size_t pos) const {
#ifdef _WIN32
        if (nativeLayout) {
            FLOAT px = 0.0f;
            FLOAT py = 0.0f;
            DWRITE_HIT_TEST_METRICS htm;

            HRESULT hr = nativeLayout->HitTestTextPosition(static_cast<UINT32>(pos), FALSE, &px, &py, &htm);
            if (SUCCEEDED(hr)) {
                return core::Rect(px, py, 2.0f, htm.height);
            }
        }
#endif
        auto metrics = FallbackFontHelper::ComputeMetrics(font.GetSize());
        float lineH = metrics.lineHeight;

        if (lines.empty() || text.empty()) {
            return core::Rect(0.0f, 0.0f, 2.0f, lineH);
        }

        for (const auto& l : lines) {
            if (pos >= l.startIndex && pos <= l.startIndex + l.length) {
                float currentX = 0.0f;
                size_t p = l.startIndex;
                while (p < pos && p < l.startIndex + l.length) {
                    size_t bytes = 0;
                    uint32_t cp = FallbackFontHelper::DecodeUtf8(text.data() + p, text.size() - p, bytes);
                    currentX += FallbackFontHelper::IsFullwidth(cp) ? (metrics.charWidth * 2.0f) : metrics.charWidth;
                    p += bytes;
                }
                return core::Rect(l.bounds.Left() + currentX, l.bounds.Top(), 2.0f, l.bounds.height);
            }
        }

        const auto& lastLine = lines.back();
        float currentX = 0.0f;
        size_t p = lastLine.startIndex;
        size_t endP = lastLine.startIndex + lastLine.length;
        while (p < endP && p < text.size()) {
            size_t bytes = 0;
            uint32_t cp = FallbackFontHelper::DecodeUtf8(text.data() + p, text.size() - p, bytes);
            currentX += FallbackFontHelper::IsFullwidth(cp) ? (metrics.charWidth * 2.0f) : metrics.charWidth;
            p += bytes;
        }
        return core::Rect(lastLine.bounds.Left() + currentX, lastLine.bounds.Top(), 2.0f, lastLine.bounds.height);
    }
};

} // namespace text
} // namespace cppui
