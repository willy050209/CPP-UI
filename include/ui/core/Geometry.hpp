#pragma once

#include "compat/compat.hpp"

namespace cppui::core {

/// <summary>
/// 2D 浮點數座標點
/// </summary>
struct Point {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Point() noexcept = default;
    constexpr Point(float px, float py) noexcept : x(px), y(py) {}

    constexpr bool operator==(const Point& other) const noexcept {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Point& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 2D 尺寸結構
/// </summary>
struct Size {
    float width = 0.0f;
    float height = 0.0f;

    constexpr Size() noexcept = default;
    constexpr Size(float w, float h) noexcept : width(w), height(h) {}

    constexpr bool IsEmpty() const noexcept {
        return width <= 0.0f || height <= 0.0f;
    }

    constexpr bool operator==(const Size& other) const noexcept {
        return width == other.width && height == other.height;
    }

    constexpr bool operator!=(const Size& other) const noexcept {
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

    constexpr Rect() noexcept = default;
    constexpr Rect(float px, float py, float w, float h) noexcept
        : x(px), y(py), width(w), height(h) {}
    constexpr Rect(const Point& origin, const Size& size) noexcept
        : x(origin.x), y(origin.y), width(size.width), height(size.height) {}

    constexpr float Left() const noexcept { return x; }
    constexpr float Top() const noexcept { return y; }
    constexpr float Right() const noexcept { return x + width; }
    constexpr float Bottom() const noexcept { return y + height; }

    constexpr Point Origin() const noexcept { return Point(x, y); }
    constexpr Size GetSize() const noexcept { return Size(width, height); }

    constexpr bool Contains(const Point& p) const noexcept {
        return p.x >= x && p.x <= Right() && p.y >= y && p.y <= Bottom();
    }

    constexpr bool operator==(const Rect& other) const noexcept {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    constexpr bool operator!=(const Rect& other) const noexcept {
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

    constexpr Thickness() noexcept = default;
    constexpr Thickness(float uniform) noexcept
        : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    constexpr Thickness(float horizontal, float vertical) noexcept
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    constexpr Thickness(float l, float t, float r, float b) noexcept
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

    constexpr Color() noexcept = default;
    constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
        : r(red), g(green), b(blue), a(alpha) {}

    static constexpr Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept {
        return Color(r, g, b, a);
    }
};

namespace Colors {
    inline constexpr Color Transparent = Color(0, 0, 0, 0);
    inline constexpr Color Black       = Color(0, 0, 0, 255);
    inline constexpr Color White       = Color(255, 255, 255, 255);
    inline constexpr Color Red         = Color(255, 0, 0, 255);
    inline constexpr Color Green       = Color(0, 255, 0, 255);
    inline constexpr Color Blue        = Color(0, 0, 255, 255);
    inline constexpr Color SteelBlue   = Color(70, 130, 180, 255);
} // namespace Colors

} // namespace cppui::core
