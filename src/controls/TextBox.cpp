#include "ui/controls/TextBox.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

const input::RoutedEvent TextBox::TextChangedEvent(103, "TextChanged", input::RoutingStrategy::Bubble);

TextBox::TextBox() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetFocusable(true);
    SetTabStop(true);
    SetPadding(theme.Metrics().paddingInput);
    SetBorderThickness(theme.Metrics().borderThicknessDefault);
    SetCornerRadius(theme.Metrics().cornerRadiusSmall);
    SetBackground(theme.Colors().surfaceBackground);
    SetBorderBrush(theme.Colors().controlBorder);
    SetForeground(theme.Colors().textPrimary);

    m_focusBorderColor = theme.Colors().controlFocusedBorder;
    m_placeholderColor = theme.Colors().textPlaceholder;
    m_font.SetFamily(theme.Typography().fontFamily);
    m_font.SetSize(theme.Typography().fontSizeBody);
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
    m_hasExplicitForeground = false;

    OnPointerPressed([this](input::PointerEventArgs& e) {
        input::FocusManager::Get().SetFocus(this);
        EnsureLayout();

        float left = m_padding.left + m_borderThickness.left;
        core::Point localPos = e.GetPosition(this);
        float clickRelX = localPos.x - left;

        // 利用 TextLayout 命中測試反查游標字元索引
        text::TextHitTestResult hit = m_textLayout.HitTestPoint(core::Point((std::max)(0.0f, clickRelX), 0.0f));
        m_caretIndex = hit.textPosition;
        if (hit.isTrailingHit && m_caretIndex < m_text.size()) {
            m_caretIndex++;
            while (m_caretIndex < m_text.size() && (static_cast<uint8_t>(m_text[m_caretIndex]) & 0xC0) == 0x80) {
                m_caretIndex++;
            }
        }

        InvalidateVisual();
        e.SetHandled(true);
    });

    OnTextInput([this](input::TextInputEventArgs& e) {
        compat::string_view inserted = e.GetText();
        if (inserted.empty()) return;

        InsertText(inserted);
        e.SetHandled(true);
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        uint32_t vk = e.GetVirtualKey();

        if (vk == 0x08) { // Backspace
            if (m_isReadOnly) {
                e.SetHandled(true);
                return;
            }
            if (m_caretIndex > 0) {
                // 回退一個 UTF-8 字元
                size_t eraseStart = m_caretIndex - 1;
                while (eraseStart > 0 && (static_cast<uint8_t>(m_text[eraseStart]) & 0xC0) == 0x80) {
                    eraseStart--;
                }
                size_t count = m_caretIndex - eraseStart;
                m_text.erase(eraseStart, count);
                m_caretIndex = eraseStart;

                InvalidateLayout();
                InvalidateMeasure();
                InvalidateVisual();
                RaiseTextChanged();
            }
            e.SetHandled(true);
        } else if (vk == 0x2E) { // Delete
            if (m_isReadOnly) {
                e.SetHandled(true);
                return;
            }
            if (m_caretIndex < m_text.size()) {
                // 前進一個 UTF-8 字元刪除
                size_t eraseEnd = m_caretIndex + 1;
                while (eraseEnd < m_text.size() && (static_cast<uint8_t>(m_text[eraseEnd]) & 0xC0) == 0x80) {
                    eraseEnd++;
                }
                m_text.erase(m_caretIndex, eraseEnd - m_caretIndex);

                InvalidateLayout();
                InvalidateMeasure();
                InvalidateVisual();
                RaiseTextChanged();
            }
            e.SetHandled(true);
        } else if (vk == 0x25) { // Arrow Left
            if (m_caretIndex > 0) {
                m_caretIndex--;
                while (m_caretIndex > 0 && (static_cast<uint8_t>(m_text[m_caretIndex]) & 0xC0) == 0x80) {
                    m_caretIndex--;
                }
                InvalidateVisual();
            }
            e.SetHandled(true);
        } else if (vk == 0x27) { // Arrow Right
            if (m_caretIndex < m_text.size()) {
                m_caretIndex++;
                while (m_caretIndex < m_text.size() && (static_cast<uint8_t>(m_text[m_caretIndex]) & 0xC0) == 0x80) {
                    m_caretIndex++;
                }
                InvalidateVisual();
            }
            e.SetHandled(true);
        } else if (vk == 0x24) { // Home
            m_caretIndex = 0;
            InvalidateVisual();
            e.SetHandled(true);
        } else if (vk == 0x23) { // End
            m_caretIndex = m_text.size();
            InvalidateVisual();
            e.SetHandled(true);
        }
    });

    OnGotFocus([this](input::FocusEventArgs&) {
        InvalidateVisual();
    });

    OnLostFocus([this](input::FocusEventArgs&) {
        InvalidateVisual();
    });
}

TextBox::TextBox(compat::string_view placeholder) : TextBox() {
    SetPlaceholder(placeholder);
}

void TextBox::SetText(compat::string_view text) {
    std::string newText(text.data(), text.size());
    if (m_maxLength > 0 && newText.size() > m_maxLength) {
        newText.erase(m_maxLength);
    }
    if (m_text != newText) {
        m_text = std::move(newText);
        m_caretIndex = m_text.size();
        m_selectionStart = (std::min)(m_selectionStart, m_text.size());
        m_selectionLength = (std::min)(m_selectionLength, m_text.size() - m_selectionStart);
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
        RaiseTextChanged();
        OnTextChangedCore(m_text);
        if (m_textChangedHandler) {
            m_textChangedHandler(m_text);
        }
    }
}

void TextBox::InsertText(compat::string_view text) {
    if (text.empty() || m_isReadOnly) return;
    compat::string_view toInsert = text;
    if (m_maxLength > 0) {
        if (m_text.size() >= m_maxLength) return;
        size_t avail = m_maxLength - m_text.size();
        if (toInsert.size() > avail) {
            toInsert = toInsert.substr(0, avail);
        }
    }
    m_text.insert(m_caretIndex, toInsert.data(), toInsert.size());
    m_caretIndex += toInsert.size();
    InvalidateLayout();
    InvalidateMeasure();
    InvalidateVisual();
    RaiseTextChanged();
}

void TextBox::SetReadOnly(bool readOnly) noexcept {
    if (m_isReadOnly != readOnly) {
        m_isReadOnly = readOnly;
        InvalidateVisual();
    }
}

void TextBox::SetMaxLength(size_t maxLen) {
    m_maxLength = maxLen;
    if (m_maxLength > 0 && m_text.size() > m_maxLength) {
        m_text.erase(m_maxLength);
        m_caretIndex = (std::min)(m_caretIndex, m_text.size());
        m_selectionStart = (std::min)(m_selectionStart, m_text.size());
        m_selectionLength = (std::min)(m_selectionLength, m_text.size() - m_selectionStart);
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
        RaiseTextChanged();
    }
}

void TextBox::Clear() {
    SetText("");
    m_caretIndex = 0;
    m_selectionStart = 0;
    m_selectionLength = 0;
}

void TextBox::SelectAll() {
    m_selectionStart = 0;
    m_selectionLength = m_text.size();
    InvalidateVisual();
}

void TextBox::Select(size_t start, size_t length) {
    m_selectionStart = (std::min)(start, m_text.size());
    m_selectionLength = (std::min)(length, m_text.size() - m_selectionStart);
    InvalidateVisual();
}

std::string TextBox::GetSelectedText() const {
    if (m_selectionLength == 0 || m_selectionStart >= m_text.size()) {
        return "";
    }
    return m_text.substr(m_selectionStart, m_selectionLength);
}

void TextBox::SetIsPassword(bool isPassword) {
    if (m_isPassword != isPassword) {
        m_isPassword = isPassword;
        InvalidateLayout();
        InvalidateVisual();
    }
}

void TextBox::SetPasswordChar(char ch) {
    if (m_passwordChar != ch) {
        m_passwordChar = ch;
        if (m_isPassword) {
            InvalidateLayout();
            InvalidateVisual();
        }
    }
}

void TextBox::SetPlaceholder(compat::string_view placeholder) {
    if (m_placeholder != placeholder) {
        m_placeholder = std::string(placeholder.data(), placeholder.size());
        InvalidateLayout();
        InvalidateVisual();
    }
}

void TextBox::SetCaretIndex(size_t index) {
    m_caretIndex = (std::min)(index, m_text.size());
    InvalidateVisual();
}

void TextBox::SetFontSize(float size) {
    if (m_font.GetSize() != size) {
        m_font.SetSize(size);
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void TextBox::SetFontFamily(compat::string_view family) {
    if (m_font.GetFamily() != family) {
        m_font.SetFamily(family);
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void TextBox::SetPlaceholderColor(core::Color color) {
    if (m_placeholderColor != color) {
        m_placeholderColor = color;
        InvalidateVisual();
    }
}

void TextBox::OnTextChanged(std::function<void(compat::string_view)> handler) {
    AddHandler(TextChangedEvent, [this, handler](input::RoutedEventArgs&) {
        handler(m_text);
    });
}

void TextBox::InvalidateLayout() {
    m_isLayoutDirty = true;
}

void TextBox::EnsureLayout() {
    if (!m_isLayoutDirty) return;

    std::string displayText = m_text;
    if (m_isPassword && !m_text.empty()) {
        displayText = std::string(m_text.size(), m_passwordChar);
    }

    // 單行文字以超寬約束排版，防止自動折行
    m_textLayout = text::TextLayoutEngine::Get().CreateLayout(displayText, m_font, 100000.0f);
    if (!m_placeholder.empty()) {
        m_placeholderLayout = text::TextLayoutEngine::Get().CreateLayout(m_placeholder, m_font, 100000.0f);
    }
    m_isLayoutDirty = false;
}

void TextBox::RaiseTextChanged() {
    input::RoutedEventArgs args(TextChangedEvent, this);
    RaiseEvent(args);
}

float TextBox::GetCaretXOffset() const {
    if (m_text.empty() || m_caretIndex == 0) return 0.0f;
    return m_textLayout.HitTestTextPosition(m_caretIndex).x;
}

core::Size TextBox::MeasureOverride(core::Size availableSize) {
    EnsureLayout();

    float contentW = 0.0f;
    float contentH = m_font.GetSize() * 1.3f;

    if (!m_text.empty()) {
        contentW = m_textLayout.GetSize().width;
        contentH = (std::max)(contentH, m_textLayout.GetSize().height);
    } else if (!m_placeholder.empty()) {
        contentW = m_placeholderLayout.GetSize().width;
        contentH = (std::max)(contentH, m_placeholderLayout.GetSize().height);
    }

    // 提供標準單行輸入框預設最小寬高 (100x20)
    contentW = (std::max)(contentW, 100.0f);
    contentH = (std::max)(contentH, 20.0f);

    core::Size desired = InflateSize(core::Size(contentW, contentH));
    (void)availableSize;
    return desired;
}

void TextBox::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    EnsureLayout();

    core::Color border = IsFocused() ? m_focusBorderColor : m_borderBrush;

    // 繪製背景
    if (m_cornerRadius > 0.0f) {
        dc.DrawRoundedRectangle(bounds, m_cornerRadius, m_background);
    } else {
        dc.DrawRectangle(bounds, m_background);
    }

    // 繪製外框
    if (border.a > 0 && m_borderThickness.top > 0.0f) {
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), border, m_borderThickness.top);
        dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), border, m_borderThickness.bottom);
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), border, m_borderThickness.left);
        dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), border, m_borderThickness.right);
    }

    // 計算內容文字繪製區域與剪裁
    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float innerW = (std::max)(0.0f, bounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
    float innerH = (std::max)(0.0f, bounds.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    core::Rect innerBounds(left, top, innerW, innerH);
    dc.PushClip(innerBounds);

    float textY = top + (innerH - m_font.GetSize() * 1.25f) * 0.5f;
    core::Point textOrigin(left, textY);

    if (m_text.empty()) {
        if (!m_placeholder.empty()) {
            dc.DrawTextLayout(m_placeholderLayout, textOrigin, m_placeholderColor);
        }
    } else {
        dc.DrawTextLayout(m_textLayout, textOrigin, m_foreground);
    }

    // 繪製游標 (獲焦時)
    if (IsFocused()) {
        float caretX = textOrigin.x + GetCaretXOffset();
        float caretH = m_font.GetSize() * 1.2f;
        dc.DrawLine(core::Point(caretX, textY), core::Point(caretX, textY + caretH), m_caretColor, 1.5f);
    }

    dc.PopClip();
}

void TextBox::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    if (!m_hasExplicitBackground) {
        m_background = theme.Colors().surfaceBackground;
    }
    if (!m_hasExplicitBorder) {
        m_borderBrush = theme.Colors().controlBorder;
        m_focusBorderColor = theme.Colors().controlFocusedBorder;
    }
    m_placeholderColor = theme.Colors().textPlaceholder;
    if (!m_hasExplicitFont) {
        m_font.SetFamily(theme.Typography().fontFamily);
        m_font.SetSize(theme.Typography().fontSizeBody);
        InvalidateLayout();
    }
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
