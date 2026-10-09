/**
 * @file Control.cpp
 * @brief 基礎控制項基底類別實作，涵蓋 Windows Forms / WPF 規格對標之屬性、排版與繪圖
 */

#include "ui/controls/Control.hpp"
#include "ui/app/Form.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>
#include <stdexcept>

namespace cppui {
namespace controls {

Control::Control() {
    SetFocusable(false);

    // 掛載指標進入與離開事件轉發
    OnPointerEntered([this](input::PointerEventArgs&) {
        if (!m_enabled) return;
        if (m_mouseEnterHandler) {
            m_mouseEnterHandler();
        }
    });

    OnPointerExited([this](input::PointerEventArgs&) {
        if (!m_enabled) return;
        if (m_mouseLeaveHandler) {
            m_mouseLeaveHandler();
        }
    });
}

Control::~Control() {
    // 若當前捕獲滑鼠則自動釋放，避免懸空游標捕獲
    if (HasMouseCapture()) {
        ReleaseMouseCapture();
    }
}

// --- 1. 識別與自訂資料 (Identity & Tag) ---

void Control::SetName(compat::string_view name) {
    m_name = std::string(name.data(), name.size());
}

// --- 2. 文字協定與同步 (Text Protocol) ---

compat::string_view Control::GetText() const noexcept {
    return m_text;
}

void Control::SetText(compat::string_view text) {
    std::string newStr(text.data(), text.size());
    if (m_text != newStr) {
        m_text = std::move(newStr);
        InvalidateMeasure();
        InvalidateVisual();
        OnTextChangedCore(m_text);
        if (m_textChangedHandler) {
            m_textChangedHandler(m_text);
        }
    }
}

// --- 3. 可用狀態與可見性 (State & Availability) ---

void Control::SetEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        SetFocusable(enabled);
        InvalidateVisual();
        OnEnabledChangedCore(enabled);
        if (m_enabledChangedHandler) {
            m_enabledChangedHandler(enabled);
        }
    }
}

bool Control::IsVisible() const noexcept {
    return core::Visual::IsVisible() && (GetVisibility() == layout::Visibility::Visible);
}

void Control::SetVisible(bool visible) {
    bool oldVisible = IsVisible();
    core::Visual::SetVisible(visible);
    SetVisibility(visible ? layout::Visibility::Visible : layout::Visibility::Collapsed);
    if (oldVisible != visible) {
        InvalidateMeasure();
        InvalidateVisual();
        OnVisibleChangedCore(visible);
        if (m_visibleChangedHandler) {
            m_visibleChangedHandler(visible);
        }
    }
}

// --- 4. 幾何座標、邊界與客戶區 (Bounds & Client Area) ---

float Control::GetRight() const noexcept {
    return GetLeft() + GetSize().width;
}

float Control::GetBottom() const noexcept {
    return GetTop() + GetSize().height;
}

core::Point Control::GetLocation() const noexcept {
    if (m_isArrangeValid) {
        return core::Point(m_renderBounds.x, m_renderBounds.y);
    }
    return m_location;
}

void Control::SetLocation(core::Point location) {
    if (m_location != location) {
        m_location = location;
        m_renderBounds.x = location.x;
        m_renderBounds.y = location.y;
        SetVisualOffset(location);
        InvalidateArrange();
        OnMoveCore(location);
        if (m_locationChangedHandler) {
            m_locationChangedHandler(location);
        }
    }
}

core::Size Control::GetSize() const noexcept {
    if (m_isArrangeValid && m_renderBounds.width > 0.0f && m_renderBounds.height > 0.0f) {
        return core::Size(m_renderBounds.width, m_renderBounds.height);
    }
    float w = GetWidth();
    float h = GetHeight();
    return core::Size((std::max)(0.0f, w >= 0.0f ? w : m_desiredSize.width),
                      (std::max)(0.0f, h >= 0.0f ? h : m_desiredSize.height));
}

void Control::SetSize(core::Size size) {
    core::Size oldSize = GetSize();
    SetWidth(size.width);
    SetHeight(size.height);
    if (oldSize != size) {
        OnResizeCore(size);
        if (m_sizeChangedHandler) {
            m_sizeChangedHandler(size);
        }
    }
}

core::Rect Control::GetBounds() const noexcept {
    core::Point loc = GetLocation();
    core::Size sz = GetSize();
    return core::Rect(loc, sz);
}

void Control::SetBounds(float x, float y, float width, float height) {
    SetLocation(x, y);
    SetSize(width, height);
}

void Control::SetBounds(const core::Rect& bounds) {
    SetBounds(bounds.x, bounds.y, bounds.width, bounds.height);
}

core::Rect Control::GetClientRectangle() const noexcept {
    core::Size clientSz = GetClientSize();
    return core::Rect(0.0f, 0.0f, clientSz.width, clientSz.height);
}

core::Size Control::GetClientSize() const noexcept {
    return DeflateSize(GetSize());
}

// --- 5. 排版約束 (Anchor & Dock) ---

void Control::SetAnchor(AnchorStyles anchor) {
    if (m_anchor != anchor) {
        m_anchor = anchor;
        InvalidateArrange();
    }
}

void Control::SetDock(DockStyle dock) {
    if (m_dock != dock) {
        m_dock = dock;
        InvalidateArrange();
    }
}

// --- 6. 字型與環境屬性 (Font & Style Hierarchy) ---

compat::string_view Control::GetFontFamily() const noexcept {
    if (m_hasExplicitFont && !m_fontFamily.empty()) {
        return compat::string_view(m_fontFamily.data(), m_fontFamily.size());
    }
    if (auto parentCtrl = GetParent()) {
        return parentCtrl->GetFontFamily();
    }
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    return compat::string_view(theme.Typography().fontFamily.data(), theme.Typography().fontFamily.size());
}

void Control::SetFontFamily(compat::string_view family) {
    m_fontFamily = std::string(family.data(), family.size());
    m_hasExplicitFont = true;
    InvalidateMeasure();
    InvalidateVisual();
}

float Control::GetFontSize() const noexcept {
    if (m_hasExplicitFont && m_fontSize > 0.0f) {
        return m_fontSize;
    }
    if (auto parentCtrl = GetParent()) {
        return parentCtrl->GetFontSize();
    }
    return theme::ThemeManager::Get().GetCurrentTheme().Typography().fontSizeBody;
}

void Control::SetFontSize(float size) {
    m_fontSize = (std::max)(1.0f, size);
    m_hasExplicitFont = true;
    InvalidateMeasure();
    InvalidateVisual();
}

text::FontWeight Control::GetFontWeight() const noexcept {
    if (m_hasExplicitFont) {
        return m_fontWeight;
    }
    if (auto parentCtrl = GetParent()) {
        return parentCtrl->GetFontWeight();
    }
    return text::FontWeight::Normal;
}

void Control::SetFontWeight(text::FontWeight weight) {
    m_fontWeight = weight;
    m_hasExplicitFont = true;
    InvalidateMeasure();
    InvalidateVisual();
}

text::FontStyle Control::GetFontStyle() const noexcept {
    if (m_hasExplicitFont) {
        return m_fontStyle;
    }
    if (auto parentCtrl = GetParent()) {
        return parentCtrl->GetFontStyle();
    }
    return text::FontStyle::Normal;
}

void Control::SetFontStyle(text::FontStyle style) {
    m_fontStyle = style;
    m_hasExplicitFont = true;
    InvalidateMeasure();
    InvalidateVisual();
}

text::FontDescriptor Control::GetFont() const {
    return text::FontDescriptor(GetFontFamily(), GetFontSize(), GetFontWeight(), GetFontStyle());
}

void Control::SetFont(const text::FontDescriptor& font) {
    SetFontFamily(font.GetFamily());
    SetFontSize(font.GetSize());
    SetFontWeight(font.GetWeight());
    SetFontStyle(font.GetStyle());
}

// --- 8. 外觀色彩與邊框 (Appearance & Padding) ---

void Control::SetPadding(const core::Thickness& padding) {
    if (m_padding != padding) {
        m_padding = padding;
        InvalidateMeasure();
    }
}

void Control::SetBackground(core::Color bg) {
    m_hasExplicitBackground = true;
    if (m_background != bg) {
        m_background = bg;
        InvalidateVisual();
    }
}

void Control::SetForeground(core::Color fg) {
    m_hasExplicitForeground = true;
    if (m_foreground != fg) {
        m_foreground = fg;
        InvalidateVisual();
    }
}

void Control::SetBorderBrush(core::Color brush) {
    m_hasExplicitBorder = true;
    if (m_borderBrush != brush) {
        m_borderBrush = brush;
        InvalidateVisual();
    }
}

void Control::ApplyTheme(const theme::Theme& theme) {
    if (!m_hasExplicitForeground) {
        m_foreground = theme.Colors().textPrimary;
    }
    for (size_t i = 0; i < GetVisualChildrenCount(); ++i) {
        if (auto* childCtrl = dynamic_cast<Control*>(GetVisualChild(i))) {
            childCtrl->ApplyTheme(theme);
        }
    }
    InvalidateVisual();
}

void Control::SetBorderThickness(const core::Thickness& thickness) {
    if (m_borderThickness != thickness) {
        m_borderThickness = thickness;
        InvalidateMeasure();
    }
}

void Control::SetCornerRadius(float radius) {
    if (m_cornerRadius != radius) {
        m_cornerRadius = (std::max)(0.0f, radius);
        InvalidateVisual();
    }
}

core::Size Control::DeflateSize(core::Size size) const noexcept {
    float hInset = m_padding.Horizontal() + m_borderThickness.Horizontal();
    float vInset = m_padding.Vertical() + m_borderThickness.Vertical();
    return core::Size(
        (std::max)(0.0f, size.width - hInset),
        (std::max)(0.0f, size.height - vInset)
    );
}

core::Size Control::InflateSize(core::Size size) const noexcept {
    float hInset = m_padding.Horizontal() + m_borderThickness.Horizontal();
    float vInset = m_padding.Vertical() + m_borderThickness.Vertical();
    return core::Size(size.width + hInset, size.height + vInset);
}

void Control::RenderBackgroundAndBorder(rendering::DrawingContext& dc, const core::Rect& bounds) {
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    // 1. 繪製背景
    if (m_background.a > 0) {
        if (m_cornerRadius > 0.5f) {
            dc.DrawRoundedRectangle(bounds, m_cornerRadius, m_background);
        } else {
            dc.DrawRectangle(bounds, m_background);
        }
    }

    // 2. 繪製邊框
    if (m_borderBrush.a > 0) {
        if (m_borderThickness.top > 0.0f) {
            dc.DrawLine(core::Point(bounds.x, bounds.y),
                        core::Point(bounds.Right(), bounds.y),
                        m_borderBrush, m_borderThickness.top);
        }
        if (m_borderThickness.bottom > 0.0f) {
            dc.DrawLine(core::Point(bounds.x, bounds.Bottom()),
                        core::Point(bounds.Right(), bounds.Bottom()),
                        m_borderBrush, m_borderThickness.bottom);
        }
        if (m_borderThickness.left > 0.0f) {
            dc.DrawLine(core::Point(bounds.x, bounds.y),
                        core::Point(bounds.x, bounds.Bottom()),
                        m_borderBrush, m_borderThickness.left);
        }
        if (m_borderThickness.right > 0.0f) {
            dc.DrawLine(core::Point(bounds.Right(), bounds.y),
                        core::Point(bounds.Right(), bounds.Bottom()),
                        m_borderBrush, m_borderThickness.right);
        }
    }
}

// --- 9. 視覺樹與階層導航 (Hierarchy Navigation) ---

Control* Control::GetParent() const noexcept {
    const core::Visual* cur = GetVisualParent();
    while (cur) {
        if (auto ctrl = dynamic_cast<const Control*>(cur)) {
            return const_cast<Control*>(ctrl);
        }
        cur = cur->GetVisualParent();
    }
    return nullptr;
}

app::Form* Control::FindForm() const noexcept {
    const core::Visual* cur = this;
    while (cur) {
        if (auto form = dynamic_cast<const app::Form*>(cur)) {
            return const_cast<app::Form*>(form);
        }
        cur = cur->GetVisualParent();
    }
    return nullptr;
}

Control* Control::GetTopLevelControl() const noexcept {
    Control* top = const_cast<Control*>(this);
    Control* parent = top->GetParent();
    while (parent) {
        top = parent;
        parent = top->GetParent();
    }
    return top;
}

bool Control::Contains(const Control* control) const noexcept {
    if (!control) return false;
    const core::Visual* cur = control->GetVisualParent();
    while (cur) {
        if (cur == this) return true;
        cur = cur->GetVisualParent();
    }
    return false;
}

std::vector<Control*> Control::GetChildControls() const {
    std::vector<Control*> result;
    size_t count = GetVisualChildrenCount();
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        if (auto ctrl = dynamic_cast<Control*>(GetVisualChild(i))) {
            result.push_back(ctrl);
        }
    }
    return result;
}

// --- 10. Z-Order 圖層堆疊管理 (Z-Order Management) ---

void Control::BringToFront() {
    if (auto parent = GetVisualParent()) {
        parent->BringVisualChildToFront(this);
        InvalidateVisual();
    }
}

void Control::SendToBack() {
    if (auto parent = GetVisualParent()) {
        parent->SendVisualChildToBack(this);
        InvalidateVisual();
    }
}

// --- 11. 焦點與互動選擇 (Focus & Selection) ---

bool Control::Focus() {
    if (!IsFocusable() || !IsEnabled() || !IsVisible()) {
        return false;
    }
    return input::FocusManager::Get().SetFocus(this);
}

bool Control::ContainsFocus() const noexcept {
    if (IsFocused()) return true;
    size_t count = GetVisualChildrenCount();
    for (size_t i = 0; i < count; ++i) {
        if (auto childCtrl = dynamic_cast<const Control*>(GetVisualChild(i))) {
            if (childCtrl->ContainsFocus()) return true;
        } else if (auto childElem = dynamic_cast<const core::UIElement*>(GetVisualChild(i))) {
            if (childElem->IsFocused()) return true;
        }
    }
    return false;
}

// --- 12. 滑鼠指標捕獲 (Mouse Capture) ---

bool Control::CaptureMouse() {
    return input::InputManager::Get().CapturePointer(this);
}

void Control::ReleaseMouseCapture() {
    input::InputManager::Get().ReleasePointerCapture(this);
}

bool Control::HasMouseCapture() const noexcept {
    return input::InputManager::Get().GetCapturedElement() == this;
}

// --- 13. 佈局生命週期管理 (Layout Lifecycle) ---

void Control::SuspendLayout() noexcept {
    ++m_layoutSuspendCount;
}

void Control::ResumeLayout(bool performLayout) {
    if (m_layoutSuspendCount > 0) {
        --m_layoutSuspendCount;
        if (m_layoutSuspendCount == 0 && (performLayout || m_isLayoutPending)) {
            m_isLayoutPending = false;
            PerformLayout();
        }
    }
}

void Control::PerformLayout() {
    InvalidateMeasure();
    InvalidateArrange();
}

// --- 14. 重繪與即時更新 (Repaint & Refresh) ---

void Control::Invalidate() {
    InvalidateVisual();
}

void Control::Invalidate(const core::Rect& rect) {
    (void)rect;
    InvalidateVisual();
}

void Control::Update() {
    if (!IsArrangeValid()) {
        InvalidateArrange();
    }
}

void Control::Refresh() {
    Invalidate();
    Update();
}

// --- 15. 圖形互操作與自訂繪製 (Graphics Interop) ---

std::unique_ptr<graphics::Graphics> Control::CreateGraphics() {
    core::Size sz = GetClientSize();
    uint32_t w = static_cast<uint32_t>((std::max)(1.0f, sz.width));
    uint32_t h = static_cast<uint32_t>((std::max)(1.0f, sz.height));
    auto bmp = std::make_shared<graphics::Bitmap>(w, h);
    return std::make_unique<graphics::Graphics>(bmp);
}

void Control::OnPaint(graphics::Graphics& g) {
    (void)g;
}

void Control::OnRender(rendering::DrawingContext& dc) {
    core::Size curSize = GetSize();
    core::Rect bounds(0.0f, 0.0f, curSize.width, curSize.height);
    RenderBackgroundAndBorder(dc, bounds);

    // GDI+ 風格統一繪圖排程派發
    graphics::Graphics g = graphics::Graphics::FromDrawingContext(dc);
    OnPaint(g);
    if (m_paintHandler) {
        m_paintHandler(g);
    }
}

// --- 16. 座標與矩形轉換 (Coordinate Conversion) ---

core::Rect Control::RectangleToScreen(const core::Rect& clientRect) const {
    core::Point p = PointToScreen(core::Point(clientRect.x, clientRect.y));
    return core::Rect(p.x, p.y, clientRect.width, clientRect.height);
}

core::Rect Control::RectangleToClient(const core::Rect& screenRect) const {
    core::Point p = PointToClient(core::Point(screenRect.x, screenRect.y));
    return core::Rect(p.x, p.y, screenRect.width, screenRect.height);
}

// --- 17. 常用事件監聽器掛載 (Event Listeners) ---

void Control::OnClick(std::function<void()> handler) {
    m_clickHandler = std::move(handler);
}

void Control::OnDoubleClick(std::function<void()> handler) {
    m_doubleClickHandler = std::move(handler);
}

void Control::OnTextChanged(std::function<void(compat::string_view)> handler) {
    m_textChangedHandler = std::move(handler);
}

void Control::OnEnabledChanged(std::function<void(bool)> handler) {
    m_enabledChangedHandler = std::move(handler);
}

void Control::OnVisibleChanged(std::function<void(bool)> handler) {
    m_visibleChangedHandler = std::move(handler);
}

void Control::OnSizeChanged(std::function<void(core::Size)> handler) {
    m_sizeChangedHandler = std::move(handler);
}

void Control::OnLocationChanged(std::function<void(core::Point)> handler) {
    m_locationChangedHandler = std::move(handler);
}

void Control::OnMouseEnter(std::function<void()> handler) {
    m_mouseEnterHandler = std::move(handler);
}

void Control::OnMouseLeave(std::function<void()> handler) {
    m_mouseLeaveHandler = std::move(handler);
}

void Control::OnPaintHandler(std::function<void(graphics::Graphics&)> handler) {
    m_paintHandler = std::move(handler);
}

// --- Protected Virtual Hooks ---

void Control::OnClickCore() {
    if (m_clickHandler) {
        m_clickHandler();
    }
}

void Control::OnDoubleClickCore() {
    if (m_doubleClickHandler) {
        m_doubleClickHandler();
    }
}

void Control::OnTextChangedCore(compat::string_view newText) {
    (void)newText;
}

void Control::OnEnabledChangedCore(bool enabled) {
    (void)enabled;
}

void Control::OnVisibleChangedCore(bool visible) {
    (void)visible;
}

void Control::OnResizeCore(core::Size newSize) {
    (void)newSize;
}

void Control::OnMoveCore(core::Point newLocation) {
    (void)newLocation;
}

} // namespace controls
} // namespace cppui
