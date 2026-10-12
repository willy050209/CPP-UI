/**
 * @file Bitmap.cpp
 * @brief GDI+ 風格 32-bit 點陣影像實作，具備執行緒安全自動同步
 */

#include "ui/graphics/Bitmap.hpp"
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <fstream>

namespace cppui {
namespace graphics {

Bitmap::Bitmap(uint32_t width, uint32_t height, PixelFormat format)
    : m_width(width), m_height(height), m_format(format) {
    if (width == 0 || height == 0) {
        throw std::invalid_argument("Bitmap dimensions must be positive.");
    }
    m_pixels.resize(static_cast<size_t>(width) * height, 0);
}

Bitmap::Bitmap(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format)
    : m_width(width), m_height(height), m_format(format) {
    if (width == 0 || height == 0) {
        throw std::invalid_argument("Bitmap dimensions must be positive.");
    }
    size_t required = static_cast<size_t>(width) * height;
    if (pixels.size() < required) {
        throw std::invalid_argument("Provided pixel buffer size is smaller than image dimensions.");
    }
    m_pixels.assign(pixels.data(), pixels.data() + required);
}

Bitmap::Bitmap(compat::string_view filename)
    : m_width(64), m_height(64), m_format(PixelFormat::Format32bppArgb) {
    if (filename.empty()) {
        throw std::invalid_argument("Filename cannot be empty.");
    }
    // 程序化產生棋盤測試紋理以供未有檔案解碼庫時使用
    m_pixels.resize(static_cast<size_t>(m_width) * m_height, 0);
    for (uint32_t y = 0; y < m_height; ++y) {
        for (uint32_t x = 0; x < m_width; ++x) {
            bool check = ((x / 8) + (y / 8)) % 2 == 0;
            m_pixels[y * m_width + x] = check ? 0xFFFFFFFF : 0xFF808080;
        }
    }
}

Bitmap::Bitmap(const Bitmap& other)
    : std::enable_shared_from_this<Bitmap>(other) {
    std::shared_lock<std::shared_mutex> lock(other.m_mutex);
    m_width    = other.m_width;
    m_height   = other.m_height;
    m_format   = other.m_format;
    m_pixels   = other.m_pixels;
    m_isLocked = other.m_isLocked;
}

Bitmap::Bitmap(Bitmap&& other) noexcept
    : std::enable_shared_from_this<Bitmap>(std::move(other)) {
    std::unique_lock<std::shared_mutex> lock(other.m_mutex);
    m_width    = other.m_width;
    m_height   = other.m_height;
    m_format   = other.m_format;
    m_pixels   = std::move(other.m_pixels);
    m_isLocked = other.m_isLocked;
    other.m_width = 0;
    other.m_height = 0;
    other.m_isLocked = false;
}

Bitmap& Bitmap::operator=(const Bitmap& other) {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::shared_lock<std::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);

        m_width    = other.m_width;
        m_height   = other.m_height;
        m_format   = other.m_format;
        m_pixels   = other.m_pixels;
        m_isLocked = other.m_isLocked;
    }
    return *this;
}

Bitmap& Bitmap::operator=(Bitmap&& other) noexcept {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::unique_lock<std::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);

        m_width    = other.m_width;
        m_height   = other.m_height;
        m_format   = other.m_format;
        m_pixels   = std::move(other.m_pixels);
        m_isLocked = other.m_isLocked;
        other.m_width = 0;
        other.m_height = 0;
        other.m_isLocked = false;
    }
    return *this;
}

uint32_t Bitmap::GetWidth() const noexcept {
    return m_width;
}

uint32_t Bitmap::GetHeight() const noexcept {
    return m_height;
}

PixelFormat Bitmap::GetPixelFormat() const noexcept {
    return m_format;
}

uint32_t Bitmap::GetStride() const noexcept {
    return m_width * static_cast<uint32_t>(sizeof(uint32_t));
}

core::Color Bitmap::GetPixel(uint32_t x, uint32_t y) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (x >= m_width || y >= m_height) {
        throw std::out_of_range("Coordinates out of bitmap bounds.");
    }
    uint32_t argb = m_pixels[static_cast<size_t>(y) * m_width + x];
    return core::Color::FromArgb(argb);
}

void Bitmap::SetPixel(uint32_t x, uint32_t y, core::Color color) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (x >= m_width || y >= m_height) {
        throw std::out_of_range("Coordinates out of bitmap bounds.");
    }
    m_pixels[static_cast<size_t>(y) * m_width + x] = color.ToArgb();
}

void Bitmap::Clear(core::Color color) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    uint32_t argb = color.ToArgb();
    std::fill(m_pixels.begin(), m_pixels.end(), argb);
}

compat::span<const uint32_t> Bitmap::GetPixelSpan() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return compat::span<const uint32_t>(m_pixels.data(), m_pixels.size());
}

compat::span<uint32_t> Bitmap::GetPixelSpanMut() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return compat::span<uint32_t>(m_pixels.data(), m_pixels.size());
}

void* Bitmap::LockBits(const core::Rect& rect, uint32_t flags) {
    (void)rect;
    (void)flags;
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_isLocked = true;
    return m_pixels.data();
}

void Bitmap::UnlockBits() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_isLocked = false;
}

std::shared_ptr<Bitmap> Bitmap::Clone() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    if (m_width == 0 || m_height == 0) {
        throw std::invalid_argument("Bitmap dimensions must be positive to clone.");
    }

    auto result = std::make_shared<Bitmap>(m_width, m_height, m_format);
    std::copy(m_pixels.begin(), m_pixels.end(), result->m_pixels.begin());
    return result;
}

std::shared_ptr<Bitmap> Bitmap::Clone(const core::Rect& rect, PixelFormat format) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    int32_t rx = static_cast<int32_t>(rect.x);
    int32_t ry = static_cast<int32_t>(rect.y);
    int32_t rw = static_cast<int32_t>(rect.width);
    int32_t rh = static_cast<int32_t>(rect.height);

    if (rw <= 0 || rh <= 0) {
        throw std::invalid_argument("Clone rectangle dimensions must be positive.");
    }

    uint32_t targetW = static_cast<uint32_t>(rw);
    uint32_t targetH = static_cast<uint32_t>(rh);
    auto result = std::make_shared<Bitmap>(targetW, targetH, format);

    for (uint32_t dy = 0; dy < targetH; ++dy) {
        int32_t sy = ry + static_cast<int32_t>(dy);
        if (sy < 0 || sy >= static_cast<int32_t>(m_height)) continue;

        for (uint32_t dx = 0; dx < targetW; ++dx) {
            int32_t sx = rx + static_cast<int32_t>(dx);
            if (sx < 0 || sx >= static_cast<int32_t>(m_width)) continue;

            uint32_t px = m_pixels[static_cast<size_t>(sy) * m_width + sx];
            result->m_pixels[static_cast<size_t>(dy) * targetW + dx] = px;
        }
    }
    return result;
}

void Bitmap::RotateFlip(RotateFlipType rotateFlipType) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    bool flipX = false;
    bool flipY = false;
    int32_t rotateAngle = 0;

    switch (rotateFlipType) {
        case RotateFlipType::RotateNoneFlipNone:
            return;
        case RotateFlipType::Rotate90FlipNone:
            rotateAngle = 90;
            break;
        case RotateFlipType::Rotate180FlipNone:
            rotateAngle = 180;
            break;
        case RotateFlipType::Rotate270FlipNone:
            rotateAngle = 270;
            break;
        case RotateFlipType::RotateNoneFlipX:
            flipX = true;
            break;
        case RotateFlipType::RotateNoneFlipY:
            flipY = true;
            break;
        case RotateFlipType::RotateNoneFlipXY:
            flipX = true;
            flipY = true;
            break;
    }

    uint32_t srcW = m_width;
    uint32_t srcH = m_height;
    uint32_t dstW = (rotateAngle == 90 || rotateAngle == 270) ? srcH : srcW;
    uint32_t dstH = (rotateAngle == 90 || rotateAngle == 270) ? srcW : srcH;

    std::vector<uint32_t> temp(static_cast<size_t>(dstW) * dstH);

    for (uint32_t y = 0; y < srcH; ++y) {
        for (uint32_t x = 0; x < srcW; ++x) {
            uint32_t tx = x;
            uint32_t ty = y;

            if (flipX) tx = srcW - 1 - tx;
            if (flipY) ty = srcH - 1 - ty;

            uint32_t rx = tx;
            uint32_t ry = ty;

            if (rotateAngle == 90) {
                rx = srcH - 1 - ty;
                ry = tx;
            } else if (rotateAngle == 180) {
                rx = srcW - 1 - tx;
                ry = srcH - 1 - ty;
            } else if (rotateAngle == 270) {
                rx = ty;
                ry = srcW - 1 - tx;
            }

            temp[static_cast<size_t>(ry) * dstW + rx] = m_pixels[static_cast<size_t>(y) * srcW + x];
        }
    }

    m_width  = dstW;
    m_height = dstH;
    m_pixels = std::move(temp);
}

/// <summary>
/// 將影像儲存為未壓縮 32-bit BMP 點陣圖檔案。
/// </summary>
bool Bitmap::SaveAsBmp(compat::string_view filename) const {
    if (filename.empty() || m_width == 0 || m_height == 0) return false;
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    std::ofstream out(std::string(filename.data(), filename.size()), std::ios::binary);
    if (!out) return false;

    const uint32_t imageSize = m_width * m_height * 4;
    const uint32_t fileSize = 54 + imageSize;

    uint8_t fileHeader[14] = {
        'B', 'M',
        static_cast<uint8_t>(fileSize & 0xFF),
        static_cast<uint8_t>((fileSize >> 8) & 0xFF),
        static_cast<uint8_t>((fileSize >> 16) & 0xFF),
        static_cast<uint8_t>((fileSize >> 24) & 0xFF),
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    const int32_t biHeight = -static_cast<int32_t>(m_height);
    uint8_t infoHeader[40] = {
        40, 0, 0, 0,
        static_cast<uint8_t>(m_width & 0xFF),
        static_cast<uint8_t>((m_width >> 8) & 0xFF),
        static_cast<uint8_t>((m_width >> 16) & 0xFF),
        static_cast<uint8_t>((m_width >> 24) & 0xFF),
        static_cast<uint8_t>(biHeight & 0xFF),
        static_cast<uint8_t>((biHeight >> 8) & 0xFF),
        static_cast<uint8_t>((biHeight >> 16) & 0xFF),
        static_cast<uint8_t>((biHeight >> 24) & 0xFF),
        1, 0,
        32, 0,
        0, 0, 0, 0,
        static_cast<uint8_t>(imageSize & 0xFF),
        static_cast<uint8_t>((imageSize >> 8) & 0xFF),
        static_cast<uint8_t>((imageSize >> 16) & 0xFF),
        static_cast<uint8_t>((imageSize >> 24) & 0xFF),
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    out.write(reinterpret_cast<const char*>(fileHeader), 14);
    out.write(reinterpret_cast<const char*>(infoHeader), 40);
    out.write(reinterpret_cast<const char*>(m_pixels.data()), imageSize);
    return true;
}

} // namespace graphics
} // namespace cppui
