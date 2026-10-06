#pragma once

#include "compat/compat.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 2D 浮點數座標點
/// </summary>
struct Point {
    float x = 0.0f;
    float y = 0.0f;

    COMPAT_CONSTEXPR_14 Point() noexcept = default;
    COMPAT_CONSTEXPR_14 Point(float px, float py) noexcept : x(px), y(py) {}

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Point& other) const noexcept {
        return x == other.x && y == other.y;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Point& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 2D 尺寸結構
/// </summary>
struct Size {
    float width = 0.0f;
    float height = 0.0f;

    COMPAT_CONSTEXPR_14 Size() noexcept = default;
    COMPAT_CONSTEXPR_14 Size(float w, float h) noexcept : width(w), height(h) {}

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept {
        return width <= 0.0f || height <= 0.0f;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Size& other) const noexcept {
        return width == other.width && height == other.height;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Size& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 2D 矩形邊界結構
/// </summary>
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    COMPAT_CONSTEXPR_14 Rect() noexcept = default;
    COMPAT_CONSTEXPR_14 Rect(float px, float py, float w, float h) noexcept
        : x(px), y(py), width(w), height(h) {}
    COMPAT_CONSTEXPR_14 Rect(const Point& origin, const Size& size) noexcept
        : x(origin.x), y(origin.y), width(size.width), height(size.height) {}

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Left() const noexcept { return x; }
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Top() const noexcept { return y; }
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Right() const noexcept { return x + width; }
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Bottom() const noexcept { return y + height; }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point Origin() const noexcept { return Point(x, y); }
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size GetSize() const noexcept { return Size(width, height); }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept {
        return width <= 0.0f || height <= 0.0f;
    }

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

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool Contains(const Point& p) const noexcept {
        return p.x >= x && p.x <= Right() && p.y >= y && p.y <= Bottom();
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Rect& other) const noexcept {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

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

    COMPAT_CONSTEXPR_14 Thickness() noexcept = default;
    COMPAT_CONSTEXPR_14 Thickness(float uniform) noexcept
        : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    COMPAT_CONSTEXPR_14 Thickness(float horizontal, float vertical) noexcept
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    COMPAT_CONSTEXPR_14 Thickness(float l, float t, float r, float b) noexcept
        : left(l), top(t), right(r), bottom(b) {}
};

/// <summary>
/// RGBA 32-bit 顏色結構
/// </summary>
struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    COMPAT_CONSTEXPR_14 Color() noexcept = default;
    COMPAT_CONSTEXPR_14 Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
        : r(red), g(green), b(blue), a(alpha) {}

    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept {
        return Color(r, g, b, a);
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
