/**
 * @file TextBlock.hpp
 * @brief 高效能文字排版與顯示元件，橋接文字排版管線並支援字型、對齊、折行與裁切
 */

#pragma once

#include <string>
#include "compat/compat.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {

namespace theme {
class Theme;
} // namespace theme

namespace controls {

/// <summary>
/// 高效能文字排版與顯示元件 (TextBlock)
/// 橋接 M3 文字排版管線，支援多行折行、字型變更與精確邊界度量。
/// </summary>
class TextBlock : public core::UIElement {
public:
    /// <summary>
    /// 建構空白文字區塊元件。
    /// </summary>
    TextBlock();

    /// <summary>
    /// 建構具備初始文字之文字區塊元件。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    explicit TextBlock(compat::string_view text);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~TextBlock() override = default;

    /// <summary>
    /// 取得當前顯示文字視圖。
    /// </summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetText() const noexcept { return m_text; }

    /// <summary>
    /// 設定顯示文字內容。
    /// </summary>
    /// <param name="text">欲顯示之文字字串視圖。</param>
    void SetText(compat::string_view text);

    /// <summary>
    /// 取得字型描述項常數引用。
    /// </summary>
    /// <returns>FontDescriptor 引用。</returns>
    COMPAT_NODISCARD const text::FontDescriptor& GetFont() const noexcept { return m_font; }

    /// <summary>
    /// 設定字型描述項。
    /// </summary>
    /// <param name="font">FontDescriptor 物件。</param>
    void SetFont(const text::FontDescriptor& font);

    /// <summary>
    /// 取得字型大小 (pt/px)。
    /// </summary>
    /// <returns>字型大小數值。</returns>
    COMPAT_NODISCARD float GetFontSize() const noexcept { return m_font.GetSize(); }

    /// <summary>
    /// 設定字型大小。
    /// </summary>
    /// <param name="size">字型大小數值。</param>
    void SetFontSize(float size);

    /// <summary>
    /// 取得字型家族名稱。
    /// </summary>
    /// <returns>字型家族字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetFontFamily() const noexcept { return m_font.GetFamily(); }

    /// <summary>
    /// 設定字型家族名稱。
    /// </summary>
    /// <param name="family">字型家族字串視圖。</param>
    void SetFontFamily(compat::string_view family);

    /// <summary>
    /// 取得字型粗細。
    /// </summary>
    /// <returns>FontWeight 列舉值。</returns>
    COMPAT_NODISCARD text::FontWeight GetFontWeight() const noexcept { return m_font.GetWeight(); }

    /// <summary>
    /// 設定字型粗細。
    /// </summary>
    /// <param name="weight">FontWeight 列舉值。</param>
    void SetFontWeight(text::FontWeight weight);

    /// <summary>
    /// 取得字型樣式 (常規/斜體)。
    /// </summary>
    /// <returns>FontStyle 列舉值。</returns>
    COMPAT_NODISCARD text::FontStyle GetFontStyle() const noexcept { return m_font.GetStyle(); }

    /// <summary>
    /// 設定字型樣式。
    /// </summary>
    /// <param name="style">FontStyle 列舉值。</param>
    void SetFontStyle(text::FontStyle style);

    /// <summary>
    /// 取得文字前景色彩。
    /// </summary>
    /// <returns>前景色彩。</returns>
    COMPAT_NODISCARD core::Color GetForeground() const noexcept { return m_foreground; }

    /// <summary>
    /// 設定文字前景色彩。
    /// </summary>
    /// <param name="foreground">前景色彩。</param>
    void SetForeground(core::Color foreground);

    /// <summary>
    /// 取得文字折行模式。
    /// </summary>
    /// <returns>TextWrapping 列舉值。</returns>
    COMPAT_NODISCARD text::TextWrapping GetTextWrapping() const noexcept { return m_wrapping; }

    /// <summary>
    /// 設定文字折行模式。
    /// </summary>
    /// <param name="wrapping">TextWrapping 列舉值。</param>
    void SetTextWrapping(text::TextWrapping wrapping);

    /// <summary>
    /// 取得文字對齊方式。
    /// </summary>
    /// <returns>TextAlignment 列舉值。</returns>
    COMPAT_NODISCARD text::TextAlignment GetTextAlignment() const noexcept { return m_alignment; }

    /// <summary>
    /// 設定文字對齊方式。
    /// </summary>
    /// <param name="alignment">TextAlignment 列舉值。</param>
    void SetTextAlignment(text::TextAlignment alignment);

    /// <summary>
    /// 取得文字裁切省略模式。
    /// </summary>
    /// <returns>TextTrimming 列舉值。</returns>
    COMPAT_NODISCARD text::TextTrimming GetTextTrimming() const noexcept { return m_trimming; }

    /// <summary>
    /// 設定文字裁切省略模式。
    /// </summary>
    /// <param name="trimming">TextTrimming 列舉值。</param>
    void SetTextTrimming(text::TextTrimming trimming);

    /// <summary>
    /// 覆寫尺寸度量，呼叫排版引擎計算文字所需邊界大小。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>文字所需期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製文字佈局至繪圖上下文。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至文字區塊。
    /// </summary>
    /// <param name="theme">欲套用之 Theme 常數引用。</param>
    void ApplyTheme(const theme::Theme& theme);

private:
    std::string           m_text;
    text::FontDescriptor  m_font;
    core::Color           m_foreground{ 255, 255, 255, 255 };
    text::TextWrapping    m_wrapping = text::TextWrapping::WordWrap;
    text::TextAlignment   m_alignment = text::TextAlignment::Left;
    text::TextTrimming    m_trimming = text::TextTrimming::None;

    text::TextLayout      m_textLayout;
    bool                  m_isLayoutDirty = true;
    bool                  m_hasExplicitForeground = false;
    bool                  m_hasExplicitFont = false;
};

} // namespace controls
} // namespace cppui
