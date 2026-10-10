# ImageFilters 類別 (cppui::graphics::ImageFilters)

`ImageFilters` 類別定義於 `include/ui/graphics/ImageFilters.hpp`，提供全方位的現代高階影像處理演算法套件。包含色彩空間轉換（灰階）、全域與局部二值化、六大模糊平滑濾鏡、一階/二階邊緣偵測與 Canny 邊緣偵測演算法。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/ImageFilters.hpp"`
* **實作檔**：`src/graphics/ImageFilters.cpp`
* **設計準則**：
  - 核心演算法均為純函數或靜態方法。
  - 同時提供「就地修改 (In-place)」與「回傳新副本 (Pure Function)」API。
  - 所有演算法均接受 [`ParallelOptions`](graphics_paralleloptions.md) 平行化與視窗分塊設定。

---

## 2. 相關列舉型別 (Enums)

### 2.1 灰階轉換模式 (`GrayscaleMethod`)
```cpp
enum class GrayscaleMethod : uint8_t {
    BT601 = 0,     // ITU-R BT.601 標準: (77R + 150G + 29B + 128) >> 8
    BT709 = 1,     // ITU-R BT.709 標準: (54R + 183G + 19B + 128) >> 8
    Average = 2,   // 算術平均法: (R + G + B) / 3
    Lightness = 3  // 去飽和明度法: (max(R,G,B) + min(R,G,B)) / 2
};
```

### 2.2 二值化門檻模式 (`ThresholdType`)
```cpp
enum class ThresholdType : uint8_t {
    Binary = 0,         // v >= T ? 255 : 0
    BinaryInverted = 1, // v >= T ? 0 : 255
    Truncate = 2,       // v > T ? T : v
    ToZero = 3          // v >= T ? v : 0
};
```

### 2.3 邊緣偵測算子 (`EdgeOperator`)
```cpp
enum class EdgeOperator : uint8_t {
    Sobel = 0,     // Sobel 算子
    Prewitt = 1,   // Prewitt 算子
    Scharr = 2,    // Scharr 算子 (具備更高旋轉對稱性)
    Laplacian = 3  // Laplacian 二階微分算子
};
```

---

## 3. 核心 API 方法 (API Reference)

### 3.1 灰階轉換 (Grayscale)
```cpp
// 就地修改
static void Grayscale(Bitmap& bmp, GrayscaleMethod method = GrayscaleMethod::BT601, const ParallelOptions& options = ParallelOptions::Sequential());

// 純函數回傳新 Bitmap
COMPAT_NODISCARD static std::shared_ptr<Bitmap> ToGrayscale(const Bitmap& src, GrayscaleMethod method = GrayscaleMethod::BT601, const ParallelOptions& options = ParallelOptions::Sequential());
```

### 3.2 二值化 (Thresholding & Binarization)
```cpp
// 1. 全域常數閥值二值化
static void Threshold(Bitmap& bmp, uint8_t threshold, ThresholdType type = ThresholdType::Binary, const ParallelOptions& options = ParallelOptions::Sequential());

// 2. 大津法自動最佳閥值計算 (Otsu's Method)
COMPAT_NODISCARD static uint8_t ComputeOtsuThreshold(const Bitmap& bmp);

// 3. 大津法自動二值化
static void OtsuThreshold(Bitmap& bmp, ThresholdType type = ThresholdType::Binary, const ParallelOptions& options = ParallelOptions::Sequential());

// 4. 局部自適應二值化 (Adaptive Thresholding)
static void AdaptiveThreshold(Bitmap& bmp, int32_t blockSize, int32_t c, bool useGaussian = false, const ParallelOptions& options = ParallelOptions::Sequential());
```
* **AdaptiveThreshold 特性**：內部利用 **2D 積分圖 (Integral Image)** 達成不論 `blockSize` 多大皆以 $O(1)$ 常數時間完成視窗均值計算，徹底消除大視窗下的效能劣勢。

### 3.3 六大模糊平滑濾鏡 (Blur Filters)

| 方法名稱 | 參數簽名 | 演算法特點與用途 |
| :--- | :--- | :--- |
| **`BoxBlur`** | `(Bitmap& bmp, int32_t radius, const ParallelOptions& options = ParallelOptions::Sequential())` | 方框模糊。內部以可分離 1D 核心快速計算，邊界採 `Replicate` 填充。 |
| **`MeanBlur`** | `(Bitmap& bmp, int32_t kernelSize, const ParallelOptions& options = ParallelOptions::Sequential())` | 均值模糊。`kernelSize` 必須為大於等於 1 之奇數。 |
| **`GaussianBlur`** | `(Bitmap& bmp, float sigmaX, float sigmaY = 0.0f, const ParallelOptions& options = ParallelOptions::Sequential(), bool preferSkia = true)` | 高斯平滑。若 `preferSkia = true` 且系統啟用 Skia，自動透過 `SkiaImageBridge` 進行硬體/SIMD 向量加速；否則無縫回退至 CPU 定點數卷積引擎。 |
| **`MedianBlur`** | `(Bitmap& bmp, int32_t kernelSize = 3, const ParallelOptions& options = ParallelOptions::Sequential())` | 中值模糊。$3 \times 3$ 內部採用 9 元素無分支排序網路 (Branchless Sorting Network)，能以極低時鐘週期完全去除胡椒鹽極值噪點。 |
| **`MotionBlur`** | `(Bitmap& bmp, int32_t length, float angleDegrees, const ParallelOptions& options = ParallelOptions::Sequential())` | 動態模糊。模擬沿 `angleDegrees` 角度與長度 `length` 的定向運動拖影。 |
| **`BilateralFilter`** | `(Bitmap& bmp, int32_t radius, float sigmaColor, float sigmaSpace, const ParallelOptions& options = ParallelOptions::Sequential())` | 雙邊濾波。結合空間歐氏距離高斯權重與 256-entry 色彩差異 LUT，平滑內部紋理的同時絕對保留高對比邊界。 |

### 3.4 邊緣偵測 (Edge Detection)
```cpp
// 1. 一階/二階微分梯度運算
static void DetectEdges(Bitmap& bmp, EdgeOperator op = EdgeOperator::Sobel, const ParallelOptions& options = ParallelOptions::Sequential());

// 2. Canny 多階段邊緣偵測
static void Canny(Bitmap& bmp, uint8_t lowThreshold, uint8_t highThreshold, float gaussianSigma = 1.0f, const ParallelOptions& options = ParallelOptions::Sequential());
```
* **Canny 演算法實作流程**：
  1. 高斯平滑前置降噪（避免細微紋理誤判為邊界）。
  2. 轉灰階並以 Sobel 算子計算梯度強度 $M$ 與方向 $\theta$。
  3. 將 $\theta$ 量化為 4 個主軸方向（0°、45°、90°、135°）。
  4. **非極大值抑制 (NMS)**：沿梯度垂直法向量方向比較相鄰像素，將邊界細化為單像素寬度。
  5. **雙門檻遲滯追蹤 (Hysteresis Tracking)**：強邊界 ($M \ge \text{highThreshold}$) 標註為 255；弱邊界 ($\text{lowThreshold} \le M < \text{highThreshold}$) 僅在 8-連通鄰域有強邊界連通時保留，其餘噪點壓制為 0。

---

## 4. 範例程式碼 (Usage Examples)

```cpp
#include "ui/graphics/ImageFilters.hpp"
#include "ui/graphics/ParallelOptions.hpp"

using namespace cppui::graphics;

Bitmap photo(1280, 720);
// 載入影像後...

// 1. 多執行緒大津法自動二值化
ParallelOptions parOpts = ParallelOptions::Parallel(4, 32);
ImageFilters::OtsuThreshold(photo, ThresholdType::Binary, parOpts);

// 2. 邊緣偵測與 Canny 細化
ImageFilters::Canny(photo, 40, 100, 1.0f, parOpts);

// 3. 雙邊保邊濾波
ImageFilters::BilateralFilter(photo, 3, 25.0f, 3.0f, parOpts);
```
