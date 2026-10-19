#include "ui/layout/Grid.hpp"
#include <algorithm>
#include <numeric>

namespace cppui {
namespace layout {

std::unordered_map<const core::UIElement*, Grid::CellInfo>& Grid::GetAttachedStorage() {
    static std::unordered_map<const core::UIElement*, Grid::CellInfo> s_storage;
    return s_storage;
}

void Grid::SetRow(core::UIElement& element, size_t row) {
    GetAttachedStorage()[&element].row = row;
    element.InvalidateMeasure();
}

size_t Grid::GetRow(const core::UIElement& element) noexcept {
    auto& s = GetAttachedStorage();
    auto it = s.find(&element);
    return (it != s.end()) ? it->second.row : 0;
}

void Grid::SetColumn(core::UIElement& element, size_t col) {
    GetAttachedStorage()[&element].col = col;
    element.InvalidateMeasure();
}

size_t Grid::GetColumn(const core::UIElement& element) noexcept {
    auto& s = GetAttachedStorage();
    auto it = s.find(&element);
    return (it != s.end()) ? it->second.col : 0;
}

void Grid::SetRowSpan(core::UIElement& element, size_t span) {
    GetAttachedStorage()[&element].rowSpan = (std::max)(size_t(1), span);
    element.InvalidateMeasure();
}

size_t Grid::GetRowSpan(const core::UIElement& element) noexcept {
    auto& s = GetAttachedStorage();
    auto it = s.find(&element);
    return (it != s.end()) ? it->second.rowSpan : 1;
}

void Grid::SetColumnSpan(core::UIElement& element, size_t span) {
    GetAttachedStorage()[&element].colSpan = (std::max)(size_t(1), span);
    element.InvalidateMeasure();
}

size_t Grid::GetColumnSpan(const core::UIElement& element) noexcept {
    auto& s = GetAttachedStorage();
    auto it = s.find(&element);
    return (it != s.end()) ? it->second.colSpan : 1;
}

void Grid::AddRow(RowDefinition row) {
    m_rowDefinitions.push_back(row);
    InvalidateMeasure();
}

void Grid::AddColumn(ColumnDefinition col) {
    m_columnDefinitions.push_back(col);
    InvalidateMeasure();
}

core::Size Grid::MeasureOverride(core::Size availableSize) {
    std::vector<RowDefinition> rows = m_rowDefinitions;
    if (rows.empty()) rows.emplace_back(GridLength::Star(1.0f));

    std::vector<ColumnDefinition> cols = m_columnDefinitions;
    if (cols.empty()) cols.emplace_back(GridLength::Star(1.0f));

    // 1. 初始化 Pixel 尺寸
    for (auto& r : rows) {
        if (r.height.type == GridUnitType::Pixel) {
            r.actualHeight = (std::max)(r.minHeight, (std::min)(r.height.value, r.maxHeight));
        } else {
            r.actualHeight = 0.0f;
        }
    }
    for (auto& c : cols) {
        if (c.width.type == GridUnitType::Pixel) {
            c.actualWidth = (std::max)(c.minWidth, (std::min)(c.width.value, c.maxWidth));
        } else {
            c.actualWidth = 0.0f;
        }
    }

    // 2. 測量 Auto 單元格
    for (auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) continue;

        size_t r = (std::min)(GetRow(*child), rows.size() - 1);
        size_t c = (std::min)(GetColumn(*child), cols.size() - 1);

        if (rows[r].height.type == GridUnitType::Auto || cols[c].width.type == GridUnitType::Auto) {
            child->Measure(availableSize);
            core::Size desired = child->GetDesiredSize();

            if (rows[r].height.type == GridUnitType::Auto) {
                rows[r].actualHeight = (std::max)(rows[r].actualHeight, desired.height);
            }
            if (cols[c].width.type == GridUnitType::Auto) {
                cols[c].actualWidth = (std::max)(cols[c].actualWidth, desired.width);
            }
        }
    }

    // 3. 計算剩餘空間分配給 Star 單元格
    float allocatedW = 0.0f;
    float totalStarW = 0.0f;
    for (const auto& c : cols) {
        if (c.width.type != GridUnitType::Star) allocatedW += c.actualWidth;
        else totalStarW += c.width.value;
    }

    float allocatedH = 0.0f;
    float totalStarH = 0.0f;
    for (const auto& r : rows) {
        if (r.height.type != GridUnitType::Star) allocatedH += r.actualHeight;
        else totalStarH += r.height.value;
    }

    float remainingW = (std::max)(0.0f, availableSize.width - allocatedW);
    float remainingH = (std::max)(0.0f, availableSize.height - allocatedH);

    for (auto& c : cols) {
        if (c.width.type == GridUnitType::Star && totalStarW > 0.0f) {
            c.actualWidth = (c.width.value / totalStarW) * remainingW;
            c.actualWidth = (std::max)(c.minWidth, (std::min)(c.actualWidth, c.maxWidth));
        }
    }

    for (auto& r : rows) {
        if (r.height.type == GridUnitType::Star && totalStarH > 0.0f) {
            r.actualHeight = (r.height.value / totalStarH) * remainingH;
            r.actualHeight = (std::max)(r.minHeight, (std::min)(r.actualHeight, r.maxHeight));
        }
    }

    // 4. 測量 Star 單元格的子節點
    for (auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) continue;

        size_t r = (std::min)(GetRow(*child), rows.size() - 1);
        size_t c = (std::min)(GetColumn(*child), cols.size() - 1);
        size_t rSpan = (std::min)(GetRowSpan(*child), rows.size() - r);
        size_t cSpan = (std::min)(GetColumnSpan(*child), cols.size() - c);

        float cellW = 0.0f;
        for (size_t i = 0; i < cSpan; ++i) cellW += cols[c + i].actualWidth;

        float cellH = 0.0f;
        for (size_t i = 0; i < rSpan; ++i) cellH += rows[r + i].actualHeight;

        child->Measure(core::Size(cellW, cellH));
    }

    float totalW = 0.0f;
    for (const auto& c : cols) totalW += c.actualWidth;

    float totalH = 0.0f;
    for (const auto& r : rows) totalH += r.actualHeight;

    return core::Size(totalW, totalH);
}

void Grid::ArrangeOverride(core::Size finalSize) {
    std::vector<RowDefinition> rows = m_rowDefinitions;
    if (rows.empty()) rows.emplace_back(GridLength::Star(1.0f));

    std::vector<ColumnDefinition> cols = m_columnDefinitions;
    if (cols.empty()) cols.emplace_back(GridLength::Star(1.0f));

    // 計算 Auto 單元格尺寸 (使用子元件的 DesiredSize)
    for (auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) continue;
        size_t r = (std::min)(GetRow(*child), rows.size() - 1);
        size_t c = (std::min)(GetColumn(*child), cols.size() - 1);
        if (rows[r].height.type == GridUnitType::Auto) {
            rows[r].actualHeight = (std::max)(rows[r].actualHeight, child->GetDesiredSize().height);
        }
        if (cols[c].width.type == GridUnitType::Auto) {
            cols[c].actualWidth = (std::max)(cols[c].actualWidth, child->GetDesiredSize().width);
        }
    }

    // 計算最終欄寬與列高
    float allocatedW = 0.0f;
    float totalStarW = 0.0f;
    for (auto& c : cols) {
        if (c.width.type == GridUnitType::Pixel) c.actualWidth = c.width.value;
        if (c.width.type != GridUnitType::Star) allocatedW += c.actualWidth;
        else totalStarW += c.width.value;
    }

    float allocatedH = 0.0f;
    float totalStarH = 0.0f;
    for (auto& r : rows) {
        if (r.height.type == GridUnitType::Pixel) r.actualHeight = r.height.value;
        if (r.height.type != GridUnitType::Star) allocatedH += r.actualHeight;
        else totalStarH += r.height.value;
    }

    float remainingW = (std::max)(0.0f, finalSize.width - allocatedW);
    float remainingH = (std::max)(0.0f, finalSize.height - allocatedH);

    for (auto& c : cols) {
        if (c.width.type == GridUnitType::Star && totalStarW > 0.0f) {
            c.actualWidth = (c.width.value / totalStarW) * remainingW;
        }
    }

    for (auto& r : rows) {
        if (r.height.type == GridUnitType::Star && totalStarH > 0.0f) {
            r.actualHeight = (r.height.value / totalStarH) * remainingH;
        }
    }

    // 計算各單元格起點 offset
    std::vector<float> colOffsets(cols.size() + 1, 0.0f);
    for (size_t i = 0; i < cols.size(); ++i) {
        colOffsets[i + 1] = colOffsets[i] + cols[i].actualWidth;
    }

    std::vector<float> rowOffsets(rows.size() + 1, 0.0f);
    for (size_t i = 0; i < rows.size(); ++i) {
        rowOffsets[i + 1] = rowOffsets[i] + rows[i].actualHeight;
    }

    // Arrange 各子元件
    for (auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) continue;

        size_t r = (std::min)(GetRow(*child), rows.size() - 1);
        size_t c = (std::min)(GetColumn(*child), cols.size() - 1);
        size_t rSpan = (std::min)(GetRowSpan(*child), rows.size() - r);
        size_t cSpan = (std::min)(GetColumnSpan(*child), cols.size() - c);

        float x = colOffsets[c];
        float y = rowOffsets[r];
        float w = colOffsets[c + cSpan] - x;
        float h = rowOffsets[r + rSpan] - y;

        child->Arrange(core::Rect(x, y, w, h));
    }
}

} // namespace layout
} // namespace cppui
