#include "ui/core/UIElement.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/InputManager.hpp"
#include <algorithm>

namespace cppui {
namespace core {

UIElement::~UIElement() {
    input::FocusManager::Get().NotifyElementDestroyed(this);
    input::InputManager::Get().NotifyElementDestroyed(this);
}

void UIElement::SetMargin(const Thickness& margin) {
    if (m_margin.left != margin.left || m_margin.top != margin.top ||
        m_margin.right != margin.right || m_margin.bottom != margin.bottom) {
        m_margin = margin;
        InvalidateMeasure();
    }
}

void UIElement::SetWidth(float width) {
    if (m_width != width) {
        m_width = width;
        InvalidateMeasure();
    }
}

void UIElement::SetHeight(float height) {
    if (m_height != height) {
        m_height = height;
        InvalidateMeasure();
    }
}

void UIElement::SetMinWidth(float minWidth) {
    if (m_minWidth != minWidth) {
        m_minWidth = minWidth;
        InvalidateMeasure();
    }
}

void UIElement::SetMaxWidth(float maxWidth) {
    if (m_maxWidth != maxWidth) {
        m_maxWidth = maxWidth;
        InvalidateMeasure();
    }
}

void UIElement::SetMinHeight(float minHeight) {
    if (m_minHeight != minHeight) {
        m_minHeight = minHeight;
        InvalidateMeasure();
    }
}

void UIElement::SetMaxHeight(float maxHeight) {
    if (m_maxHeight != maxHeight) {
        m_maxHeight = maxHeight;
        InvalidateMeasure();
    }
}

void UIElement::SetHorizontalAlignment(layout::HorizontalAlignment alignment) {
    if (m_horizontalAlignment != alignment) {
        m_horizontalAlignment = alignment;
        InvalidateArrange();
    }
}

void UIElement::SetVerticalAlignment(layout::VerticalAlignment alignment) {
    if (m_verticalAlignment != alignment) {
        m_verticalAlignment = alignment;
        InvalidateArrange();
    }
}

void UIElement::SetVisibility(layout::Visibility visibility) {
    if (m_visibility != visibility) {
        m_visibility = visibility;
        SetVisible(visibility == layout::Visibility::Visible);
        InvalidateMeasure();
    }
}

UIElement* UIElement::GetParentUIElement() const noexcept {
    return dynamic_cast<UIElement*>(GetVisualParent());
}

void UIElement::InvalidateMeasure() {
    if (!m_isMeasureValid) return;
    m_isMeasureValid = false;
    m_isArrangeValid = false;

    UIElement* parent = GetParentUIElement();
    if (parent) {
        parent->InvalidateMeasure();
    }
}

void UIElement::InvalidateArrange() {
    if (!m_isArrangeValid) return;
    m_isArrangeValid = false;

    UIElement* parent = GetParentUIElement();
    if (parent) {
        parent->InvalidateArrange();
    }
}

void UIElement::InvalidateVisual() {
    m_isVisualValid = false;
}

void UIElement::Measure(Size availableSize) {
    if (m_visibility == layout::Visibility::Collapsed) {
        m_desiredSize = Size(0.0f, 0.0f);
        m_isMeasureValid = true;
        return;
    }

    if (m_isMeasureValid && availableSize == m_previousAvailableSize) {
        return; // 快取命中
    }

    // 扣除 Margin 保留空間
    float marginW = m_margin.left + m_margin.right;
    float marginH = m_margin.top + m_margin.bottom;
    float contentAvailableW = std::max(0.0f, availableSize.width - marginW);
    float contentAvailableH = std::max(0.0f, availableSize.height - marginH);

    // 套用明確 Width / Height 約束
    if (m_width >= 0.0f) {
        contentAvailableW = std::min(contentAvailableW, m_width);
    }
    if (m_height >= 0.0f) {
        contentAvailableH = std::min(contentAvailableH, m_height);
    }

    // 套用 Min/Max 約束
    contentAvailableW = compat::clamp(contentAvailableW, m_minWidth, m_maxWidth);
    contentAvailableH = compat::clamp(contentAvailableH, m_minHeight, m_maxHeight);

    // 呼叫內容測量自訂點
    Size unconstrainedContentSize = MeasureOverride(Size(contentAvailableW, contentAvailableH));

    // 結合明確尺寸與測量尺寸
    float finalContentW = (m_width >= 0.0f) ? m_width : unconstrainedContentSize.width;
    float finalContentH = (m_height >= 0.0f) ? m_height : unconstrainedContentSize.height;

    // 再次約束於 Min/Max
    finalContentW = compat::clamp(finalContentW, m_minWidth, m_maxWidth);
    finalContentH = compat::clamp(finalContentH, m_minHeight, m_maxHeight);

    // 快取 DesiredSize (包含 Margin)
    m_desiredSize = Size(finalContentW + marginW, finalContentH + marginH);
    m_previousAvailableSize = availableSize;
    m_isMeasureValid = true;
}

void UIElement::Arrange(Rect finalRect) {
    if (m_visibility == layout::Visibility::Collapsed) {
        m_renderBounds = Rect(0.0f, 0.0f, 0.0f, 0.0f);
        SetVisualOffset(Point(0.0f, 0.0f));
        SetVisualBounds(Rect(0.0f, 0.0f, 0.0f, 0.0f));
        m_isArrangeValid = true;
        return;
    }

    if (m_isArrangeValid && finalRect == m_previousFinalRect) {
        return; // 快取命中
    }

    // 扣除 Margin 幾何
    float marginW = m_margin.left + m_margin.right;
    float marginH = m_margin.top + m_margin.bottom;
    float availW = std::max(0.0f, finalRect.width - marginW);
    float availH = std::max(0.0f, finalRect.height - marginH);

    // 計算內容目標寬度
    float contentDesiredW = std::max(0.0f, m_desiredSize.width - marginW);
    float contentDesiredH = std::max(0.0f, m_desiredSize.height - marginH);

    float arrangeW = (m_width >= 0.0f) ? m_width : contentDesiredW;
    float arrangeH = (m_height >= 0.0f) ? m_height : contentDesiredH;

    if (m_horizontalAlignment == layout::HorizontalAlignment::Stretch && m_width < 0.0f) {
        arrangeW = availW;
    } else {
        arrangeW = std::min(availW, arrangeW);
    }

    if (m_verticalAlignment == layout::VerticalAlignment::Stretch && m_height < 0.0f) {
        arrangeH = availH;
    } else {
        arrangeH = std::min(availH, arrangeH);
    }

    arrangeW = compat::clamp(arrangeW, m_minWidth, m_maxWidth);
    arrangeH = compat::clamp(arrangeH, m_minHeight, m_maxHeight);

    // 根據 HorizontalAlignment 決定 X 偏移
    float offsetX = 0.0f;
    switch (m_horizontalAlignment) {
        case layout::HorizontalAlignment::Left:
            offsetX = m_margin.left;
            break;
        case layout::HorizontalAlignment::Center:
            offsetX = m_margin.left + (availW - arrangeW) * 0.5f;
            break;
        case layout::HorizontalAlignment::Right:
            offsetX = finalRect.width - m_margin.right - arrangeW;
            break;
        case layout::HorizontalAlignment::Stretch:
            offsetX = m_margin.left;
            break;
    }

    // 根據 VerticalAlignment 決定 Y 偏移
    float offsetY = 0.0f;
    switch (m_verticalAlignment) {
        case layout::VerticalAlignment::Top:
            offsetY = m_margin.top;
            break;
        case layout::VerticalAlignment::Center:
            offsetY = m_margin.top + (availH - arrangeH) * 0.5f;
            break;
        case layout::VerticalAlignment::Bottom:
            offsetY = finalRect.height - m_margin.bottom - arrangeH;
            break;
        case layout::VerticalAlignment::Stretch:
            offsetY = m_margin.top;
            break;
    }

    float finalX = finalRect.x + offsetX;
    float finalY = finalRect.y + offsetY;

    m_renderBounds = Rect(finalX, finalY, arrangeW, arrangeH);
    SetVisualOffset(Point(finalX, finalY));
    SetVisualBounds(Rect(0.0f, 0.0f, arrangeW, arrangeH));

    // 派發給子類別 ArrangeOverride
    ArrangeOverride(Size(arrangeW, arrangeH));

    m_previousFinalRect = finalRect;
    m_isArrangeValid = true;
}

Size UIElement::MeasureOverride(Size availableSize) {
    // 預設實作：測量所有子視覺節點 (若為 UIElement)
    float maxW = 0.0f;
    float maxH = 0.0f;
    for (auto& childVisual : m_visualChildren) {
        auto* childUI = dynamic_cast<UIElement*>(childVisual.get());
        if (childUI && childUI->IsVisible()) {
            childUI->Measure(availableSize);
            maxW = std::max(maxW, childUI->GetDesiredSize().width);
            maxH = std::max(maxH, childUI->GetDesiredSize().height);
        }
    }
    return Size(maxW, maxH);
}

void UIElement::ArrangeOverride(Size finalSize) {
    // 預設實作：將所有子節點直接排滿 finalSize
    Rect childRect(0.0f, 0.0f, finalSize.width, finalSize.height);
    for (auto& childVisual : m_visualChildren) {
        auto* childUI = dynamic_cast<UIElement*>(childVisual.get());
        if (childUI && childUI->IsVisible()) {
            childUI->Arrange(childRect);
        }
    }
}

void UIElement::AddHandler(const input::RoutedEvent& routedEvent, RoutedEventHandler handler, bool handledEventsToo) {
    if (!handler) return;
    m_eventHandlers.push_back({ routedEvent.GetId(), std::move(handler), handledEventsToo });
}

void UIElement::RemoveHandler(const input::RoutedEvent& routedEvent) {
    uint32_t id = routedEvent.GetId();
    m_eventHandlers.erase(
        std::remove_if(m_eventHandlers.begin(), m_eventHandlers.end(),
                       [id](const HandlerEntry& e) { return e.eventId == id; }),
        m_eventHandlers.end()
    );
}

void UIElement::RaiseEvent(input::RoutedEventArgs& e) {
    if (!e.GetRoutedEvent()) return;
    input::EventRoute route(this);
    if (e.GetRoutedEvent()->GetStrategy() == input::RoutingStrategy::Direct) {
        route.DispatchDirect(*e.GetRoutedEvent(), e);
    } else if (e.GetRoutedEvent()->GetStrategy() == input::RoutingStrategy::Bubble) {
        route.DispatchBubble(*e.GetRoutedEvent(), e);
    } else {
        route.DispatchTunnel(*e.GetRoutedEvent(), e);
    }
}

void UIElement::RaiseEventInternal(const input::RoutedEvent& routedEvent, input::RoutedEventArgs& e) {
    uint32_t id = routedEvent.GetId();

    // 1. 呼叫虛擬掛載點
    if (id == input::StandardEvents::PointerPressed.GetId() || id == input::StandardEvents::PreviewPointerPressed.GetId()) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            OnPointerPressedCore(*pe);
        }
    } else if (id == input::StandardEvents::PointerReleased.GetId() || id == input::StandardEvents::PreviewPointerReleased.GetId()) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            OnPointerReleasedCore(*pe);
        }
    } else if (id == input::StandardEvents::PointerMoved.GetId() || id == input::StandardEvents::PreviewPointerMoved.GetId()) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            OnPointerMovedCore(*pe);
        }
    } else if (id == input::StandardEvents::PointerWheel.GetId()) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            OnPointerWheelCore(*pe);
        }
    } else if (id == input::StandardEvents::PointerEntered.GetId()) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            OnPointerEnteredCore(*pe);
        }
    } else if (id == input::StandardEvents::PointerExited.GetId()) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            OnPointerExitedCore(*pe);
        }
    } else if (id == input::StandardEvents::KeyDown.GetId() || id == input::StandardEvents::PreviewKeyDown.GetId()) {
        if (auto* ke = dynamic_cast<input::KeyEventArgs*>(&e)) {
            OnKeyDownCore(*ke);
        }
    } else if (id == input::StandardEvents::KeyUp.GetId() || id == input::StandardEvents::PreviewKeyUp.GetId()) {
        if (auto* ke = dynamic_cast<input::KeyEventArgs*>(&e)) {
            OnKeyUpCore(*ke);
        }
    } else if (id == input::StandardEvents::TextInput.GetId()) {
        if (auto* te = dynamic_cast<input::TextInputEventArgs*>(&e)) {
            OnTextInputCore(*te);
        }
    } else if (id == input::StandardEvents::ImeComposition.GetId()) {
        if (auto* ie = dynamic_cast<input::ImeCompositionEventArgs*>(&e)) {
            OnImeCompositionCore(*ie);
        }
    } else if (id == input::StandardEvents::GotFocus.GetId()) {
        if (auto* fe = dynamic_cast<input::FocusEventArgs*>(&e)) {
            OnGotFocusCore(*fe);
        }
    } else if (id == input::StandardEvents::LostFocus.GetId()) {
        if (auto* fe = dynamic_cast<input::FocusEventArgs*>(&e)) {
            OnLostFocusCore(*fe);
        }
    }

    // 2. 派發給註冊的監聽器列表 (複製一份避免回呼中修改容器引發迭代器失效)
    auto handlersCopy = m_eventHandlers;
    for (auto& entry : handlersCopy) {
        if (entry.eventId == id) {
            if (!e.IsHandled() || entry.handledEventsToo) {
                entry.handler(e);
            }
        }
    }
}

void UIElement::OnPointerPressed(PointerEventHandler handler) {
    AddHandler(input::StandardEvents::PointerPressed, [handler](input::RoutedEventArgs& e) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            handler(*pe);
        }
    });
}

void UIElement::OnPointerReleased(PointerEventHandler handler) {
    AddHandler(input::StandardEvents::PointerReleased, [handler](input::RoutedEventArgs& e) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            handler(*pe);
        }
    });
}

void UIElement::OnPointerMoved(PointerEventHandler handler) {
    AddHandler(input::StandardEvents::PointerMoved, [handler](input::RoutedEventArgs& e) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            handler(*pe);
        }
    });
}

void UIElement::OnPointerWheel(PointerEventHandler handler) {
    AddHandler(input::StandardEvents::PointerWheel, [handler](input::RoutedEventArgs& e) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            handler(*pe);
        }
    });
}

void UIElement::OnPointerEntered(PointerEventHandler handler) {
    AddHandler(input::StandardEvents::PointerEntered, [handler](input::RoutedEventArgs& e) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            handler(*pe);
        }
    });
}

void UIElement::OnPointerExited(PointerEventHandler handler) {
    AddHandler(input::StandardEvents::PointerExited, [handler](input::RoutedEventArgs& e) {
        if (auto* pe = dynamic_cast<input::PointerEventArgs*>(&e)) {
            handler(*pe);
        }
    });
}

void UIElement::OnKeyDown(KeyEventHandler handler) {
    AddHandler(input::StandardEvents::KeyDown, [handler](input::RoutedEventArgs& e) {
        if (auto* ke = dynamic_cast<input::KeyEventArgs*>(&e)) {
            handler(*ke);
        }
    });
}

void UIElement::OnKeyUp(KeyEventHandler handler) {
    AddHandler(input::StandardEvents::KeyUp, [handler](input::RoutedEventArgs& e) {
        if (auto* ke = dynamic_cast<input::KeyEventArgs*>(&e)) {
            handler(*ke);
        }
    });
}

void UIElement::OnTextInput(TextInputEventHandler handler) {
    AddHandler(input::StandardEvents::TextInput, [handler](input::RoutedEventArgs& e) {
        if (auto* te = dynamic_cast<input::TextInputEventArgs*>(&e)) {
            handler(*te);
        }
    });
}

void UIElement::OnImeComposition(ImeCompositionEventHandler handler) {
    AddHandler(input::StandardEvents::ImeComposition, [handler](input::RoutedEventArgs& e) {
        if (auto* ie = dynamic_cast<input::ImeCompositionEventArgs*>(&e)) {
            handler(*ie);
        }
    });
}

void UIElement::OnGotFocus(FocusEventHandler handler) {
    AddHandler(input::StandardEvents::GotFocus, [handler](input::RoutedEventArgs& e) {
        if (auto* fe = dynamic_cast<input::FocusEventArgs*>(&e)) {
            handler(*fe);
        }
    });
}

void UIElement::OnLostFocus(FocusEventHandler handler) {
    AddHandler(input::StandardEvents::LostFocus, [handler](input::RoutedEventArgs& e) {
        if (auto* fe = dynamic_cast<input::FocusEventArgs*>(&e)) {
            handler(*fe);
        }
    });
}

Visual* UIElement::HitTest(Point localPoint) {
    if (!m_isHitTestVisible) return nullptr;
    return Visual::HitTest(localPoint);
}

void UIElement::SetFocusedInternal(bool focused) noexcept {
    if (m_isFocused != focused) {
        m_isFocused = focused;
        InvalidateVisual();
    }
}

std::shared_ptr<void> UIElement::GetDataContext() const {
    if (m_dataContext) {
        return m_dataContext;
    }
    if (auto parent = GetParentUIElement()) {
        return parent->GetDataContext();
    }
    return nullptr;
}

void UIElement::SetDataContext(std::shared_ptr<void> dataContext) {
    m_dataContext = dataContext;
}

} // namespace core
} // namespace cppui
