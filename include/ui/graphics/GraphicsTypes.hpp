/**
 * @file GraphicsTypes.hpp
 * @brief GDI+ 風格現代圖形子系統列舉型別定義
 */

#pragma once

#include <cstdint>

namespace cppui {
namespace graphics {

/// <summary>
/// 線條虛線樣式列舉 (DashStyle)
/// </summary>
enum class DashStyle : uint8_t {
    Solid = 0,
    Dash = 1,
    Dot = 2,
    DashDot = 3,
    DashDotDot = 4,
    Custom = 5
};

/// <summary>
/// 線段端點樣式列舉 (LineCap)
/// </summary>
enum class LineCap : uint8_t {
    Flat = 0,
    Square = 1,
    Round = 2,
    Triangle = 3,
    NoAnchor = 0x10,
    SquareAnchor = 0x11,
    RoundAnchor = 0x12,
    DiamondAnchor = 0x13,
    ArrowAnchor = 0x14
};

/// <summary>
/// 線段轉角連接樣式列舉 (LineJoin)
/// </summary>
enum class LineJoin : uint8_t {
    Miter = 0,
    Bevel = 1,
    Round = 2,
    MiterClipped = 3
};

/// <summary>
/// 封閉幾何路徑填色模式列舉 (FillMode)
/// </summary>
enum class FillMode : uint8_t {
    Alternate = 0,
    Winding = 1
};

/// <summary>
/// 平滑化反鋸齒模式列舉 (SmoothingMode)
/// </summary>
enum class SmoothingMode : uint8_t {
    Default = 0,
    HighSpeed = 1,
    HighQuality = 2,
    None = 3,
    AntiAlias = 4
};

/// <summary>
/// 影像縮放插值模式列舉 (InterpolationMode)
/// </summary>
enum class InterpolationMode : uint8_t {
    Default = 0,
    Low = 1,
    High = 2,
    Bilinear = 3,
    Bicubic = 4,
    NearestNeighbor = 5,
    HighQualityBilinear = 6,
    HighQualityBicubic = 7
};

/// <summary>
/// 像素資料格式列舉 (PixelFormat)
/// </summary>
enum class PixelFormat : uint8_t {
    Format32bppArgb = 0,
    Format32bppRgb = 1,
    Format24bppRgb = 2,
    Format8bppIndexed = 3
};

/// <summary>
/// PictureBox 圖片呈現模式列舉 (PictureBoxSizeMode)
/// </summary>
enum class PictureBoxSizeMode : uint8_t {
    Normal = 0,
    StretchImage = 1,
    AutoSize = 2,
    CenterImage = 3,
    Zoom = 4
};

/// <summary>
/// 畫刷具體類型列舉 (BrushType)
/// </summary>
enum class BrushType : uint8_t {
    SolidColor = 0,
    LinearGradient = 1,
    Texture = 2
};

/// <summary>
/// 影像旋轉與翻轉類型列舉 (RotateFlipType)
/// </summary>
enum class RotateFlipType : uint8_t {
    RotateNoneFlipNone = 0,
    Rotate90FlipNone = 1,
    Rotate180FlipNone = 2,
    Rotate270FlipNone = 3,
    RotateNoneFlipX = 4,
    RotateNoneFlipY = 5,
    RotateNoneFlipXY = 6
};

/// <summary>
/// 影像編碼與檔案儲存格式列舉 (ImageFormat)
/// </summary>
enum class ImageFormat : uint8_t {
    Auto = 0,   // 依副檔名自動推斷格式
    Bmp  = 1,   // Windows 點陣圖 (*.bmp)
    Png  = 2,   // Portable Network Graphics (*.png)
    Jpeg = 3,   // JPEG / JPG (*.jpg, *.jpeg)
    Gif  = 4,   // Graphics Interchange Format (*.gif)
    Webp = 5,   // WebP 影像格式 (*.webp)
    Heif = 6,   // High Efficiency Image File Format (*.heic, *.heif)
    Tiff = 7    // Tagged Image File Format (*.tif, *.tiff)
};

} // namespace graphics
} // namespace cppui
