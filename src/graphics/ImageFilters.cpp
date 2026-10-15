/*
 * File: ImageFilters.cpp
 * Purpose: Implements high-performance image processing filters (grayscale, thresholding, blurs, edges).
 */

#include "ui/graphics/ImageFilters.hpp"
#include "ui/graphics/PixelOps.hpp"
#include "ui/graphics/Convolution.hpp"
#include "ui/graphics/SkiaImageBridge.hpp"
#include "compat/Compat.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <queue>

namespace cppui {
namespace graphics {

namespace {

COMPAT_ALWAYS_INLINE static uint8_t LuminanceBT601(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<uint8_t>((77 * r + 150 * g + 29 * b + 128) >> 8);
}

COMPAT_ALWAYS_INLINE static uint8_t LuminanceBT709(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<uint8_t>((54 * r + 183 * g + 19 * b + 128) >> 8);
}

COMPAT_ALWAYS_INLINE static uint8_t LuminanceAverage(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<uint8_t>((static_cast<uint32_t>(r) + g + b) / 3);
}

COMPAT_ALWAYS_INLINE static uint8_t LuminanceLightness(uint8_t r, uint8_t g, uint8_t b) noexcept {
    uint8_t maxVal = (std::max)({ r, g, b });
    uint8_t minVal = (std::min)({ r, g, b });
    return static_cast<uint8_t>((static_cast<uint32_t>(maxVal) + minVal) / 2);
}

COMPAT_ALWAYS_INLINE static uint8_t ComputePixelLuminance(uint32_t p, GrayscaleMethod method) noexcept {
    uint8_t r = static_cast<uint8_t>((p >> 16) & 0xFF);
    uint8_t g = static_cast<uint8_t>((p >> 8) & 0xFF);
    uint8_t b = static_cast<uint8_t>(p & 0xFF);
    switch (method) {
        case GrayscaleMethod::BT709:
            return LuminanceBT709(r, g, b);
        case GrayscaleMethod::Average:
            return LuminanceAverage(r, g, b);
        case GrayscaleMethod::Lightness:
            return LuminanceLightness(r, g, b);
        case GrayscaleMethod::BT601:
        default:
            return LuminanceBT601(r, g, b);
    }
}

// 9 元素排序網路 (Branchless Sorting Network) 找出中位數 (索引 4)
COMPAT_ALWAYS_INLINE static uint8_t Median9(uint8_t* arr) noexcept {
    auto MinMax = [](uint8_t& a, uint8_t& b) noexcept {
        if (a > b) std::swap(a, b);
    };

    MinMax(arr[0], arr[1]); MinMax(arr[3], arr[4]); MinMax(arr[6], arr[7]);
    MinMax(arr[1], arr[2]); MinMax(arr[4], arr[5]); MinMax(arr[7], arr[8]);
    MinMax(arr[0], arr[1]); MinMax(arr[3], arr[4]); MinMax(arr[6], arr[7]);
    MinMax(arr[0], arr[3]); MinMax(arr[3], arr[6]); MinMax(arr[0], arr[3]);
    MinMax(arr[1], arr[4]); MinMax(arr[4], arr[7]); MinMax(arr[1], arr[4]);
    MinMax(arr[2], arr[5]); MinMax(arr[5], arr[8]); MinMax(arr[2], arr[5]);
    MinMax(arr[1], arr[3]); MinMax(arr[5], arr[7]); MinMax(arr[2], arr[6]);
    MinMax(arr[4], arr[6]); MinMax(arr[2], arr[4]); MinMax(arr[2], arr[3]);
    MinMax(arr[4], arr[5]);
    return arr[4];
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// 1. 灰階轉換 (Grayscale)
// -----------------------------------------------------------------------------

void ImageFilters::Grayscale(Bitmap& bmp, GrayscaleMethod method, const ParallelOptions& options) {
    PixelOps::ForEach(bmp, [method](uint32_t& p) {
        uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
        uint8_t y = ComputePixelLuminance(p, method);
        p = (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(y) << 16) |
            (static_cast<uint32_t>(y) << 8)  |
            static_cast<uint32_t>(y);
    }, options);
}

std::shared_ptr<Bitmap> ImageFilters::ToGrayscale(const Bitmap& src, GrayscaleMethod method, const ParallelOptions& options) {
    auto clone = src.Clone();
    if (clone) {
        Grayscale(*clone, method, options);
    }
    return clone;
}

// -----------------------------------------------------------------------------
// 2. 二值化 (Thresholding)
// -----------------------------------------------------------------------------

void ImageFilters::Threshold(Bitmap& bmp, uint8_t threshold, ThresholdType type, const ParallelOptions& options) {
    PixelOps::ForEach(bmp, [threshold, type](uint32_t& p) {
        uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
        uint8_t y = LuminanceBT601(
            static_cast<uint8_t>((p >> 16) & 0xFF),
            static_cast<uint8_t>((p >> 8) & 0xFF),
            static_cast<uint8_t>(p & 0xFF)
        );

        uint8_t outY = 0;
        switch (type) {
            case ThresholdType::Binary:
                outY = (y > threshold) ? 255 : 0;
                break;
            case ThresholdType::BinaryInverted:
                outY = (y > threshold) ? 0 : 255;
                break;
            case ThresholdType::Truncate:
                outY = (y > threshold) ? threshold : y;
                break;
            case ThresholdType::ToZero:
                outY = (y > threshold) ? y : 0;
                break;
        }

        p = (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(outY) << 16) |
            (static_cast<uint32_t>(outY) << 8)  |
            static_cast<uint32_t>(outY);
    }, options);
}

uint8_t ImageFilters::ComputeOtsuThreshold(const Bitmap& bmp) {
    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    const uint64_t total = static_cast<uint64_t>(width) * height;
    if (total == 0) return 128;

    uint32_t hist[256] = { 0 };
    PixelOps::ForEach(bmp, [&hist](const uint32_t& p) {
        uint8_t y = LuminanceBT601(
            static_cast<uint8_t>((p >> 16) & 0xFF),
            static_cast<uint8_t>((p >> 8) & 0xFF),
            static_cast<uint8_t>(p & 0xFF)
        );
        hist[y]++;
    });

    float sum = 0.0f;
    for (int32_t i = 0; i < 256; ++i) {
        sum += static_cast<float>(i) * static_cast<float>(hist[i]);
    }

    float sumB = 0.0f;
    uint64_t wB = 0;
    float maxVariance = 0.0f;
    uint8_t bestThreshold = 0;

    for (int32_t t = 0; t < 256; ++t) {
        wB += hist[t];
        if (wB == 0) continue;
        uint64_t wF = total - wB;
        if (wF == 0) break;

        sumB += static_cast<float>(t) * static_cast<float>(hist[t]);
        float mB = sumB / static_cast<float>(wB);
        float mF = (sum - sumB) / static_cast<float>(wF);

        float betweenVar = static_cast<float>(wB) * static_cast<float>(wF) * (mB - mF) * (mB - mF);
        if (betweenVar > maxVariance) {
            maxVariance = betweenVar;
            bestThreshold = static_cast<uint8_t>(t);
        }
    }

    return bestThreshold;
}

void ImageFilters::OtsuThreshold(Bitmap& bmp, ThresholdType type, const ParallelOptions& options) {
    uint8_t optT = ComputeOtsuThreshold(bmp);
    Threshold(bmp, optT, type, options);
}

void ImageFilters::AdaptiveThreshold(Bitmap& bmp, int32_t blockSize, int32_t c, bool useGaussian, const ParallelOptions& options) {
    if (blockSize < 3 || (blockSize % 2 == 0)) {
        throw std::invalid_argument("ImageFilters::AdaptiveThreshold: blockSize must be odd and >= 3");
    }

    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    if (width == 0 || height == 0) return;

    // 1. 抽取純灰階亮度快取
    std::vector<uint8_t> gray(static_cast<size_t>(width) * height);
    auto srcSpan = bmp.GetPixelSpan();
    for (size_t i = 0; i < gray.size(); ++i) {
        uint32_t p = srcSpan[i];
        gray[i] = LuminanceBT601(
            static_cast<uint8_t>((p >> 16) & 0xFF),
            static_cast<uint8_t>((p >> 8) & 0xFF),
            static_cast<uint8_t>(p & 0xFF)
        );
    }

    if (useGaussian) {
        // 使用高斯模糊作為局部加權均值
        Bitmap grayBmp(width, height);
        auto grayMut = grayBmp.GetPixelSpanMut();
        for (size_t i = 0; i < gray.size(); ++i) {
            uint8_t g = gray[i];
            grayMut[i] = (0xFF << 24) | (g << 16) | (g << 8) | g;
        }
        float sigma = static_cast<float>(blockSize) / 6.0f;
        GaussianBlur(grayBmp, sigma, sigma, options, false);
        auto meanSpan = grayBmp.GetPixelSpan();

        PixelOps::ForEach(bmp, [c, &meanSpan, width](uint32_t x, uint32_t y, uint32_t& p) {
            size_t idx = static_cast<size_t>(y) * width + x;
            uint8_t g = LuminanceBT601(
                static_cast<uint8_t>((p >> 16) & 0xFF),
                static_cast<uint8_t>((p >> 8) & 0xFF),
                static_cast<uint8_t>(p & 0xFF)
            );
            uint8_t meanVal = static_cast<uint8_t>(meanSpan[idx] & 0xFF);
            int32_t thresh = static_cast<int32_t>(meanVal) - c;
            uint8_t outY = (static_cast<int32_t>(g) >= thresh) ? 255 : 0;
            uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
            p = (static_cast<uint32_t>(a) << 24) | (outY << 16) | (outY << 8) | outY;
        }, options);
        return;
    }

    // 方框均值局部適應二值化 (以 2D 積分圖 Integral Image 達成 O(1) 視窗計算)
    size_t intW = static_cast<size_t>(width) + 1;
    size_t intH = static_cast<size_t>(height) + 1;
    std::vector<uint32_t> integral(intW * intH, 0);

    for (uint32_t y = 0; y < height; ++y) {
        uint32_t rowSum = 0;
        for (uint32_t x = 0; x < width; ++x) {
            rowSum += gray[static_cast<size_t>(y) * width + x];
            integral[(static_cast<size_t>(y) + 1) * intW + (x + 1)] =
                integral[static_cast<size_t>(y) * intW + (x + 1)] + rowSum;
        }
    }

    const int32_t rad = blockSize / 2;

    PixelOps::ForEach(bmp, [rad, c, &integral, &gray, intW, width, height](uint32_t x, uint32_t y, uint32_t& p) {
        int32_t ix = static_cast<int32_t>(x);
        int32_t iy = static_cast<int32_t>(y);

        int32_t x1 = (std::max)(0, ix - rad);
        int32_t y1 = (std::max)(0, iy - rad);
        int32_t x2 = (std::min)(static_cast<int32_t>(width) - 1, ix + rad);
        int32_t y2 = (std::min)(static_cast<int32_t>(height) - 1, iy + rad);

        uint32_t count = static_cast<uint32_t>((x2 - x1 + 1) * (y2 - y1 + 1));
        size_t idxTL = static_cast<size_t>(y1) * intW + x1;
        size_t idxTR = static_cast<size_t>(y1) * intW + (x2 + 1);
        size_t idxBL = static_cast<size_t>(y2 + 1) * intW + x1;
        size_t idxBR = static_cast<size_t>(y2 + 1) * intW + (x2 + 1);

        uint32_t sum = integral[idxBR] - integral[idxBL] - integral[idxTR] + integral[idxTL];
        int32_t meanVal = static_cast<int32_t>(sum / count);

        uint8_t curVal = gray[static_cast<size_t>(y) * width + x];
        uint8_t outY = (static_cast<int32_t>(curVal) >= (meanVal - c)) ? 255 : 0;
        uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
        p = (static_cast<uint32_t>(a) << 24) | (outY << 16) | (outY << 8) | outY;
    }, options);
}

// -----------------------------------------------------------------------------
// 3. 模糊平滑濾鏡 (Blur Filters)
// -----------------------------------------------------------------------------

void ImageFilters::BoxBlur(Bitmap& bmp, int32_t radius, const ParallelOptions& options) {
    if (radius < 0) {
        throw std::invalid_argument("ImageFilters::BoxBlur: radius must be >= 0");
    }
    if (radius == 0) return;

    auto kernel = ConvolutionEngine::CreateBoxKernel(radius, radius);
    Bitmap dst(bmp.GetWidth(), bmp.GetHeight());
    ConvolutionEngine::ConvolveSeparable(bmp, dst, kernel, BorderMode::Replicate, options);
    std::memcpy(bmp.GetPixelSpanMut().data(), dst.GetPixelSpan().data(), static_cast<size_t>(bmp.GetWidth()) * bmp.GetHeight() * sizeof(uint32_t));
}

void ImageFilters::MeanBlur(Bitmap& bmp, int32_t kernelSize, const ParallelOptions& options) {
    if (kernelSize < 1 || (kernelSize % 2 == 0)) {
        throw std::invalid_argument("ImageFilters::MeanBlur: kernelSize must be odd and >= 1");
    }
    if (kernelSize == 1) return;
    BoxBlur(bmp, (kernelSize - 1) / 2, options);
}

void ImageFilters::GaussianBlur(Bitmap& bmp, float sigmaX, float sigmaY, const ParallelOptions& options, bool preferSkia) {
    if (sigmaX <= 0.0f) {
        throw std::invalid_argument("ImageFilters::GaussianBlur: sigmaX must be > 0");
    }
    if (sigmaY <= 0.0f) {
        sigmaY = sigmaX;
    }

    if (preferSkia && SkiaImageBridge::IsSkiaAvailable()) {
        if (SkiaImageBridge::TryGaussianBlur(bmp, sigmaX, sigmaY)) {
            return;
        }
    }

    auto kernel = ConvolutionEngine::CreateGaussianKernel(sigmaX, sigmaY);
    Bitmap dst(bmp.GetWidth(), bmp.GetHeight());
    ConvolutionEngine::ConvolveSeparable(bmp, dst, kernel, BorderMode::Replicate, options);
    std::memcpy(bmp.GetPixelSpanMut().data(), dst.GetPixelSpan().data(), static_cast<size_t>(bmp.GetWidth()) * bmp.GetHeight() * sizeof(uint32_t));
}

void ImageFilters::MedianBlur(Bitmap& bmp, int32_t kernelSize, const ParallelOptions& options) {
    if (kernelSize != 3 && kernelSize != 5) {
        throw std::invalid_argument("ImageFilters::MedianBlur: currently supports kernelSize 3 or 5");
    }

    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    if (width == 0 || height == 0) return;

    Bitmap srcCopy(bmp);
    const auto srcSpan = srcCopy.GetPixelSpan();
    const uint32_t* srcPixels = srcSpan.data();

    const int32_t rad = kernelSize / 2;

    PixelOps::ForEach(bmp, [rad, kernelSize, srcPixels, width, height](uint32_t x, uint32_t y, uint32_t& p) {
        int32_t ix = static_cast<int32_t>(x);
        int32_t iy = static_cast<int32_t>(y);

        if (kernelSize == 3) {
            uint8_t rVals[9];
            uint8_t gVals[9];
            uint8_t bVals[9];
            size_t idx = 0;

            for (int32_t dy = -1; dy <= 1; ++dy) {
                int32_t sy = compat::clamp(iy + dy, 0, static_cast<int32_t>(height) - 1);
                for (int32_t dx = -1; dx <= 1; ++dx) {
                    int32_t sx = compat::clamp(ix + dx, 0, static_cast<int32_t>(width) - 1);
                    uint32_t px = srcPixels[static_cast<size_t>(sy) * width + sx];
                    rVals[idx] = static_cast<uint8_t>((px >> 16) & 0xFF);
                    gVals[idx] = static_cast<uint8_t>((px >> 8) & 0xFF);
                    bVals[idx] = static_cast<uint8_t>(px & 0xFF);
                    idx++;
                }
            }

            uint8_t rMed = Median9(rVals);
            uint8_t gMed = Median9(gVals);
            uint8_t bMed = Median9(bVals);
            uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
            p = (static_cast<uint32_t>(a) << 24) | (rMed << 16) | (gMed << 8) | bMed;
        } else {
            // 5x5 一般排序
            std::vector<uint8_t> rVals; rVals.reserve(25);
            std::vector<uint8_t> gVals; gVals.reserve(25);
            std::vector<uint8_t> bVals; bVals.reserve(25);

            for (int32_t dy = -rad; dy <= rad; ++dy) {
                int32_t sy = compat::clamp(iy + dy, 0, static_cast<int32_t>(height) - 1);
                for (int32_t dx = -rad; dx <= rad; ++dx) {
                    int32_t sx = compat::clamp(ix + dx, 0, static_cast<int32_t>(width) - 1);
                    uint32_t px = srcPixels[static_cast<size_t>(sy) * width + sx];
                    rVals.push_back(static_cast<uint8_t>((px >> 16) & 0xFF));
                    gVals.push_back(static_cast<uint8_t>((px >> 8) & 0xFF));
                    bVals.push_back(static_cast<uint8_t>(px & 0xFF));
                }
            }
            std::nth_element(rVals.begin(), rVals.begin() + 12, rVals.end());
            std::nth_element(gVals.begin(), gVals.begin() + 12, gVals.end());
            std::nth_element(bVals.begin(), bVals.begin() + 12, bVals.end());
            uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
            p = (static_cast<uint32_t>(a) << 24) | (rVals[12] << 16) | (gVals[12] << 8) | bVals[12];
        }
    }, options);
}

void ImageFilters::MotionBlur(Bitmap& bmp, int32_t length, float angleDegrees, const ParallelOptions& options) {
    if (length < 1) {
        throw std::invalid_argument("ImageFilters::MotionBlur: length must be >= 1");
    }
    if (length == 1) return;

    int32_t kSize = length;
    if (kSize % 2 == 0) kSize++;
    int32_t rad = kSize / 2;

    float radAngle = angleDegrees * 3.14159265358979323846f / 180.0f;
    float cosA = std::cos(radAngle);
    float sinA = std::sin(radAngle);

    Kernel2D kernel;
    kernel.width = kSize;
    kernel.height = kSize;
    kernel.anchorX = rad;
    kernel.anchorY = rad;
    kernel.weights.assign(static_cast<size_t>(kSize) * kSize, 0);

    int32_t count = 0;
    for (int32_t step = -rad; step <= rad; ++step) {
        int32_t kx = rad + static_cast<int32_t>(std::round(step * cosA));
        int32_t ky = rad + static_cast<int32_t>(std::round(step * sinA));
        kx = compat::clamp(kx, 0, kSize - 1);
        ky = compat::clamp(ky, 0, kSize - 1);
        size_t idx = static_cast<size_t>(ky) * kSize + kx;
        if (kernel.weights[idx] == 0) {
            kernel.weights[idx] = 1;
            count++;
        }
    }
    kernel.divisor = (count > 0) ? count : 1;

    Bitmap dst(bmp.GetWidth(), bmp.GetHeight());
    ConvolutionEngine::Convolve2D(bmp, dst, kernel, BorderMode::Replicate, options);
    std::memcpy(bmp.GetPixelSpanMut().data(), dst.GetPixelSpan().data(), static_cast<size_t>(bmp.GetWidth()) * bmp.GetHeight() * sizeof(uint32_t));
}

void ImageFilters::BilateralFilter(
    Bitmap& bmp,
    int32_t radius,
    float sigmaColor,
    float sigmaSpace,
    const ParallelOptions& options
) {
    if (radius < 1 || sigmaColor <= 0.0f || sigmaSpace <= 0.0f) {
        throw std::invalid_argument("ImageFilters::BilateralFilter: radius must be >= 1 and sigmas > 0");
    }

    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    if (width == 0 || height == 0) return;

    Bitmap srcCopy(bmp);
    const auto srcSpan = srcCopy.GetPixelSpan();
    const uint32_t* srcPixels = srcSpan.data();

    // 1. 預先建立空間權重表格
    int32_t kSize = 2 * radius + 1;
    std::vector<float> spaceWeights(static_cast<size_t>(kSize) * kSize);
    float twoSpaceSq = 2.0f * sigmaSpace * sigmaSpace;
    for (int32_t dy = -radius; dy <= radius; ++dy) {
        for (int32_t dx = -radius; dx <= radius; ++dx) {
            float distSq = static_cast<float>(dx * dx + dy * dy);
            spaceWeights[(dy + radius) * kSize + (dx + radius)] = std::exp(-distSq / twoSpaceSq);
        }
    }

    // 2. 預先建立強度差異 LUT (0~255)
    float colorLUT[256];
    float twoColorSq = 2.0f * sigmaColor * sigmaColor;
    for (int32_t diff = 0; diff < 256; ++diff) {
        colorLUT[diff] = std::exp(-static_cast<float>(diff * diff) / twoColorSq);
    }

    // 3. 執行保邊濾波
    PixelOps::ForEach(bmp, [=, &spaceWeights, &colorLUT](uint32_t x, uint32_t y, uint32_t& p) {
        int32_t ix = static_cast<int32_t>(x);
        int32_t iy = static_cast<int32_t>(y);

        uint32_t centerPixel = srcPixels[static_cast<size_t>(y) * width + x];
        int32_t cr = static_cast<int32_t>((centerPixel >> 16) & 0xFF);
        int32_t cg = static_cast<int32_t>((centerPixel >> 8) & 0xFF);
        int32_t cb = static_cast<int32_t>(centerPixel & 0xFF);
        int32_t cLum = (77 * cr + 150 * cg + 29 * cb + 128) >> 8;

        float sumR = 0.0f;
        float sumG = 0.0f;
        float sumB = 0.0f;
        float totalWeight = 0.0f;

        for (int32_t dy = -radius; dy <= radius; ++dy) {
            int32_t sy = compat::clamp(iy + dy, 0, static_cast<int32_t>(height) - 1);
            for (int32_t dx = -radius; dx <= radius; ++dx) {
                int32_t sx = compat::clamp(ix + dx, 0, static_cast<int32_t>(width) - 1);
                uint32_t np = srcPixels[static_cast<size_t>(sy) * width + sx];

                int32_t nr = static_cast<int32_t>((np >> 16) & 0xFF);
                int32_t ng = static_cast<int32_t>((np >> 8) & 0xFF);
                int32_t nb = static_cast<int32_t>(np & 0xFF);
                int32_t nLum = (77 * nr + 150 * ng + 29 * nb + 128) >> 8;

                int32_t lumDiff = std::abs(cLum - nLum);
                float w = spaceWeights[(dy + radius) * kSize + (dx + radius)] * colorLUT[lumDiff];

                sumR += nr * w;
                sumG += ng * w;
                sumB += nb * w;
                totalWeight += w;
            }
        }

        if (totalWeight > 0.0f) {
            uint8_t outR = static_cast<uint8_t>(compat::clamp(static_cast<int32_t>(std::round(sumR / totalWeight)), 0, 255));
            uint8_t outG = static_cast<uint8_t>(compat::clamp(static_cast<int32_t>(std::round(sumG / totalWeight)), 0, 255));
            uint8_t outB = static_cast<uint8_t>(compat::clamp(static_cast<int32_t>(std::round(sumB / totalWeight)), 0, 255));
            uint8_t a = static_cast<uint8_t>((p >> 24) & 0xFF);
            p = (static_cast<uint32_t>(a) << 24) | (outR << 16) | (outG << 8) | outB;
        }
    }, options);
}

// -----------------------------------------------------------------------------
// 4. 邊緣偵測 (Edge Detection)
// -----------------------------------------------------------------------------

void ImageFilters::DetectEdges(Bitmap& bmp, EdgeOperator op, const ParallelOptions& options) {
    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    if (width == 0 || height == 0) return;

    std::vector<uint8_t> gray(static_cast<size_t>(width) * height);
    auto srcSpan = bmp.GetPixelSpan();
    for (size_t i = 0; i < gray.size(); ++i) {
        gray[i] = LuminanceBT601(
            static_cast<uint8_t>((srcSpan[i] >> 16) & 0xFF),
            static_cast<uint8_t>((srcSpan[i] >> 8) & 0xFF),
            static_cast<uint8_t>(srcSpan[i] & 0xFF)
        );
    }

    auto getGray = [&](int32_t x, int32_t y) -> int32_t {
        x = compat::clamp(x, 0, static_cast<int32_t>(width) - 1);
        y = compat::clamp(y, 0, static_cast<int32_t>(height) - 1);
        return static_cast<int32_t>(gray[static_cast<size_t>(y) * width + x]);
    };

    if (op == EdgeOperator::Laplacian) {
        PixelOps::ForEach(bmp, [&](uint32_t x, uint32_t y, uint32_t& p) {
            int32_t ix = static_cast<int32_t>(x);
            int32_t iy = static_cast<int32_t>(y);
            int32_t val = getGray(ix, iy - 1) + getGray(ix - 1, iy) +
                          getGray(ix + 1, iy) + getGray(ix, iy + 1) -
                          4 * getGray(ix, iy);
            uint8_t mag = static_cast<uint8_t>(compat::clamp(std::abs(val), 0, 255));
            p = (0xFF << 24) | (mag << 16) | (mag << 8) | mag;
        }, options);
        return;
    }

    PixelOps::ForEach(bmp, [&](uint32_t x, uint32_t y, uint32_t& p) {
        int32_t ix = static_cast<int32_t>(x);
        int32_t iy = static_cast<int32_t>(y);

        int32_t p00 = getGray(ix - 1, iy - 1);
        int32_t p01 = getGray(ix,     iy - 1);
        int32_t p02 = getGray(ix + 1, iy - 1);
        int32_t p10 = getGray(ix - 1, iy);
        int32_t p12 = getGray(ix + 1, iy);
        int32_t p20 = getGray(ix - 1, iy + 1);
        int32_t p21 = getGray(ix,     iy + 1);
        int32_t p22 = getGray(ix + 1, iy + 1);

        int32_t gx = 0;
        int32_t gy = 0;

        if (op == EdgeOperator::Prewitt) {
            gx = (-p00 + p02) + (-p10 + p12) + (-p20 + p22);
            gy = (-p00 - p01 - p02) + (p20 + p21 + p22);
        } else if (op == EdgeOperator::Scharr) {
            gx = 3 * (-p00 + p02) + 10 * (-p10 + p12) + 3 * (-p20 + p22);
            gy = (-3 * p00 - 10 * p01 - 3 * p02) + (3 * p20 + 10 * p21 + 3 * p22);
            gx /= 2;
            gy /= 2;
        } else {
            gx = (-p00 + p02) + 2 * (-p10 + p12) + (-p20 + p22);
            gy = (-p00 - 2 * p01 - p02) + (p20 + 2 * p21 + p22);
        }

        int32_t mag = static_cast<int32_t>(std::round(std::sqrt(gx * gx + gy * gy)));
        uint8_t outM = static_cast<uint8_t>(compat::clamp(mag, 0, 255));
        p = (0xFF << 24) | (outM << 16) | (outM << 8) | outM;
    }, options);
}

void ImageFilters::Canny(
    Bitmap& bmp,
    uint8_t lowThreshold,
    uint8_t highThreshold,
    float gaussianSigma,
    const ParallelOptions& options
) {
    if (lowThreshold > highThreshold) {
        throw std::invalid_argument("ImageFilters::Canny: lowThreshold cannot be greater than highThreshold");
    }

    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    if (width == 0 || height == 0) return;

    // 1. 高斯濾波降噪
    if (gaussianSigma > 0.0f) {
        GaussianBlur(bmp, gaussianSigma, gaussianSigma, options, false);
    }

    // 2. 轉灰階
    std::vector<uint8_t> gray(static_cast<size_t>(width) * height);
    auto srcSpan = bmp.GetPixelSpan();
    for (size_t i = 0; i < gray.size(); ++i) {
        gray[i] = LuminanceBT601(
            static_cast<uint8_t>((srcSpan[i] >> 16) & 0xFF),
            static_cast<uint8_t>((srcSpan[i] >> 8) & 0xFF),
            static_cast<uint8_t>(srcSpan[i] & 0xFF)
        );
    }

    // 3. Sobel 梯度大小與方向
    std::vector<float> mag(static_cast<size_t>(width) * height, 0.0f);
    std::vector<uint8_t> sector(static_cast<size_t>(width) * height, 0);

    for (uint32_t y = 1; y + 1 < height; ++y) {
        for (uint32_t x = 1; x + 1 < width; ++x) {
            int32_t p00 = gray[(y - 1) * width + (x - 1)];
            int32_t p01 = gray[(y - 1) * width + x];
            int32_t p02 = gray[(y - 1) * width + (x + 1)];
            int32_t p10 = gray[y * width + (x - 1)];
            int32_t p12 = gray[y * width + (x + 1)];
            int32_t p20 = gray[(y + 1) * width + (x - 1)];
            int32_t p21 = gray[(y + 1) * width + x];
            int32_t p22 = gray[(y + 1) * width + (x + 1)];

            int32_t gx = (-p00 + p02) + 2 * (-p10 + p12) + (-p20 + p22);
            int32_t gy = (-p00 - 2 * p01 - p02) + (p20 + 2 * p21 + p22);

            float g = std::sqrt(static_cast<float>(gx * gx + gy * gy));
            size_t idx = static_cast<size_t>(y) * width + x;
            mag[idx] = g;

            // 梯度方向量化為 4 個主軸 (0, 45, 90, 135)
            float angle = std::atan2(static_cast<float>(gy), static_cast<float>(gx)) * 180.0f / 3.14159265358979323846f;
            if (angle < 0.0f) angle += 180.0f;

            if ((angle >= 0.0f && angle < 22.5f) || (angle >= 157.5f && angle <= 180.0f)) {
                sector[idx] = 0; // 水平 (0°)
            } else if (angle >= 22.5f && angle < 67.5f) {
                sector[idx] = 1; // 對角 (45°)
            } else if (angle >= 67.5f && angle < 112.5f) {
                sector[idx] = 2; // 垂直 (90°)
            } else {
                sector[idx] = 3; // 反對角 (135°)
            }
        }
    }

    // 4. 非極大值抑制 (Non-Maximum Suppression, NMS)
    std::vector<uint8_t> nms(static_cast<size_t>(width) * height, 0);

    for (uint32_t y = 1; y + 1 < height; ++y) {
        for (uint32_t x = 1; x + 1 < width; ++x) {
            size_t idx = static_cast<size_t>(y) * width + x;
            float m = mag[idx];
            if (m < static_cast<float>(lowThreshold)) continue;

            float m1 = 0.0f, m2 = 0.0f;
            switch (sector[idx]) {
                case 0: // 水平
                    m1 = mag[idx - 1];
                    m2 = mag[idx + 1];
                    break;
                case 1: // 45°
                    m1 = mag[(y - 1) * width + (x + 1)];
                    m2 = mag[(y + 1) * width + (x - 1)];
                    break;
                case 2: // 90°
                    m1 = mag[(y - 1) * width + x];
                    m2 = mag[(y + 1) * width + x];
                    break;
                case 3: // 135°
                    m1 = mag[(y - 1) * width + (x - 1)];
                    m2 = mag[(y + 1) * width + (x + 1)];
                    break;
            }

            if (m >= m1 && m >= m2) {
                nms[idx] = static_cast<uint8_t>(compat::clamp(static_cast<int32_t>(std::round(m)), 0, 255));
            }
        }
    }

    // 5. 雙門檻遲滯追蹤 (Hysteresis Thresholding)
    std::vector<uint8_t> edges(static_cast<size_t>(width) * height, 0);
    std::queue<size_t> strongQueue;

    for (size_t i = 0; i < nms.size(); ++i) {
        if (nms[i] >= highThreshold) {
            edges[i] = 255;
            strongQueue.push(i);
        }
    }

    const int32_t dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    const int32_t dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

    while (!strongQueue.empty()) {
        size_t cur = strongQueue.front();
        strongQueue.pop();

        int32_t cx = static_cast<int32_t>(cur % width);
        int32_t cy = static_cast<int32_t>(cur / width);

        for (int32_t d = 0; d < 8; ++d) {
            int32_t nx = cx + dx[d];
            int32_t ny = cy + dy[d];
            if (nx >= 0 && nx < static_cast<int32_t>(width) && ny >= 0 && ny < static_cast<int32_t>(height)) {
                size_t nIdx = static_cast<size_t>(ny) * width + nx;
                if (edges[nIdx] == 0 && nms[nIdx] >= lowThreshold) {
                    edges[nIdx] = 255;
                    strongQueue.push(nIdx);
                }
            }
        }
    }

    // 寫回目標 Bitmap
    PixelOps::ForEach(bmp, [&edges, width](uint32_t x, uint32_t y, uint32_t& p) {
        uint8_t val = edges[static_cast<size_t>(y) * width + x];
        p = (0xFF << 24) | (val << 16) | (val << 8) | val;
    }, options);
}

} // namespace graphics
} // namespace cppui
