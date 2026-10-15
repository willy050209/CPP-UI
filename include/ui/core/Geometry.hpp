/*
 * File: Geometry.hpp
 * Purpose: Declares core 2D geometric primitives including Point, Size, Rect,
 *          Thickness, and Color structures with constexpr support.
 */

#pragma once

#include "compat/Compat.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 2D 浮點數座標點 (Point)
/// </summary>
struct Point {
    float x = 0.0f;
    float y = 0.0f;

    /// <summary>
    /// 預設建構座標點 (0, 0)。
    /// </summary>
    COMPAT_CONSTEXPR_14 Point() noexcept = default;

    /// <summary>
    /// 以指定 X 與 Y 座標建構座標點。
    /// </summary>
    /// <param name="px">X 軸座標值。</param>
    /// <param name="py">Y 軸座標值。</param>
    COMPAT_CONSTEXPR_14 Point(float px, float py) noexcept : x(px), y(py) {}

    /// <summary>
    /// 判斷兩座標點是否相等。
    /// </summary>
    /// <param name="other">待比較的另一座標點。</param>
    /// <returns>若兩者 X 與 Y 完全相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Point& other) const noexcept {
        return x == other.x && y == other.y;
    }

    /// <summary>
    /// 判斷兩座標點是否不相等。
    /// </summary>
    /// <param name="other">待比較的另一座標點。</param>
    /// <returns>若任一軸向不相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Point& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 2D 尺寸結構 (Size)
/// </summary>
struct Size {
    float width = 0.0f;
    float height = 0.0f;

    /// <summary>
    /// 預設建構尺寸 (0, 0)。
    /// </summary>
    COMPAT_CONSTEXPR_14 Size() noexcept = default;

    /// <summary>
    /// 以指定寬度與高度建構尺寸。
    /// </summary>
    /// <param name="w">寬度值。</param>
    /// <param name="h">高度值。</param>
    COMPAT_CONSTEXPR_14 Size(float w, float h) noexcept : width(w), height(h) {}

    /// <summary>
    /// 檢查尺寸是否為空 (寬度或高度小於等於 0)。
    /// </summary>
    /// <returns>若尺寸為空傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept {
        return width <= 0.0f || height <= 0.0f;
    }

    /// <summary>
    /// 判斷兩尺寸是否相等。
    /// </summary>
    /// <param name="other">待比較的另一尺寸。</param>
    /// <returns>若寬高皆相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Size& other) const noexcept {
        return width == other.width && height == other.height;
    }

    /// <summary>
    /// 判斷兩尺寸是否不相等。
    /// </summary>
    /// <param name="other">待比較的另一尺寸。</param>
    /// <returns>若寬或高不相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Size& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 2D 矩形邊界結構 (Rect)
/// </summary>
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    /// <summary>
    /// 預設建構矩形 (0, 0, 0, 0)。
    /// </summary>
    COMPAT_CONSTEXPR_14 Rect() noexcept = default;

    /// <summary>
    /// 以指定左上角座標與寬高建構矩形。
    /// </summary>
    /// <param name="px">左上角 X 座標。</param>
    /// <param name="py">左上角 Y 座標。</param>
    /// <param name="w">矩形寬度。</param>
    /// <param name="h">矩形高度。</param>
    COMPAT_CONSTEXPR_14 Rect(float px, float py, float w, float h) noexcept
        : x(px), y(py), width(w), height(h) {}

    /// <summary>
    /// 以原點與尺寸結構建構矩形。
    /// </summary>
    /// <param name="origin">原點座標。</param>
    /// <param name="size">尺寸大小。</param>
    COMPAT_CONSTEXPR_14 Rect(const Point& origin, const Size& size) noexcept
        : x(origin.x), y(origin.y), width(size.width), height(size.height) {}

    /// <summary>
    /// 取得矩形左邊緣 X 座標。
    /// </summary>
    /// <returns>左側 X 座標。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Left() const noexcept { return x; }

    /// <summary>
    /// 取得矩形頂邊緣 Y 座標。
    /// </summary>
    /// <returns>頂部 Y 座標。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Top() const noexcept { return y; }

    /// <summary>
    /// 取得矩形右邊緣 X 座標 (x + width)。
    /// </summary>
    /// <returns>右側 X 座標。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Right() const noexcept { return x + width; }

    /// <summary>
    /// 取得矩形底邊緣 Y 座標 (y + height)。
    /// </summary>
    /// <returns>底部 Y 座標。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Bottom() const noexcept { return y + height; }

    /// <summary>
    /// 取得矩形左上角原點。
    /// </summary>
    /// <returns>Point 原點座標。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point Origin() const noexcept { return Point(x, y); }

    /// <summary>
    /// 取得矩形寬高尺寸。
    /// </summary>
    /// <returns>Size 尺寸結構。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size GetSize() const noexcept { return Size(width, height); }

    /// <summary>
    /// 檢查矩形是否為空 (寬度或高度小於等於 0)。
    /// </summary>
    /// <returns>若為空傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept {
        return width <= 0.0f || height <= 0.0f;
    }

    /// <summary>
    /// 計算兩矩形之相交區域。
    /// </summary>
    /// <param name="other">待求交集的另一矩形。</param>
    /// <returns>相交矩形，若無重疊則傳回空矩形 (0, 0, 0, 0)。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect Intersect(const Rect& other) const noexcept {
        float l = (x > other.x) ? x : other.x;
        float t = (y > other.y) ? y : other.y;
        float r = (Right() < other.Right()) ? Right() : other.Right();
        float b = (Bottom() < other.Bottom()) ? Bottom() : other.Bottom();
        if (r <= l || b <= t) {
            return Rect(0.0f, 0.0f, 0.0f, 0.0f);
        }
        return Rect(l, t, r - l, b - t);
    }

    /// <summary>
    /// 判定指定座標點是否落於矩形範圍內 (含邊界)。
    /// </summary>
    /// <param name="p">待檢測之座標點。</param>
    /// <returns>若包含該點傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool Contains(const Point& p) const noexcept {
        return p.x >= x && p.x <= Right() && p.y >= y && p.y <= Bottom();
    }

    /// <summary>
    /// 判斷兩矩形是否相等。
    /// </summary>
    /// <param name="other">待比較的另一矩形。</param>
    /// <returns>若原點與大小皆相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Rect& other) const noexcept {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    /// <summary>
    /// 判斷兩矩形是否不相等。
    /// </summary>
    /// <param name="other">待比較的另一矩形。</param>
    /// <returns>若任一屬性不相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Rect& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 邊界厚度（用於 Margin / Padding / Border）
/// </summary>
struct Thickness {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    /// <summary>
    /// 預設建構厚度 (0, 0, 0, 0)。
    /// </summary>
    COMPAT_CONSTEXPR_14 Thickness() noexcept = default;

    /// <summary>
    /// 以統一值建構四邊等寬厚度。
    /// </summary>
    /// <param name="uniform">四邊邊界厚度值。</param>
    COMPAT_CONSTEXPR_14 Thickness(float uniform) noexcept
        : left(uniform), top(uniform), right(uniform), bottom(uniform) {}

    /// <summary>
    /// 以水平與垂直值建構對稱厚度。
    /// </summary>
    /// <param name="horizontal">左右邊界厚度。</param>
    /// <param name="vertical">上下邊界厚度。</param>
    COMPAT_CONSTEXPR_14 Thickness(float horizontal, float vertical) noexcept
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}

    /// <summary>
    /// 分別指定四邊建構厚度。
    /// </summary>
    /// <param name="l">左邊界厚度。</param>
    /// <param name="t">頂部邊界厚度。</param>
    /// <param name="r">右邊界厚度。</param>
    /// <param name="b">底部邊界厚度。</param>
    COMPAT_CONSTEXPR_14 Thickness(float l, float t, float r, float b) noexcept
        : left(l), top(t), right(r), bottom(b) {}

    /// <summary>
    /// 取得水平厚度總和 (left + right)。
    /// </summary>
    /// <returns>水平總寬度。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Horizontal() const noexcept { return left + right; }

    /// <summary>
    /// 取得垂直厚度總和 (top + bottom)。
    /// </summary>
    /// <returns>垂直總高度。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Vertical() const noexcept { return top + bottom; }

    /// <summary>
    /// 判斷兩厚度是否相等。
    /// </summary>
    /// <param name="other">待比較的另一厚度。</param>
    /// <returns>若四邊皆相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Thickness& other) const noexcept {
        return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
    }

    /// <summary>
    /// 判斷兩厚度是否不相等。
    /// </summary>
    /// <param name="other">待比較的另一厚度。</param>
    /// <returns>若任一邊不相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Thickness& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// RGBA 32-bit 顏色結構
/// </summary>
struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    /// <summary>
    /// 預設建構黑色不透明顏色 (0, 0, 0, 255)。
    /// </summary>
    COMPAT_CONSTEXPR Color() noexcept = default;

    /// <summary>
    /// 以指定 RGBA 分量建構顏色。
    /// </summary>
    /// <param name="red">紅色通道 (0-255)。</param>
    /// <param name="green">綠色通道 (0-255)。</param>
    /// <param name="blue">藍色通道 (0-255)。</param>
    /// <param name="alpha">不透明度通道 (0-255)，預設為 255。</param>
    COMPAT_CONSTEXPR Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
        : r(red), g(green), b(blue), a(alpha) {}

    /// <summary>
    /// 從 ARGB 分量靜態建構 Color 實例。
    /// </summary>
    /// <param name="a">不透明度通道 (0-255)。</param>
    /// <param name="r">紅色通道 (0-255)。</param>
    /// <param name="g">綠色通道 (0-255)。</param>
    /// <param name="b">藍色通道 (0-255)。</param>
    /// <returns>對應之 Color 物件。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept {
        return Color(r, g, b, a);
    }

    /// <summary>
    /// 從 RGB 分量靜態建構不透明 (Alpha=255) Color 實例。
    /// </summary>
    /// <param name="r">紅色通道 (0-255)。</param>
    /// <param name="g">綠色通道 (0-255)。</param>
    /// <param name="b">藍色通道 (0-255)。</param>
    /// <returns>對應之 Color 物件。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR Color FromArgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
        return Color(r, g, b, 255);
    }

    /// <summary>
    /// 從 32 位元整數 (0xAARRGGBB) 靜態建構 Color 實例。
    /// </summary>
    /// <param name="argb">32 位元 ARGB 封裝值。</param>
    /// <returns>對應之 Color 物件。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR Color FromArgb(uint32_t argb) noexcept {
        return Color(
            static_cast<uint8_t>((argb >> 16) & 0xFF),
            static_cast<uint8_t>((argb >> 8) & 0xFF),
            static_cast<uint8_t>(argb & 0xFF),
            static_cast<uint8_t>((argb >> 24) & 0xFF)
        );
    }

    /// <summary>
    /// 從指定的基底顏色與新 Alpha 分量建構 Color 實例。
    /// </summary>
    /// <param name="a">不透明度通道 (0-255)。</param>
    /// <param name="baseColor">基底顏色。</param>
    /// <returns>對應之 Color 物件。</returns>
    COMPAT_NODISCARD static COMPAT_CONSTEXPR Color FromArgb(uint8_t a, const Color& baseColor) noexcept {
        return Color(baseColor.r, baseColor.g, baseColor.b, a);
    }

    /// <summary>
    /// 將當前顏色轉換為 32 位元 ARGB 整數值 (0xAARRGGBB)。
    /// </summary>
    /// <returns>32 位元 ARGB 整數。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR uint32_t ToArgb() const noexcept {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8) |
               static_cast<uint32_t>(b);
    }

    /// <summary>
    /// 檢查此顏色是否為未初始化的空顏色 (全 0)。
    /// </summary>
    /// <returns>若 RGBA 全為 0 傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR bool IsEmpty() const noexcept {
        return r == 0 && g == 0 && b == 0 && a == 0;
    }

    /// <summary>
    /// 計算並取得顏色之亮度 (Brightness / Lightness, 0.0f ~ 1.0f)。
    /// </summary>
    /// <returns>亮度純量值。</returns>
    COMPAT_NODISCARD float GetBrightness() const noexcept {
        float minVal = static_cast<float>(r < g ? (r < b ? r : b) : (g < b ? g : b)) / 255.0f;
        float maxVal = static_cast<float>(r > g ? (r > b ? r : b) : (g > b ? g : b)) / 255.0f;
        return (minVal + maxVal) / 2.0f;
    }

    /// <summary>
    /// 計算並取得顏色之色相 (Hue, 0.0f ~ 360.0f)。
    /// </summary>
    /// <returns>色相角度值。</returns>
    COMPAT_NODISCARD float GetHue() const noexcept {
        if (r == g && g == b) return 0.0f;
        float rf = static_cast<float>(r) / 255.0f;
        float gf = static_cast<float>(g) / 255.0f;
        float bf = static_cast<float>(b) / 255.0f;
        float minVal = rf < gf ? (rf < bf ? rf : bf) : (gf < bf ? gf : bf);
        float maxVal = rf > gf ? (rf > bf ? rf : bf) : (gf > bf ? gf : bf);
        float delta = maxVal - minVal;
        if (delta <= 0.0f) return 0.0f;

        float hue = 0.0f;
        if (rf == maxVal) {
            hue = (gf - bf) / delta;
        } else if (gf == maxVal) {
            hue = 2.0f + (bf - rf) / delta;
        } else {
            hue = 4.0f + (rf - gf) / delta;
        }
        hue *= 60.0f;
        if (hue < 0.0f) {
            hue += 360.0f;
        }
        return hue;
    }

    /// <summary>
    /// 計算並取得顏色之飽和度 (Saturation, 0.0f ~ 1.0f)。
    /// </summary>
    /// <returns>飽和度純量值。</returns>
    COMPAT_NODISCARD float GetSaturation() const noexcept {
        if (r == g && g == b) return 0.0f;
        float rf = static_cast<float>(r) / 255.0f;
        float gf = static_cast<float>(g) / 255.0f;
        float bf = static_cast<float>(b) / 255.0f;
        float minVal = rf < gf ? (rf < bf ? rf : bf) : (gf < bf ? gf : bf);
        float maxVal = rf > gf ? (rf > bf ? rf : bf) : (gf > bf ? gf : bf);
        float delta = maxVal - minVal;
        if (delta <= 0.0f) return 0.0f;

        float lightness = (minVal + maxVal) / 2.0f;
        if (lightness <= 0.5f) {
            return delta / (maxVal + minVal);
        }
        return delta / (2.0f - maxVal - minVal);
    }

    /// <summary>
    /// 判斷兩顏色是否完全相等。
    /// </summary>
    /// <param name="other">待比較的另一顏色。</param>
    /// <returns>若 RGBA 四通道皆相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR bool operator==(const Color& other) const noexcept {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    /// <summary>
    /// 判斷兩顏色是否不相等。
    /// </summary>
    /// <param name="other">待比較的另一顏色。</param>
    /// <returns>若任一通道不相等傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR bool operator!=(const Color& other) const noexcept {
        return !(*this == other);
    }
};

namespace Colors {
    COMPAT_INLINE_VAR constexpr Color Transparent = Color(0, 0, 0, 0);
    COMPAT_INLINE_VAR constexpr Color Black       = Color(0, 0, 0, 255);
    COMPAT_INLINE_VAR constexpr Color White       = Color(255, 255, 255, 255);
    COMPAT_INLINE_VAR constexpr Color Red         = Color(255, 0, 0, 255);
    COMPAT_INLINE_VAR constexpr Color Green       = Color(0, 255, 0, 255);
    COMPAT_INLINE_VAR constexpr Color Blue        = Color(0, 0, 255, 255);
    COMPAT_INLINE_VAR constexpr Color SteelBlue     = Color(70, 130, 180, 255);
    COMPAT_INLINE_VAR constexpr Color Yellow        = Color(255, 255, 0, 255);
    COMPAT_INLINE_VAR constexpr Color DarkSlateGray = Color(47, 79, 79, 255);
    COMPAT_INLINE_VAR constexpr Color Gray          = Color(128, 128, 128, 255);
    COMPAT_INLINE_VAR constexpr Color LightGray     = Color(211, 211, 211, 255);
} // namespace Colors

} // namespace core
} // namespace cppui
