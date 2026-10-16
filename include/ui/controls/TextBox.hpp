/**
 * @file TextBox.hpp
 * @brief 單行文字編輯輸入框，支援鍵盤互動、游標定位、提示字與文字變更事件
 */

#pragma once

#include <string>
#include <functional>
#include "compat/Compat.hpp"
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
    ~TextBox() override;

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
    /// 取得文字編輯輸入框內部游標或組字游標之邊界矩形 (相對於本元件局部座標)。
    /// </summary>
    /// <returns>游標局部矩形。</returns>
    COMPAT_NODISCARD core::Rect GetCaretRect() const;

    /// <summary>
    /// 取得目前 IME 組字字串。
    /// </summary>
    /// <returns>組字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetCompositionText() const noexcept { return m_compositionText; }

    /// <summary>
    /// 取得目前 IME 組字游標在組字串中的字元偏移量。
    /// </summary>
    /// <returns>組字游標字元偏移。</returns>
    COMPAT_NODISCARD int32_t GetCompositionCursor() const noexcept { return m_compositionCursor; }

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
    /// 檢查文字輸入框是否處於唯讀狀態 (ReadOnly)。
    /// </summary>
    /// <returns>若為唯讀傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsReadOnly() const noexcept { return m_isReadOnly; }

    /// <summary>
    /// 設定文字輸入框是否為唯讀模式。
    /// </summary>
    /// <param name="readOnly">true 啟用唯讀，false 允許編輯。</param>
    void SetReadOnly(bool readOnly) noexcept;

    /// <summary>
    /// 取得最大輸入字元長度限制 (0 表示無限制)。
    /// </summary>
    /// <returns>字元數上限。</returns>
    COMPAT_NODISCARD size_t GetMaxLength() const noexcept { return m_maxLength; }

    /// <summary>
    /// 設定最大輸入字元長度限制 (0 表示無限制)。
    /// </summary>
    /// <param name="maxLen">字元數上限。</param>
    void SetMaxLength(size_t maxLen);

    /// <summary>
    /// 清空文字框之所有內容，並將游標重置至開頭。
    /// </summary>
    void Clear();

    /// <summary>
    /// 全選文字內容 (SelectAll)。
    /// </summary>
    void SelectAll();

    /// <summary>
    /// 設定選取文字之起點與長度。
    /// </summary>
    /// <param name="start">選取起始索引。</param>
    /// <param name="length">選取字元長度。</param>
    void Select(size_t start, size_t length);

    /// <summary>
    /// 取得目前選取區段之起始索引。
    /// </summary>
    /// <returns>選取起始索引。</returns>
    COMPAT_NODISCARD size_t GetSelectionStart() const noexcept { return m_selectionStart; }

    /// <summary>
    /// 取得目前選取區段之長度。
    /// </summary>
    /// <returns>選取長度。</returns>
    COMPAT_NODISCARD size_t GetSelectionLength() const noexcept { return m_selectionLength; }

    /// <summary>
    /// 取得目前反白選取之字串內容。
    /// </summary>
    /// <returns>選取文字副本。</returns>
    COMPAT_NODISCARD std::string GetSelectedText() const;

    /// <summary>
    /// 將選取文字複製至剪貼簿 (Copy)。
    /// </summary>
    void Copy();

    /// <summary>
    /// 剪下選取文字至剪貼簿 (Cut)。
    /// </summary>
    void Cut();

    /// <summary>
    /// 自剪貼簿貼上文字至當前游標處 (Paste)。
    /// </summary>
    void Paste();

    /// <summary>
    /// 檢查是否啟用密碼隱藏遮罩模式。
    /// </summary>
    /// <returns>若啟用密碼遮罩傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsPassword() const noexcept { return m_isPassword; }

    /// <summary>
    /// 設定是否啟用密碼隱藏遮罩模式。
    /// </summary>
    /// <param name="isPassword">true 啟用密碼遮罩，false 顯示明文。</param>
    void SetIsPassword(bool isPassword);

    /// <summary>
    /// 取得密碼掩碼字元。
    /// </summary>
    /// <returns>遮罩字元 (例如 '*')。</returns>
    COMPAT_NODISCARD char GetPasswordChar() const noexcept { return m_passwordChar; }

    /// <summary>
    /// 設定密碼掩碼字元。
    /// </summary>
    /// <param name="ch">遮罩字元 (例如 '*')。</param>
    void SetPasswordChar(char ch);

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

    /// <summary>
    /// 套用主題規格代幣至文字輸入框。
    /// </summary>
    /// <param name="theme">欲套用之 Theme 常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void InvalidateLayout();
    void EnsureLayout();
    void RaiseTextChanged();
    float GetCaretXOffset() const;
    void UpdateImeCompositionWindow();

    std::string          m_text;
    std::string          m_placeholder;
    size_t               m_caretIndex = 0;
    bool                 m_isReadOnly = false;
    size_t               m_maxLength = 0;
    size_t               m_selectionStart = 0;
    size_t               m_selectionLength = 0;
    bool                 m_isPassword = false;
    char                 m_passwordChar = '*';

    std::string          m_compositionText;
    int32_t              m_compositionCursor = 0;
    text::TextLayout     m_compositionLayout;
    bool                 m_isCompositionLayoutDirty = true;

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
