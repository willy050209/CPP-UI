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

    TextBox();
    explicit TextBox(compat::string_view placeholder);
    ~TextBox() override = default;

    COMPAT_NODISCARD compat::string_view GetText() const noexcept { return m_text; }
    void SetText(compat::string_view text);
    void InsertText(compat::string_view text);

    COMPAT_NODISCARD compat::string_view GetPlaceholder() const noexcept { return m_placeholder; }
    void SetPlaceholder(compat::string_view placeholder);

    COMPAT_NODISCARD size_t GetCaretIndex() const noexcept { return m_caretIndex; }
    void SetCaretIndex(size_t index);

    COMPAT_NODISCARD float GetFontSize() const noexcept { return m_font.GetSize(); }
    void SetFontSize(float size);

    COMPAT_NODISCARD compat::string_view GetFontFamily() const noexcept { return m_font.GetFamily(); }
    void SetFontFamily(compat::string_view family);

    COMPAT_NODISCARD core::Color GetPlaceholderColor() const noexcept { return m_placeholderColor; }
    void SetPlaceholderColor(core::Color color);

    // 便利回呼
    void OnTextChanged(std::function<void(compat::string_view)> handler);

    core::Size MeasureOverride(core::Size availableSize) override;
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
