/**
 * @file TextLayoutEngine.hpp
 * @brief 高階多行文字排版求解器，處理斷行、修剪、對齊與尺寸測量
 */

#pragma once

#include <memory>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 高階多行排版求解器 (TextLayoutEngine)
/// </summary>
class TextLayoutEngine {
public:
    /// <summary>取得排版引擎全域單例引用。</summary>
    /// <returns>TextLayoutEngine 實例引用。</returns>
    static TextLayoutEngine& Get();

    /// <summary>建構排版引擎實例。</summary>
    TextLayoutEngine();

    /// <summary>解構排版引擎實例。</summary>
    ~TextLayoutEngine();

    // 禁止複製
    TextLayoutEngine(const TextLayoutEngine&) = delete;
    TextLayoutEngine& operator=(const TextLayoutEngine&) = delete;

    /// <summary>
    /// 依據字串、字型與排版約束建立完整多行 TextLayout 物件。
    /// </summary>
    /// <param name="text">待排版文字視圖。</param>
    /// <param name="font">字型描述項。</param>
    /// <param name="maxWidth">最大可用寬度約束。</param>
    /// <param name="maxHeight">最大可用高度約束。</param>
    /// <param name="alignment">水平對齊方式。</param>
    /// <param name="wrapping">文字折行策略。</param>
    /// <param name="trimming">文字修剪省略策略。</param>
    /// <returns>排版成果 TextLayout 物件。</returns>
    COMPAT_NODISCARD TextLayout CreateLayout(
        compat::string_view text,
        const FontDescriptor& font,
        float maxWidth = 1e9f,
        float maxHeight = 1e9f,
        TextAlignment alignment = TextAlignment::Left,
        TextWrapping wrapping = TextWrapping::WordWrap,
        TextTrimming trimming = TextTrimming::None
    ) const;

    /// <summary>
    /// 快速文字尺寸度量 (提供 M4 Measure/Arrange 階段高速查詢)。
    /// </summary>
    /// <param name="text">待度量文字視圖。</param>
    /// <param name="font">字型描述項。</param>
    /// <param name="maxWidth">最大可用寬度約束。</param>
    /// <returns>文字所需 Size 寬高尺寸。</returns>
    COMPAT_NODISCARD core::Size MeasureText(
        compat::string_view text,
        const FontDescriptor& font,
        float maxWidth = 1e9f
    ) const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace text
} // namespace cppui
