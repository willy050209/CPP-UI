#pragma once

#include <vector>
#include <algorithm>
#include "ui/rendering/backend/IRenderContext.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cppui {
namespace rendering {

class SoftwareRenderTarget : public IRenderTarget {
public:
    SoftwareRenderTarget() = default;

    void Resize(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;
        m_pixels.resize(static_cast<size_t>(width) * height, 0xFF000000);
    }

    uint32_t GetWidth() const noexcept override { return m_width; }
    uint32_t GetHeight() const noexcept override { return m_height; }

    uint32_t* GetPixelBuffer() noexcept override {
        return m_pixels.data();
    }

    const uint32_t* GetPixelBuffer() const noexcept {
        return m_pixels.data();
    }

    COMPAT_NODISCARD compat::span<uint32_t> GetPixelSpan() noexcept {
        return compat::span<uint32_t>(m_pixels.data(), m_pixels.size());
    }

    COMPAT_NODISCARD compat::span<const uint32_t> GetPixelSpan() const noexcept {
        return compat::span<const uint32_t>(m_pixels.data(), m_pixels.size());
    }

    void Clear(core::Color color) override {
        // Windows DIB 預設為 BGRA 32-bit 格式
        uint32_t pixelValue = (static_cast<uint32_t>(color.a) << 24) |
                              (static_cast<uint32_t>(color.r) << 16) |
                              (static_cast<uint32_t>(color.g) << 8)  |
                              (static_cast<uint32_t>(color.b));
        std::fill(m_pixels.begin(), m_pixels.end(), pixelValue);
    }

    void FillRect(const core::Rect& rect, core::Color color) override {
        if (m_width == 0 || m_height == 0) return;

        int32_t x0 = std::max(0, static_cast<int32_t>(rect.Left()));
        int32_t y0 = std::max(0, static_cast<int32_t>(rect.Top()));
        int32_t x1 = std::min(static_cast<int32_t>(m_width), static_cast<int32_t>(rect.Right()));
        int32_t y1 = std::min(static_cast<int32_t>(m_height), static_cast<int32_t>(rect.Bottom()));

        if (x0 >= x1 || y0 >= y1) return;

        uint32_t pixelValue = (static_cast<uint32_t>(color.a) << 24) |
                              (static_cast<uint32_t>(color.r) << 16) |
                              (static_cast<uint32_t>(color.g) << 8)  |
                              (static_cast<uint32_t>(color.b));

        for (int32_t y = y0; y < y1; ++y) {
            uint32_t* row = &m_pixels[static_cast<size_t>(y) * m_width + x0];
            std::fill(row, row + (x1 - x0), pixelValue);
        }
    }

    void DrawText(compat::string_view text, core::Point origin, compat::string_view family, float size, uint16_t weight, uint8_t style, core::Color color) override {
        if (text.empty() || m_width == 0 || m_height == 0 || color.a == 0) return;
#ifdef _WIN32
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (wideLen <= 0) return;
        std::vector<wchar_t> wideText(static_cast<size_t>(wideLen));
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wideText.data(), wideLen);

        std::vector<wchar_t> wideFamily(family.size() + 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, family.data(), static_cast<int>(family.size()), wideFamily.data(), static_cast<int>(family.size()));

        HDC memDC = CreateCompatibleDC(nullptr);
        if (!memDC) return;

        int fontH = static_cast<int>(size);
        HFONT hFont = CreateFontW(
            fontH, 0, 0, 0,
            weight,
            (style == 2) ? TRUE : FALSE,
            FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            wideFamily.data()
        );
        HGDIOBJ oldFont = SelectObject(memDC, hFont);

        SIZE textSize = {};
        GetTextExtentPoint32W(memDC, wideText.data(), wideLen, &textSize);

        int textW = textSize.cx;
        int textH = textSize.cy;
        if (textW <= 0 || textH <= 0) {
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            DeleteDC(memDC);
            return;
        }

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = textW;
        bmi.bmiHeader.biHeight = -textH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP hbm = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!hbm || !bits) {
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            DeleteDC(memDC);
            return;
        }
        HGDIOBJ oldBm = SelectObject(memDC, hbm);

        RECT rc = { 0, 0, textW, textH };
        ::FillRect(memDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(255, 255, 255));
        TextOutW(memDC, 0, 0, wideText.data(), wideLen);

        const uint32_t* mask = static_cast<const uint32_t*>(bits);
        int32_t dstX0 = static_cast<int32_t>(origin.x);
        int32_t dstY0 = static_cast<int32_t>(origin.y);

        float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

        for (int y = 0; y < textH; ++y) {
            int dstY = dstY0 + y;
            if (dstY < 0 || dstY >= static_cast<int>(m_height)) continue;

            for (int x = 0; x < textW; ++x) {
                int dstX = dstX0 + x;
                if (dstX < 0 || dstX >= static_cast<int>(m_width)) continue;

                uint32_t maskVal = mask[y * textW + x] & 0xFF;
                if (maskVal == 0) continue;

                float alpha = (static_cast<float>(maskVal) / 255.0f) * srcAlphaNorm;
                if (alpha <= 0.0f) continue;

                size_t dstIdx = static_cast<size_t>(dstY) * m_width + dstX;
                uint32_t dstPixel = m_pixels[dstIdx];

                uint8_t dstB = static_cast<uint8_t>(dstPixel & 0xFF);
                uint8_t dstG = static_cast<uint8_t>((dstPixel >> 8) & 0xFF);
                uint8_t dstR = static_cast<uint8_t>((dstPixel >> 16) & 0xFF);

                uint8_t outR = static_cast<uint8_t>(color.r * alpha + dstR * (1.0f - alpha));
                uint8_t outG = static_cast<uint8_t>(color.g * alpha + dstG * (1.0f - alpha));
                uint8_t outB = static_cast<uint8_t>(color.b * alpha + dstB * (1.0f - alpha));

                m_pixels[dstIdx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
            }
        }

        SelectObject(memDC, oldFont);
        DeleteObject(hFont);
        SelectObject(memDC, oldBm);
        DeleteObject(hbm);
        DeleteDC(memDC);
#endif
    }

    void DrawTextLayout(void* nativeLayoutHandle, core::Point origin, core::Color color) override;

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<uint32_t> m_pixels;
};

class SoftwareRenderContext : public IRenderContext {
public:
    SoftwareRenderContext() = default;
    ~SoftwareRenderContext() override { Destroy(); }

    bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) override;
    void Resize(uint32_t width, uint32_t height) override;
    IRenderTarget* BeginFrame() override;
    void EndFrame() override;
    void Destroy() override;

private:
    void* m_nativeWindowHandle = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    SoftwareRenderTarget m_target;

#ifdef _WIN32
    HWND m_hwnd = nullptr;
    HDC  m_hdc = nullptr;
    BITMAPINFO m_bmi = {};
#endif
};

} // namespace rendering
} // namespace cppui
