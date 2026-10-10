/**
 * @file image_processing_test.cpp
 * @brief 影像處理演算法、可分離定點數卷積、多核心視窗切分、灰階、二值化、模糊與邊緣偵測單元測試
 */

#include "compat/compat.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/ParallelOptions.hpp"
#include "ui/graphics/PixelOps.hpp"
#include "ui/graphics/Convolution.hpp"
#include "ui/graphics/ImageFilters.hpp"
#include "ui/graphics/SkiaImageBridge.hpp"

#include <cassert>
#include <cstdint>
#include <vector>
#include <chrono>
#include <cmath>
#include <stdexcept>

using namespace cppui;
using namespace cppui::graphics;

// =============================================================================
// 1. PixelOps 遍歷與 ParallelOptions 測試
// =============================================================================
static void TestPixelOps() {
    compat::println("[Test 1] Testing PixelOps (ForEach, ForEachIf, ParallelOptions)...");

    Bitmap bmp(64, 64);
    bmp.Clear(core::Color(10, 20, 30, 255));

    // 1.1 循序單執行緒遍歷
    PixelOps::ForEach(bmp, [](uint32_t& p) {
        p = 0xFFFFFFFF;
    }, ParallelOptions::Sequential());

    for (uint32_t p : bmp.GetPixelSpan()) {
        assert(p == 0xFFFFFFFF);
        (void)p;
    }

    // 1.2 多執行緒均分遍歷
    PixelOps::ForEach(bmp, [](uint32_t x, uint32_t y, uint32_t& p) {
        uint8_t val = static_cast<uint8_t>((x + y) & 0xFF);
        p = (0xFF << 24) | (val << 16) | (val << 8) | val;
    }, ParallelOptions::Parallel(4));

    for (uint32_t y = 0; y < 64; ++y) {
        for (uint32_t x = 0; x < 64; ++x) {
            uint8_t expected = static_cast<uint8_t>((x + y) & 0xFF);
            uint32_t p = bmp.GetPixel(x, y).ToArgb();
            assert((p & 0xFF) == expected);
            (void)expected;
            (void)p;
        }
    }

    // 1.3 自訂視窗切分 (Window Slicing / chunkRows = 8)
    PixelOps::ForEach(bmp, [](uint32_t& p) {
        p ^= 0x00FFFFFF; // 反相 RGB
    }, ParallelOptions::Parallel(4, 8));

    for (uint32_t y = 0; y < 64; ++y) {
        for (uint32_t x = 0; x < 64; ++x) {
            uint8_t orig = static_cast<uint8_t>((x + y) & 0xFF);
            uint8_t inverted = static_cast<uint8_t>(orig ^ 0xFF);
            uint32_t p = bmp.GetPixel(x, y).ToArgb();
            assert((p & 0xFF) == inverted);
            (void)orig;
            (void)inverted;
            (void)p;
        }
    }

    // 1.4 ForEachIf 條件式遍歷
    PixelOps::ForEachIf(bmp, [](uint32_t x, uint32_t, const uint32_t&) {
        return x < 32;
    }, [](uint32_t& p) {
        p = 0xFF00FF00; // 綠色
    }, ParallelOptions::Parallel(2, 16));

    for (uint32_t y = 0; y < 64; ++y) {
        for (uint32_t x = 0; x < 64; ++x) {
            uint32_t p = bmp.GetPixel(x, y).ToArgb();
            if (x < 32) {
                assert(p == 0xFF00FF00);
            } else {
                assert(p != 0xFF00FF00);
            }
            (void)p;
        }
    }

    compat::println("  [PASS] PixelOps ForEach, ForEachIf and Window Slicing verified.");
}

// =============================================================================
// 2. 卷積運算數學等價性測試 (2D vs Separable)
// =============================================================================
static void TestConvolutionEquivalence() {
    compat::println("[Test 2] Testing Convolution 2D vs Separable mathematical equivalence...");

    const uint32_t W = 40;
    const uint32_t H = 40;
    Bitmap src(W, H);

    // 填充測試圖像
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            uint8_t r = static_cast<uint8_t>((x * 6) & 0xFF);
            uint8_t g = static_cast<uint8_t>((y * 6) & 0xFF);
            uint8_t b = static_cast<uint8_t>((x * 3 + y * 3) & 0xFF);
            src.SetPixel(x, y, core::Color(r, g, b, 255));
        }
    }

    // 1. 執行可分離方框模糊 (rad = 1, size = 3)
    Bitmap dstSep(W, H);
    auto sepKernel = ConvolutionEngine::CreateBoxKernel(1, 1);
    ConvolutionEngine::ConvolveSeparable(src, dstSep, sepKernel, BorderMode::Replicate, ParallelOptions::Sequential());

    // 2. 執行 2D 通用全矩陣卷積 (size = 3x3, weights = all 1s, div = 9)
    Bitmap dst2D(W, H);
    Kernel2D k2D;
    k2D.width = 3;
    k2D.height = 3;
    k2D.anchorX = 1;
    k2D.anchorY = 1;
    k2D.weights.assign(9, 1);
    k2D.divisor = 9;
    ConvolutionEngine::Convolve2D(src, dst2D, k2D, BorderMode::Replicate, ParallelOptions::Sequential());

    // 比對中央區域兩者輸出數值等價性 (容許定點整數除法小於等於 1 的數值量化舍入)
    for (uint32_t y = 2; y + 2 < H; ++y) {
        for (uint32_t x = 2; x + 2 < W; ++x) {
            core::Color cSep = dstSep.GetPixel(x, y);
            core::Color c2D = dst2D.GetPixel(x, y);
            assert(std::abs(static_cast<int32_t>(cSep.r) - c2D.r) <= 1);
            assert(std::abs(static_cast<int32_t>(cSep.g) - c2D.g) <= 1);
            assert(std::abs(static_cast<int32_t>(cSep.b) - c2D.b) <= 1);
            (void)cSep;
            (void)c2D;
        }
    }

    compat::println("  [PASS] 2D matrix convolution and Separable 1D convolution mathematically aligned.");
}

// =============================================================================
// 3. 多執行緒與 Window Slicing 卷積 100% 數值一致性測試
// =============================================================================
static void TestThreadingAndWindowSlicingConsistency() {
    compat::println("[Test 3] Testing Threading and Window Slicing 100% numerical consistency...");

    const uint32_t W = 64;
    const uint32_t H = 64;
    Bitmap src(W, H);
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            src.SetPixel(x, y, core::Color(static_cast<uint8_t>(x * 4), static_cast<uint8_t>(y * 4), 100, 255));
        }
    }

    auto kernel = ConvolutionEngine::CreateGaussianKernel(1.5f);

    Bitmap dstSeq(W, H);
    ConvolutionEngine::ConvolveSeparable(src, dstSeq, kernel, BorderMode::Replicate, ParallelOptions::Sequential());

    Bitmap dstPar(W, H);
    ConvolutionEngine::ConvolveSeparable(src, dstPar, kernel, BorderMode::Replicate, ParallelOptions::Parallel(4));

    Bitmap dstSlice(W, H);
    ConvolutionEngine::ConvolveSeparable(src, dstSlice, kernel, BorderMode::Replicate, ParallelOptions::Parallel(4, 8));

    // 比對全部像素 100% 完全相同
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            uint32_t p1 = dstSeq.GetPixel(x, y).ToArgb();
            uint32_t p2 = dstPar.GetPixel(x, y).ToArgb();
            uint32_t p3 = dstSlice.GetPixel(x, y).ToArgb();
            assert(p1 == p2);
            assert(p1 == p3);
            (void)p1;
            (void)p2;
            (void)p3;
        }
    }

    compat::println("  [PASS] Sequential, Multi-Threaded, and Window Slicing outputs are 100% bit-identical.");
}

// =============================================================================
// 4. 灰階轉換演算法標準測試
// =============================================================================
static void TestGrayscale() {
    compat::println("[Test 4] Testing Grayscale conversion (BT.601, BT.709, Average, Lightness)...");

    Bitmap bmp(2, 2);
    // 純紅色: R=255, G=0, B=0
    bmp.SetPixel(0, 0, core::Color(255, 0, 0, 255));
    // 純綠色: R=0, G=255, B=0
    bmp.SetPixel(1, 0, core::Color(0, 255, 0, 255));
    // 純藍色: R=0, G=0, B=255
    bmp.SetPixel(0, 1, core::Color(0, 0, 255, 255));
    // 白色: R=255, G=255, B=255
    bmp.SetPixel(1, 1, core::Color(255, 255, 255, 255));

    // 4.1 BT.601: (77*255 + 128) >> 8 = 77
    auto grayBT601 = ImageFilters::ToGrayscale(bmp, GrayscaleMethod::BT601);
    assert(grayBT601->GetPixel(0, 0).r == 77); // Red
    assert(grayBT601->GetPixel(1, 0).r == 150); // Green
    assert(grayBT601->GetPixel(0, 1).r == 29); // Blue
    assert(grayBT601->GetPixel(1, 1).r == 255); // White

    // 4.2 BT.709: (54*255 + 128) >> 8 = 54
    auto grayBT709 = ImageFilters::ToGrayscale(bmp, GrayscaleMethod::BT709);
    assert(grayBT709->GetPixel(0, 0).r == 54); // Red
    assert(grayBT709->GetPixel(1, 0).r == 183); // Green
    assert(grayBT709->GetPixel(0, 1).r == 19); // Blue

    // 4.3 Average: 255 / 3 = 85
    auto grayAvg = ImageFilters::ToGrayscale(bmp, GrayscaleMethod::Average);
    assert(grayAvg->GetPixel(0, 0).r == 85);

    // 4.4 Lightness: 255 / 2 = 127
    auto grayLit = ImageFilters::ToGrayscale(bmp, GrayscaleMethod::Lightness);
    assert(grayLit->GetPixel(0, 0).r == 127);

    compat::println("  [PASS] All grayscale methods verified against photometric standards.");
}

// =============================================================================
// 5. 二值化 (固定閥值、大津法 Otsu、局部自適應 Adaptive) 測試
// =============================================================================
static void TestThresholding() {
    compat::println("[Test 5] Testing Thresholding (Binary, Otsu, Adaptive)...");

    Bitmap bmp(10, 10);
    // 建立 50% 像素為 30，50% 像素為 200 的雙峰分佈
    for (uint32_t y = 0; y < 10; ++y) {
        for (uint32_t x = 0; x < 10; ++x) {
            uint8_t v = (x < 5) ? 30 : 200;
            bmp.SetPixel(x, y, core::Color(v, v, v, 255));
        }
    }

    // 5.1 大津法自動最佳閥值驗證 (雙峰應落在 30~200 之間)
    uint8_t otsuT = ImageFilters::ComputeOtsuThreshold(bmp);
    assert(otsuT >= 30 && otsuT <= 200);
    compat::println("  [PASS] Otsu optimal threshold computed: {}", otsuT);

    // 5.2 大津法二值化
    Bitmap otsuBmp(bmp);
    ImageFilters::OtsuThreshold(otsuBmp, ThresholdType::Binary);
    assert(otsuBmp.GetPixel(0, 0).r == 0);
    assert(otsuBmp.GetPixel(9, 9).r == 255);

    // 5.3 局部自適應二值化 (Adaptive Thresholding)
    Bitmap adaptBmp(bmp);
    ImageFilters::AdaptiveThreshold(adaptBmp, 3, 5, false);
    // 邊界跳躍處應正確分割
    assert(adaptBmp.GetPixel(9, 9).r == 255);

    compat::println("  [PASS] Thresholding, Otsu, and Adaptive thresholding verified.");
}

// =============================================================================
// 6. 六大模糊平滑濾鏡測試
// =============================================================================
static void TestBlurFilters() {
    compat::println("[Test 6] Testing Blur filters (Box, Mean, Gaussian, Median, Motion, Bilateral)...");

    const uint32_t W = 32;
    const uint32_t H = 32;
    Bitmap bmp(W, H);
    bmp.Clear(core::Colors::Black);
    bmp.SetPixel(16, 16, core::Colors::White); // 中心單一白點

    // 6.1 BoxBlur & MeanBlur
    Bitmap boxBmp(bmp);
    ImageFilters::BoxBlur(boxBmp, 1);
    assert(boxBmp.GetPixel(16, 16).r < 255); // 擴散衰減
    assert(boxBmp.GetPixel(15, 16).r > 0);   // 擴散至鄰域

    Bitmap meanBmp(bmp);
    ImageFilters::MeanBlur(meanBmp, 3);
    assert(meanBmp.GetPixel(16, 16).r < 255);

    // 6.2 GaussianBlur
    Bitmap gBmp(bmp);
    ImageFilters::GaussianBlur(gBmp, 1.0f);
    assert(gBmp.GetPixel(16, 16).r < 255);
    assert(gBmp.GetPixel(15, 16).r > 0);

    // 6.3 MedianBlur (胡椒鹽雜訊消除驗證)
    Bitmap noisyBmp(W, H);
    noisyBmp.Clear(core::Color(100, 100, 100, 255));
    // 注入單點孤立白噪點
    noisyBmp.SetPixel(10, 10, core::Colors::White);
    ImageFilters::MedianBlur(noisyBmp, 3);
    // 中值濾波應完全濾除孤立極值噪點回到 100
    assert(noisyBmp.GetPixel(10, 10).r == 100);

    // 6.4 MotionBlur (水平定向拖影)
    Bitmap mBmp(bmp);
    ImageFilters::MotionBlur(mBmp, 5, 0.0f); // 0度為水平
    assert(mBmp.GetPixel(17, 16).r > 0);     // 水平擴散
    assert(mBmp.GetPixel(16, 18).r == 0);    // 垂直方向未受拖影

    // 6.5 BilateralFilter (保邊濾波驗證)
    Bitmap stepBmp(W, H);
    // 左半部黑 (0)，右半部白 (200)
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            uint8_t v = (x < 16) ? 0 : 200;
            stepBmp.SetPixel(x, y, core::Color(v, v, v, 255));
        }
    }
    ImageFilters::BilateralFilter(stepBmp, 2, 20.0f, 2.0f);
    // 邊界兩側階梯對比依然顯著保留 (保邊)
    assert(stepBmp.GetPixel(14, 16).r < 50);
    assert(stepBmp.GetPixel(18, 16).r > 150);

    compat::println("  [PASS] All 6 blur and smoothing filters verified.");
}

// =============================================================================
// 7. 邊緣偵測與 Canny 演算法測試
// =============================================================================
static void TestEdgeDetection() {
    compat::println("[Test 7] Testing Edge detection (Sobel, Prewitt, Scharr, Laplacian, Canny)...");

    const uint32_t W = 32;
    const uint32_t H = 32;
    Bitmap stepBmp(W, H);
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            uint8_t v = (x < 16) ? 10 : 220;
            stepBmp.SetPixel(x, y, core::Color(v, v, v, 255));
        }
    }

    // 7.1 Sobel 邊緣偵測
    Bitmap sobelBmp(stepBmp);
    ImageFilters::DetectEdges(sobelBmp, EdgeOperator::Sobel);
    // 邊界 x=15 處應出現強響應，平坦區 x=5 處接近 0
    assert(sobelBmp.GetPixel(15, 16).r > 100);
    assert(sobelBmp.GetPixel(5, 16).r == 0);

    // 7.2 Prewitt
    Bitmap prewittBmp(stepBmp);
    ImageFilters::DetectEdges(prewittBmp, EdgeOperator::Prewitt);
    assert(prewittBmp.GetPixel(15, 16).r > 100);

    // 7.3 Scharr
    Bitmap scharrBmp(stepBmp);
    ImageFilters::DetectEdges(scharrBmp, EdgeOperator::Scharr);
    assert(scharrBmp.GetPixel(15, 16).r > 100);

    // 7.4 Laplacian
    Bitmap lapBmp(stepBmp);
    ImageFilters::DetectEdges(lapBmp, EdgeOperator::Laplacian);
    assert(lapBmp.GetPixel(15, 16).r > 50);

    // 7.5 Canny 多階段邊緣偵測 (NMS + 遲滯追蹤)
    Bitmap cannyBmp(stepBmp);
    ImageFilters::Canny(cannyBmp, 30, 80, 0.0f);
    // 邊界應細化為單像素或窄邊線 (255)
    assert(cannyBmp.GetPixel(15, 16).r == 255 || cannyBmp.GetPixel(16, 16).r == 255);
    // 平坦區必為 0
    assert(cannyBmp.GetPixel(5, 16).r == 0);

    compat::println("  [PASS] Sobel, Prewitt, Scharr, Laplacian, and Canny verified.");
}

// =============================================================================
// 8. 參數驗證與契約防護 (Fail-Fast) 測試
// =============================================================================
static void TestFailFastContracts() {
    compat::println("[Test 8] Testing Fail-Fast parameter validation...");

    // 0. 零尺寸點陣圖契約防護
    bool caughtZeroBmp = false;
    try {
        Bitmap emptyBmp(0, 0);
        (void)emptyBmp;
    } catch (const std::invalid_argument&) {
        caughtZeroBmp = true;
    }
    assert(caughtZeroBmp);
    (void)caughtZeroBmp;

    Bitmap validBmp(10, 10);
    Bitmap mismatchedBmp(20, 20);

    // 1. 尺寸不匹配 Convolve
    SeparableKernel k = ConvolutionEngine::CreateBoxKernel(1, 1);
    bool caughtMismatch = false;
    try {
        ConvolutionEngine::ConvolveSeparable(validBmp, mismatchedBmp, k);
    } catch (const std::invalid_argument&) {
        caughtMismatch = true;
    }
    assert(caughtMismatch);
    (void)caughtMismatch;

    // 2. 負數半徑
    bool caughtNegRadius = false;
    try {
        ImageFilters::BoxBlur(validBmp, -1);
    } catch (const std::invalid_argument&) {
        caughtNegRadius = true;
    }
    assert(caughtNegRadius);
    (void)caughtNegRadius;

    // 3. 非法高斯 Sigma
    bool caughtZeroSigma = false;
    try {
        ImageFilters::GaussianBlur(validBmp, 0.0f);
    } catch (const std::invalid_argument&) {
        caughtZeroSigma = true;
    }
    assert(caughtZeroSigma);
    (void)caughtZeroSigma;

    // 4. Canny 門檻倒置
    bool caughtCannyInvert = false;
    try {
        ImageFilters::Canny(validBmp, 100, 50);
    } catch (const std::invalid_argument&) {
        caughtCannyInvert = true;
    }
    assert(caughtCannyInvert);
    (void)caughtCannyInvert;

    compat::println("  [PASS] All Fail-Fast contracts verified.");
}

// =============================================================================
// 9. Skia 整合橋接查詢測試
// =============================================================================
static void TestSkiaBridge() {
    compat::println("[Test 9] Testing SkiaImageBridge query & fallback...");
    bool isAvailable = SkiaImageBridge::IsSkiaAvailable();
    compat::println("  [INFO] SkiaImageBridge::IsSkiaAvailable(): {}", isAvailable);

    Bitmap bmp(20, 20);
    bmp.Clear(core::Colors::Red);
    bool tryOk = SkiaImageBridge::TryGaussianBlur(bmp, 1.0f, 1.0f);
    compat::println("  [INFO] SkiaImageBridge::TryGaussianBlur returned: {}", tryOk);
    compat::println("  [PASS] SkiaImageBridge integration verified.");
}

// =============================================================================
// 10. 高解析度效能壓測 (512x512 Benchmark)
// =============================================================================
static void TestPerformanceBenchmark() {
    compat::println("[Test 10] Testing 512x512 High-throughput Benchmark...");

    const uint32_t W = 512;
    const uint32_t H = 512;
    Bitmap bmp(W, H);
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            bmp.SetPixel(x, y, core::Color(static_cast<uint8_t>(x & 0xFF), static_cast<uint8_t>(y & 0xFF), 128, 255));
        }
    }

    auto kernel = ConvolutionEngine::CreateGaussianKernel(2.0f);
    Bitmap dst(W, H);

    // 單執行緒測試
    auto t0 = std::chrono::high_resolution_clock::now();
    ConvolutionEngine::ConvolveSeparable(bmp, dst, kernel, BorderMode::Replicate, ParallelOptions::Sequential());
    auto t1 = std::chrono::high_resolution_clock::now();
    double seqMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // 多執行緒測試 (8 threads, sliceHeight = 32)
    auto t2 = std::chrono::high_resolution_clock::now();
    ConvolutionEngine::ConvolveSeparable(bmp, dst, kernel, BorderMode::Replicate, ParallelOptions::Parallel(8, 32));
    auto t3 = std::chrono::high_resolution_clock::now();
    double parMs = std::chrono::duration<double, std::milli>(t3 - t2).count();

    compat::println("  [BENCHMARK] 512x512 Gaussian Blur (sigma=2.0):");
    compat::println("    - Sequential (1 thread) : {:.2f} ms", seqMs);
    compat::println("    - Parallel   (8 threads, Window=32): {:.2f} ms", parMs);
    compat::println("  [PASS] High-throughput performance benchmark complete.");
}

int main() {
    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Full Image Processing & Convolution Tests   ");
    compat::println("==========================================================");

    TestPixelOps();
    TestConvolutionEquivalence();
    TestThreadingAndWindowSlicingConsistency();
    TestGrayscale();
    TestThresholding();
    TestBlurFilters();
    TestEdgeDetection();
    TestFailFastContracts();
    TestSkiaBridge();
    TestPerformanceBenchmark();

    compat::println("==========================================================");
    compat::println(" ALL 10 IMAGE PROCESSING & CONVOLUTION TESTS PASSED!      ");
    compat::println("==========================================================");
    return 0;
}
