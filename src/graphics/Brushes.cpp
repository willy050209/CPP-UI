/**
 * @file Brushes.cpp
 * @brief GDI+ 對標之常用標準純色畫刷集合 (Stock Brushes) 實作
 */

#include "ui/graphics/Brushes.hpp"
#include "ui/graphics/Colors.hpp"

namespace cppui {
namespace graphics {

const SolidBrush& Brushes::Transparent() {
    static const SolidBrush brush(Colors::Transparent);
    return brush;
}

const SolidBrush& Brushes::Black() {
    static const SolidBrush brush(Colors::Black);
    return brush;
}

const SolidBrush& Brushes::White() {
    static const SolidBrush brush(Colors::White);
    return brush;
}

const SolidBrush& Brushes::Red() {
    static const SolidBrush brush(Colors::Red);
    return brush;
}

const SolidBrush& Brushes::Green() {
    static const SolidBrush brush(Colors::Green);
    return brush;
}

const SolidBrush& Brushes::Blue() {
    static const SolidBrush brush(Colors::Blue);
    return brush;
}

const SolidBrush& Brushes::Yellow() {
    static const SolidBrush brush(Colors::Yellow);
    return brush;
}

const SolidBrush& Brushes::Cyan() {
    static const SolidBrush brush(Colors::Cyan);
    return brush;
}

const SolidBrush& Brushes::Magenta() {
    static const SolidBrush brush(Colors::Magenta);
    return brush;
}

const SolidBrush& Brushes::Gray() {
    static const SolidBrush brush(Colors::Gray);
    return brush;
}

const SolidBrush& Brushes::LightGray() {
    static const SolidBrush brush(Colors::LightGray);
    return brush;
}

const SolidBrush& Brushes::DarkGray() {
    static const SolidBrush brush(Colors::DarkGray);
    return brush;
}

const SolidBrush& Brushes::Orange() {
    static const SolidBrush brush(Colors::Orange);
    return brush;
}

const SolidBrush& Brushes::Purple() {
    static const SolidBrush brush(Colors::Purple);
    return brush;
}

const SolidBrush& Brushes::Brown() {
    static const SolidBrush brush(Colors::Brown);
    return brush;
}

const SolidBrush& Brushes::SteelBlue() {
    static const SolidBrush brush(Colors::SteelBlue);
    return brush;
}

const SolidBrush& Brushes::Navy() {
    static const SolidBrush brush(Colors::Navy);
    return brush;
}

const SolidBrush& Brushes::Gold() {
    static const SolidBrush brush(Colors::Gold);
    return brush;
}

const SolidBrush& Brushes::Lime() {
    static const SolidBrush brush(Colors::Lime);
    return brush;
}

const SolidBrush& Brushes::Pink() {
    static const SolidBrush brush(Colors::Pink);
    return brush;
}

const SolidBrush& Brushes::DarkRed() {
    static const SolidBrush brush(Colors::DarkRed);
    return brush;
}

const SolidBrush& Brushes::DarkBlue() {
    static const SolidBrush brush(Colors::DarkBlue);
    return brush;
}

const SolidBrush& Brushes::DarkGreen() {
    static const SolidBrush brush(Colors::DarkGreen);
    return brush;
}

const SolidBrush& Brushes::Olive() {
    static const SolidBrush brush(Colors::Olive);
    return brush;
}

const SolidBrush& Brushes::Teal() {
    static const SolidBrush brush(Colors::Teal);
    return brush;
}

const SolidBrush& Brushes::Maroon() {
    static const SolidBrush brush(Colors::Maroon);
    return brush;
}

const SolidBrush& Brushes::Coral() {
    static const SolidBrush brush(Colors::Coral);
    return brush;
}

const SolidBrush& Brushes::Silver() {
    static const SolidBrush brush(Colors::Silver);
    return brush;
}

} // namespace graphics
} // namespace cppui
