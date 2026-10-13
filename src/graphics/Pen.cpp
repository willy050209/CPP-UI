/**
 * @file Pen.cpp
 * @brief GDI+ 風格幾何畫筆實作，具備執行緒安全自動同步
 */

#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Brush.hpp"
#include <stdexcept>

namespace cppui {
namespace graphics {

Pen::Pen(core::Color color, float width)
    : m_color(color), m_width(width) {
    if (width < 0.0f) {
        throw std::invalid_argument("Pen width cannot be negative.");
    }
}

Pen::Pen(const Brush& brush, float width)
    : m_width(width) {
    if (width < 0.0f) {
        throw std::invalid_argument("Pen width cannot be negative.");
    }
    if (brush.GetType() == BrushType::SolidColor) {
        const auto& sb = static_cast<const SolidBrush&>(brush);
        m_color = sb.GetColor();
    } else if (brush.GetType() == BrushType::LinearGradient) {
        const auto& lb = static_cast<const LinearGradientBrush&>(brush);
        m_color = lb.GetStartColor();
    } else {
        m_color = core::Color(0, 0, 0, 255);
    }
}

Pen::Pen(const Pen& other) {
    compat::shared_lock<compat::shared_mutex> lock(other.m_mutex);
    m_color     = other.m_color;
    m_width     = other.m_width;
    m_dashStyle = other.m_dashStyle;
    m_lineCap   = other.m_lineCap;
    m_lineJoin  = other.m_lineJoin;
}

Pen::Pen(Pen&& other) noexcept {
    std::unique_lock<compat::shared_mutex> lock(other.m_mutex);
    m_color     = other.m_color;
    m_width     = other.m_width;
    m_dashStyle = other.m_dashStyle;
    m_lineCap   = other.m_lineCap;
    m_lineJoin  = other.m_lineJoin;
}

Pen& Pen::operator=(const Pen& other) {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        compat::shared_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);

        m_color     = other.m_color;
        m_width     = other.m_width;
        m_dashStyle = other.m_dashStyle;
        m_lineCap   = other.m_lineCap;
        m_lineJoin  = other.m_lineJoin;
    }
    return *this;
}

Pen& Pen::operator=(Pen&& other) noexcept {
    if (this != &other) {
        std::unique_lock<compat::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::unique_lock<compat::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);

        m_color     = other.m_color;
        m_width     = other.m_width;
        m_dashStyle = other.m_dashStyle;
        m_lineCap   = other.m_lineCap;
        m_lineJoin  = other.m_lineJoin;
    }
    return *this;
}

core::Color Pen::GetColor() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_color;
}

void Pen::SetColor(core::Color color) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_color = color;
}

float Pen::GetWidth() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_width;
}

void Pen::SetWidth(float width) {
    if (width < 0.0f) {
        throw std::invalid_argument("Pen width cannot be negative.");
    }
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_width = width;
}

DashStyle Pen::GetDashStyle() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_dashStyle;
}

void Pen::SetDashStyle(DashStyle style) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_dashStyle = style;
}

LineCap Pen::GetLineCap() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_lineCap;
}

void Pen::SetLineCap(LineCap cap) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_lineCap = cap;
}

LineJoin Pen::GetLineJoin() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    return m_lineJoin;
}

void Pen::SetLineJoin(LineJoin join) {
    std::unique_lock<compat::shared_mutex> lock(m_mutex);
    m_lineJoin = join;
}

Pen Pen::Clone() const {
    compat::shared_lock<compat::shared_mutex> lock(m_mutex);
    Pen result(m_color, m_width);
    result.SetDashStyle(m_dashStyle);
    result.SetLineCap(m_lineCap);
    result.SetLineJoin(m_lineJoin);
    return result;
}

} // namespace graphics
} // namespace cppui
