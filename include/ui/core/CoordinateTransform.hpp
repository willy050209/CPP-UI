/*
 * File: CoordinateTransform.hpp
 * Purpose: Declares CoordinateTransform class for bidirectional conversions
 *          between physical screen pixels and device-independent logical pixels (DIP).
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 座標轉換管理類別 (CoordinateTransform)
/// 封裝物理像素 (Physical Pixels) 與設備無關邏輯像素 (Logical DIP) 之間的相互轉換，
/// 提供 Point, Size, Rect, Thickness 與純量值在任意 DPI 縮放下的嚴格換算與邊界保護。
/// </summary>
class CoordinateTransform {
public:
    /// <summary>
    /// 建構函式：指定當前 DPI 縮放係數 (預設 1.0f)。
    /// </summary>
    /// <param name="dpiScale">DPI 縮放比率，若小於等於 0 則預設為 1.0f。</param>
    COMPAT_CONSTEXPR_14 explicit CoordinateTransform(float dpiScale = 1.0f) noexcept
        : m_dpiScale((dpiScale > 0.0f) ? dpiScale : 1.0f) {}

    /// <summary>
    /// 取得當前 DPI 縮放係數 (例如 1.0f, 1.25f, 1.5f, 2.0f)。
    /// </summary>
    /// <returns>DPI 縮放比率。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetDpiScale() const noexcept {
        return m_dpiScale;
    }

    /// <summary>
    /// 設定當前 DPI 縮放係數 (小於或等於 0 自動回退為 1.0f)。
    /// </summary>
    /// <param name="dpiScale">DPI 縮放比率。</param>
    COMPAT_CONSTEXPR_14 void SetDpiScale(float dpiScale) noexcept {
        m_dpiScale = (dpiScale > 0.0f) ? dpiScale : 1.0f;
    }

    // =========================================================================
    // 實例方法：物理像素 (Physical Pixels) -> 邏輯像素 (Logical DIP)
    // =========================================================================

    /// <summary>
    /// 將物理像素座標點轉換為邏輯 DIP 座標點。
    /// </summary>
    /// <param name="physicalPoint">物理像素座標點。</param>
    /// <returns>邏輯 DIP 座標點。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point ToLogical(Point physicalPoint) const noexcept {
        return Point(physicalPoint.x / m_dpiScale, physicalPoint.y / m_dpiScale);
    }

    /// <summary>
    /// 將物理像素尺寸轉換為邏輯 DIP 尺寸。
    /// </summary>
    /// <param name="physicalSize">物理像素尺寸。</param>
    /// <returns>邏輯 DIP 尺寸。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size ToLogical(Size physicalSize) const noexcept {
        return Size(physicalSize.width / m_dpiScale, physicalSize.height / m_dpiScale);
    }

    /// <summary>
    /// 將物理像素矩形轉換為邏輯 DIP 矩形。
    /// </summary>
    /// <param name="physicalRect">物理像素矩形。</param>
    /// <returns>邏輯 DIP 矩形。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect ToLogical(Rect physicalRect) const noexcept {
        return Rect(
            physicalRect.x / m_dpiScale,
            physicalRect.y / m_dpiScale,
            physicalRect.width / m_dpiScale,
            physicalRect.height / m_dpiScale
        );
    }

    /// <summary>
    /// 將物理像素厚度轉換為邏輯 DIP 厚度。
    /// </summary>
    /// <param name="physicalThickness">物理像素厚度。</param>
    /// <returns>邏輯 DIP 厚度。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Thickness ToLogical(Thickness physicalThickness) const noexcept {
        return Thickness(
            physicalThickness.left / m_dpiScale,
            physicalThickness.top / m_dpiScale,
            physicalThickness.right / m_dpiScale,
            physicalThickness.bottom / m_dpiScale
        );
    }

    /// <summary>
    /// 將物理像素數值純量轉換為邏輯 DIP 純量。
    /// </summary>
    /// <param name="physicalValue">物理像素浮點值。</param>
    /// <returns>邏輯 DIP 浮點值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float ToLogical(float physicalValue) const noexcept {
        return physicalValue / m_dpiScale;
    }

    // =========================================================================
    // 實例方法：邏輯像素 (Logical DIP) -> 物理像素 (Physical Pixels)
    // =========================================================================

    /// <summary>
    /// 將邏輯 DIP 座標點轉換為物理像素座標點。
    /// </summary>
    /// <param name="logicalPoint">邏輯 DIP 座標點。</param>
    /// <returns>物理像素座標點。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point ToPhysical(Point logicalPoint) const noexcept {
        return Point(logicalPoint.x * m_dpiScale, logicalPoint.y * m_dpiScale);
    }

    /// <summary>
    /// 將邏輯 DIP 尺寸轉換為物理像素尺寸。
    /// </summary>
    /// <param name="logicalSize">邏輯 DIP 尺寸。</param>
    /// <returns>物理像素尺寸。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size ToPhysical(Size logicalSize) const noexcept {
        return Size(logicalSize.width * m_dpiScale, logicalSize.height * m_dpiScale);
    }

    /// <summary>
    /// 將邏輯 DIP 矩形轉換為物理像素矩形。
    /// </summary>
    /// <param name="logicalRect">邏輯 DIP 矩形。</param>
    /// <returns>物理像素矩形。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect ToPhysical(Rect logicalRect) const noexcept {
        return Rect(
            logicalRect.x * m_dpiScale,
            logicalRect.y * m_dpiScale,
            logicalRect.width * m_dpiScale,
            logicalRect.height * m_dpiScale
        );
    }

    /// <summary>
    /// 將邏輯 DIP 厚度轉換為物理像素厚度。
    /// </summary>
    /// <param name="logicalThickness">邏輯 DIP 厚度。</param>
    /// <returns>物理像素厚度。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Thickness ToPhysical(Thickness logicalThickness) const noexcept {
        return Thickness(
            logicalThickness.left * m_dpiScale,
            logicalThickness.top * m_dpiScale,
            logicalThickness.right * m_dpiScale,
            logicalThickness.bottom * m_dpiScale
        );
    }

    /// <summary>
    /// 將邏輯 DIP 數值純量轉換為物理像素純量。
    /// </summary>
    /// <param name="logicalValue">邏輯 DIP 浮點值。</param>
    /// <returns>物理像素浮點值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float ToPhysical(float logicalValue) const noexcept {
        return logicalValue * m_dpiScale;
    }

    // =========================================================================
    // 靜態無狀態輔助方法 (Stateless Helpers)
    // =========================================================================

    /// <summary>
    /// 以指定縮放比將物理像素座標點轉換為邏輯 DIP。
    /// </summary>
    /// <param name="pt">物理像素座標點。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>邏輯 DIP 座標點。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Point PhysicalToLogical(Point pt, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Point(pt.x / s, pt.y / s);
    }

    /// <summary>
    /// 以指定縮放比將邏輯 DIP 座標點轉換為物理像素。
    /// </summary>
    /// <param name="pt">邏輯 DIP 座標點。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>物理像素座標點。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Point LogicalToPhysical(Point pt, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Point(pt.x * s, pt.y * s);
    }

    /// <summary>
    /// 以指定縮放比將物理像素尺寸轉換為邏輯 DIP。
    /// </summary>
    /// <param name="sz">物理像素尺寸。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>邏輯 DIP 尺寸。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Size PhysicalToLogical(Size sz, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Size(sz.width / s, sz.height / s);
    }

    /// <summary>
    /// 以指定縮放比將邏輯 DIP 尺寸轉換為物理像素。
    /// </summary>
    /// <param name="sz">邏輯 DIP 尺寸。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>物理像素尺寸。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Size LogicalToPhysical(Size sz, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Size(sz.width * s, sz.height * s);
    }

    /// <summary>
    /// 以指定縮放比將物理像素矩形轉換為邏輯 DIP。
    /// </summary>
    /// <param name="r">物理像素矩形。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>邏輯 DIP 矩形。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Rect PhysicalToLogical(Rect r, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Rect(r.x / s, r.y / s, r.width / s, r.height / s);
    }

    /// <summary>
    /// 以指定縮放比將邏輯 DIP 矩形轉換為物理像素。
    /// </summary>
    /// <param name="r">邏輯 DIP 矩形。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>物理像素矩形。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Rect LogicalToPhysical(Rect r, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Rect(r.x * s, r.y * s, r.width * s, r.height * s);
    }

    /// <summary>
    /// 以指定縮放比將物理像素厚度轉換為邏輯 DIP。
    /// </summary>
    /// <param name="t">物理像素厚度。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>邏輯 DIP 厚度。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Thickness PhysicalToLogical(Thickness t, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Thickness(t.left / s, t.top / s, t.right / s, t.bottom / s);
    }

    /// <summary>
    /// 以指定縮放比將邏輯 DIP 厚度轉換為物理像素。
    /// </summary>
    /// <param name="t">邏輯 DIP 厚度。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>物理像素厚度。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Thickness LogicalToPhysical(Thickness t, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return Thickness(t.left * s, t.top * s, t.right * s, t.bottom * s);
    }

    /// <summary>
    /// 以指定縮放比將物理像素純量轉換為邏輯 DIP。
    /// </summary>
    /// <param name="val">物理像素浮點值。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>邏輯 DIP 浮點值。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 float PhysicalToLogical(float val, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return val / s;
    }

    /// <summary>
    /// 以指定縮放比將邏輯 DIP 純量轉換為物理像素。
    /// </summary>
    /// <param name="val">邏輯 DIP 浮點值。</param>
    /// <param name="scale">DPI 縮放比。</param>
    /// <returns>物理像素浮點值。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 float LogicalToPhysical(float val, float scale) noexcept {
        float s = (scale > 0.0f) ? scale : 1.0f;
        return val * s;
    }

private:
    float m_dpiScale = 1.0f;
};

} // namespace core
} // namespace cppui
