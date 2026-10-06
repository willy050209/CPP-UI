#pragma once

#include <memory>
#include "compat/compat.hpp"
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
    static TextLayoutEngine& Get();

    TextLayoutEngine();
    ~TextLayoutEngine();

    // 禁止複製
    TextLayoutEngine(const TextLayoutEngine&) = delete;
    TextLayoutEngine& operator=(const TextLayoutEngine&) = delete;

    /// <summary>
    /// 依據字串、字型與排版約束建立完整多行 TextLayout 物件
    /// </summary>
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
    /// 快速文字尺寸度量 (提供 M4 Measure/Arrange 階段高速查詢)
    /// </summary>
    COMPAT_NODISCARD core::Size MeasureText(
        compat::string_view text,
        const FontDescriptor& font,
        float maxWidth = 1e9f
    ) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
