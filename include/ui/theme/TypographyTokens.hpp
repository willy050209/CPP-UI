/*
 * File: TypographyTokens.hpp
 * Purpose: Declares typographic hierarchy and font specification tokens for CPPUI.
 */

#pragma once

#include <string>
#include "compat/Compat.hpp"
#include "ui/text/Font.hpp"

namespace cppui {
namespace theme {

/// <summary>
/// 集中式排版與字型代幣結構 (TypographyTokens)
/// 定義全域字型家族、等寬家族與階層式字級規範。
/// </summary>
struct TypographyTokens {
    std::string fontFamily = "Segoe UI";
    std::string fontFamilyMonospace = "Consolas";

    // 字級大小比例尺 (pt / DIP)
    float fontSizeCaption = 11.0f;
    float fontSizeBody = 14.0f;
    float fontSizeSubtitle = 16.0f;
    float fontSizeTitle = 20.0f;
    float fontSizeHeader = 24.0f;

    // 字重常數
    text::FontWeight fontWeightRegular = text::FontWeight::Regular;
    text::FontWeight fontWeightMedium = text::FontWeight::Medium;
    text::FontWeight fontWeightSemiBold = text::FontWeight::SemiBold;
    text::FontWeight fontWeightBold = text::FontWeight::Bold;
};

} // namespace theme
} // namespace cppui
