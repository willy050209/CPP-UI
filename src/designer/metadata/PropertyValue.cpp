/**
 * @file PropertyValue.cpp
 * @brief PropertyValue 變體型別名稱查詢與字串轉換實作
 */

#include "ui/designer/metadata/PropertyValue.hpp"
#include <sstream>
#include <iomanip>

namespace cppui {
namespace designer {

compat::string_view PropertyValue::GetTypeName() const noexcept {
    switch (m_value.index()) {
        case 0:  return "None";
        case 1:  return "Boolean";
        case 2:  return "Int32";
        case 3:  return "Int64";
        case 4:  return "Double";
        case 5:  return "String";
        case 6:  return "Point";
        case 7:  return "Size";
        case 8:  return "Rect";
        case 9:  return "Thickness";
        case 10: return "HorizontalAlignment";
        case 11: return "VerticalAlignment";
        case 12: return "Orientation";
        case 13: return "Visibility";
        case 14: return "Color";
        case 15: return "Brush";
        case 16: return "Font";
        case 17: return "ThemeTokenRef";
        default: return "Unknown";
    }
}

std::string PropertyValue::ToString() const {
    if (IsEmpty()) {
        return "";
    }
    if (const bool* b = std::get_if<bool>(&m_value)) {
        return *b ? "true" : "false";
    }
    if (const int32_t* i32 = std::get_if<int32_t>(&m_value)) {
        return std::to_string(*i32);
    }
    if (const int64_t* i64 = std::get_if<int64_t>(&m_value)) {
        return std::to_string(*i64);
    }
    if (const double* d = std::get_if<double>(&m_value)) {
        std::ostringstream ss;
        ss << *d;
        return ss.str();
    }
    if (const std::string* s = std::get_if<std::string>(&m_value)) {
        return *s;
    }
    if (const core::Point* pt = std::get_if<core::Point>(&m_value)) {
        std::ostringstream ss;
        ss << pt->x << "," << pt->y;
        return ss.str();
    }
    if (const core::Size* sz = std::get_if<core::Size>(&m_value)) {
        std::ostringstream ss;
        ss << sz->width << "," << sz->height;
        return ss.str();
    }
    if (const core::Rect* rc = std::get_if<core::Rect>(&m_value)) {
        std::ostringstream ss;
        ss << rc->x << "," << rc->y << "," << rc->width << "," << rc->height;
        return ss.str();
    }
    if (const core::Thickness* th = std::get_if<core::Thickness>(&m_value)) {
        std::ostringstream ss;
        ss << th->left << "," << th->top << "," << th->right << "," << th->bottom;
        return ss.str();
    }
    if (const layout::HorizontalAlignment* ha = std::get_if<layout::HorizontalAlignment>(&m_value)) {
        switch (*ha) {
            case layout::HorizontalAlignment::Left:    return "Left";
            case layout::HorizontalAlignment::Center:  return "Center";
            case layout::HorizontalAlignment::Right:   return "Right";
            case layout::HorizontalAlignment::Stretch: return "Stretch";
        }
    }
    if (const layout::VerticalAlignment* va = std::get_if<layout::VerticalAlignment>(&m_value)) {
        switch (*va) {
            case layout::VerticalAlignment::Top:     return "Top";
            case layout::VerticalAlignment::Center:  return "Center";
            case layout::VerticalAlignment::Bottom:  return "Bottom";
            case layout::VerticalAlignment::Stretch: return "Stretch";
        }
    }
    if (const layout::Orientation* o = std::get_if<layout::Orientation>(&m_value)) {
        return (*o == layout::Orientation::Horizontal) ? "Horizontal" : "Vertical";
    }
    if (const layout::Visibility* v = std::get_if<layout::Visibility>(&m_value)) {
        switch (*v) {
            case layout::Visibility::Visible:   return "Visible";
            case layout::Visibility::Hidden:    return "Hidden";
            case layout::Visibility::Collapsed: return "Collapsed";
        }
    }
    if (const core::Color* c = std::get_if<core::Color>(&m_value)) {
        std::ostringstream ss;
        ss << "#"
           << std::hex << std::setfill('0')
           << std::setw(2) << static_cast<uint32_t>(c->a)
           << std::setw(2) << static_cast<uint32_t>(c->r)
           << std::setw(2) << static_cast<uint32_t>(c->g)
           << std::setw(2) << static_cast<uint32_t>(c->b);
        return ss.str();
    }
    if (const BrushDescriptor* brush = std::get_if<BrushDescriptor>(&m_value)) {
        if (brush->type == BrushDescriptorType::Solid) {
            std::ostringstream ss;
            ss << "Solid(#"
               << std::hex << std::setfill('0')
               << std::setw(2) << static_cast<uint32_t>(brush->color.a)
               << std::setw(2) << static_cast<uint32_t>(brush->color.r)
               << std::setw(2) << static_cast<uint32_t>(brush->color.g)
               << std::setw(2) << static_cast<uint32_t>(brush->color.b) << ")";
            return ss.str();
        }
        return "LinearGradient";
    }
    if (const text::FontDescriptor* font = std::get_if<text::FontDescriptor>(&m_value)) {
        std::ostringstream ss;
        ss << font->GetFamily() << "," << font->GetSize();
        return ss.str();
    }
    if (const ThemeTokenRef* tok = std::get_if<ThemeTokenRef>(&m_value)) {
        return std::string("@Theme:") + tok->tokenName;
    }
    return "";
}

} // namespace designer
} // namespace cppui
