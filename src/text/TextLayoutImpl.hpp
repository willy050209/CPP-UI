#pragma once

#include "ui/text/TextLayout.hpp"
#include "DirectWriteTextEngine.hpp"
#include <string>
#include <vector>

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
        res.textPosition = 0;
        res.isInside = false;
        res.isTrailingHit = false;
        res.cursorRect = core::Rect(0.0f, 0.0f, 2.0f, font.GetSize());
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
        return core::Rect(0.0f, 0.0f, 2.0f, font.GetSize());
    }
};

} // namespace text
} // namespace cppui
