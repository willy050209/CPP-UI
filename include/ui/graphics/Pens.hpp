/**
 * @file Pens.hpp
 * @brief GDI+ 對標之常用標準畫筆集合 (Stock Pens)
 */

#pragma once

#include "ui/graphics/Pen.hpp"

namespace cppui {
namespace graphics {

/// <summary>
/// 提供常用標準色彩與 1.0f 寬度之靜態畫筆集合 (Pens)
/// </summary>
class Pens {
public:
    Pens() = delete;

    /// <summary>透明畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Transparent();
    /// <summary>黑色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Black();
    /// <summary>白色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& White();
    /// <summary>紅色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Red();
    /// <summary>綠色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Green();
    /// <summary>藍色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Blue();
    /// <summary>黃色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Yellow();
    /// <summary>青色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Cyan();
    /// <summary>洋紅色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Magenta();
    /// <summary>灰色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Gray();
    /// <summary>淺灰色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& LightGray();
    /// <summary>深灰色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& DarkGray();
    /// <summary>橙色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Orange();
    /// <summary>紫色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Purple();
    /// <summary>棕色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Brown();
    /// <summary>鐵藍色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& SteelBlue();
    /// <summary>海軍藍畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Navy();
    /// <summary>金色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Gold();
    /// <summary>萊姆綠畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Lime();
    /// <summary>粉紅色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Pink();
    /// <summary>深紅色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& DarkRed();
    /// <summary>深藍色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& DarkBlue();
    /// <summary>深綠色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& DarkGreen();
    /// <summary>橄欖綠畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Olive();
    /// <summary>鴨翅綠 (藍綠色) 畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Teal();
    /// <summary>栗色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Maroon();
    /// <summary>珊瑚色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Coral();
    /// <summary>銀色畫筆。</summary>
    COMPAT_NODISCARD static const Pen& Silver();
};

} // namespace graphics
} // namespace cppui
