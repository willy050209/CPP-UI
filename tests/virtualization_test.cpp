/**
 * @file virtualization_test.cpp
 * @brief UI 虛擬化排版核心 (VirtualizingStackPanel) 與 ListBox 巨量資料單元測試
 */

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include "compat/compat.hpp"
#include "ui/layout/VirtualizingStackPanel.hpp"
#include "ui/controls/ListBox.hpp"
#include "ui/controls/TextBlock.hpp"

using namespace cppui;

static void TestVirtualizingStackPanelBasic() {
    auto panel = std::make_shared<layout::VirtualizingStackPanel>();
    panel->SetItemCount(10000);
    panel->SetItemHeight(24.0f);
    assert(panel->GetItemCount() == 10000);
    assert(panel->GetItemHeight() == 24.0f);

    size_t generateCount = 0;
    size_t recycleCount = 0;

    panel->SetContainerGenerator([&generateCount](size_t index) -> std::shared_ptr<core::UIElement> {
        (void)index;
        generateCount++;
        return std::make_shared<controls::TextBlock>("Item");
    });

    panel->SetContainerRecycler([&recycleCount](size_t index, core::UIElement* container) {
        (void)index;
        (void)container;
        recycleCount++;
    });

    // 視區高度 120px (約 5 個項目可見 + 緩衝區)
    panel->UpdateViewport(0.0f, 120.0f);
    assert(panel->GetActiveContainerCount() > 0);
    assert(panel->GetActiveContainerCount() <= 10);
    assert(generateCount == panel->GetActiveContainerCount());

    // 測量高度應為總量虛擬估算高 10000 * 24 = 240000px
    panel->Measure(core::Size(200.0f, 600.0f));
    assert(panel->GetDesiredSize().height == 240000.0f);

    // 滾動視區至第 5000 項目
    float offset = 5000.0f * 24.0f;
    panel->UpdateViewport(offset, 120.0f);
    // 容器應自池中回收複用，總生成數不應大幅暴增
    assert(panel->GetActiveContainerCount() > 0);
    assert(recycleCount > 0);

    // 清空重置
    panel->ClearPool();
    assert(panel->GetActiveContainerCount() == 0);
    assert(panel->GetRecycledPoolCount() == 0);

    std::cout << "[PASS] TestVirtualizingStackPanelBasic" << std::endl;
}

static void TestListBoxVirtualization() {
    controls::ListBox listBox;
    assert(listBox.IsVirtualizationEnabled());

    // 快速注入 50,000 筆資料
    for (size_t i = 0; i < 50000; ++i) {
        listBox.AddItem("Virtual Row #" + std::to_string(i));
    }
    assert(listBox.GetItemCount() == 50000);

    // 測量階段
    core::Size desired = listBox.MeasureOverride(core::Size(300.0f, 500.0f));
    assert(desired.height > 100000.0f);

    // 滾動與選取測試
    listBox.SetSelectedIndex(25000);
    assert(listBox.GetSelectedIndex() == 25000);
    assert(listBox.GetSelectedItem() == "Virtual Row #25000");

    listBox.ScrollIntoView(25000);
    assert(listBox.GetScrollOffset() > 0.0f);

    // 虛擬化開關切換
    listBox.SetVirtualizationEnabled(false);
    assert(!listBox.IsVirtualizationEnabled());
    listBox.SetVirtualizationEnabled(true);
    assert(listBox.IsVirtualizationEnabled());

    std::cout << "[PASS] TestListBoxVirtualization" << std::endl;
}

int main() {
    TestVirtualizingStackPanelBasic();
    TestListBoxVirtualization();
    std::cout << "All Virtualization Tests Passed Successfully!" << std::endl;
    return 0;
}
