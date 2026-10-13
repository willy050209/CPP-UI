/**
 * @file TreeView.cpp
 * @brief 階層樹狀檢視控制項實作
 */

#include "ui/controls/TreeView.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cppui {
namespace controls {

// ---------------------------------------------------------------------------
// TreeViewItem 實作
// ---------------------------------------------------------------------------

TreeViewItem::TreeViewItem()
    : m_text()
    , m_isExpanded(false)
    , m_expandingTriggered(false)
    , m_parent(nullptr) {
}

TreeViewItem::TreeViewItem(compat::string_view text)
    : m_text(text.data(), text.size())
    , m_isExpanded(false)
    , m_expandingTriggered(false)
    , m_parent(nullptr) {
}

void TreeViewItem::SetText(compat::string_view text) {
    m_text.assign(text.data(), text.size());
}

void TreeViewItem::SetExpanded(bool expanded) {
    if (m_isExpanded != expanded) {
        m_isExpanded = expanded;
        if (m_isExpanded) {
            TriggerExpanding();
        }
    }
}

std::shared_ptr<TreeViewItem> TreeViewItem::AddItem(compat::string_view text) {
    auto item = std::make_shared<TreeViewItem>(text);
    AddItem(item);
    return item;
}

void TreeViewItem::AddItem(std::shared_ptr<TreeViewItem> item) {
    if (!item) {
        throw std::invalid_argument("TreeViewItem child cannot be null");
    }
    item->SetParent(this);
    m_items.push_back(std::move(item));
}

void TreeViewItem::RemoveItem(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return;
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        (*it)->SetParent(nullptr);
        m_items.erase(it);
    }
}

void TreeViewItem::ClearItems() {
    for (auto& item : m_items) {
        if (item) {
            item->SetParent(nullptr);
        }
    }
    m_items.clear();
}

void TreeViewItem::TriggerExpanding() {
    if (!m_expandingTriggered && m_expandingHandler) {
        m_expandingTriggered = true;
        m_expandingHandler(*this);
    }
}

// ---------------------------------------------------------------------------
// TreeView 實作
// ---------------------------------------------------------------------------

TreeView::TreeView() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetBackground(theme.Colors().surfaceBackground);
    SetBorderBrush(theme.Colors().controlBorder);
    SetBorderThickness(core::Thickness(1.0f));
    SetPadding(core::Thickness(2.0f));
    SetCornerRadius(theme.Metrics().cornerRadiusSmall);
    SetForeground(theme.Colors().textPrimary);

    m_selectionColor = theme.Colors().accentPrimary;
    m_selectionTextColor = theme.Colors().textOnAccent;
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
    m_hasExplicitForeground = false;

    InitializeHandlers();
}

void TreeView::InitializeHandlers() {
    SetFocusable(true);
    SetTabStop(true);

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!m_enabled || !e.IsLeftButton()) return;
        input::FocusManager::Get().SetFocus(this);

        core::Point pt = e.GetPosition(this);
        float topOffset = m_padding.top + m_borderThickness.top;
        float leftOffset = m_padding.left + m_borderThickness.left;
        float relY = pt.y - topOffset + m_scrollOffset;

        if (relY >= 0.0f) {
            size_t idx = static_cast<size_t>(relY / m_itemHeight);
            auto flat = GetFlattenedVisibleItems();
            if (idx < flat.size()) {
                auto& node = flat[idx];
                float nodeX = leftOffset + static_cast<float>(node.level) * m_indentWidth;

                // 檢查是否點擊箭頭區域 (16px 寬度)
                if (pt.x >= nodeX && pt.x <= nodeX + 16.0f && (node.item->HasChildren() || node.item->HasExpandingHandler())) {
                    node.item->SetExpanded(!node.item->IsExpanded());
                    InvalidateVisual();
                } else {
                    SetSelectedItem(node.item);
                }
            }
        }
        e.SetHandled(true);
    });

    OnPointerWheel([this](input::PointerEventArgs& e) {
        if (!m_enabled) return;
        float delta = static_cast<float>(e.GetWheelDeltaY());
        if (delta != 0.0f) {
            SetScrollOffset(m_scrollOffset - delta * 0.5f);
            e.SetHandled(true);
        }
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        if (!m_enabled) return;
        input::Key key = e.GetKey();
        auto flat = GetFlattenedVisibleItems();
        if (flat.empty()) return;

        int32_t currentIndex = -1;
        for (size_t i = 0; i < flat.size(); ++i) {
            if (flat[i].item == m_selectedItem) {
                currentIndex = static_cast<int32_t>(i);
                break;
            }
        }

        if (key == input::Key::Up) { // Up
            if (currentIndex > 0) {
                SetSelectedItem(flat[static_cast<size_t>(currentIndex - 1)].item);
                e.SetHandled(true);
            } else if (currentIndex == -1) {
                SetSelectedItem(flat[0].item);
                e.SetHandled(true);
            }
        } else if (key == input::Key::Down) { // Down
            if (currentIndex + 1 < static_cast<int32_t>(flat.size())) {
                SetSelectedItem(flat[static_cast<size_t>(currentIndex + 1)].item);
                e.SetHandled(true);
            } else if (currentIndex == -1) {
                SetSelectedItem(flat[0].item);
                e.SetHandled(true);
            }
        } else if (key == input::Key::Right) { // Right
            if (m_selectedItem) {
                if (!m_selectedItem->IsExpanded() && (m_selectedItem->HasChildren() || m_selectedItem->HasExpandingHandler())) {
                    m_selectedItem->SetExpanded(true);
                    InvalidateVisual();
                    e.SetHandled(true);
                } else if (m_selectedItem->IsExpanded() && m_selectedItem->HasChildren()) {
                    SetSelectedItem(m_selectedItem->GetItems()[0].get());
                    e.SetHandled(true);
                }
            }
        } else if (key == input::Key::Left) { // Left
            if (m_selectedItem) {
                if (m_selectedItem->IsExpanded()) {
                    m_selectedItem->SetExpanded(false);
                    InvalidateVisual();
                    e.SetHandled(true);
                } else if (m_selectedItem->GetParent()) {
                    SetSelectedItem(m_selectedItem->GetParent());
                    e.SetHandled(true);
                }
            }
        }
    });
}

std::shared_ptr<TreeViewItem> TreeView::AddRootItem(compat::string_view text) {
    auto item = std::make_shared<TreeViewItem>(text);
    AddRootItem(item);
    return item;
}

void TreeView::AddRootItem(std::shared_ptr<TreeViewItem> item) {
    if (!item) {
        throw std::invalid_argument("TreeView root item cannot be null");
    }
    item->SetParent(nullptr);
    m_rootItems.push_back(std::move(item));
    InvalidateVisual();
}

void TreeView::RemoveRootItem(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return;
    auto it = std::find(m_rootItems.begin(), m_rootItems.end(), item);
    if (it != m_rootItems.end()) {
        if (m_selectedItem == it->get()) {
            SetSelectedItem(nullptr);
        }
        m_rootItems.erase(it);
        InvalidateVisual();
    }
}

void TreeView::ClearItems() {
    SetSelectedItem(nullptr);
    m_rootItems.clear();
    m_scrollOffset = 0.0f;
    InvalidateVisual();
}

void TreeView::SetSelectedItem(TreeViewItem* item) {
    if (m_selectedItem != item) {
        m_selectedItem = item;
        if (m_selectionHandler) {
            m_selectionHandler(m_selectedItem);
        }
        InvalidateVisual();
    }
}

void TreeView::SetItemHeight(float height) {
    if (height <= 0.0f) {
        throw std::invalid_argument("ItemHeight must be positive");
    }
    if (std::abs(m_itemHeight - height) > 0.001f) {
        m_itemHeight = height;
        InvalidateVisual();
    }
}

void TreeView::SetIndentWidth(float indent) {
    if (indent < 0.0f) {
        throw std::invalid_argument("IndentWidth cannot be negative");
    }
    if (std::abs(m_indentWidth - indent) > 0.001f) {
        m_indentWidth = indent;
        InvalidateVisual();
    }
}

void TreeView::SetScrollOffset(float offset) {
    float clamped = (std::max)(0.0f, offset);
    if (std::abs(m_scrollOffset - clamped) > 0.001f) {
        m_scrollOffset = clamped;
        InvalidateVisual();
    }
}

void TreeView::OnSelectedItemChanged(SelectionChangedHandler handler) {
    m_selectionHandler = std::move(handler);
}

void TreeView::FlattenItemRecursive(TreeViewItem* item, uint32_t level, std::vector<TreeViewFlatNode>& out) const {
    if (!item) return;
    out.push_back(TreeViewFlatNode{ item, level });
    if (item->IsExpanded()) {
        for (const auto& child : item->GetItems()) {
            FlattenItemRecursive(child.get(), level + 1, out);
        }
    }
}

std::vector<TreeViewFlatNode> TreeView::GetFlattenedVisibleItems() const {
    std::vector<TreeViewFlatNode> flat;
    for (const auto& root : m_rootItems) {
        FlattenItemRecursive(root.get(), 0, flat);
    }
    return flat;
}

core::Size TreeView::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    auto flat = GetFlattenedVisibleItems();
    float height = m_padding.Vertical() + m_borderThickness.Vertical() + m_itemHeight * static_cast<float>(flat.size());
    return core::Size(150.0f, (std::max)(24.0f, height));
}

void TreeView::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    if (m_cornerRadius > 0.0f) {
        dc.DrawRoundedRectangle(bounds, m_cornerRadius, m_background);
    } else {
        dc.DrawRectangle(bounds, m_background);
    }

    if (m_borderBrush.a > 0 && m_borderThickness.top > 0.0f) {
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), m_borderBrush, m_borderThickness.top);
        dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), m_borderBrush, m_borderThickness.bottom);
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), m_borderBrush, m_borderThickness.left);
        dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), m_borderBrush, m_borderThickness.right);
    }

    float contentX = bounds.x + m_padding.left + m_borderThickness.left;
    float contentY = bounds.y + m_padding.top + m_borderThickness.top;
    float contentW = (std::max)(0.0f, bounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
    float contentH = (std::max)(0.0f, bounds.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    if (contentW <= 0.0f || contentH <= 0.0f) return;

    dc.PushClip(core::Rect(contentX, contentY, contentW, contentH));

    auto flat = GetFlattenedVisibleItems();
    float maxScroll = (std::max)(0.0f, static_cast<float>(flat.size()) * m_itemHeight - contentH);
    m_scrollOffset = compat::clamp(m_scrollOffset, 0.0f, maxScroll);

    size_t first = 0;
    size_t last = flat.size();
    if (m_itemHeight > 0.0f) {
        first = static_cast<size_t>(m_scrollOffset / m_itemHeight);
        size_t visibleCount = static_cast<size_t>(std::ceil(contentH / m_itemHeight)) + 2;
        last = (std::min)(flat.size(), first + visibleCount);
    }

    for (size_t i = first; i < last; ++i) {
        const auto& node = flat[i];
        float itemY = contentY + static_cast<float>(i) * m_itemHeight - m_scrollOffset;
        if (itemY + m_itemHeight < contentY || itemY > contentY + contentH) {
            continue;
        }

        core::Rect itemRect(contentX, itemY, contentW, m_itemHeight);
        if (node.item == m_selectedItem) {
            dc.DrawRectangle(itemRect, m_selectionColor);
        }

        float nodeX = contentX + static_cast<float>(node.level) * m_indentWidth;
        core::Color textColor = (node.item == m_selectedItem) ? m_selectionTextColor : m_foreground;
        float textY = itemY + (m_itemHeight - GetFont().GetSize() * 1.25f) * 0.5f;

        // 繪製展開/收合箭頭 (若有子項目或延遲載入)
        if (node.item->HasChildren() || node.item->HasExpandingHandler()) {
            compat::string_view arrow = node.item->IsExpanded() ? "v" : ">";
            dc.DrawText(arrow, core::Point(nodeX + 2.0f, textY), GetFont(), textColor);
        }

        // 繪製文字標籤
        dc.DrawText(node.item->GetText(), core::Point(nodeX + 16.0f, textY), GetFont(), textColor);
    }

    dc.PopClip();
}

void TreeView::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    if (!m_hasExplicitBackground) {
        m_background = theme.Colors().surfaceBackground;
    }
    if (!m_hasExplicitBorder) {
        m_borderBrush = theme.Colors().controlBorder;
    }
    if (!m_hasExplicitForeground) {
        m_foreground = theme.Colors().textPrimary;
    }
    m_selectionColor = theme.Colors().accentPrimary;
    m_selectionTextColor = theme.Colors().textOnAccent;
}

} // namespace controls
} // namespace cppui
