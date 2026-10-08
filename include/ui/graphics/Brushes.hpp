/**
 * @file Brushes.hpp
 * @brief GDI+ 對標之常用標準純色畫刷集合 (Stock Brushes)
 */

#pragma once

#include "ui/graphics/Brush.hpp"

namespace cppui {
namespace graphics {

/// <summary>
/// 提供常用標準色彩之靜態純色畫刷集合 (Brushes)
/// </summary>
class Brushes {
public:
    Brushes() = delete;

    /// <summary>透明畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Transparent();
    /// <summary>黑色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Black();
    /// <summary>白色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& White();
    /// <summary>紅色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Red();
    /// <summary>綠色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Green();
    /// <summary>藍色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Blue();
    /// <summary>黃色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Yellow();
    /// <summary>青色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Cyan();
    /// <summary>洋紅色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Magenta();
    /// <summary>灰色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Gray();
    /// <summary>淺灰色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& LightGray();
    /// <summary>深灰色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& DarkGray();
    /// <summary>橙色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Orange();
    /// <summary>紫色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Purple();
    /// <summary>棕色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Brown();
    /// <summary>鐵藍色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& SteelBlue();
    /// <summary>海軍藍畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Navy();
    /// <summary>金色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Gold();
    /// <summary>萊姆綠畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Lime();
    /// <summary>粉紅色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Pink();
    /// <summary>深紅色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& DarkRed();
    /// <summary>深藍色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& DarkBlue();
    /// <summary>深綠色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& DarkGreen();
    /// <summary>橄欖綠畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Olive();
    /// <summary>鴨翅綠 (藍綠色) 畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Teal();
    /// <summary>栗色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Maroon();
    /// <summary>珊瑚色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Coral();
    /// <summary>銀色畫刷。</summary>
    COMPAT_NODISCARD static const SolidBrush& Silver();
};

} // namespace graphics
} // namespace cppui
