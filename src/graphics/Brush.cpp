/**
 * @file Brush.cpp
 * @brief GDI+ 風格畫刷實作 (SolidBrush, LinearGradientBrush, TextureBrush)
 */

#include "ui/graphics/Brush.hpp"
#include "ui/graphics/Bitmap.hpp"
#include <stdexcept>
#include <cmath>

namespace cppui {
namespace graphics {

// ==========================================
// SolidBrush 實作
// ==========================================

SolidBrush::SolidBrush(core::Color color)
    : m_color(color) {}

SolidBrush::SolidBrush(const SolidBrush& other) {
    compat::shared_lock<compat::shared_mutex> lock(other.m_mutex);
    m_color = other.m_color;
}

SolidBrush::SolidBrush(SolidBrush&& other) noexcept {
    std::unique_lock<compat::shared_mutex> lock(other.m_mutex);
    m_color = other.m_color;
}

SolidBrush& SolidBrush::operator=(const SolidBrush& other) {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        compat::shared_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);
        m_color = other.m_color;
    }
    return *this;
}

SolidBrush& SolidBrush::operator=(SolidBrush&& other) noexcept {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::unique_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);
        m_color = other.m_color;
    }
    return *this;
}

core::Color SolidBrush::GetColor() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_color;
}

void SolidBrush::SetColor(core::Color color) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_color = color;
}

std::unique_ptr<Brush> SolidBrush::Clone() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return std::make_unique<SolidBrush>(m_color);
}

// ==========================================
// LinearGradientBrush 實作
// ==========================================

LinearGradientBrush::LinearGradientBrush(core::Point p1, core::Point p2, core::Color c1, core::Color c2)
    : m_startPoint(p1), m_endPoint(p2), m_startColor(c1), m_endColor(c2) {}

LinearGradientBrush::LinearGradientBrush(const core::Rect& rect, core::Color c1, core::Color c2, float angleDegrees)
    : m_startColor(c1), m_endColor(c2) {
    float rad = angleDegrees * 3.14159265358979323846f / 180.0f;
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    float cx = rect.x + rect.width * 0.5f;
    float cy = rect.y + rect.height * 0.5f;
    float hx = rect.width * 0.5f;
    float hy = rect.height * 0.5f;

    m_startPoint = core::Point(cx - hx * cosA, cy - hy * sinA);
    m_endPoint   = core::Point(cx + hx * cosA, cy + hy * sinA);
}

LinearGradientBrush::LinearGradientBrush(const LinearGradientBrush& other) {
    compat::shared_lock<compat::shared_mutex> lock(other.m_mutex);
    m_startPoint = other.m_startPoint;
    m_endPoint   = other.m_endPoint;
    m_startColor = other.m_startColor;
    m_endColor   = other.m_endColor;
}

LinearGradientBrush::LinearGradientBrush(LinearGradientBrush&& other) noexcept {
    std::unique_lock<compat::shared_mutex> lock(other.m_mutex);
    m_startPoint = other.m_startPoint;
    m_endPoint   = other.m_endPoint;
    m_startColor = other.m_startColor;
    m_endColor   = other.m_endColor;
}

LinearGradientBrush& LinearGradientBrush::operator=(const LinearGradientBrush& other) {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        compat::shared_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);
        m_startPoint = other.m_startPoint;
        m_endPoint   = other.m_endPoint;
        m_startColor = other.m_startColor;
        m_endColor   = other.m_endColor;
    }
    return *this;
}

LinearGradientBrush& LinearGradientBrush::operator=(LinearGradientBrush&& other) noexcept {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::unique_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);
        m_startPoint = other.m_startPoint;
        m_endPoint   = other.m_endPoint;
        m_startColor = other.m_startColor;
        m_endColor   = other.m_endColor;
    }
    return *this;
}

core::Point LinearGradientBrush::GetStartPoint() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_startPoint;
}

void LinearGradientBrush::SetStartPoint(core::Point p) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_startPoint = p;
}

core::Point LinearGradientBrush::GetEndPoint() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_endPoint;
}

void LinearGradientBrush::SetEndPoint(core::Point p) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_endPoint = p;
}

core::Color LinearGradientBrush::GetStartColor() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_startColor;
}

void LinearGradientBrush::SetStartColor(core::Color c) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_startColor = c;
}

core::Color LinearGradientBrush::GetEndColor() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_endColor;
}

void LinearGradientBrush::SetEndColor(core::Color c) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_endColor = c;
}

std::unique_ptr<Brush> LinearGradientBrush::Clone() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return std::make_unique<LinearGradientBrush>(m_startPoint, m_endPoint, m_startColor, m_endColor);
}

/// <summary>
/// 計算二維座標點於此線性漸層定義下的色彩插值結果。
/// </summary>
/// <param name="pt">二維空間座標點。</param>
/// <returns>插值計算後之 Color。</returns>
core::Color LinearGradientBrush::GetColorAt(core::Point pt) const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    float dx = m_endPoint.x - m_startPoint.x;
    float dy = m_endPoint.y - m_startPoint.y;
    float lenSq = dx * dx + dy * dy;
    if (lenSq <= 0.00001f) {
        return m_startColor;
    }
    float t = ((pt.x - m_startPoint.x) * dx + (pt.y - m_startPoint.y) * dy) / lenSq;
    t = compat::clamp(t, 0.0f, 1.0f);

    uint8_t a = static_cast<uint8_t>(static_cast<float>(m_startColor.a) + t * (static_cast<float>(m_endColor.a) - static_cast<float>(m_startColor.a)));
    uint8_t r = static_cast<uint8_t>(static_cast<float>(m_startColor.r) + t * (static_cast<float>(m_endColor.r) - static_cast<float>(m_startColor.r)));
    uint8_t g = static_cast<uint8_t>(static_cast<float>(m_startColor.g) + t * (static_cast<float>(m_endColor.g) - static_cast<float>(m_startColor.g)));
    uint8_t b = static_cast<uint8_t>(static_cast<float>(m_startColor.b) + t * (static_cast<float>(m_endColor.b) - static_cast<float>(m_startColor.b)));
    return core::Color(r, g, b, a);
}

// ==========================================
// TextureBrush 實作
// ==========================================

TextureBrush::TextureBrush(std::shared_ptr<Bitmap> image)
    : m_image(image) {
    if (!image) {
        throw std::invalid_argument("TextureBrush image cannot be null.");
    }
}

TextureBrush::TextureBrush(const TextureBrush& other) {
    compat::shared_lock<compat::shared_mutex> lock(other.m_mutex);
    m_image = other.m_image;
}

TextureBrush::TextureBrush(TextureBrush&& other) noexcept {
    std::unique_lock<compat::shared_mutex> lock(other.m_mutex);
    m_image = std::move(other.m_image);
}

TextureBrush& TextureBrush::operator=(const TextureBrush& other) {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        compat::shared_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);
        m_image = other.m_image;
    }
    return *this;
}

TextureBrush& TextureBrush::operator=(TextureBrush&& other) noexcept {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::unique_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);
        m_image = std::move(other.m_image);
    }
    return *this;
}

std::shared_ptr<Bitmap> TextureBrush::GetImage() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_image;
}

std::unique_ptr<Brush> TextureBrush::Clone() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return std::make_unique<TextureBrush>(m_image);
}

} // namespace graphics
} // namespace cppui
