/**
 * @file ToolboxPanel.cpp
 * @brief 視覺化表單設計器垂直分類控制項工具箱實作
 */

#include "ui/designer/toolbox/ToolboxPanel.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include "ui/theme/ThemeManager.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {
namespace designer {
namespace toolbox {

namespace {

bool CaseInsensitiveContains(compat::string_view source, compat::string_view target) {
    if (target.empty()) return true;
    if (source.size() < target.size()) return false;

    auto it = std::search(
        source.begin(), source.end(),
        target.begin(), target.end(),
        [](char ch1, char ch2) {
            return std::tolower(static_cast<unsigned char>(ch1)) ==
                   std::tolower(static_cast<unsigned char>(ch2));
        }
    );
    return it != source.end();
}

std::vector<ToolboxCategory> BuildDefaultCategories() {
    return {
        {
            "常用控制項 (Common Controls)",
            true,
            {
                { NodeType::Button,      "🔘", "按鈕 (Button)",           "Button" },
                { NodeType::Label,       "🏷", "標籤 (Label)",            "Label" },
                { NodeType::TextBox,     "🔤", "文字輸入框 (TextBox)",     "TextBox" },
                { NodeType::TextBlock,   "📝", "文字區塊 (TextBlock)",     "TextBlock" },
                { NodeType::CheckBox,    "☑", "核取方塊 (CheckBox)",     "CheckBox" },
                { NodeType::RadioButton, "🔘", "單選按鈕 (RadioButton)",  "RadioButton" },
                { NodeType::ComboBox,    "▾", "下拉選單 (ComboBox)",     "ComboBox" },
                { NodeType::Slider,      "🎚", "滑桿 (Slider)",           "Slider" },
                { NodeType::ProgressBar, "📊", "進度條 (ProgressBar)",    "ProgressBar" }
            }
        },
        {
            "版面容器 (Containers & Layout)",
            true,
            {
                { NodeType::Grid,         "🔲", "網格佈局 (Grid)",          "Grid" },
                { NodeType::StackPanel,   "📑", "堆疊面板 (StackPanel)",    "StackPanel" },
                { NodeType::Canvas,       "🎨", "絕對畫布 (Canvas)",        "Canvas" },
                { NodeType::Border,       "🖼", "外框邊界 (Border)",        "Border" },
                { NodeType::ScrollViewer, "📜", "捲動檢視器 (ScrollViewer)", "ScrollViewer" }
            }
        },
        {
            "資料與集合 (Data & Lists)",
            true,
            {
                { NodeType::ListBox,    "📋", "清單方塊 (ListBox)",       "ListBox" },
                { NodeType::DataGrid,   "📊", "資料表格 (DataGrid)",      "DataGrid" },
                { NodeType::TreeView,   "🌲", "樹狀檢視 (TreeView)",      "TreeView" },
                { NodeType::PictureBox, "🖼", "圖片方塊 (PictureBox)",    "PictureBox" },
                { NodeType::Separator,  "➖", "分隔線 (Separator)",       "Separator" }
            }
        },
        {
            "元件與對話框 (Components & Dialogs)",
            true,
            {
                { NodeType::Timer,          "⏱", "計時器 (Timer)",            "Timer" },
                { NodeType::MessageBox,     "💬", "訊息對話框 (MessageBox)",    "MessageBox" },
                { NodeType::OpenFileDialog, "📂", "開啟檔案 (OpenFileDialog)", "OpenFileDialog" },
                { NodeType::SaveFileDialog, "💾", "儲存檔案 (SaveFileDialog)", "SaveFileDialog" }
            }
        }
    };
}

struct LayoutRow {
    bool isHeader = false;
    size_t categoryIndex = 0;
    NodeType itemType = NodeType::Button;
    std::string icon;
    std::string text;
    core::Rect rect;
};

} // anonymous namespace

struct ToolboxPanel::Impl {
    std::vector<ToolboxCategory> categories = BuildDefaultCategories();
    compat::optional<NodeType> selectedType;
    std::string filterText;
    float scrollY = 0.0f;

    std::function<void(NodeType)> onDoubleClicked;
    ItemDropCallback onDropped;

    // 互動狀態
    int32_t hoveredRowIndex = -1;
    bool isDragging = false;
    core::Point dragStartPos;
    compat::optional<NodeType> draggingType;

    // 點擊計時器（雙擊偵測）
    NodeType lastClickedType = NodeType::Button;
    std::chrono::steady_clock::time_point lastClickTime;

    std::vector<LayoutRow> ComputeLayout(float width) const {
        std::vector<LayoutRow> rows;
        float y = 4.0f; // 頂部微調留白
        const float headerHeight = 27.0f;
        const float itemHeight = 29.0f;

        for (size_t c = 0; c < categories.size(); ++c) {
            const auto& cat = categories[c];

            // 統計篩選後的項目數
            size_t visibleCount = 0;
            for (const auto& itm : cat.items) {
                if (filterText.empty() ||
                    CaseInsensitiveContains(itm.displayName, filterText) ||
                    CaseInsensitiveContains(itm.englishName, filterText)) {
                    ++visibleCount;
                }
            }

            if (!filterText.empty() && visibleCount == 0) {
                continue;
            }

            LayoutRow hRow;
            hRow.isHeader = true;
            hRow.categoryIndex = c;
            hRow.text = (cat.isExpanded ? "▼ " : "▶ ") + cat.name + " (" + std::to_string(visibleCount) + ")";
            hRow.rect = core::Rect(4.0f, y, width - 12.0f, headerHeight);
            rows.push_back(hRow);
            y += headerHeight + 2.0f;

            if (cat.isExpanded) {
                for (const auto& itm : cat.items) {
                    if (!filterText.empty() &&
                        !CaseInsensitiveContains(itm.displayName, filterText) &&
                        !CaseInsensitiveContains(itm.englishName, filterText)) {
                        continue;
                    }

                    LayoutRow iRow;
                    iRow.isHeader = false;
                    iRow.categoryIndex = c;
                    iRow.itemType = itm.type;
                    iRow.icon = itm.icon;
                    iRow.text = itm.displayName;
                    iRow.rect = core::Rect(8.0f, y, width - 16.0f, itemHeight);
                    rows.push_back(iRow);
                    y += itemHeight + 2.0f;
                }
            }
            y += 4.0f;
        }

        return rows;
    }
};

ToolboxPanel::ToolboxPanel()
    : m_impl(std::make_unique<Impl>()) {
    SetWidth(230.0f);
}

ToolboxPanel::~ToolboxPanel() = default;

const std::vector<ToolboxCategory>& ToolboxPanel::GetCategories() const noexcept {
    return m_impl->categories;
}

void ToolboxPanel::SetCategoryExpanded(size_t categoryIndex, bool expanded) {
    if (categoryIndex < m_impl->categories.size()) {
        m_impl->categories[categoryIndex].isExpanded = expanded;
        InvalidateVisual();
    }
}

void ToolboxPanel::ToggleCategory(size_t categoryIndex) {
    if (categoryIndex < m_impl->categories.size()) {
        m_impl->categories[categoryIndex].isExpanded = !m_impl->categories[categoryIndex].isExpanded;
        InvalidateVisual();
    }
}

compat::optional<NodeType> ToolboxPanel::GetSelectedType() const noexcept {
    return m_impl->selectedType;
}

void ToolboxPanel::SetSelectedType(compat::optional<NodeType> type) {
    m_impl->selectedType = type;
    InvalidateVisual();
}

void ToolboxPanel::SetFilterText(compat::string_view filter) {
    m_impl->filterText = std::string(filter);
    m_impl->scrollY = 0.0f;
    InvalidateVisual();
}

compat::string_view ToolboxPanel::GetFilterText() const noexcept {
    return m_impl->filterText;
}

void ToolboxPanel::OnItemDoubleClicked(std::function<void(NodeType)> callback) {
    m_impl->onDoubleClicked = std::move(callback);
}

void ToolboxPanel::OnItemDropped(ItemDropCallback callback) {
    m_impl->onDropped = std::move(callback);
}

void ToolboxPanel::HandlePointerPressed(input::PointerEventArgs& e) {
    OnPointerPressedCore(e);
}

void ToolboxPanel::HandlePointerMoved(input::PointerEventArgs& e) {
    OnPointerMovedCore(e);
}

void ToolboxPanel::HandlePointerReleased(input::PointerEventArgs& e) {
    OnPointerReleasedCore(e);
}

void ToolboxPanel::OnRender(rendering::DrawingContext& dc) {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    const auto& colors = theme.Colors();

    float w = GetWidth();
    float h = GetHeight();
    if (w <= 0.0f) w = 230.0f;
    if (h <= 0.0f) h = 340.0f;

    // 背景底色與邊框
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, w, h), colors.surfaceBackground);
    dc.DrawLine(core::Point(w - 1.0f, 0.0f), core::Point(w - 1.0f, h), colors.surfaceBorder, 1.0f);

    auto rows = m_impl->ComputeLayout(w);
    float totalContentH = rows.empty() ? 0.0f : (rows.back().rect.y + rows.back().rect.height + 8.0f);

    // 逐筆渲染 (以 scrollY 偏移，並嚴格視區剪裁，絕不溢出重疊下方 Hierarchy)
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        float rowY = r.rect.y - m_impl->scrollY;

        if (rowY + r.rect.height < 0.0f || rowY > h) {
            continue; // 超出邊界略過
        }

        core::Rect screenRect(r.rect.x, rowY, r.rect.width, r.rect.height);

        if (r.isHeader) {
            // 分類群組標頭
            core::Color headerBg = colors.controlBackground;
            if (static_cast<int32_t>(i) == m_impl->hoveredRowIndex) {
                headerBg = colors.controlHoverBackground;
            }
            dc.DrawRoundedRectangle(screenRect, 3.0f, headerBg);
            dc.DrawText(r.text, core::Point(screenRect.x + 6.0f, screenRect.y + 5.0f),
                        text::FontDescriptor("Segoe UI", 12.0f, text::FontWeight::Bold),
                        colors.textPrimary);
        } else {
            // 控制項項目列
            bool isSelected = (m_impl->selectedType && *m_impl->selectedType == r.itemType);
            bool isHovered = (static_cast<int32_t>(i) == m_impl->hoveredRowIndex);

            if (isSelected) {
                dc.DrawRoundedRectangle(screenRect, 4.0f, colors.accentPrimary);
            } else if (isHovered) {
                dc.DrawRoundedRectangle(screenRect, 4.0f, colors.controlHoverBackground);
            }

            core::Color itemTextColor = isSelected ? colors.textOnAccent : colors.textPrimary;

            // 圖示與名稱
            std::string label = r.icon + " " + r.text;
            dc.DrawText(label, core::Point(screenRect.x + 8.0f, screenRect.y + 6.0f),
                        text::FontDescriptor("Segoe UI", 12.5f),
                        itemTextColor);

            // 若選取中，右側繪製武裝提示點
            if (isSelected) {
                core::Rect dot(screenRect.x + screenRect.width - 16.0f, screenRect.y + 10.0f, 8.0f, 8.0f);
                dc.DrawRoundedRectangle(dot, 4.0f, colors.textOnAccent);
            }
        }
    }

    // 右側垂直滾動條 (Scrollbar)
    if (totalContentH > h && h > 20.0f) {
        const float barW = 6.0f;
        float thumbH = (std::max)(24.0f, (h / totalContentH) * (h - 4.0f));
        float maxScroll = totalContentH - h;
        float ratio = m_impl->scrollY / maxScroll;
        ratio = compat::clamp(ratio, 0.0f, 1.0f);
        float thumbY = 2.0f + ratio * (h - 4.0f - thumbH);
        core::Rect thumbRect(w - barW - 2.0f, thumbY, barW, thumbH);
        dc.DrawRoundedRectangle(thumbRect, 3.0f, core::Color(100, 100, 105));
    }
}

void ToolboxPanel::OnPointerWheelCore(input::PointerEventArgs& e) {
    int32_t delta = e.GetWheelDeltaY();
    if (delta == 0) return;

    float h = GetHeight();
    if (h <= 0.0f) h = 340.0f;
    auto rows = m_impl->ComputeLayout(GetWidth());
    float totalH = rows.empty() ? 0.0f : (rows.back().rect.y + rows.back().rect.height + 8.0f);
    float maxScroll = (std::max)(0.0f, totalH - h);

    float step = 32.0f * 1.5f;
    if (delta > 0) {
        m_impl->scrollY = (std::max)(0.0f, m_impl->scrollY - step);
    } else {
        m_impl->scrollY = (std::min)(maxScroll, m_impl->scrollY + step);
    }
    InvalidateVisual();
    e.SetHandled(true);
}

void ToolboxPanel::OnPointerPressedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    core::Point localPt(pt.x, pt.y + m_impl->scrollY);
    auto rows = m_impl->ComputeLayout(GetWidth());

    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        if (r.rect.Contains(localPt)) {
            if (r.isHeader) {
                ToggleCategory(r.categoryIndex);
                e.SetHandled(true);
                return;
            } else {
                auto now = std::chrono::steady_clock::now();
                auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_impl->lastClickTime).count();

                if (m_impl->lastClickedType == r.itemType && durationMs < 400) {
                    // 雙擊觸發
                    if (m_impl->onDoubleClicked) {
                        m_impl->onDoubleClicked(r.itemType);
                    }
                    m_impl->lastClickTime = std::chrono::steady_clock::time_point{};
                } else {
                    // 單擊選取/武裝
                    m_impl->selectedType = r.itemType;
                    m_impl->lastClickedType = r.itemType;
                    m_impl->lastClickTime = now;
                }

                m_impl->dragStartPos = pt;
                m_impl->draggingType = r.itemType;
                m_impl->isDragging = false;
                InvalidateVisual();
                e.SetHandled(true);
                return;
            }
        }
    }
}

void ToolboxPanel::OnPointerMovedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    core::Point localPt(pt.x, pt.y + m_impl->scrollY);
    auto rows = m_impl->ComputeLayout(GetWidth());

    int32_t hitIndex = -1;
    for (size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].rect.Contains(localPt)) {
            hitIndex = static_cast<int32_t>(i);
            break;
        }
    }

    if (m_impl->hoveredRowIndex != hitIndex) {
        m_impl->hoveredRowIndex = hitIndex;
        InvalidateVisual();
    }

    if (m_impl->draggingType) {
        float dx = pt.x - m_impl->dragStartPos.x;
        float dy = pt.y - m_impl->dragStartPos.y;
        if ((dx * dx + dy * dy) > 16.0f) {
            m_impl->isDragging = true;
        }
    }
}

void ToolboxPanel::OnPointerReleasedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);

    if (m_impl->isDragging && m_impl->draggingType && m_impl->onDropped) {
        m_impl->onDropped(*m_impl->draggingType, pt);
    }

    m_impl->isDragging = false;
    m_impl->draggingType.reset();
}

} // namespace toolbox
} // namespace designer
} // namespace cppui
