/**
 * @file TextBox.cpp
 * @brief 單行文字編輯輸入框實作，支援鍵盤互動、拖曳框選、多擊選取、選取高亮渲染、IME 組字與事件分發
 */

#include "ui/controls/TextBox.hpp"
#include "ui/controls/ContextMenu.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/pal/IClipboard.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include "ui/app/Form.hpp"
#include <algorithm>
#include <cctype>

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
    SetCursor(CursorType::IBeam);

    m_focusBorderColor = theme.Colors().controlFocusedBorder;
    m_placeholderColor = theme.Colors().textPlaceholder;
    core::Color accent = theme.Colors().accentPrimary;
    m_selectionColor = core::Color(accent.r, accent.g, accent.b, 120);

    m_font.SetFamily(theme.Typography().fontFamily);
    m_font.SetSize(theme.Typography().fontSizeBody);
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
    m_hasExplicitForeground = false;
    m_hasExplicitSelectionColor = false;

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!e.IsLeftButton()) return;

        input::FocusManager::Get().SetFocus(this);
        EnsureLayout();

        float left = m_padding.left + m_borderThickness.left;
        core::Point localPos = e.GetPosition(this);
        float clickRelX = localPos.x - left + m_scrollOffsetX;

        // 利用 TextLayout 命中測試反查游標字元索引
        text::TextHitTestResult hit = m_textLayout.HitTestPoint(core::Point((std::max)(0.0f, clickRelX), 0.0f));
        size_t hitIndex = hit.textPosition;
        if (hit.isTrailingHit && hitIndex < m_text.size()) {
            hitIndex = NextCharIndex(hitIndex);
        }
        hitIndex = (std::min)(hitIndex, m_text.size());

        // 若原先有未定稿組字，點擊重定游標時清空
        if (!m_compositionText.empty()) {
            m_compositionText.clear();
            m_compositionCursor = 0;
            m_isCompositionLayoutDirty = true;
        }

        int32_t clicks = (std::max)(1, e.GetClickCount());
        if (clicks >= 3) {
            // 三擊：全選
            SelectAll();
        } else if (clicks == 2 && !m_text.empty()) {
            // 雙擊：選取單詞
            size_t scanPos = (hitIndex < m_text.size()) ? hitIndex : (m_text.size() - 1);
            uint8_t targetByte = static_cast<uint8_t>(m_text[scanPos]);
            if (IsWordChar(targetByte)) {
                size_t start = scanPos;
                while (start > 0 && IsWordChar(static_cast<uint8_t>(m_text[start - 1]))) {
                    start--;
                }
                size_t end = scanPos;
                while (end < m_text.size() && IsWordChar(static_cast<uint8_t>(m_text[end]))) {
                    end++;
                }
                m_selectionStart = start;
                m_selectionLength = end - start;
                m_selectionAnchorIndex = start;
                m_caretIndex = end;
            } else if (std::isspace(targetByte)) {
                size_t start = scanPos;
                while (start > 0 && std::isspace(static_cast<uint8_t>(m_text[start - 1]))) {
                    start--;
                }
                size_t end = scanPos;
                while (end < m_text.size() && std::isspace(static_cast<uint8_t>(m_text[end]))) {
                    end++;
                }
                m_selectionStart = start;
                m_selectionLength = end - start;
                m_selectionAnchorIndex = start;
                m_caretIndex = end;
            } else {
                size_t nextPos = NextCharIndex(scanPos);
                m_selectionStart = scanPos;
                m_selectionLength = nextPos - scanPos;
                m_selectionAnchorIndex = scanPos;
                m_caretIndex = nextPos;
            }
        } else {
            // 單擊 (ClickCount == 1)
            if (e.HasShift()) {
                m_caretIndex = hitIndex;
                m_selectionStart = (std::min)(m_selectionAnchorIndex, m_caretIndex);
                m_selectionLength = (std::max)(m_selectionAnchorIndex, m_caretIndex) - m_selectionStart;
            } else {
                m_caretIndex = hitIndex;
                m_selectionAnchorIndex = hitIndex;
                m_selectionStart = hitIndex;
                m_selectionLength = 0;
                m_isSelecting = true;
                CaptureMouse();
            }
        }

        float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
        EnsureCaretVisible(innerW);
        InvalidateVisual();
        UpdateImeCompositionWindow();
        e.SetHandled(true);
    });

    OnPointerMoved([this](input::PointerEventArgs& e) {
        if (!m_isSelecting) return;

        EnsureLayout();
        float left = m_padding.left + m_borderThickness.left;
        core::Point localPos = e.GetPosition(this);
        float clickRelX = localPos.x - left + m_scrollOffsetX;

        text::TextHitTestResult hit = m_textLayout.HitTestPoint(core::Point((std::max)(0.0f, clickRelX), 0.0f));
        size_t hitIndex = hit.textPosition;
        if (hit.isTrailingHit && hitIndex < m_text.size()) {
            hitIndex = NextCharIndex(hitIndex);
        }
        hitIndex = (std::min)(hitIndex, m_text.size());

        m_caretIndex = hitIndex;
        m_selectionStart = (std::min)(m_selectionAnchorIndex, m_caretIndex);
        m_selectionLength = (std::max)(m_selectionAnchorIndex, m_caretIndex) - m_selectionStart;

        float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
        EnsureCaretVisible(innerW);
        InvalidateVisual();
        UpdateImeCompositionWindow();
        e.SetHandled(true);
    });

    OnPointerReleased([this](input::PointerEventArgs& e) {
        if (m_isSelecting) {
            m_isSelecting = false;
            ReleaseMouseCapture();
            e.SetHandled(true);
        }
    });

    OnTextInput([this](input::TextInputEventArgs& e) {
        compat::string_view inserted = e.GetText();
        if (inserted.empty()) return;

        InsertText(inserted);
        e.SetHandled(true);
    });

    OnImeComposition([this](input::ImeCompositionEventArgs& e) {
        if (m_isReadOnly) {
            e.SetHandled(true);
            return;
        }

        if (e.IsCommitted()) {
            m_compositionText.clear();
            m_compositionCursor = 0;
            m_isCompositionLayoutDirty = true;
            compat::string_view committedText = e.GetCompositionText();
            if (!committedText.empty()) {
                InsertText(committedText);
            }
            InvalidateLayout();
            InvalidateMeasure();
            InvalidateVisual();
        } else {
            compat::string_view comp = e.GetCompositionText();
            m_compositionText = std::string(comp.data(), comp.size());
            m_compositionCursor = e.GetCursorPosition();
            m_isCompositionLayoutDirty = true;
            InvalidateMeasure();
            InvalidateVisual();
        }
        UpdateImeCompositionWindow();
        e.SetHandled(true);
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        input::Key key = e.GetKey();

        if (e.HasControl()) {
            if (key == input::Key::C) { // Ctrl+C
                Copy();
                e.SetHandled(true);
                return;
            } else if (key == input::Key::X) { // Ctrl+X
                Cut();
                e.SetHandled(true);
                return;
            } else if (key == input::Key::V) { // Ctrl+V
                Paste();
                e.SetHandled(true);
                return;
            } else if (key == input::Key::A) { // Ctrl+A
                SelectAll();
                e.SetHandled(true);
                return;
            }
        }

        if (key == input::Key::Backspace) { // Backspace
            if (m_isReadOnly) {
                e.SetHandled(true);
                return;
            }
            if (m_selectionLength > 0) {
                DeleteSelectedText();
            } else if (m_caretIndex > 0) {
                size_t eraseStart = PrevCharIndex(m_caretIndex);
                m_text.erase(eraseStart, m_caretIndex - eraseStart);
                m_caretIndex = eraseStart;
                m_selectionAnchorIndex = m_caretIndex;
                m_selectionStart = m_caretIndex;
                m_selectionLength = 0;

                InvalidateLayout();
                InvalidateMeasure();
                InvalidateVisual();
                RaiseTextChanged();
                UpdateImeCompositionWindow();
            }
            float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
            EnsureCaretVisible(innerW);
            e.SetHandled(true);
        } else if (key == input::Key::Delete) { // Delete
            if (m_isReadOnly) {
                e.SetHandled(true);
                return;
            }
            if (m_selectionLength > 0) {
                DeleteSelectedText();
            } else if (m_caretIndex < m_text.size()) {
                size_t eraseEnd = NextCharIndex(m_caretIndex);
                m_text.erase(m_caretIndex, eraseEnd - m_caretIndex);
                m_selectionAnchorIndex = m_caretIndex;
                m_selectionStart = m_caretIndex;
                m_selectionLength = 0;

                InvalidateLayout();
                InvalidateMeasure();
                InvalidateVisual();
                RaiseTextChanged();
                UpdateImeCompositionWindow();
            }
            float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
            EnsureCaretVisible(innerW);
            e.SetHandled(true);
        } else if (key == input::Key::Left) { // Arrow Left
            if (e.HasShift()) {
                m_caretIndex = PrevCharIndex(m_caretIndex);
                m_selectionStart = (std::min)(m_selectionAnchorIndex, m_caretIndex);
                m_selectionLength = (std::max)(m_selectionAnchorIndex, m_caretIndex) - m_selectionStart;
            } else {
                if (m_selectionLength > 0) {
                    m_caretIndex = m_selectionStart;
                    m_selectionAnchorIndex = m_caretIndex;
                    m_selectionLength = 0;
                } else {
                    m_caretIndex = PrevCharIndex(m_caretIndex);
                    m_selectionAnchorIndex = m_caretIndex;
                }
            }
            float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
            EnsureCaretVisible(innerW);
            InvalidateVisual();
            UpdateImeCompositionWindow();
            e.SetHandled(true);
        } else if (key == input::Key::Right) { // Arrow Right
            if (e.HasShift()) {
                m_caretIndex = NextCharIndex(m_caretIndex);
                m_selectionStart = (std::min)(m_selectionAnchorIndex, m_caretIndex);
                m_selectionLength = (std::max)(m_selectionAnchorIndex, m_caretIndex) - m_selectionStart;
            } else {
                if (m_selectionLength > 0) {
                    m_caretIndex = m_selectionStart + m_selectionLength;
                    m_selectionAnchorIndex = m_caretIndex;
                    m_selectionLength = 0;
                } else {
                    m_caretIndex = NextCharIndex(m_caretIndex);
                    m_selectionAnchorIndex = m_caretIndex;
                }
            }
            float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
            EnsureCaretVisible(innerW);
            InvalidateVisual();
            UpdateImeCompositionWindow();
            e.SetHandled(true);
        } else if (key == input::Key::Home) { // Home
            if (e.HasShift()) {
                m_caretIndex = 0;
                m_selectionStart = (std::min)(m_selectionAnchorIndex, m_caretIndex);
                m_selectionLength = (std::max)(m_selectionAnchorIndex, m_caretIndex) - m_selectionStart;
            } else {
                m_caretIndex = 0;
                m_selectionAnchorIndex = 0;
                m_selectionStart = 0;
                m_selectionLength = 0;
            }
            float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
            EnsureCaretVisible(innerW);
            InvalidateVisual();
            UpdateImeCompositionWindow();
            e.SetHandled(true);
        } else if (key == input::Key::End) { // End
            if (e.HasShift()) {
                m_caretIndex = m_text.size();
                m_selectionStart = (std::min)(m_selectionAnchorIndex, m_caretIndex);
                m_selectionLength = (std::max)(m_selectionAnchorIndex, m_caretIndex) - m_selectionStart;
            } else {
                m_caretIndex = m_text.size();
                m_selectionAnchorIndex = m_caretIndex;
                m_selectionStart = m_caretIndex;
                m_selectionLength = 0;
            }
            float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
            EnsureCaretVisible(innerW);
            InvalidateVisual();
            UpdateImeCompositionWindow();
            e.SetHandled(true);
        }
    });

    OnGotFocus([this](input::FocusEventArgs&) {
        InvalidateVisual();
        UpdateImeCompositionWindow();
    });

    OnLostFocus([this](input::FocusEventArgs&) {
        if (m_isSelecting) {
            m_isSelecting = false;
            ReleaseMouseCapture();
        }
        if (!m_compositionText.empty()) {
            m_compositionText.clear();
            m_compositionCursor = 0;
            m_isCompositionLayoutDirty = true;
            InvalidateMeasure();
        }
        InvalidateVisual();
    });

    // 右鍵預設選單 (剪下、複製、貼上、全選)
    auto ctxMenu = std::make_shared<ContextMenu>();
    auto itemCut = std::make_shared<MenuItem>("剪下 (Cut)");
    itemCut->SetInputGestureText("Ctrl+X");
    itemCut->AddClickHandler([this](MenuItem&) { Cut(); });

    auto itemCopy = std::make_shared<MenuItem>("複製 (Copy)");
    itemCopy->SetInputGestureText("Ctrl+C");
    itemCopy->AddClickHandler([this](MenuItem&) { Copy(); });

    auto itemPaste = std::make_shared<MenuItem>("貼上 (Paste)");
    itemPaste->SetInputGestureText("Ctrl+V");
    itemPaste->AddClickHandler([this](MenuItem&) { Paste(); });

    auto itemSelectAll = std::make_shared<MenuItem>("全選 (Select All)");
    itemSelectAll->SetInputGestureText("Ctrl+A");
    itemSelectAll->AddClickHandler([this](MenuItem&) { SelectAll(); });

    ctxMenu->AddMenuItem(itemCut);
    ctxMenu->AddMenuItem(itemCopy);
    ctxMenu->AddMenuItem(itemPaste);
    ctxMenu->AddMenuItem(itemSelectAll);

    ContextMenuService::SetContextMenu(this, ctxMenu);
}

TextBox::~TextBox() {
    ContextMenuService::SetContextMenu(this, nullptr);
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
        m_selectionAnchorIndex = m_caretIndex;
        m_selectionStart = 0;
        m_selectionLength = 0;
        m_scrollOffsetX = 0.0f;
        m_compositionText.clear();
        m_compositionCursor = 0;
        m_isCompositionLayoutDirty = true;
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
        RaiseTextChanged();
        OnTextChangedCore(m_text);
        if (m_textChangedHandler) {
            m_textChangedHandler(m_text);
        }
        UpdateImeCompositionWindow();
    }
}

void TextBox::InsertText(compat::string_view text) {
    if (m_isReadOnly) return;
    if (m_selectionLength > 0) {
        DeleteSelectedText();
    }
    if (text.empty()) return;

    m_caretIndex = (std::min)(m_caretIndex, m_text.size());
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
    m_selectionAnchorIndex = m_caretIndex;
    m_selectionStart = m_caretIndex;
    m_selectionLength = 0;

    InvalidateLayout();
    InvalidateMeasure();
    InvalidateVisual();
    RaiseTextChanged();
    UpdateImeCompositionWindow();

    float innerW = (std::max)(0.0f, m_renderBounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
    EnsureCaretVisible(innerW);
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
        m_selectionAnchorIndex = m_caretIndex;
        m_selectionStart = (std::min)(m_selectionStart, m_text.size());
        m_selectionLength = (std::min)(m_selectionLength, m_text.size() - m_selectionStart);
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
        RaiseTextChanged();
        UpdateImeCompositionWindow();
    }
}

void TextBox::Clear() {
    SetText("");
    m_caretIndex = 0;
    m_selectionAnchorIndex = 0;
    m_selectionStart = 0;
    m_selectionLength = 0;
    m_scrollOffsetX = 0.0f;
    m_compositionText.clear();
    m_compositionCursor = 0;
    m_isCompositionLayoutDirty = true;
    UpdateImeCompositionWindow();
}

void TextBox::SelectAll() {
    m_selectionStart = 0;
    m_selectionLength = m_text.size();
    m_selectionAnchorIndex = 0;
    m_caretIndex = m_text.size();
    InvalidateVisual();
}

void TextBox::Select(size_t start, size_t length) {
    m_selectionStart = (std::min)(start, m_text.size());
    m_selectionLength = (std::min)(length, m_text.size() - m_selectionStart);
    m_selectionAnchorIndex = m_selectionStart;
    m_caretIndex = m_selectionStart + m_selectionLength;
    InvalidateVisual();
}

std::string TextBox::GetSelectedText() const {
    if (m_selectionLength == 0 || m_selectionStart >= m_text.size()) {
        return "";
    }
    return m_text.substr(m_selectionStart, m_selectionLength);
}

void TextBox::SetSelectionColor(core::Color color) {
    m_selectionColor = color;
    m_hasExplicitSelectionColor = true;
    InvalidateVisual();
}

void TextBox::Copy() {
    std::string sel = GetSelectedText();
    if (!sel.empty()) {
        pal::Clipboard::SetText(sel);
    }
}

void TextBox::Cut() {
    if (m_isReadOnly) return;
    std::string sel = GetSelectedText();
    if (!sel.empty()) {
        pal::Clipboard::SetText(sel);
        DeleteSelectedText();
    }
}

void TextBox::Paste() {
    if (m_isReadOnly) return;
    std::string clipText = pal::Clipboard::GetText();
    if (!clipText.empty()) {
        if (m_selectionLength > 0) {
            DeleteSelectedText();
        }
        InsertText(clipText);
    }
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
    m_selectionAnchorIndex = m_caretIndex;
    m_selectionStart = m_caretIndex;
    m_selectionLength = 0;
    InvalidateVisual();
    UpdateImeCompositionWindow();
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
    m_isCompositionLayoutDirty = true;
}

void TextBox::EnsureLayout() {
    if (!m_isLayoutDirty && !m_isCompositionLayoutDirty) return;

    std::string displayText = m_text;
    if (m_isPassword && !m_text.empty()) {
        displayText = std::string(m_text.size(), m_passwordChar);
    }

    if (m_isLayoutDirty) {
        // 單行文字以超寬約束排版，防止自動折行
        m_textLayout = text::TextLayoutEngine::Get().CreateLayout(displayText, m_font, 100000.0f);
        if (!m_placeholder.empty()) {
            m_placeholderLayout = text::TextLayoutEngine::Get().CreateLayout(m_placeholder, m_font, 100000.0f);
        }
        m_isLayoutDirty = false;
    }

    if (m_isCompositionLayoutDirty) {
        if (!m_compositionText.empty()) {
            m_compositionLayout = text::TextLayoutEngine::Get().CreateLayout(m_compositionText, m_font, 100000.0f);
        }
        m_isCompositionLayoutDirty = false;
    }
}

void TextBox::RaiseTextChanged() {
    input::RoutedEventArgs args(TextChangedEvent, this);
    RaiseEvent(args);
}

float TextBox::GetCaretXOffset() const {
    if (m_text.empty() || m_caretIndex == 0) return 0.0f;
    size_t clampedIndex = (std::min)(m_caretIndex, m_text.size());
    return m_textLayout.HitTestTextPosition(clampedIndex).x;
}

core::Rect TextBox::GetCaretRect() const {
    const_cast<TextBox*>(this)->EnsureLayout();
    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float innerH = (std::max)(0.0f, m_renderBounds.height - (m_padding.Vertical() + m_borderThickness.Vertical()));
    float textY = top + (innerH - m_font.GetSize() * 1.25f) * 0.5f;
    float caretH = m_font.GetSize() * 1.2f;

    float caretX = left - m_scrollOffsetX;
    if (m_compositionText.empty()) {
        caretX += GetCaretXOffset();
    } else {
        float textBeforeW = 0.0f;
        if (!m_text.empty() && m_caretIndex > 0) {
            size_t clampedIndex = (std::min)(m_caretIndex, m_text.size());
            textBeforeW = m_textLayout.HitTestTextPosition(clampedIndex).x;
        }
        float compCursorOffset = 0.0f;
        if (m_compositionCursor > 0 && !m_compositionText.empty()) {
            compCursorOffset = m_compositionLayout.HitTestTextPosition(static_cast<size_t>(m_compositionCursor)).x;
        }
        caretX += textBeforeW + compCursorOffset;
    }
    return core::Rect(caretX, textY, 2.0f, caretH);
}

void TextBox::UpdateImeCompositionWindow() {
    core::Visual* curr = GetVisualParent();
    app::Form* form = nullptr;
    while (curr) {
        if (auto* f = dynamic_cast<app::Form*>(curr)) {
            form = f;
            break;
        }
        curr = curr->GetVisualParent();
    }
    if (!form || !form->GetPlatformWindow()) {
        return;
    }

    core::Rect localCaret = GetCaretRect();
    core::Point winTopLeft = TransformToAncestor(core::Point(localCaret.x, localCaret.y), form);
    core::Rect winCaretRect(winTopLeft.x, winTopLeft.y, localCaret.width, localCaret.height);
    form->GetPlatformWindow()->SetImeCompositionWindow(winCaretRect);
}

size_t TextBox::PrevCharIndex(size_t index) const {
    if (index > m_text.size()) return m_text.size();
    if (index == 0) return 0;
    size_t prev = index - 1;
    while (prev > 0 && (static_cast<uint8_t>(m_text[prev]) & 0xC0) == 0x80) {
        prev--;
    }
    return prev;
}

size_t TextBox::NextCharIndex(size_t index) const {
    if (index >= m_text.size()) return m_text.size();
    size_t next = index + 1;
    while (next < m_text.size() && (static_cast<uint8_t>(m_text[next]) & 0xC0) == 0x80) {
        next++;
    }
    return next;
}

void TextBox::DeleteSelectedText() {
    if (m_isReadOnly) return;
    if (m_selectionLength > 0 && m_selectionStart < m_text.size()) {
        size_t len = (std::min)(m_selectionLength, m_text.size() - m_selectionStart);
        m_text.erase(m_selectionStart, len);
        m_caretIndex = m_selectionStart;
        m_selectionAnchorIndex = m_caretIndex;
        m_selectionLength = 0;
        InvalidateLayout();
        InvalidateMeasure();
        InvalidateVisual();
        RaiseTextChanged();
        UpdateImeCompositionWindow();
    }
}

void TextBox::EnsureCaretVisible(float innerW) {
    if (innerW <= 0.0f) return;
    EnsureLayout();
    float totalTextW = m_textLayout.GetSize().width;
    if (totalTextW <= innerW) {
        m_scrollOffsetX = 0.0f;
        return;
    }
    float caretX = GetCaretXOffset();
    if (caretX - m_scrollOffsetX > innerW) {
        m_scrollOffsetX = caretX - innerW + 4.0f;
    } else if (caretX - m_scrollOffsetX < 0.0f) {
        m_scrollOffsetX = (std::max)(0.0f, caretX - 4.0f);
    }
    float maxScroll = (std::max)(0.0f, totalTextW - innerW + 10.0f);
    m_scrollOffsetX = compat::clamp(m_scrollOffsetX, 0.0f, maxScroll);
}

bool TextBox::IsWordChar(uint8_t c) {
    if (c >= 0x80) return true;
    return (std::isalnum(c) != 0) || (c == '_');
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

    if (!m_compositionText.empty()) {
        contentW += m_compositionLayout.GetSize().width;
        contentH = (std::max)(contentH, m_compositionLayout.GetSize().height);
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

    EnsureCaretVisible(innerW);

    float textY = top + (innerH - m_font.GetSize() * 1.25f) * 0.5f;
    core::Point textOrigin(left - m_scrollOffsetX, textY);

    // 繪製選取文字高亮背景 (在文字底層)
    if (m_selectionLength > 0 && m_selectionStart < m_text.size() && m_compositionText.empty()) {
        size_t selStart = m_selectionStart;
        size_t selEnd = (std::min)(m_selectionStart + m_selectionLength, m_text.size());
        float x1 = m_textLayout.HitTestTextPosition(selStart).x;
        float x2 = m_textLayout.HitTestTextPosition(selEnd).x;
        float selLeft = (std::min)(x1, x2);
        float selRight = (std::max)(x1, x2);
        core::Rect selRect(left - m_scrollOffsetX + selLeft, textY, (std::max)(2.0f, selRight - selLeft), m_font.GetSize() * 1.25f);
        dc.DrawRectangle(selRect, m_selectionColor);
    }

    if (m_text.empty() && m_compositionText.empty()) {
        if (!m_placeholder.empty()) {
            dc.DrawTextLayout(m_placeholderLayout, textOrigin, m_placeholderColor);
        }
    } else if (m_compositionText.empty()) {
        dc.DrawTextLayout(m_textLayout, textOrigin, m_foreground);
    } else {
        // 組字渲染：分別繪製前置文字、組字串 (帶底線標記)、後置文字
        size_t clampedIndex = (std::min)(m_caretIndex, m_text.size());
        std::string beforeText = m_text.substr(0, clampedIndex);
        std::string afterText = m_text.substr(clampedIndex);

        float xCursor = left - m_scrollOffsetX;
        if (!beforeText.empty()) {
            auto beforeLayout = text::TextLayoutEngine::Get().CreateLayout(beforeText, m_font, 100000.0f);
            dc.DrawTextLayout(beforeLayout, core::Point(xCursor, textY), m_foreground);
            xCursor += beforeLayout.GetSize().width;
        }

        float compStart = xCursor;
        dc.DrawTextLayout(m_compositionLayout, core::Point(xCursor, textY), m_foreground);
        float compW = m_compositionLayout.GetSize().width;
        float compEnd = compStart + compW;

        // 繪製組字字串底線標記
        float underlineY = textY + m_font.GetSize() * 1.2f;
        dc.DrawLine(core::Point(compStart, underlineY), core::Point(compEnd, underlineY), m_focusBorderColor, 1.5f);

        xCursor += compW;

        if (!afterText.empty()) {
            auto afterLayout = text::TextLayoutEngine::Get().CreateLayout(afterText, m_font, 100000.0f);
            dc.DrawTextLayout(afterLayout, core::Point(xCursor, textY), m_foreground);
        }
    }

    // 繪製游標 (獲焦時)
    if (IsFocused()) {
        core::Rect caret = GetCaretRect();
        dc.DrawLine(core::Point(caret.x, caret.y), core::Point(caret.x, caret.y + caret.height), m_caretColor, 1.5f);
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
    if (!m_hasExplicitSelectionColor) {
        core::Color accent = theme.Colors().accentPrimary;
        m_selectionColor = core::Color(accent.r, accent.g, accent.b, 120);
    }
    if (!m_hasExplicitFont) {
        m_font.SetFamily(theme.Typography().fontFamily);
        m_font.SetSize(theme.Typography().fontSizeBody);
        InvalidateLayout();
    }
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
