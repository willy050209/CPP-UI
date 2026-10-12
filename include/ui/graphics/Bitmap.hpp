/**
 * @file Bitmap.hpp
 * @brief GDI+ 風格 32-bit 點陣影像類別，具備執行緒安全自動同步
 */

#pragma once

#include <cstdint>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

namespace cppui {
namespace graphics {

/// <summary>
/// 32-bit ARGB/BGRA 點陣圖形影像類別 (Bitmap)
/// 內建讀寫鎖支援多執行緒並發讀寫、高效區塊鎖定與旋轉翻轉。
/// </summary>
class Bitmap : public std::enable_shared_from_this<Bitmap> {
public:
    /// <summary>
    /// 以指定寬度與高度建構全透明初始點陣圖。
    /// </summary>
    /// <param name="width">影像寬度像素值 (必須大於 0)。</param>
    /// <param name="height">影像高度像素值 (必須大於 0)。</param>
    /// <param name="format">像素資料格式，預設為 Format32bppArgb。</param>
    /// <exception cref="std::invalid_argument">若 width 或 height 為 0 擲出例外。</exception>
    Bitmap(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::Format32bppArgb);

    /// <summary>
    /// 從外部像素記憶體跨度建構點陣圖。
    /// </summary>
    /// <param name="width">影像寬度像素值。</param>
    /// <param name="height">影像高度像素值。</param>
    /// <param name="pixels">32-bit ARGB 像素陣列跨度。</param>
    /// <param name="format">像素格式。</param>
    /// <exception cref="std::invalid_argument">若像素跨度大小小於 width * height 擲出例外。</exception>
    Bitmap(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format = PixelFormat::Format32bppArgb);

    /// <summary>
    /// 從檔案路徑載入影像。
    /// </summary>
    /// <param name="filename">影像檔案路徑。</param>
    /// <exception cref="std::invalid_argument">若路徑為空或載入失敗擲出例外。</exception>
    explicit Bitmap(compat::string_view filename);

    /// <summary>執行緒安全拷貝建構子。</summary>
    /// <param name="other">來源影像。</param>
    Bitmap(const Bitmap& other);

    /// <summary>移動建構子。</summary>
    /// <param name="other">來源右值影像。</param>
    Bitmap(Bitmap&& other) noexcept;

    /// <summary>執行緒安全拷貝賦值運算子。</summary>
    /// <param name="other">來源影像。</param>
    /// <returns>自我引用。</returns>
    Bitmap& operator=(const Bitmap& other);

    /// <summary>移動賦值運算子。</summary>
    /// <param name="other">來源右值影像。</param>
    /// <returns>自我引用。</returns>
    Bitmap& operator=(Bitmap&& other) noexcept;

    /// <summary>解構子。</summary>
    ~Bitmap();

    /// <summary>取得影像寬度像素值。</summary>
    /// <returns>寬度。</returns>
    COMPAT_NODISCARD uint32_t GetWidth() const noexcept;

    /// <summary>取得影像高度像素值。</summary>
    /// <returns>高度。</returns>
    COMPAT_NODISCARD uint32_t GetHeight() const noexcept;

    /// <summary>取得像素資料格式。</summary>
    /// <returns>PixelFormat 列舉值。</returns>
    COMPAT_NODISCARD PixelFormat GetPixelFormat() const noexcept;

    /// <summary>取得每行掃描線跨度位元組數 (Stride)。</summary>
    /// <returns>跨度字節數。</returns>
    COMPAT_NODISCARD uint32_t GetStride() const noexcept;

    /// <summary>
    /// 讀取指定座標之像素色彩。
    /// </summary>
    /// <param name="x">X 座標。</param>
    /// <param name="y">Y 座標。</param>
    /// <returns>Color 色彩物件。</returns>
    /// <exception cref="std::out_of_range">若座標超出邊界擲出例外。</exception>
    COMPAT_NODISCARD core::Color GetPixel(uint32_t x, uint32_t y) const;

    /// <summary>
    /// 設定指定座標之像素色彩。
    /// </summary>
    /// <param name="x">X 座標。</param>
    /// <param name="y">Y 座標。</param>
    /// <param name="color">目標色彩。</param>
    /// <exception cref="std::out_of_range">若座標超出邊界擲出例外。</exception>
    void SetPixel(uint32_t x, uint32_t y, core::Color color);

    /// <summary>
    /// 以指定色彩填滿清除整幅影像。
    /// </summary>
    /// <param name="color">填滿色彩。</param>
    void Clear(core::Color color);

    /// <summary>取得唯讀連續像素陣列跨度。</summary>
    /// <returns>span 跨度。</returns>
    COMPAT_NODISCARD compat::span<const uint32_t> GetPixelSpan() const;

    /// <summary>取得可寫入之連續像素陣列跨度。</summary>
    /// <returns>span 跨度。</returns>
    COMPAT_NODISCARD compat::span<uint32_t> GetPixelSpanMut();

    /// <summary>
    /// 鎖定記憶體區塊以供直接讀寫。
    /// </summary>
    /// <param name="rect">鎖定矩形範圍。</param>
    /// <param name="flags">讀寫標記。</param>
    /// <returns>指向像素起始緩衝區的指針。</returns>
    void* LockBits(const core::Rect& rect, uint32_t flags);

    /// <summary>解除記憶體區塊鎖定。</summary>
    void UnlockBits();

    /// <summary>
    /// 深拷貝複製整幅點陣圖為新 Bitmap 物件。
    /// 直接透過連續記憶體 std::copy 高效複製，省去逐像素邊界判定開銷。
    /// </summary>
    /// <returns>新建立之 Bitmap 共享指標。</returns>
    /// <exception cref="std::invalid_argument">若來源影像寬高不合法擲出例外。</exception>
    COMPAT_NODISCARD std::shared_ptr<Bitmap> Clone() const;

    /// <summary>
    /// 擷取局部矩形並深拷貝為新 Bitmap。
    /// </summary>
    /// <param name="rect">來源裁切矩形。</param>
    /// <param name="format">目標像素格式。</param>
    /// <returns>新建立之 Bitmap 共享指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<Bitmap> Clone(const core::Rect& rect, PixelFormat format) const;

    /// <summary>
    /// 旋轉或翻轉影像像素。
    /// </summary>
    /// <param name="rotateFlipType">旋轉與翻轉類型。</param>
    void RotateFlip(RotateFlipType rotateFlipType);

    /// <summary>
    /// 將影像儲存為未壓縮 32-bit BMP 點陣圖檔案。
    /// </summary>
    /// <param name="filename">輸出檔案路徑。</param>
    /// <returns>成功傳回 true，失敗傳回 false。</returns>
    bool SaveAsBmp(compat::string_view filename) const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace graphics
} // namespace cppui
