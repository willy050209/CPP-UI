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
#ifdef DrawText
#undef DrawText
#endif
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

    void DrawText(compat::string_view text, core::Point origin, compat::string_view family, float size, uint16_t weight, uint8_t style, core::Color color) override;
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
