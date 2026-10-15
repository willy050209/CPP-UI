/**
 * @file DataGrid.hpp
 * @brief 高階多欄表格控制項，支援欄寬計算 (Auto/Pixel/Star)、標頭排序、斑馬線背景與選取行高亮
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 表格欄位長度單位列舉。
/// </summary>
enum class DataGridLengthUnit : uint32_t {
    Auto,
    Pixel,
    Star
};

/// <summary>
/// 表格欄位寬度規格描述。
/// </summary>
struct DataGridLength {
    DataGridLengthUnit unit = DataGridLengthUnit::Auto;
    float value = 1.0f;

    /// <summary>
    /// 自動計算寬度。
    /// </summary>
    static DataGridLength Auto() noexcept {
        DataGridLength l;
        l.unit = DataGridLengthUnit::Auto;
        l.value = 1.0f;
        return l;
    }

    /// <summary>
    /// 固定像素寬度。
    /// </summary>
    /// <param name="pixels">像素值。</param>
    static DataGridLength Pixel(float pixels) noexcept {
        DataGridLength l;
        l.unit = DataGridLengthUnit::Pixel;
        l.value = pixels;
        return l;
    }

    /// <summary>
    /// 比例加權寬度 (Star)。
    /// </summary>
    /// <param name="stars">加權比例，預設為 1.0f。</param>
    static DataGridLength Star(float stars = 1.0f) noexcept {
        DataGridLength l;
        l.unit = DataGridLengthUnit::Star;
        l.value = stars;
        return l;
    }
};

/// <summary>
/// 欄位排序方向列舉。
/// </summary>
enum class DataGridSortDirection : uint32_t {
    None,
    Ascending,
    Descending
};

/// <summary>
/// 表格欄位定義資訊。
/// </summary>
struct DataGridColumn {
    std::string header;
    DataGridLength width = DataGridLength::Auto();
    DataGridSortDirection sortDirection = DataGridSortDirection::None;
    float actualWidth = 0.0f;
};

/// <summary>
/// 高階多欄表格控制項 (DataGrid)
/// 支援 Auto/Pixel/Star 欄寬運算、欄位點擊排序、斑馬線列背景與虛擬化行切片渲染。
/// </summary>
class DataGrid : public Control {
public:
    using SelectionChangedHandler = std::function<void(int32_t)>;
    using SortHandler = std::function<void(size_t columnIndex, DataGridSortDirection direction)>;

    /// <summary>
    /// 建構預設表格控制項。
    /// </summary>
    DataGrid();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~DataGrid() override = default;

    /// <summary>
    /// 新增欄位。
    /// </summary>
    /// <param name="header">欄位標題文字。</param>
    /// <param name="width">欄寬規格 (預設為 Auto)。</param>
    void AddColumn(compat::string_view header, DataGridLength width = DataGridLength::Auto());

    /// <summary>
    /// 新增欄位實例。
    /// </summary>
    /// <param name="column">欄位結構。</param>
    void AddColumn(const DataGridColumn& column);

    /// <summary>
    /// 取得欄位總數。
    /// </summary>
    /// <returns>欄位數量。</returns>
    COMPAT_NODISCARD size_t GetColumnCount() const noexcept { return m_columns.size(); }

    /// <summary>
    /// 取得指定索引之欄位資訊 (唯讀)。
    /// </summary>
    /// <param name="index">欄位索引。</param>
    /// <returns>欄位常數引用。</returns>
    /// <exception cref="std::out_of_range">當 index 超出範圍時擲出。</exception>
    COMPAT_NODISCARD const DataGridColumn& GetColumn(size_t index) const;

    /// <summary>
    /// 取得指定索引之欄位資訊 (可修改)。
    /// </summary>
    /// <param name="index">欄位索引。</param>
    /// <returns>欄位引用。</returns>
    /// <exception cref="std::out_of_range">當 index 超出範圍時擲出。</exception>
    COMPAT_NODISCARD DataGridColumn& GetColumn(size_t index);

    /// <summary>
    /// 新增一列資料。
    /// </summary>
    /// <param name="rowValues">該列所有儲存格字串值。</param>
    void AddRow(std::vector<std::string> rowValues);

    /// <summary>
    /// 設定指定儲存格之文字內容。
    /// </summary>
    /// <param name="row">列索引。</param>
    /// <param name="col">欄索引。</param>
    /// <param name="value">儲存格內容。</param>
    /// <exception cref="std::out_of_range">當 row 或 col 超出範圍時擲出。</exception>
    void SetCellValue(size_t row, size_t col, compat::string_view value);

    /// <summary>
    /// 取得指定儲存格之文字內容。
    /// </summary>
    /// <param name="row">列索引。</param>
    /// <param name="col">欄索引。</param>
    /// <returns>儲存格文字視圖。</returns>
    /// <exception cref="std::out_of_range">當 row 或 col 超出範圍時擲出。</exception>
    COMPAT_NODISCARD compat::string_view GetCellValue(size_t row, size_t col) const;

    /// <summary>
    /// 取得目前資料列總數。
    /// </summary>
    /// <returns>資料列數量。</returns>
    COMPAT_NODISCARD size_t GetRowCount() const noexcept { return m_rows.size(); }

    /// <summary>
    /// 清除所有資料列。
    /// </summary>
    void ClearRows();

    /// <summary>
    /// 取得目前選取列索引 (-1 表示無選取)。
    /// </summary>
    /// <returns>選取列索引。</returns>
    COMPAT_NODISCARD int32_t GetSelectedIndex() const noexcept { return m_selectedIndex; }

    /// <summary>
    /// 設定目前選取列索引 (-1 表示清除選取)。
    /// </summary>
    /// <param name="index">列索引。</param>
    /// <exception cref="std::out_of_range">當 index 小於 -1 或大於等於資料列總數時擲出。</exception>
    void SetSelectedIndex(int32_t index);

    /// <summary>
    /// 註冊選取列變更事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式。</param>
    void OnSelectedIndexChanged(SelectionChangedHandler handler);

    /// <summary>
    /// 註冊標頭排序事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式。</param>
    void OnSort(SortHandler handler);

    /// <summary>
    /// 對指定欄位執行排序切換 (None -> Ascending -> Descending -> Ascending)。
    /// </summary>
    /// <param name="columnIndex">欄位索引。</param>
    /// <exception cref="std::out_of_range">當 columnIndex 超出範圍時擲出。</exception>
    void SortByColumn(size_t columnIndex);

    /// <summary>
    /// 取得標頭列高度 (像素)。
    /// </summary>
    /// <returns>標頭高度。</returns>
    COMPAT_NODISCARD float GetHeaderHeight() const noexcept { return m_headerHeight; }

    /// <summary>
    /// 設定標頭列高度 (像素)。
    /// </summary>
    /// <param name="height">高度數值，必須大於 0.0f。</param>
    /// <exception cref="std::invalid_argument">當 height 小於或等於 0.0f 時擲出。</exception>
    void SetHeaderHeight(float height);

    /// <summary>
    /// 取得資料列高度 (像素)。
    /// </summary>
    /// <returns>資料列高度。</returns>
    COMPAT_NODISCARD float GetRowHeight() const noexcept { return m_rowHeight; }

    /// <summary>
    /// 設定資料列高度 (像素)。
    /// </summary>
    /// <param name="height">高度數值，必須大於 0.0f。</param>
    /// <exception cref="std::invalid_argument">當 height 小於或等於 0.0f 時擲出。</exception>
    void SetRowHeight(float height);

    /// <summary>
    /// 查詢是否啟用斑馬線交替列背景。
    /// </summary>
    /// <returns>若啟用傳回 true。</returns>
    COMPAT_NODISCARD bool IsAlternatingRowBackground() const noexcept { return m_alternatingRowBackground; }

    /// <summary>
    /// 設定是否啟用斑馬線交替列背景。
    /// </summary>
    /// <param name="enabled">true 啟用，false 關閉。</param>
    void SetAlternatingRowBackground(bool enabled);

    /// <summary>
    /// 取得垂直滾動偏移量 (像素)。
    /// </summary>
    /// <returns>滾動偏移值。</returns>
    COMPAT_NODISCARD float GetScrollOffset() const noexcept { return m_scrollOffset; }

    /// <summary>
    /// 設定垂直滾動偏移量。
    /// </summary>
    /// <param name="offset">滾動偏移值。</param>
    void SetScrollOffset(float offset);

    /// <summary>
    /// 排版測量階段，計算表格期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製表格標頭、格線、斑馬線背景與儲存格文字。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣。
    /// </summary>
    /// <param name="theme">主題引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void InitializeHandlers();
    void RecalculateColumnWidths(float totalWidth);

    std::vector<DataGridColumn>          m_columns;
    std::vector<std::vector<std::string>> m_rows;
    int32_t                              m_selectedIndex = -1;
    float                                m_headerHeight = 28.0f;
    float                                m_rowHeight = 24.0f;
    float                                m_scrollOffset = 0.0f;
    bool                                 m_alternatingRowBackground = true;

    core::Color m_headerBackground;
    core::Color m_headerForeground;
    core::Color m_gridLineColor;
    core::Color m_alternatingColor;
    core::Color m_selectionColor;
    core::Color m_selectionTextColor;

    SelectionChangedHandler m_selectionHandler;
    SortHandler             m_sortHandler;
};

} // namespace controls
} // namespace cppui
