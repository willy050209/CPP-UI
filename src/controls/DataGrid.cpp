/**
 * @file DataGrid.cpp
 * @brief 高階多欄表格控制項實作
 */

#include "ui/controls/DataGrid.hpp"
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

DataGrid::DataGrid() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetBackground(theme.Colors().surfaceBackground);
    SetBorderBrush(theme.Colors().controlBorder);
    SetBorderThickness(core::Thickness(1.0f));
    SetPadding(core::Thickness(0.0f));
    SetForeground(theme.Colors().textPrimary);

    m_headerBackground = theme.Colors().cardBackground;
    m_headerForeground = theme.Colors().textPrimary;
    m_gridLineColor = theme.Colors().controlBorder;
    m_alternatingColor = core::Color(theme.Colors().cardBackground.r, theme.Colors().cardBackground.g, theme.Colors().cardBackground.b, static_cast<uint8_t>(60));
    m_selectionColor = theme.Colors().accentPrimary;
    m_selectionTextColor = theme.Colors().textOnAccent;

    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
    m_hasExplicitForeground = false;

    InitializeHandlers();
}

void DataGrid::InitializeHandlers() {
    SetFocusable(true);
    SetTabStop(true);

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!m_enabled || !e.IsLeftButton()) return;
        input::FocusManager::Get().SetFocus(this);

        core::Point pt = e.GetPosition(this);
        float topOffset = m_padding.top + m_borderThickness.top;
        float leftOffset = m_padding.left + m_borderThickness.left;

        // 點擊標頭列：觸發排序
        if (pt.y >= topOffset && pt.y < topOffset + m_headerHeight) {
            float colX = leftOffset;
            for (size_t i = 0; i < m_columns.size(); ++i) {
                float w = m_columns[i].actualWidth;
                if (pt.x >= colX && pt.x < colX + w) {
                    SortByColumn(i);
                    break;
                }
                colX += w;
            }
            e.SetHandled(true);
            return;
        }

        // 點擊資料列：觸發選取行
        if (pt.y >= topOffset + m_headerHeight) {
            float dataRelY = pt.y - (topOffset + m_headerHeight) + m_scrollOffset;
            if (dataRelY >= 0.0f && m_rowHeight > 0.0f) {
                size_t r = static_cast<size_t>(dataRelY / m_rowHeight);
                if (r < m_rows.size()) {
                    SetSelectedIndex(static_cast<int32_t>(r));
                }
            }
            e.SetHandled(true);
        }
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
        if (!m_enabled || m_rows.empty()) return;
        input::Key key = e.GetKey();

        if (key == input::Key::Up) { // Up
            if (m_selectedIndex > 0) {
                SetSelectedIndex(m_selectedIndex - 1);
                e.SetHandled(true);
            } else if (m_selectedIndex == -1) {
                SetSelectedIndex(0);
                e.SetHandled(true);
            }
        } else if (key == input::Key::Down) { // Down
            if (m_selectedIndex + 1 < static_cast<int32_t>(m_rows.size())) {
                SetSelectedIndex(m_selectedIndex + 1);
                e.SetHandled(true);
            } else if (m_selectedIndex == -1) {
                SetSelectedIndex(0);
                e.SetHandled(true);
            }
        }
    });
}

void DataGrid::AddColumn(compat::string_view header, DataGridLength width) {
    DataGridColumn col;
    col.header.assign(header.data(), header.size());
    col.width = width;
    col.sortDirection = DataGridSortDirection::None;
    col.actualWidth = 0.0f;
    AddColumn(col);
}

void DataGrid::AddColumn(const DataGridColumn& column) {
    m_columns.push_back(column);
    InvalidateVisual();
}

const DataGridColumn& DataGrid::GetColumn(size_t index) const {
    if (index >= m_columns.size()) {
        throw std::out_of_range("DataGrid column index out of range");
    }
    return m_columns[index];
}

DataGridColumn& DataGrid::GetColumn(size_t index) {
    if (index >= m_columns.size()) {
        throw std::out_of_range("DataGrid column index out of range");
    }
    return m_columns[index];
}

void DataGrid::AddRow(std::vector<std::string> rowValues) {
    m_rows.push_back(std::move(rowValues));
    InvalidateVisual();
}

void DataGrid::SetCellValue(size_t row, size_t col, compat::string_view value) {
    if (row >= m_rows.size()) {
        throw std::out_of_range("DataGrid row index out of range");
    }
    if (col >= m_rows[row].size()) {
        m_rows[row].resize(col + 1);
    }
    m_rows[row][col].assign(value.data(), value.size());
    InvalidateVisual();
}

compat::string_view DataGrid::GetCellValue(size_t row, size_t col) const {
    if (row >= m_rows.size()) {
        throw std::out_of_range("DataGrid row index out of range");
    }
    if (col >= m_rows[row].size()) {
        throw std::out_of_range("DataGrid col index out of range");
    }
    return m_rows[row][col];
}

void DataGrid::ClearRows() {
    m_rows.clear();
    m_selectedIndex = -1;
    m_scrollOffset = 0.0f;
    InvalidateVisual();
}

void DataGrid::SetSelectedIndex(int32_t index) {
    if (index < -1 || index >= static_cast<int32_t>(m_rows.size())) {
        throw std::out_of_range("DataGrid selected index out of range");
    }
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        if (m_selectionHandler) {
            m_selectionHandler(m_selectedIndex);
        }
        InvalidateVisual();
    }
}

void DataGrid::OnSelectedIndexChanged(SelectionChangedHandler handler) {
    m_selectionHandler = std::move(handler);
}

void DataGrid::OnSort(SortHandler handler) {
    m_sortHandler = std::move(handler);
}

void DataGrid::SortByColumn(size_t columnIndex) {
    if (columnIndex >= m_columns.size()) {
        throw std::out_of_range("DataGrid sort column index out of range");
    }

    DataGridSortDirection nextDir = DataGridSortDirection::Ascending;
    if (m_columns[columnIndex].sortDirection == DataGridSortDirection::Ascending) {
        nextDir = DataGridSortDirection::Descending;
    } else if (m_columns[columnIndex].sortDirection == DataGridSortDirection::Descending) {
        nextDir = DataGridSortDirection::Ascending;
    }

    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (i == columnIndex) {
            m_columns[i].sortDirection = nextDir;
        } else {
            m_columns[i].sortDirection = DataGridSortDirection::None;
        }
    }

    // 依據字串字典序排序內部列資料
    std::sort(m_rows.begin(), m_rows.end(), [columnIndex, nextDir](const std::vector<std::string>& a, const std::vector<std::string>& b) {
        const std::string& valA = (columnIndex < a.size()) ? a[columnIndex] : "";
        const std::string& valB = (columnIndex < b.size()) ? b[columnIndex] : "";
        if (nextDir == DataGridSortDirection::Ascending) {
            return valA < valB;
        } else {
            return valA > valB;
        }
    });

    if (m_sortHandler) {
        m_sortHandler(columnIndex, nextDir);
    }

    InvalidateVisual();
}

void DataGrid::SetHeaderHeight(float height) {
    if (height <= 0.0f) {
        throw std::invalid_argument("HeaderHeight must be positive");
    }
    if (std::abs(m_headerHeight - height) > 0.001f) {
        m_headerHeight = height;
        InvalidateVisual();
    }
}

void DataGrid::SetRowHeight(float height) {
    if (height <= 0.0f) {
        throw std::invalid_argument("RowHeight must be positive");
    }
    if (std::abs(m_rowHeight - height) > 0.001f) {
        m_rowHeight = height;
        InvalidateVisual();
    }
}

void DataGrid::SetAlternatingRowBackground(bool enabled) {
    if (m_alternatingRowBackground != enabled) {
        m_alternatingRowBackground = enabled;
        InvalidateVisual();
    }
}

void DataGrid::SetScrollOffset(float offset) {
    float clamped = (std::max)(0.0f, offset);
    if (std::abs(m_scrollOffset - clamped) > 0.001f) {
        m_scrollOffset = clamped;
        InvalidateVisual();
    }
}

void DataGrid::RecalculateColumnWidths(float totalWidth) {
    if (m_columns.empty()) return;

    float allocatedWidth = 0.0f;
    float totalStars = 0.0f;

    for (auto& col : m_columns) {
        if (col.width.unit == DataGridLengthUnit::Pixel) {
            col.actualWidth = (std::max)(20.0f, col.width.value);
            allocatedWidth += col.actualWidth;
        } else if (col.width.unit == DataGridLengthUnit::Auto) {
            col.actualWidth = (std::max)(60.0f, static_cast<float>(col.header.size()) * 9.0f + 24.0f);
            allocatedWidth += col.actualWidth;
        } else if (col.width.unit == DataGridLengthUnit::Star) {
            totalStars += (col.width.value > 0.0f ? col.width.value : 1.0f);
        }
    }

    float remainingWidth = (std::max)(0.0f, totalWidth - allocatedWidth);
    if (totalStars > 0.0f && remainingWidth > 0.0f) {
        for (auto& col : m_columns) {
            if (col.width.unit == DataGridLengthUnit::Star) {
                float weight = (col.width.value > 0.0f ? col.width.value : 1.0f);
                col.actualWidth = (remainingWidth * weight) / totalStars;
            }
        }
    } else {
        for (auto& col : m_columns) {
            if (col.width.unit == DataGridLengthUnit::Star) {
                col.actualWidth = 80.0f;
            }
        }
    }
}

core::Size DataGrid::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    float totalWidth = 0.0f;
    for (const auto& col : m_columns) {
        if (col.width.unit == DataGridLengthUnit::Pixel) {
            totalWidth += col.width.value;
        } else {
            totalWidth += 100.0f;
        }
    }
    float height = m_headerHeight + m_rowHeight * static_cast<float>(m_rows.size()) + m_padding.Vertical() + m_borderThickness.Vertical();
    return core::Size((std::max)(200.0f, totalWidth), (std::max)(100.0f, height));
}

void DataGrid::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    // 繪製背景與邊框
    dc.DrawRectangle(bounds, m_background);

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

    RecalculateColumnWidths(contentW);

    // 1. 繪製標頭列
    core::Rect headerRect(contentX, contentY, contentW, m_headerHeight);
    dc.DrawRectangle(headerRect, m_headerBackground);
    dc.DrawLine(core::Point(contentX, contentY + m_headerHeight), core::Point(contentX + contentW, contentY + m_headerHeight), m_gridLineColor, 1.0f);

    float curColX = contentX;
    float fontSize = GetFont().GetSize();
    float headerTextY = contentY + (m_headerHeight - fontSize * 1.25f) * 0.5f;

    for (size_t c = 0; c < m_columns.size(); ++c) {
        const auto& col = m_columns[c];
        dc.DrawText(col.header, core::Point(curColX + 8.0f, headerTextY), GetFont(), m_headerForeground);

        // 繪製排序指示箭頭
        if (col.sortDirection == DataGridSortDirection::Ascending) {
            dc.DrawText("^", core::Point(curColX + col.actualWidth - 16.0f, headerTextY), GetFont(), m_headerForeground);
        } else if (col.sortDirection == DataGridSortDirection::Descending) {
            dc.DrawText("v", core::Point(curColX + col.actualWidth - 16.0f, headerTextY), GetFont(), m_headerForeground);
        }

        // 欄位垂直分隔線
        if (c + 1 < m_columns.size()) {
            dc.DrawLine(core::Point(curColX + col.actualWidth, contentY), core::Point(curColX + col.actualWidth, contentY + m_headerHeight), m_gridLineColor, 1.0f);
        }
        curColX += col.actualWidth;
    }

    // 2. 繪製資料列區域
    float dataY = contentY + m_headerHeight;
    float dataH = (std::max)(0.0f, contentH - m_headerHeight);
    if (dataH <= 0.0f) return;

    dc.PushClip(core::Rect(contentX, dataY, contentW, dataH));

    float maxScroll = (std::max)(0.0f, static_cast<float>(m_rows.size()) * m_rowHeight - dataH);
    m_scrollOffset = compat::clamp(m_scrollOffset, 0.0f, maxScroll);

    size_t firstRow = 0;
    size_t lastRow = m_rows.size();
    if (m_rowHeight > 0.0f) {
        firstRow = static_cast<size_t>(m_scrollOffset / m_rowHeight);
        size_t visibleCount = static_cast<size_t>(std::ceil(dataH / m_rowHeight)) + 2;
        lastRow = (std::min)(m_rows.size(), firstRow + visibleCount);
    }

    for (size_t r = firstRow; r < lastRow; ++r) {
        float rowY = dataY + static_cast<float>(r) * m_rowHeight - m_scrollOffset;
        if (rowY + m_rowHeight < dataY || rowY > dataY + dataH) continue;

        core::Rect rowRect(contentX, rowY, contentW, m_rowHeight);

        // 列背景 (選取高亮或斑馬線交替)
        if (static_cast<int32_t>(r) == m_selectedIndex) {
            dc.DrawRectangle(rowRect, m_selectionColor);
        } else if (m_alternatingRowBackground && (r % 2 == 1)) {
            dc.DrawRectangle(rowRect, m_alternatingColor);
        }

        // 繪製水平格線
        dc.DrawLine(core::Point(contentX, rowY + m_rowHeight), core::Point(contentX + contentW, rowY + m_rowHeight), m_gridLineColor, 0.5f);

        // 繪製儲存格內容
        float cellX = contentX;
        float cellTextY = rowY + (m_rowHeight - fontSize * 1.25f) * 0.5f;
        core::Color textColor = (static_cast<int32_t>(r) == m_selectedIndex) ? m_selectionTextColor : m_foreground;

        const auto& rowValues = m_rows[r];
        for (size_t c = 0; c < m_columns.size(); ++c) {
            const auto& col = m_columns[c];
            if (c < rowValues.size()) {
                dc.DrawText(rowValues[c], core::Point(cellX + 8.0f, cellTextY), GetFont(), textColor);
            }
            // 垂直格線
            if (c + 1 < m_columns.size()) {
                dc.DrawLine(core::Point(cellX + col.actualWidth, rowY), core::Point(cellX + col.actualWidth, rowY + m_rowHeight), m_gridLineColor, 0.5f);
            }
            cellX += col.actualWidth;
        }
    }

    dc.PopClip();
}

void DataGrid::ApplyTheme(const theme::Theme& theme) {
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
    m_headerBackground = theme.Colors().cardBackground;
    m_headerForeground = theme.Colors().textPrimary;
    m_gridLineColor = theme.Colors().controlBorder;
    m_alternatingColor = core::Color(theme.Colors().cardBackground.r, theme.Colors().cardBackground.g, theme.Colors().cardBackground.b, static_cast<uint8_t>(60));
    m_selectionColor = theme.Colors().accentPrimary;
    m_selectionTextColor = theme.Colors().textOnAccent;
}

} // namespace controls
} // namespace cppui
