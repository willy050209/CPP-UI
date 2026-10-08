# Bitmap 類別 (cppui::graphics::Bitmap)

`Bitmap` 類別代表記憶體中未經壓縮的 32-bit 點陣影像圖面（Surface）。它封裝像素資料緩衝區，並內建讀寫鎖（`mutable std::shared_mutex`）以達成多執行緒自動並發同步。它可用於離線繪製畫布、影像濾鏡處理、子矩形裁切、90° 步進旋轉翻轉以及與 [`PictureBox`](controls_picturebox.md) 控制項綁定顯示。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/Bitmap.hpp"`
* **實作檔**：`src/graphics/Bitmap.cpp`
* **基底類別**：`std::enable_shared_from_this<Bitmap>`
* **記憶體佈局**：
  - 每個像素佔用 32 位元（4 位元組），預設為 `PixelFormat::Format32bppArgb`。
  - 行掃描線跨度（Stride）：$\text{Stride} = \text{Width} \times 4$ 位元組。
  - 記憶體中為連續的一維陣列（Row-Major 順序），索引計算公式：
    $$\text{Index} = y \times \text{Width} + x$$

---

## 2. 語法與宣告 (Class Syntax)

```cpp
namespace cppui {
namespace graphics {

class Bitmap : public std::enable_shared_from_this<Bitmap> {
public:
    // 建構子
    Bitmap(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::Format32bppArgb);
    Bitmap(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format = PixelFormat::Format32bppArgb);
    explicit Bitmap(compat::string_view filename);

    // 執行緒安全拷貝與移動語意
    Bitmap(const Bitmap& other);
    Bitmap(Bitmap&& other) noexcept;
    Bitmap& operator=(const Bitmap& other);
    Bitmap& operator=(Bitmap&& other) noexcept;
    ~Bitmap() = default;

    // 規格與尺寸查詢
    COMPAT_NODISCARD uint32_t GetWidth() const noexcept;
    COMPAT_NODISCARD uint32_t GetHeight() const noexcept;
    COMPAT_NODISCARD PixelFormat GetPixelFormat() const noexcept;
    COMPAT_NODISCARD uint32_t GetStride() const noexcept;

    // 像素存取 (邊界檢查 + 自動鎖定)
    COMPAT_NODISCARD core::Color GetPixel(uint32_t x, uint32_t y) const;
    void SetPixel(uint32_t x, uint32_t y, core::Color color);
    void Clear(core::Color color);

    // 記憶體跨度 (Span) 借用
    COMPAT_NODISCARD compat::span<const uint32_t> GetPixelSpan() const;
    COMPAT_NODISCARD compat::span<uint32_t> GetPixelSpanMut();

    // 高效區塊記憶體鎖定
    void* LockBits(const core::Rect& rect, uint32_t flags);
    void UnlockBits();

    // 裁剪深拷貝與幾何變換
    COMPAT_NODISCARD std::shared_ptr<Bitmap> Clone(const core::Rect& rect, PixelFormat format) const;
    void RotateFlip(RotateFlipType rotateFlipType);
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 建構子與例外規格 (Constructors & Exceptions)

### 3.1 尺寸建構子
```cpp
Bitmap(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::Format32bppArgb);
```
* **功能**：建構指定寬高之透明空影像（所有像素初始化為 `0x00000000`）。
* **前置檢查**：`width > 0 && height > 0`。
* **例外規格**：若 `width == 0` 或 `height == 0`，立即擲出 `std::invalid_argument("Bitmap: width and height must be greater than zero.")`。

### 3.2 外部緩衝區載入建構子
```cpp
Bitmap(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format = PixelFormat::Format32bppArgb);
```
* **功能**：從外部提供的連續 32 位元 ARGB 像素陣列複製建立影像。
* **前置檢查**：`pixels.size() >= width * height` 且 `width > 0 && height > 0`。
* **例外規格**：若像素跨度大小不足或尺寸為零，擲出 `std::invalid_argument`。

### 3.3 檔案載入建構子
```cpp
explicit Bitmap(compat::string_view filename);
```
* **功能**：從磁碟圖檔載入點陣影像。
* **例外規格**：若檔名為空或檔案不存在/格式無法解析，擲出 `std::invalid_argument`。

---

## 4. 像素存取與操作 (Pixel Operations)

| 方法 | 參數 | 回傳值 | 說明 | 異常處理 |
| :--- | :--- | :--- | :--- | :--- |
| `GetPixel(x, y)` | `uint32_t x, y` | `core::Color` | 取得指定坐標之像素色彩。具備共用讀鎖保護。 | 若 $x \ge \text{width}$ 或 $y \ge \text{height}$，擲出 `std::out_of_range`。 |
| `SetPixel(x, y, color)` | `uint32_t x, y, Color` | `void` | 寫入指定坐標之像素色彩。具備獨佔寫鎖保護。 | 若 $x \ge \text{width}$ 或 $y \ge \text{height}$，擲出 `std::out_of_range`。 |
| `Clear(color)` | `core::Color color` | `void` | 以指定色彩填滿整幅影像所有像素。 | 保證無例外。 |
| `GetPixelSpan()` | 無 | `compat::span<const uint32_t>` | 借用唯讀連續像素緩衝區視圖（零拷貝）。 | 保證無例外。 |
| `GetPixelSpanMut()` | 無 | `compat::span<uint32_t>` | 借用可修改之連續像素緩衝區視圖。 | 保證無例外。 |

---

## 5. 高效區塊鎖定 API (`LockBits` / `UnlockBits`)

在需要每秒處理數百萬像素的影像處理（如濾鏡、卷積模糊、色彩遮罩運算）時，頻繁呼叫 `GetPixel` / `SetPixel` 會因逐個像素獲取鎖而產生額外開銷。`LockBits` 提供直接指標存取：

```cpp
void* LockBits(const core::Rect& rect, uint32_t flags);
void UnlockBits();
```

1. **`LockBits`**：
   - 自動獲取內部的獨佔寫入鎖（`std::unique_lock`）並標記鎖定狀態。
   - 傳回指向影像內部第一列像素資料起始位址的原始指標 (`uint32_t*`)。
   - 在呼叫 `UnlockBits` 之前，任何其他執行緒對此影像的讀寫皆會安全阻塞等待。
2. **`UnlockBits`**：
   - 解除鎖定狀態並釋放獨佔鎖。

> [!CAUTION]
> 呼叫 `LockBits` 後**必須**確保成對呼叫 `UnlockBits()`，否則將導致後續操作死鎖。建議使用 RAII 物件進行包裝管理。

---

## 6. 裁剪與幾何變換 API (Cropping & Transforms)

### 6.1 子矩形裁剪複製 (`Clone`)
```cpp
COMPAT_NODISCARD std::shared_ptr<Bitmap> Clone(const core::Rect& rect, PixelFormat format) const;
```
* **功能**：擷取由 `rect` 指定的子矩形區域，並產生獨立的新 `Bitmap` 物件。
* **邊界防護**：若 `rect` 部分超出原圖邊界，會自動與原圖邊界相交（Intersection）夾止；若完全在邊界之外則產生 1x1 最小空白影像。

### 6.2 旋轉與翻轉 (`RotateFlip`)
```cpp
void RotateFlip(RotateFlipType rotateFlipType);
```
* **功能**：就地（In-Place）修改像素資料，執行 90° 步進旋轉及水平/垂直鏡像翻轉。
* **參數**：[`RotateFlipType`](graphics_enums.md#9-影像旋轉與翻轉列舉-rotatefliptype)（如 `Rotate90FlipNone`, `Rotate180FlipNone`, `RotateNoneFlipX` 等）。
* **尺寸自動切換**：當進行 90° 或 270° 旋轉時，影像的寬度與高度將自動互換（$\text{NewWidth} = \text{Height}, \text{NewHeight} = \text{Width}$）。

---

## 7. 範例程式碼 (Examples)

### 7.1 建立與逐點繪製影像

```cpp
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/Colors.hpp"
#include "compat/compat.hpp"

std::shared_ptr<cppui::graphics::Bitmap> CreateCheckerboard(uint32_t size) {
    auto bmp = std::make_shared<cppui::graphics::Bitmap>(size, size);

    for (uint32_t y = 0; y < size; ++y) {
        for (uint32_t x = 0; x < size; ++x) {
            bool isDark = ((x / 16) + (y / 16)) % 2 == 0;
            bmp->SetPixel(x, y, isDark ? cppui::graphics::Colors::DarkSlateGray : cppui::graphics::Colors::White);
        }
    }

    return bmp;
}
```

### 7.2 使用 LockBits 實現高速灰階濾鏡 (Grayscale Filter)

```cpp
#include "ui/graphics/Bitmap.hpp"

void ApplyGrayscaleFilter(cppui::graphics::Bitmap& bmp) {
    uint32_t width = bmp.GetWidth();
    uint32_t height = bmp.GetHeight();

    // 鎖定全圖緩衝區
    core::Rect fullRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    auto* rawPixels = static_cast<uint32_t*>(bmp.LockBits(fullRect, 0));

    // 高速指針線性遍歷 (無逐點鎖開銷)
    size_t totalPixels = static_cast<size_t>(width) * height;
    for (size_t i = 0; i < totalPixels; ++i) {
        uint32_t argb = rawPixels[i];
        uint32_t a = (argb >> 24) & 0xFF;
        uint32_t r = (argb >> 16) & 0xFF;
        uint32_t g = (argb >> 8) & 0xFF;
        uint32_t b = argb & 0xFF;

        // 計算亮度 (標準 NTSC 灰階權重)
        uint32_t gray = static_cast<uint32_t>(0.299f * r + 0.587f * g + 0.114f * b);

        rawPixels[i] = (a << 24) | (gray << 16) | (gray << 8) | gray;
    }

    // 釋放鎖定
    bmp.UnlockBits();
}
```

### 7.3 影像旋轉與 PictureBox 顯示

```cpp
#include "ui/graphics/Bitmap.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

void SetupRotatedAvatar(cppui::controls::PictureBox& picBox, std::shared_ptr<cppui::graphics::Bitmap> avatarBmp) {
    // 順時針旋轉 90 度
    avatarBmp->RotateFlip(cppui::graphics::RotateFlipType::Rotate90FlipNone);

    // 綁定至 PictureBox 控制項展示
    picBox.SetImage(avatarBmp);
    picBox.SetSizeMode(cppui::graphics::PictureBoxSizeMode::Zoom);
}
```

---

## 8. 請參閱 (See Also)

* [`Graphics` 繪圖中樞 (支援 `FromImage` 離線光柵)](graphics_graphics.md)
* [`Color` 色彩結構詳解](graphics_color.md)
* [`TextureBrush` 紋理材質畫刷](graphics_brush.md#6-點陣材質畫刷texturebrush)
* [`PictureBox` 圖片顯示控制項](controls_picturebox.md)
* [`RotateFlipType` 列舉型別說明](graphics_enums.md#9-影像旋轉與翻轉列舉-rotatefliptype)
