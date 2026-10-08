/**
 * @file Colors.hpp
 * @brief GDI+ 對標之完整預定義常數調色盤
 */

#pragma once

#include "ui/core/Geometry.hpp"

namespace cppui {
namespace graphics {
namespace Colors {

using core::Color;

COMPAT_INLINE_VAR constexpr Color Transparent          = Color(0, 0, 0, 0);
COMPAT_INLINE_VAR constexpr Color AliceBlue            = Color(240, 248, 255, 255);
COMPAT_INLINE_VAR constexpr Color AntiqueWhite          = Color(250, 235, 215, 255);
COMPAT_INLINE_VAR constexpr Color Aqua                 = Color(0, 255, 255, 255);
COMPAT_INLINE_VAR constexpr Color Aquamarine           = Color(127, 255, 212, 255);
COMPAT_INLINE_VAR constexpr Color Azure                = Color(240, 255, 255, 255);
COMPAT_INLINE_VAR constexpr Color Beige                = Color(245, 245, 220, 255);
COMPAT_INLINE_VAR constexpr Color Bisque               = Color(255, 228, 196, 255);
COMPAT_INLINE_VAR constexpr Color Black                = Color(0, 0, 0, 255);
COMPAT_INLINE_VAR constexpr Color BlanchedAlmond       = Color(255, 235, 205, 255);
COMPAT_INLINE_VAR constexpr Color Blue                 = Color(0, 0, 255, 255);
COMPAT_INLINE_VAR constexpr Color BlueViolet           = Color(138, 43, 226, 255);
COMPAT_INLINE_VAR constexpr Color Brown                = Color(165, 42, 42, 255);
COMPAT_INLINE_VAR constexpr Color BurlyWood            = Color(222, 184, 135, 255);
COMPAT_INLINE_VAR constexpr Color CadetBlue            = Color(95, 158, 160, 255);
COMPAT_INLINE_VAR constexpr Color Chartreuse           = Color(127, 255, 0, 255);
COMPAT_INLINE_VAR constexpr Color Chocolate            = Color(210, 105, 30, 255);
COMPAT_INLINE_VAR constexpr Color Coral                = Color(255, 127, 80, 255);
COMPAT_INLINE_VAR constexpr Color CornflowerBlue       = Color(100, 149, 237, 255);
COMPAT_INLINE_VAR constexpr Color Cornsilk             = Color(255, 248, 220, 255);
COMPAT_INLINE_VAR constexpr Color Crimson              = Color(220, 20, 60, 255);
COMPAT_INLINE_VAR constexpr Color Cyan                 = Color(0, 255, 255, 255);
COMPAT_INLINE_VAR constexpr Color DarkBlue             = Color(0, 0, 139, 255);
COMPAT_INLINE_VAR constexpr Color DarkCyan             = Color(0, 139, 139, 255);
COMPAT_INLINE_VAR constexpr Color DarkGoldenrod        = Color(184, 134, 11, 255);
COMPAT_INLINE_VAR constexpr Color DarkGray             = Color(169, 169, 169, 255);
COMPAT_INLINE_VAR constexpr Color DarkGreen            = Color(0, 100, 0, 255);
COMPAT_INLINE_VAR constexpr Color DarkKhaki            = Color(189, 183, 107, 255);
COMPAT_INLINE_VAR constexpr Color DarkMagenta          = Color(139, 0, 139, 255);
COMPAT_INLINE_VAR constexpr Color DarkOliveGreen       = Color(85, 107, 47, 255);
COMPAT_INLINE_VAR constexpr Color DarkOrange           = Color(255, 140, 0, 255);
COMPAT_INLINE_VAR constexpr Color DarkOrchid           = Color(153, 50, 204, 255);
COMPAT_INLINE_VAR constexpr Color DarkRed              = Color(139, 0, 0, 255);
COMPAT_INLINE_VAR constexpr Color DarkSalmon           = Color(233, 150, 122, 255);
COMPAT_INLINE_VAR constexpr Color DarkSeaGreen         = Color(143, 188, 143, 255);
COMPAT_INLINE_VAR constexpr Color DarkSlateBlue        = Color(72, 61, 139, 255);
COMPAT_INLINE_VAR constexpr Color DarkSlateGray        = Color(47, 79, 79, 255);
COMPAT_INLINE_VAR constexpr Color DarkTurquoise        = Color(0, 206, 209, 255);
COMPAT_INLINE_VAR constexpr Color DarkViolet           = Color(148, 0, 211, 255);
COMPAT_INLINE_VAR constexpr Color DeepPink             = Color(255, 20, 147, 255);
COMPAT_INLINE_VAR constexpr Color DeepSkyBlue          = Color(0, 191, 255, 255);
COMPAT_INLINE_VAR constexpr Color DimGray              = Color(105, 105, 105, 255);
COMPAT_INLINE_VAR constexpr Color DodgerBlue           = Color(30, 144, 255, 255);
COMPAT_INLINE_VAR constexpr Color Firebrick            = Color(178, 34, 34, 255);
COMPAT_INLINE_VAR constexpr Color FloralWhite          = Color(255, 250, 240, 255);
COMPAT_INLINE_VAR constexpr Color ForestGreen          = Color(34, 139, 34, 255);
COMPAT_INLINE_VAR constexpr Color Fuchsia              = Color(255, 0, 255, 255);
COMPAT_INLINE_VAR constexpr Color Gainsboro            = Color(220, 220, 220, 255);
COMPAT_INLINE_VAR constexpr Color GhostWhite           = Color(248, 248, 255, 255);
COMPAT_INLINE_VAR constexpr Color Gold                 = Color(255, 215, 0, 255);
COMPAT_INLINE_VAR constexpr Color Goldenrod            = Color(218, 165, 32, 255);
COMPAT_INLINE_VAR constexpr Color Gray                 = Color(128, 128, 128, 255);
COMPAT_INLINE_VAR constexpr Color Green                = Color(0, 128, 0, 255);
COMPAT_INLINE_VAR constexpr Color GreenYellow          = Color(173, 255, 47, 255);
COMPAT_INLINE_VAR constexpr Color Honeydew             = Color(240, 255, 240, 255);
COMPAT_INLINE_VAR constexpr Color HotPink              = Color(255, 105, 180, 255);
COMPAT_INLINE_VAR constexpr Color IndianRed            = Color(205, 92, 92, 255);
COMPAT_INLINE_VAR constexpr Color Indigo               = Color(75, 0, 130, 255);
COMPAT_INLINE_VAR constexpr Color Ivory                = Color(255, 255, 240, 255);
COMPAT_INLINE_VAR constexpr Color Khaki                = Color(240, 230, 140, 255);
COMPAT_INLINE_VAR constexpr Color Lavender             = Color(230, 230, 250, 255);
COMPAT_INLINE_VAR constexpr Color LavenderBlush        = Color(255, 240, 245, 255);
COMPAT_INLINE_VAR constexpr Color LawnGreen            = Color(124, 252, 0, 255);
COMPAT_INLINE_VAR constexpr Color LemonChiffon         = Color(255, 250, 205, 255);
COMPAT_INLINE_VAR constexpr Color LightBlue            = Color(173, 216, 230, 255);
COMPAT_INLINE_VAR constexpr Color LightCoral           = Color(240, 128, 128, 255);
COMPAT_INLINE_VAR constexpr Color LightCyan            = Color(224, 255, 255, 255);
COMPAT_INLINE_VAR constexpr Color LightGoldenrodYellow = Color(250, 250, 210, 255);
COMPAT_INLINE_VAR constexpr Color LightGray            = Color(211, 211, 211, 255);
COMPAT_INLINE_VAR constexpr Color LightGreen           = Color(144, 238, 144, 255);
COMPAT_INLINE_VAR constexpr Color LightPink            = Color(255, 182, 193, 255);
COMPAT_INLINE_VAR constexpr Color LightSalmon          = Color(255, 160, 122, 255);
COMPAT_INLINE_VAR constexpr Color LightSeaGreen        = Color(32, 178, 170, 255);
COMPAT_INLINE_VAR constexpr Color LightSkyBlue         = Color(135, 206, 250, 255);
COMPAT_INLINE_VAR constexpr Color LightSlateGray       = Color(119, 136, 153, 255);
COMPAT_INLINE_VAR constexpr Color LightSteelBlue       = Color(176, 196, 222, 255);
COMPAT_INLINE_VAR constexpr Color LightYellow          = Color(255, 255, 224, 255);
COMPAT_INLINE_VAR constexpr Color Lime                 = Color(0, 255, 0, 255);
COMPAT_INLINE_VAR constexpr Color LimeGreen            = Color(50, 205, 50, 255);
COMPAT_INLINE_VAR constexpr Color Linen                = Color(250, 240, 230, 255);
COMPAT_INLINE_VAR constexpr Color Magenta              = Color(255, 0, 255, 255);
COMPAT_INLINE_VAR constexpr Color Maroon               = Color(128, 0, 0, 255);
COMPAT_INLINE_VAR constexpr Color MediumAquamarine     = Color(102, 205, 170, 255);
COMPAT_INLINE_VAR constexpr Color MediumBlue           = Color(0, 0, 205, 255);
COMPAT_INLINE_VAR constexpr Color MediumOrchid         = Color(186, 85, 211, 255);
COMPAT_INLINE_VAR constexpr Color MediumPurple         = Color(147, 112, 219, 255);
COMPAT_INLINE_VAR constexpr Color MediumSeaGreen       = Color(60, 179, 113, 255);
COMPAT_INLINE_VAR constexpr Color MediumSlateBlue      = Color(123, 104, 238, 255);
COMPAT_INLINE_VAR constexpr Color MediumSpringGreen    = Color(0, 250, 154, 255);
COMPAT_INLINE_VAR constexpr Color MediumTurquoise      = Color(72, 209, 204, 255);
COMPAT_INLINE_VAR constexpr Color MediumVioletRed      = Color(199, 21, 133, 255);
COMPAT_INLINE_VAR constexpr Color MidnightBlue         = Color(25, 25, 112, 255);
COMPAT_INLINE_VAR constexpr Color MintCream            = Color(245, 255, 250, 255);
COMPAT_INLINE_VAR constexpr Color MistyRose            = Color(255, 228, 225, 255);
COMPAT_INLINE_VAR constexpr Color Moccasin             = Color(255, 228, 181, 255);
COMPAT_INLINE_VAR constexpr Color NavajoWhite          = Color(255, 222, 173, 255);
COMPAT_INLINE_VAR constexpr Color Navy                 = Color(0, 0, 128, 255);
COMPAT_INLINE_VAR constexpr Color OldLace              = Color(253, 245, 230, 255);
COMPAT_INLINE_VAR constexpr Color Olive                = Color(128, 128, 0, 255);
COMPAT_INLINE_VAR constexpr Color OliveDrab            = Color(107, 142, 35, 255);
COMPAT_INLINE_VAR constexpr Color Orange               = Color(255, 165, 0, 255);
COMPAT_INLINE_VAR constexpr Color OrangeRed            = Color(255, 69, 0, 255);
COMPAT_INLINE_VAR constexpr Color Orchid               = Color(218, 112, 214, 255);
COMPAT_INLINE_VAR constexpr Color PaleGoldenrod        = Color(238, 232, 170, 255);
COMPAT_INLINE_VAR constexpr Color PaleGreen            = Color(152, 251, 152, 255);
COMPAT_INLINE_VAR constexpr Color PaleTurquoise        = Color(175, 238, 238, 255);
COMPAT_INLINE_VAR constexpr Color PaleVioletRed        = Color(219, 112, 147, 255);
COMPAT_INLINE_VAR constexpr Color PapayaWhip           = Color(255, 239, 213, 255);
COMPAT_INLINE_VAR constexpr Color PeachPuff            = Color(255, 218, 185, 255);
COMPAT_INLINE_VAR constexpr Color Peru                 = Color(205, 133, 63, 255);
COMPAT_INLINE_VAR constexpr Color Pink                 = Color(255, 192, 203, 255);
COMPAT_INLINE_VAR constexpr Color Plum                 = Color(221, 160, 221, 255);
COMPAT_INLINE_VAR constexpr Color PowderBlue           = Color(176, 224, 230, 255);
COMPAT_INLINE_VAR constexpr Color Purple               = Color(128, 0, 128, 255);
COMPAT_INLINE_VAR constexpr Color Red                  = Color(255, 0, 0, 255);
COMPAT_INLINE_VAR constexpr Color RosyBrown            = Color(188, 143, 143, 255);
COMPAT_INLINE_VAR constexpr Color RoyalBlue            = Color(65, 105, 225, 255);
COMPAT_INLINE_VAR constexpr Color SaddleBrown          = Color(139, 69, 19, 255);
COMPAT_INLINE_VAR constexpr Color Salmon               = Color(250, 128, 114, 255);
COMPAT_INLINE_VAR constexpr Color SandyBrown           = Color(244, 164, 96, 255);
COMPAT_INLINE_VAR constexpr Color SeaGreen             = Color(46, 139, 87, 255);
COMPAT_INLINE_VAR constexpr Color SeaShell             = Color(255, 245, 238, 255);
COMPAT_INLINE_VAR constexpr Color Sienna               = Color(160, 82, 45, 255);
COMPAT_INLINE_VAR constexpr Color Silver               = Color(192, 192, 192, 255);
COMPAT_INLINE_VAR constexpr Color SkyBlue              = Color(135, 206, 235, 255);
COMPAT_INLINE_VAR constexpr Color SlateBlue            = Color(106, 90, 205, 255);
COMPAT_INLINE_VAR constexpr Color SlateGray            = Color(112, 128, 144, 255);
COMPAT_INLINE_VAR constexpr Color Snow                 = Color(255, 250, 250, 255);
COMPAT_INLINE_VAR constexpr Color SpringGreen          = Color(0, 255, 127, 255);
COMPAT_INLINE_VAR constexpr Color SteelBlue            = Color(70, 130, 180, 255);
COMPAT_INLINE_VAR constexpr Color Tan                  = Color(210, 180, 140, 255);
COMPAT_INLINE_VAR constexpr Color Teal                 = Color(0, 128, 128, 255);
COMPAT_INLINE_VAR constexpr Color Thistle              = Color(216, 191, 216, 255);
COMPAT_INLINE_VAR constexpr Color Tomato               = Color(255, 99, 71, 255);
COMPAT_INLINE_VAR constexpr Color Turquoise            = Color(64, 224, 208, 255);
COMPAT_INLINE_VAR constexpr Color Violet               = Color(238, 130, 238, 255);
COMPAT_INLINE_VAR constexpr Color Wheat                = Color(245, 222, 179, 255);
COMPAT_INLINE_VAR constexpr Color White                = Color(255, 255, 255, 255);
COMPAT_INLINE_VAR constexpr Color WhiteSmoke           = Color(245, 245, 245, 255);
COMPAT_INLINE_VAR constexpr Color Yellow               = Color(255, 255, 0, 255);
COMPAT_INLINE_VAR constexpr Color YellowGreen          = Color(154, 205, 50, 255);

} // namespace Colors
} // namespace graphics
} // namespace cppui
