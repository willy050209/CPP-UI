/**
 * @file datagrid_test.cpp
 * @brief 表格控制項 (DataGrid, Auto/Pixel/Star 欄寬, 排序, 斑馬線) 單元測試
 */

#include <cassert>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "compat/Compat.hpp"
#include "ui/controls/DataGrid.hpp"

using namespace cppui::controls;

static void TestDataGridColumnsAndRows() {
    DataGrid grid;
    assert(grid.GetColumnCount() == 0);
    assert(grid.GetRowCount() == 0);

    grid.AddColumn("ID", DataGridLength::Pixel(60.0f));
    grid.AddColumn("Name", DataGridLength::Star(2.0f));
    grid.AddColumn("Score", DataGridLength::Auto());

    assert(grid.GetColumnCount() == 3);
    assert(grid.GetColumn(0).header == "ID");
    assert(grid.GetColumn(0).width.unit == DataGridLengthUnit::Pixel);
    assert(grid.GetColumn(1).header == "Name");
    assert(grid.GetColumn(1).width.unit == DataGridLengthUnit::Star);
    assert(grid.GetColumn(2).header == "Score");
    assert(grid.GetColumn(2).width.unit == DataGridLengthUnit::Auto);

    // 注入資料列
    grid.AddRow({ "101", "Charlie", "88" });
    grid.AddRow({ "102", "Alice", "95" });
    grid.AddRow({ "103", "Bob", "82" });
    assert(grid.GetRowCount() == 3);

    assert(grid.GetCellValue(0, 1) == "Charlie");
    assert(grid.GetCellValue(1, 1) == "Alice");
    assert(grid.GetCellValue(2, 1) == "Bob");

    // 修改儲存格內容
    grid.SetCellValue(1, 1, "Alicia");
    assert(grid.GetCellValue(1, 1) == "Alicia");

    // 選取列測試
    int32_t selectedNotified = -1;
    grid.OnSelectedIndexChanged([&selectedNotified](int32_t idx) {
        selectedNotified = idx;
    });

    grid.SetSelectedIndex(2);
    assert(grid.GetSelectedIndex() == 2);
    assert(selectedNotified == 2);

    grid.SetSelectedIndex(-1);
    assert(grid.GetSelectedIndex() == -1);

    std::cout << "[PASS] TestDataGridColumnsAndRows" << std::endl;
}

static void TestDataGridSorting() {
    DataGrid grid;
    grid.AddColumn("Name", DataGridLength::Star());
    grid.AddColumn("Age", DataGridLength::Pixel(80.0f));

    grid.AddRow({ "Charlie", "30" });
    grid.AddRow({ "Alice", "25" });
    grid.AddRow({ "Bob", "28" });

    size_t sortedCol = 999;
    DataGridSortDirection sortedDir = DataGridSortDirection::None;
    grid.OnSort([&sortedCol, &sortedDir](size_t col, DataGridSortDirection dir) {
        sortedCol = col;
        sortedDir = dir;
    });

    // 第一次點擊 Name 欄位：遞增排序 (Ascending)
    grid.SortByColumn(0);
    assert(sortedCol == 0);
    assert(sortedDir == DataGridSortDirection::Ascending);
    assert(grid.GetColumn(0).sortDirection == DataGridSortDirection::Ascending);
    assert(grid.GetCellValue(0, 0) == "Alice");
    assert(grid.GetCellValue(1, 0) == "Bob");
    assert(grid.GetCellValue(2, 0) == "Charlie");

    // 第二次點擊 Name 欄位：遞減排序 (Descending)
    grid.SortByColumn(0);
    assert(sortedDir == DataGridSortDirection::Descending);
    assert(grid.GetColumn(0).sortDirection == DataGridSortDirection::Descending);
    assert(grid.GetCellValue(0, 0) == "Charlie");
    assert(grid.GetCellValue(1, 0) == "Bob");
    assert(grid.GetCellValue(2, 0) == "Alice");

    std::cout << "[PASS] TestDataGridSorting" << std::endl;
}

static void TestDataGridFailFast() {
    DataGrid grid;
    bool caught1 = false;
    try {
        (void)grid.GetColumn(5);
    } catch (const std::out_of_range&) {
        caught1 = true;
    }
    assert(caught1);

    bool caught2 = false;
    try {
        (void)grid.GetCellValue(0, 0);
    } catch (const std::out_of_range&) {
        caught2 = true;
    }
    assert(caught2);

    bool caught3 = false;
    try {
        grid.SetHeaderHeight(0.0f);
    } catch (const std::invalid_argument&) {
        caught3 = true;
    }
    assert(caught3);

    bool caught4 = false;
    try {
        grid.SetRowHeight(-5.0f);
    } catch (const std::invalid_argument&) {
        caught4 = true;
    }
    assert(caught4);

    std::cout << "[PASS] TestDataGridFailFast" << std::endl;
}

int main() {
    TestDataGridColumnsAndRows();
    TestDataGridSorting();
    TestDataGridFailFast();
    std::cout << "All DataGrid Tests Passed Successfully!" << std::endl;
    return 0;
}
