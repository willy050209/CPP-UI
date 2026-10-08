/**
 * @file TextBox.hpp
 * @brief 單行文字編輯輸入框，支援鍵盤互動、游標定位、提示字與文字變更事件
 */

#pragma once

#include <string>
#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 單行文字編輯輸入框 (TextBox)
/// 支援滑鼠點擊獲焦定位、鍵盤即時輸入、退格刪除、方向鍵游標移動、佔位提示字 (Placeholder) 與焦點外框光暈。
/// </summary>
class TextBox : public Control {
public:
    static const input::RoutedEvent TextChangedEvent;

    /// <summary>
    /// 建構空白文字編輯輸入框。
    /// </summary>
    TextBox();

    /// <summary>
    /// 建構具備提示文字之文字編輯輸入框。
    /// </summary>
    /// <param name="placeholder">佔位提示字串視圖。</param>
    explicit TextBox(compat::string_view placeholder);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~TextBox() override = default;

    /// <summary>
    /// 取得當前輸入之文字視圖。
    /// </summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetText() const noexcept override { return m_text; }

    /// <summary>
    /// 設定文字編輯輸入框之文字內容。
    /// </summary>
    /// <param name="text">欲設定之字串視圖。</param>
    void SetText(compat::string_view text) override;

    /// <summary>
    /// 於當前游標位置插入文字。
    /// </summary>
    /// <param name="text">欲插入之字串視圖。</param>
    void InsertText(compat::string_view text);

    /// <summary>
    /// 取得佔位提示字串視圖。
    /// </summary>
    /// <returns>提示字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetPlaceholder() const noexcept { return m_placeholder; }

    /// <summary>
    /// 設定佔位提示字串。
    /// </summary>
    /// <param name="placeholder">佔位提示字串視圖。</param>
    void SetPlaceholder(compat::string_view placeholder);

    /// <summary>
    /// 取得文字游標所在之字元索引。
    /// </summary>
    /// <returns>游標字元索引。</returns>
    COMPAT_NODISCARD size_t GetCaretIndex() const noexcept { return m_caretIndex; }

    /// <summary>
    /// 設定文字游標所在之字元索引。
    /// </summary>
    /// <param name="index">游標字元索引。</param>
    void SetCaretIndex(size_t index);

    /// <summary>
    /// 取得字型大小。
    /// </summary>
    /// <returns>字型大小像素值。</returns>
    COMPAT_NODISCARD float GetFontSize() const noexcept { return m_font.GetSize(); }

    /// <summary>
    /// 設定字型大小。
    /// </summary>
    /// <param name="size">字型大小像素值。</param>
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
    /// 取得佔位提示字色彩。
    /// </summary>
    /// <returns>提示字色彩。</returns>
    COMPAT_NODISCARD core::Color GetPlaceholderColor() const noexcept { return m_placeholderColor; }

    /// <summary>
    /// 設定佔位提示字色彩。
    /// </summary>
    /// <param name="color">提示字色彩。</param>
    void SetPlaceholderColor(core::Color color);

    /// <summary>
    /// 註冊文字變更回呼函式。
    /// </summary>
    /// <param name="handler">文字變更處理常式。</param>
    void OnTextChanged(std::function<void(compat::string_view)> handler);

    /// <summary>
    /// 覆寫排版測量階段，計算包含內距與邊框之外觀尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>元件期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製文字輸入框背景、邊框、提示字/編輯文字與閃爍游標。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

private:
    void InvalidateLayout();
    void EnsureLayout();
    void RaiseTextChanged();
    float GetCaretXOffset() const;

    std::string          m_text;
    std::string          m_placeholder;
    size_t               m_caretIndex = 0;

    text::FontDescriptor m_font;
    core::Color          m_placeholderColor{ 130, 145, 165, 255 };
    core::Color          m_caretColor{ 240, 245, 255, 255 };
    core::Color          m_focusBorderColor{ 0, 160, 255, 255 };

    text::TextLayout     m_textLayout;
    text::TextLayout     m_placeholderLayout;
    bool                 m_isLayoutDirty = true;
};

} // namespace controls
} // namespace cppui
