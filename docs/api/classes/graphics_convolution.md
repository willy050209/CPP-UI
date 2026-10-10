# ConvolutionEngine 類別 (cppui::graphics::ConvolutionEngine)

`ConvolutionEngine` 定義於 `include/ui/graphics/Convolution.hpp`，是高輸送量影像空間濾波的核心卷積運算引擎。它融合演算法維度拆解（可分離卷積）、算術定點數化、記憶體 Row-major 快取優化與三區無分支邊界分塊，並受控於 [`ParallelOptions`](graphics_paralleloptions.md)。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/Convolution.hpp"`
* **實作檔**：`src/graphics/Convolution.cpp`
* **演算法複雜度**：
  - 通用 2D 卷積：$O(W \times H \times K_w \times K_h)$
  - 可分離卷積：$O(W \times H \times (K_w + K_h))$

---

## 2. 相關資料結構 (Structures & Enums)

### 2.1 邊界處理模式 (`BorderMode`)
```cpp
enum class BorderMode : uint8_t {
    Replicate = 0, // 重複邊緣像素 (Clamp 至邊界，最不易產生黑色暗邊或異常白邊)
    Reflect   = 1, // 鏡射邊緣像素 (Mirror)
    Constant  = 2  // 常數邊緣像素 (預設全透明/黑色)
};
```

### 2.2 1D 定點數可分離卷積核 (`SeparableKernel`)
```cpp
struct SeparableKernel {
    std::vector<int32_t> hWeights; // 水平定點數整數權重
    std::vector<int32_t> vWeights; // 垂直定點數整數權重
    int32_t hDivisor{ 1 };         // 水平標準化除數 (必須 > 0)
    int32_t vDivisor{ 1 };         // 垂直標準化除數 (必須 > 0)
    int32_t radiusX{ 0 };          // 水平半徑 (核長度為 2 * radiusX + 1)
    int32_t radiusY{ 0 };          // 垂直半徑 (核長度為 2 * radiusY + 1)
};
```

### 2.3 2D 通用卷積核 (`Kernel2D`)
```cpp
struct Kernel2D {
    int32_t width{ 0 };            // 核寬度
    int32_t height{ 0 };           // 核高度
    int32_t anchorX{ 0 };          // 錨點 X 座標 (預設 width / 2)
    int32_t anchorY{ 0 };          // 錨點 Y 座標 (預設 height / 2)
    std::vector<int32_t> weights;  // 權重陣列 (大小為 width * height)
    int32_t divisor{ 1 };          // 標準化除數 (必須 != 0)
    int32_t offset{ 0 };           // 偏置常數 (加至結果)
};
```

---

## 3. 核心 API 方法 (API Reference)

### 3.1 `ConvolveSeparable`
```cpp
static void ConvolveSeparable(
    const Bitmap& src,
    Bitmap& dst,
    const SeparableKernel& kernel,
    BorderMode borderMode = BorderMode::Replicate,
    const ParallelOptions& options = ParallelOptions::Sequential()
);
```
* **功能**：執行兩階段 1D 可分離二維卷積（水平 1D Pass 輸出至臨時快取，再由垂直 1D Pass 輸出至目標圖面）。
* **參數**：
  - `src`：來源唯讀點陣圖。
  - `dst`：目標可寫點陣圖（尺寸必須與來源一致）。
  - `kernel`：可分離定點數卷積核。
  - `borderMode`：邊界填充策略。
  - `options`：平行排程與 Window Slicing 設定。
* **例外規格**：若尺寸不相符、核權重為空或除數為非正整數，擲出 `std::invalid_argument`。

### 3.2 `Convolve2D`
```cpp
static void Convolve2D(
    const Bitmap& src,
    Bitmap& dst,
    const Kernel2D& kernel,
    BorderMode borderMode = BorderMode::Replicate,
    const ParallelOptions& options = ParallelOptions::Sequential()
);
```
* **功能**：執行非分離通用二維矩陣卷積。
* **內部架構**：採用三區無分支邊界分塊（Three-Zone Scanning），中央區域完全消除 `if` 邊界分支，以純指標步進驅動定點數乘加。

### 3.3 核建構工廠方法 (Kernel Factories)

| 方法名稱 | 參數 | 說明 |
| :--- | :--- | :--- |
| `CreateGaussianKernel(sigmaX, sigmaY)` | `float sigmaX, sigmaY = 0.0f` | 建構標準高斯平滑 1D 定點數可分離核，自動以 4096 精度正規化為整數。 |
| `CreateBoxKernel(radiusX, radiusY)` | `int32_t radiusX, radiusY = -1` | 建構方框模糊（Box Blur）1D 可分離核。 |
| `CreateSobelKernelX()` | 無 | 建構 Sobel 水平方向梯度可分離核 ($h=[-1,0,1], v=[1,2,1]$)。 |
| `CreateSobelKernelY()` | 無 | 建構 Sobel 垂直方向梯度可分離核 ($h=[1,2,1], v=[-1,0,1]$)。 |
| `CreatePrewittKernelX()` | 無 | 建構 Prewitt 水平梯度可分離核。 |
| `CreatePrewittKernelY()` | 無 | 建構 Prewitt 垂直梯度可分離核。 |

---

## 4. 效能優化技術剖析

1. **定點數整數運算 (Fixed-Point Arithmetic)**：
   - 全程使用 32 位元有號整數累計 A, R, G, B 4 個通道，最後透過定點除法與 `compat::clamp` 封裝，完全消除在迴圈中將 8-bit 色彩解包轉換為 `float` 以及寫回時浮點轉整數的巨大延遲。
2. **三區無分支邊界分塊 (Three-Zone Scanning)**：
   - 每一列掃描線切割為 $[0, \text{rad})$、$[\text{rad}, W - \text{rad})$ 與 $[W - \text{rad}, W)$。
   - 中央核心區無任何邊界判斷，分支預測錯誤率為 0%，極度利於 CPU 亂序執行與 SIMD 自動向量化。
3. **視窗分塊排程 (Window Slicing)**：
   - 垂直卷積以列為單位切分工作任務，各核心存取連續實體記憶體，有效避免 L1/L2 快取抖動與偽共享。
