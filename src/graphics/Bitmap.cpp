/**
 * @file Bitmap.cpp
 * @brief GDI+ 風格 32-bit 點陣影像實作，具備執行緒安全自動同步與 Pimpl 編譯防火牆
 */

#include "ui/graphics/Bitmap.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppui {
namespace graphics {

struct Bitmap::Impl {
    uint32_t                   m_width{ 0 };
    uint32_t                   m_height{ 0 };
    PixelFormat                m_format{ PixelFormat::Format32bppArgb };
    std::vector<uint32_t>      m_pixels;
    bool                       m_isLocked{ false };
    mutable compat::shared_mutex  m_mutex;

    Impl(uint32_t width, uint32_t height, PixelFormat format)
        : m_width(width), m_height(height), m_format(format) {
        if (width > 0 && height > 0) {
            m_pixels.resize(static_cast<size_t>(width) * height, 0);
        }
    }

    Impl(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format)
        : m_width(width), m_height(height), m_format(format) {
        size_t required = static_cast<size_t>(width) * height;
        m_pixels.assign(pixels.data(), pixels.data() + required);
    }
};

namespace {

static void ValidateDimensions(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        throw std::invalid_argument("Bitmap dimensions must be positive.");
    }
    if (width > Bitmap::MAX_DIMENSION || height > Bitmap::MAX_DIMENSION) {
        throw std::invalid_argument("Bitmap dimension exceeds maximum allowed dimension (" + compat::to_string(Bitmap::MAX_DIMENSION) + ").");
    }
    uint64_t totalPixels = static_cast<uint64_t>(width) * height;
    if (totalPixels > Bitmap::MAX_PIXELS) {
        throw std::invalid_argument("Bitmap total pixel count exceeds maximum memory limit.");
    }
}

} // anonymous namespace

Bitmap::Bitmap(uint32_t width, uint32_t height, PixelFormat format) {
    ValidateDimensions(width, height);
    m_impl = std::make_unique<Impl>(width, height, format);
}

Bitmap::Bitmap(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format) {
    ValidateDimensions(width, height);
    uint64_t required = static_cast<uint64_t>(width) * height;
    if (pixels.size() < static_cast<size_t>(required)) {
        throw std::invalid_argument("Provided pixel buffer size is smaller than image dimensions.");
    }
    m_impl = std::make_unique<Impl>(width, height, pixels, format);
}

Bitmap::Bitmap(compat::string_view filename) {
    if (filename.empty()) {
        throw std::invalid_argument("Filename cannot be empty.");
    }
    // 程序化產生棋盤測試紋理以供未有檔案解碼庫時使用
    m_impl = std::make_unique<Impl>(64, 64, PixelFormat::Format32bppArgb);
    for (uint32_t y = 0; y < m_impl->m_height; ++y) {
        for (uint32_t x = 0; x < m_impl->m_width; ++x) {
            bool check = ((x / 8) + (y / 8)) % 2 == 0;
            m_impl->m_pixels[y * m_impl->m_width + x] = check ? 0xFFFFFFFF : 0xFF808080;
        }
    }
}

Bitmap::~Bitmap() = default;

Bitmap::Bitmap(const Bitmap& other)
    : std::enable_shared_from_this<Bitmap>(other) {
    if (other.m_impl) {
        compat::shared_lock<compat::shared_mutex> lock(other.m_impl->m_mutex);
        m_impl = std::make_unique<Impl>(other.m_impl->m_width, other.m_impl->m_height, other.m_impl->m_format);
        m_impl->m_pixels = other.m_impl->m_pixels;
        m_impl->m_isLocked = other.m_impl->m_isLocked;
    } else {
        m_impl = std::make_unique<Impl>(0, 0, PixelFormat::Format32bppArgb);
    }
}

Bitmap::Bitmap(Bitmap&& other) noexcept = default;

Bitmap& Bitmap::operator=(const Bitmap& other) {
    if (this != &other) {
        if (other.m_impl) {
            compat::shared_lock<compat::shared_mutex> lockOther(other.m_impl->m_mutex);
            auto newImpl = std::make_unique<Impl>(other.m_impl->m_width, other.m_impl->m_height, other.m_impl->m_format);
            newImpl->m_pixels = other.m_impl->m_pixels;
            newImpl->m_isLocked = other.m_impl->m_isLocked;
            m_impl = std::move(newImpl);
        } else {
            m_impl = nullptr;
        }
    }
    return *this;
}

Bitmap& Bitmap::operator=(Bitmap&& other) noexcept = default;

uint32_t Bitmap::GetWidth() const noexcept {
    return m_impl ? m_impl->m_width : 0;
}

uint32_t Bitmap::GetHeight() const noexcept {
    return m_impl ? m_impl->m_height : 0;
}

PixelFormat Bitmap::GetPixelFormat() const noexcept {
    return m_impl ? m_impl->m_format : PixelFormat::Format32bppArgb;
}

uint32_t Bitmap::GetStride() const noexcept {
    return m_impl ? (m_impl->m_width * static_cast<uint32_t>(sizeof(uint32_t))) : 0;
}

core::Color Bitmap::GetPixel(uint32_t x, uint32_t y) const {
    if (!m_impl) {
        throw std::out_of_range("Bitmap is uninitialized.");
    }
    compat::shared_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    if (x >= m_impl->m_width || y >= m_impl->m_height) {
        throw std::out_of_range("Coordinates out of bitmap bounds.");
    }
    uint32_t argb = m_impl->m_pixels[static_cast<size_t>(y) * m_impl->m_width + x];
    return core::Color::FromArgb(argb);
}

void Bitmap::SetPixel(uint32_t x, uint32_t y, core::Color color) {
    if (!m_impl) {
        throw std::out_of_range("Bitmap is uninitialized.");
    }
    std::unique_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    if (x >= m_impl->m_width || y >= m_impl->m_height) {
        throw std::out_of_range("Coordinates out of bitmap bounds.");
    }
    m_impl->m_pixels[static_cast<size_t>(y) * m_impl->m_width + x] = color.ToArgb();
}

void Bitmap::Clear(core::Color color) {
    if (!m_impl) return;
    std::unique_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    uint32_t argb = color.ToArgb();
    std::fill(m_impl->m_pixels.begin(), m_impl->m_pixels.end(), argb);
}

compat::span<const uint32_t> Bitmap::GetPixelSpan() const {
    if (!m_impl) return {};
    compat::shared_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    return compat::span<const uint32_t>(m_impl->m_pixels.data(), m_impl->m_pixels.size());
}

compat::span<uint32_t> Bitmap::GetPixelSpanMut() {
    if (!m_impl) return {};
    std::unique_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    return compat::span<uint32_t>(m_impl->m_pixels.data(), m_impl->m_pixels.size());
}

void* Bitmap::LockBits(const core::Rect& rect, uint32_t flags) {
    (void)rect;
    (void)flags;
    if (!m_impl) return nullptr;
    std::unique_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    m_impl->m_isLocked = true;
    return m_impl->m_pixels.data();
}

void Bitmap::UnlockBits() {
    if (!m_impl) return;
    std::unique_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    m_impl->m_isLocked = false;
}

std::shared_ptr<Bitmap> Bitmap::Clone() const {
    if (!m_impl) {
        throw std::invalid_argument("Cannot clone uninitialized bitmap.");
    }
    compat::shared_lock<compat::shared_mutex> lock(m_impl->m_mutex);
    if (m_impl->m_width == 0 || m_impl->m_height == 0) {
        throw std::invalid_argument("Bitmap dimensions must be positive to clone.");
    }
    auto result = std::make_shared<Bitmap>(m_impl->m_width, m_impl->m_height, m_impl->m_format);
    std::copy(m_impl->m_pixels.begin(), m_impl->m_pixels.end(), result->m_impl->m_pixels.begin());
    return result;
}

std::shared_ptr<Bitmap> Bitmap::Clone(const core::Rect& rect, PixelFormat format) const {
    if (!m_impl) {
        throw std::invalid_argument("Cannot clone uninitialized bitmap.");
    }
    compat::shared_lock<compat::shared_mutex> lock(m_impl->m_mutex);

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
        if (sy < 0 || sy >= static_cast<int32_t>(m_impl->m_height)) continue;

        for (uint32_t dx = 0; dx < targetW; ++dx) {
            int32_t sx = rx + static_cast<int32_t>(dx);
            if (sx < 0 || sx >= static_cast<int32_t>(m_impl->m_width)) continue;

            uint32_t px = m_impl->m_pixels[static_cast<size_t>(sy) * m_impl->m_width + sx];
            result->m_impl->m_pixels[static_cast<size_t>(dy) * targetW + dx] = px;
        }
    }
    return result;
}

void Bitmap::RotateFlip(RotateFlipType rotateFlipType) {
    if (!m_impl) return;
    std::unique_lock<compat::shared_mutex> lock(m_impl->m_mutex);

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

    uint32_t srcW = m_impl->m_width;
    uint32_t srcH = m_impl->m_height;
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

            temp[static_cast<size_t>(ry) * dstW + rx] = m_impl->m_pixels[static_cast<size_t>(y) * srcW + x];
        }
    }

    m_impl->m_width  = dstW;
    m_impl->m_height = dstH;
    m_impl->m_pixels = std::move(temp);
}

namespace {

static ImageFormat DeduceImageFormatFromExtension(compat::string_view filename) {
    if (filename.empty()) {
        return ImageFormat::Auto;
    }

    int64_t len = static_cast<int64_t>(filename.size());
    int64_t dotPos = -1;
    for (int64_t i = len - 1; i >= 0; --i) {
        char ch = filename[static_cast<size_t>(i)];
        if (ch == '/' || ch == '\\') {
            break;
        }
        if (ch == '.') {
            dotPos = i;
            break;
        }
    }

    // 嚴格判定：點必須在主檔名之後 (dotPos > 0 且前一字元非目錄分隔符號)
    if (dotPos <= 0 || dotPos + 1 >= len || filename[static_cast<size_t>(dotPos - 1)] == '/' || filename[static_cast<size_t>(dotPos - 1)] == '\\') {
        return ImageFormat::Auto;
    }

    std::string ext;
    ext.reserve(static_cast<size_t>(len - dotPos - 1));
    for (int64_t i = dotPos + 1; i < len; ++i) {
        char ch = filename[static_cast<size_t>(i)];
        if (ch >= 'A' && ch <= 'Z') {
            ext.push_back(static_cast<char>(ch - 'A' + 'a'));
        } else {
            ext.push_back(ch);
        }
    }

    if (ext == "bmp") {
        return ImageFormat::Bmp;
    }
    if (ext == "png") {
        return ImageFormat::Png;
    }
    if (ext == "jpg" || ext == "jpeg") {
        return ImageFormat::Jpeg;
    }
    if (ext == "gif") {
        return ImageFormat::Gif;
    }
    if (ext == "webp") {
        return ImageFormat::Webp;
    }
    if (ext == "heic" || ext == "heif") {
        return ImageFormat::Heif;
    }
    if (ext == "tif" || ext == "tiff") {
        return ImageFormat::Tiff;
    }

    return ImageFormat::Auto;
}

static bool WriteBmpInternal(
    compat::string_view filename,
    uint32_t width,
    uint32_t height,
    compat::span<const uint32_t> pixels
) {
    std::ofstream out(std::string(filename.data(), filename.size()), std::ios::binary);
    if (!out) return false;

    const uint64_t imageSize64 = static_cast<uint64_t>(width) * height * 4ULL;
    if (imageSize64 > 0xFFFFFFFFULL - 54ULL) {
        return false;
    }
    const uint32_t imageSize = static_cast<uint32_t>(imageSize64);
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

    // 採用無號 32 位元整數進行位移，杜絕有號負數位移警告
    const uint32_t biHeightRaw = static_cast<uint32_t>(-static_cast<int32_t>(height));
    uint8_t infoHeader[40] = {
        40, 0, 0, 0,
        static_cast<uint8_t>(width & 0xFF),
        static_cast<uint8_t>((width >> 8) & 0xFF),
        static_cast<uint8_t>((width >> 16) & 0xFF),
        static_cast<uint8_t>((width >> 24) & 0xFF),
        static_cast<uint8_t>(biHeightRaw & 0xFF),
        static_cast<uint8_t>((biHeightRaw >> 8) & 0xFF),
        static_cast<uint8_t>((biHeightRaw >> 16) & 0xFF),
        static_cast<uint8_t>((biHeightRaw >> 24) & 0xFF),
        1, 0,
        32, 0,
        0, 0, 0, 0,
        static_cast<uint8_t>(imageSize & 0xFF),
        static_cast<uint8_t>((imageSize >> 8) & 0xFF),
        static_cast<uint8_t>((imageSize >> 16) & 0xFF),
        static_cast<uint8_t>((imageSize >> 24) & 0xFF),
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    out.write(reinterpret_cast<const char*>(fileHeader), static_cast<std::streamsize>(sizeof(fileHeader)));
    out.write(reinterpret_cast<const char*>(infoHeader), static_cast<std::streamsize>(sizeof(infoHeader)));
    out.write(reinterpret_cast<const char*>(pixels.data()), static_cast<std::streamsize>(imageSize));
    out.flush();
    return out.good();
}

} // anonymous namespace

bool Bitmap::Save(compat::string_view filename, ImageFormat format) const {
    if (filename.empty() || !m_impl) {
        return false;
    }

    ImageFormat resolvedFormat = format;
    if (resolvedFormat == ImageFormat::Auto) {
        resolvedFormat = DeduceImageFormatFromExtension(filename);
        if (resolvedFormat == ImageFormat::Auto) {
            // 無副檔名或未識別副檔名，依 Fail-Fast 原則安全拒絕寫檔
            return false;
        }
    }

    // 取得共享鎖保護所有內部像素緩衝區與維度
    compat::shared_lock<compat::shared_mutex> lock(m_impl->m_mutex);

    const uint32_t width = m_impl->m_width;
    const uint32_t height = m_impl->m_height;
    if (width == 0 || height == 0 || m_impl->m_pixels.size() < static_cast<size_t>(width) * height) {
        return false;
    }

    compat::span<const uint32_t> pixels(m_impl->m_pixels.data(), static_cast<size_t>(width) * height);

    switch (resolvedFormat) {
        case ImageFormat::Bmp:
            return WriteBmpInternal(filename, width, height, pixels);

        case ImageFormat::Png:
        case ImageFormat::Jpeg:
        case ImageFormat::Gif:
        case ImageFormat::Webp:
        case ImageFormat::Heif:
        case ImageFormat::Tiff:
            // 格式列舉已定義，但內建編碼器尚未掛載
            return false;

        case ImageFormat::Auto:
        default:
            return false;
    }
}

} // namespace graphics
} // namespace cppui
