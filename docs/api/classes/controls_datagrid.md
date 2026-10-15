# DataGrid 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/DataGrid.hpp`](../../../include/ui/controls/DataGrid.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`DataGrid` 是功能完整的多欄資料表格控制項。支援三種欄寬計算模式（Auto 自動適應內容、Pixel 固定像素、Star 權重比例分配）、點擊欄位標頭排序（遞增/遞減箭頭幾何繪製）、交替斑馬線列背景（Alternating Row Background）、選取行高亮以及鍵盤方向鍵行巡覽。

---

## 輔助結構

### DataGridLength 與 DataGridLengthUnit
定義欄位寬度規則：
- `DataGridLength::Auto()`: 依據內容最寬儲存格自動計算。
- `DataGridLength::Pixel(float px)`: 固定像素寬度。
- `DataGridLength::Star(float stars = 1.0f)`: 加權比例分配剩餘寬度。

### DataGridSortDirection
- `DataGridSortDirection::None`: 無排序。
- `DataGridSortDirection::Ascending`: 遞增排序。
- `DataGridSortDirection::Descending`: 遞減排序。

---

## 核心成員方法

### 欄位與列集合操作
- `void AddColumn(compat::string_view header, DataGridLength width = DataGridLength::Auto())`: 新增表格欄位。
- `COMPAT_NODISCARD size_t GetColumnCount() const noexcept`: 取得欄位總數。
- `void AddRow(std::vector<std::string> cells)`: 新增資料列。
- `void SetCellValue(size_t row, size_t col, compat::string_view value)`: 設定特定儲存格文字。
- `COMPAT_NODISCARD compat::string_view GetCellValue(size_t row, size_t col) const`: 讀取特定儲存格文字。
- `COMPAT_NODISCARD size_t GetRowCount() const noexcept`: 取得資料列總數。
- `void ClearRows()`: 清空所有資料列。

### 選取與事件
- `COMPAT_NODISCARD int32_t GetSelectedIndex() const noexcept`: 取得目前選取列的索引（`-1` 表示未選取）。
- `void SetSelectedIndex(int32_t index)`: 設定選取列。
- `void OnSelectionChanged(SelectionChangedHandler handler)`: 註冊選取列變更回呼 `void(int32_t)`.
- `void OnSort(SortHandler handler)`: 註冊點擊標頭排序回呼 `void(size_t columnIndex, DataGridSortDirection direction)`.

---

## 使用範例

```cpp
#include "ui/controls/DataGrid.hpp"

auto grid = std::make_shared<cppui::controls::DataGrid>();
grid->SetWidth(600.0f);
grid->SetHeight(300.0f);

grid->AddColumn("ID", cppui::controls::DataGridLength::Pixel(60.0f));
grid->AddColumn("名稱", cppui::controls::DataGridLength::Star(2.0f));
grid->AddColumn("狀態", cppui::controls::DataGridLength::Star(1.0f));

grid->AddRow({ "1", "應用程式伺服器", "運行中" });
grid->AddRow({ "2", "資料庫叢集", "同步中" });
grid->AddRow({ "3", "快取節點", "離線" });

grid->OnSelectionChanged([](int32_t row) {
    compat::println("使用者選取了第 {} 列", row);
});
```
