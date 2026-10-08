/**
 * @file Pens.cpp
 * @brief GDI+ 對標之常用標準畫筆集合 (Stock Pens) 實作
 */

#include "ui/graphics/Pens.hpp"
#include "ui/graphics/Colors.hpp"

namespace cppui {
namespace graphics {

const Pen& Pens::Transparent() {
    static const Pen pen(Colors::Transparent, 1.0f);
    return pen;
}

const Pen& Pens::Black() {
    static const Pen pen(Colors::Black, 1.0f);
    return pen;
}

const Pen& Pens::White() {
    static const Pen pen(Colors::White, 1.0f);
    return pen;
}

const Pen& Pens::Red() {
    static const Pen pen(Colors::Red, 1.0f);
    return pen;
}

const Pen& Pens::Green() {
    static const Pen pen(Colors::Green, 1.0f);
    return pen;
}

const Pen& Pens::Blue() {
    static const Pen pen(Colors::Blue, 1.0f);
    return pen;
}

const Pen& Pens::Yellow() {
    static const Pen pen(Colors::Yellow, 1.0f);
    return pen;
}

const Pen& Pens::Cyan() {
    static const Pen pen(Colors::Cyan, 1.0f);
    return pen;
}

const Pen& Pens::Magenta() {
    static const Pen pen(Colors::Magenta, 1.0f);
    return pen;
}

const Pen& Pens::Gray() {
    static const Pen pen(Colors::Gray, 1.0f);
    return pen;
}

const Pen& Pens::LightGray() {
    static const Pen pen(Colors::LightGray, 1.0f);
    return pen;
}

const Pen& Pens::DarkGray() {
    static const Pen pen(Colors::DarkGray, 1.0f);
    return pen;
}

const Pen& Pens::Orange() {
    static const Pen pen(Colors::Orange, 1.0f);
    return pen;
}

const Pen& Pens::Purple() {
    static const Pen pen(Colors::Purple, 1.0f);
    return pen;
}

const Pen& Pens::Brown() {
    static const Pen pen(Colors::Brown, 1.0f);
    return pen;
}

const Pen& Pens::SteelBlue() {
    static const Pen pen(Colors::SteelBlue, 1.0f);
    return pen;
}

const Pen& Pens::Navy() {
    static const Pen pen(Colors::Navy, 1.0f);
    return pen;
}

const Pen& Pens::Gold() {
    static const Pen pen(Colors::Gold, 1.0f);
    return pen;
}

const Pen& Pens::Lime() {
    static const Pen pen(Colors::Lime, 1.0f);
    return pen;
}

const Pen& Pens::Pink() {
    static const Pen pen(Colors::Pink, 1.0f);
    return pen;
}

const Pen& Pens::DarkRed() {
    static const Pen pen(Colors::DarkRed, 1.0f);
    return pen;
}

const Pen& Pens::DarkBlue() {
    static const Pen pen(Colors::DarkBlue, 1.0f);
    return pen;
}

const Pen& Pens::DarkGreen() {
    static const Pen pen(Colors::DarkGreen, 1.0f);
    return pen;
}

const Pen& Pens::Olive() {
    static const Pen pen(Colors::Olive, 1.0f);
    return pen;
}

const Pen& Pens::Teal() {
    static const Pen pen(Colors::Teal, 1.0f);
    return pen;
}

const Pen& Pens::Maroon() {
    static const Pen pen(Colors::Maroon, 1.0f);
    return pen;
}

const Pen& Pens::Coral() {
    static const Pen pen(Colors::Coral, 1.0f);
    return pen;
}

const Pen& Pens::Silver() {
    static const Pen pen(Colors::Silver, 1.0f);
    return pen;
}

} // namespace graphics
} // namespace cppui
