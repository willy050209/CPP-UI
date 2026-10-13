/*
 * File: popup_test.cpp
 * Purpose: Comprehensive unit tests for OverlayLayer, Popup, Smart Placement,
 *          Light-Dismiss, ToolTip, ContextMenu, and ComboBox controls.
 */

#include "ui/app/Form.hpp"
#include "ui/controls/OverlayLayer.hpp"
#include "ui/controls/Popup.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/ToolTip.hpp"
#include "ui/controls/ToolTipService.hpp"
#include "ui/controls/MenuItem.hpp"
#include "ui/controls/ContextMenu.hpp"
#include "ui/controls/ComboBox.hpp"
#include "ui/input/InputManager.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace cppui;

static void TestPopupOpenCloseLifecycle() {
    auto form = std::make_shared<app::Form>();
    form->SetFormSize(800, 600);

    auto btn = std::make_shared<controls::Button>("Target Button");
    btn->SetWidth(120.0f);
    btn->SetHeight(32.0f);
    form->SetContent(btn);

    auto popupContent = std::make_shared<controls::Button>("Popup Child");
    popupContent->SetWidth(100.0f);
    popupContent->SetHeight(40.0f);

    auto popup = std::make_shared<controls::Popup>(popupContent);
    popup->SetPlacementTarget(btn.get());
    popup->SetPlacementMode(controls::PlacementMode::Bottom);

    bool openedFired = false;
    bool closedFired = false;
    popup->AddOpenedHandler([&openedFired](controls::Popup&) { openedFired = true; });
    popup->AddClosedHandler([&closedFired](controls::Popup&) { closedFired = true; });

    assert(!popup->IsOpen());
    assert(form->GetOverlayLayer()->GetFloatingElementCount() == 0);

    // 開啟 Popup
    popup->SetIsOpen(true);
    assert(popup->IsOpen());
    assert(openedFired);
    assert(form->GetOverlayLayer()->GetFloatingElementCount() == 1);

    // 關閉 Popup
    popup->SetIsOpen(false);
    assert(!popup->IsOpen());
    assert(closedFired);
    assert(form->GetOverlayLayer()->GetFloatingElementCount() == 0);

    std::cout << "[PASS] TestPopupOpenCloseLifecycle" << std::endl;
}

static void TestSmartPlacementEngineAutoFlip() {
    auto form = std::make_shared<app::Form>();
    form->SetFormSize(800, 600);

    auto btn = std::make_shared<controls::Button>("Bottom Button");
    btn->SetWidth(100.0f);
    btn->SetHeight(30.0f);
    // 將按鈕放置在靠近視窗底部的位置 (y = 550, height = 30 -> bottom = 580)
    btn->SetVisualOffset(core::Point(50.0f, 550.0f));
    form->SetContent(btn);

    auto popupContent = std::make_shared<controls::Button>("Popup Content");
    popupContent->SetWidth(100.0f);
    popupContent->SetHeight(60.0f);

    auto popup = std::make_shared<controls::Popup>(popupContent);
    popup->SetWidth(100.0f);
    popup->SetHeight(60.0f);
    popup->SetPlacementTarget(btn.get());
    popup->SetPlacementMode(controls::PlacementMode::Bottom);

    // 下方空間：600 - 580 = 20px < 60px，上方空間：550px >= 60px
    // 應自動翻轉至上方 (y = 550 - 60 = 490px)
    popup->SetIsOpen(true);
    assert(popup->IsOpen());
    assert(popup->GetVisualOffset().y == 490.0f);

    popup->SetIsOpen(false);

    // 水平右側 Clamp 測試：將按鈕放置在靠右邊緣 (x = 750, width = 100)
    btn->SetVisualOffset(core::Point(750.0f, 100.0f));
    popup->SetPlacementMode(controls::PlacementMode::Bottom);
    popup->SetIsOpen(true);

    // 視窗寬度 800，Popup 寬度 100，最大 X 應被鎖定於 700
    assert(popup->GetVisualOffset().x <= 700.0f);

    popup->SetIsOpen(false);

    std::cout << "[PASS] TestSmartPlacementEngineAutoFlip" << std::endl;
}

static void TestLightDismissExternalPointer() {
    auto form = std::make_shared<app::Form>();
    form->SetFormSize(800, 600);

    auto btn = std::make_shared<controls::Button>("Button");
    btn->SetWidth(100.0f);
    btn->SetHeight(30.0f);
    btn->SetVisualOffset(core::Point(50.0f, 50.0f));
    form->SetContent(btn);

    auto popup = std::make_shared<controls::Popup>(std::make_shared<controls::Button>("Inside"));
    popup->SetWidth(80.0f);
    popup->SetHeight(40.0f);
    popup->SetPlacementTarget(btn.get());
    popup->SetStaysOpen(false);

    popup->SetIsOpen(true);
    assert(popup->IsOpen());

    // 模擬外部點擊 (座標 400, 400，完全落在按鈕與 Popup 之外)
    pal::RawMouseEventData mouseData;
    mouseData.position = core::Point(400.0f, 400.0f);
    mouseData.button = pal::RawMouseButton::Left;
    mouseData.clickCount = 1;
    input::InputManager::Get().ProcessMouseDown(*form, mouseData, pal::KeyModifiers::None);

    // 應自動觸發 Light-Dismiss 收合
    assert(!popup->IsOpen());

    // 測試 StaysOpen == true 模式：點擊外部不應收合
    popup->SetStaysOpen(true);
    popup->SetIsOpen(true);
    assert(popup->IsOpen());

    input::InputManager::Get().ProcessMouseDown(*form, mouseData, pal::KeyModifiers::None);
    assert(popup->IsOpen());

    popup->SetIsOpen(false);

    std::cout << "[PASS] TestLightDismissExternalPointer" << std::endl;
}

static void TestLightDismissEscapeKey() {
    auto form = std::make_shared<app::Form>();
    form->SetFormSize(800, 600);

    auto popup = std::make_shared<controls::Popup>(std::make_shared<controls::Button>("Content"));
    popup->SetPlacementTarget(form.get());
    popup->SetIsOpen(true);
    assert(popup->IsOpen());

    // 模擬按下 ESC 鍵 (0x1B)
    pal::RawKeyEventData keyData;
    keyData.virtualKey = 0x1B;
    keyData.scanCode = 1;
    keyData.isRepeat = false;
    input::InputManager::Get().ProcessKeyDown(*form, keyData, pal::KeyModifiers::None);

    // 應自動收合
    assert(!popup->IsOpen());

    std::cout << "[PASS] TestLightDismissEscapeKey" << std::endl;
}

static void TestComboBoxDropdownSelection() {
    auto combo = std::make_shared<controls::ComboBox>();
    combo->SetWidth(150.0f);
    combo->SetHeight(32.0f);

    combo->AddItem("C++14");
    combo->AddItem("C++17");
    combo->AddItem("C++20");
    combo->AddItem("C++23");

    assert(combo->GetItemCount() == 4);
    assert(combo->GetSelectedIndex() == 0);
    assert(combo->GetSelectedItem() == "C++14");

    int32_t selectedIndexRecorded = -1;
    combo->AddSelectionChangedHandler([&selectedIndexRecorded](controls::ComboBox&, int32_t idx) {
        selectedIndexRecorded = idx;
    });

    combo->SetSelectedIndex(2);
    assert(combo->GetSelectedIndex() == 2);
    assert(combo->GetSelectedItem() == "C++20");
    assert(selectedIndexRecorded == 2);

    assert(!combo->IsDropDownOpen());
    combo->SetIsDropDownOpen(true);
    assert(combo->IsDropDownOpen());

    combo->SetIsDropDownOpen(false);
    assert(!combo->IsDropDownOpen());

    std::cout << "[PASS] TestComboBoxDropdownSelection" << std::endl;
}

static void TestContextMenuAndMenuItemHierarchy() {
    auto menu = std::make_shared<controls::ContextMenu>();
    auto itemFile = std::make_shared<controls::MenuItem>("File");
    auto itemEdit = std::make_shared<controls::MenuItem>("Edit");

    auto itemNew = std::make_shared<controls::MenuItem>("New");
    auto itemOpen = std::make_shared<controls::MenuItem>("Open");
    itemFile->AddMenuItem(itemNew);
    itemFile->AddMenuItem(itemOpen);

    assert(itemFile->HasSubmenu());
    assert(itemFile->GetSubmenuItems().size() == 2);
    assert(!itemEdit->HasSubmenu());

    menu->AddMenuItem(itemFile);
    menu->AddMenuItem(itemEdit);
    assert(menu->GetMenuItemCount() == 2);

    bool clicked = false;
    itemOpen->AddClickHandler([&clicked](controls::MenuItem&) {
        clicked = true;
    });

    itemOpen->TriggerClick();
    assert(clicked);

    std::cout << "[PASS] TestContextMenuAndMenuItemHierarchy" << std::endl;
}

static void TestDefensiveNullParameters() {
    controls::OverlayLayer layer;
    bool caught = false;
    try {
        layer.AddFloatingElement(nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        layer.RemoveFloatingElement(nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        controls::ToolTipService::SetToolTip(nullptr, "test");
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        controls::ContextMenuService::SetContextMenu(nullptr, nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        controls::ContextMenu menu;
        menu.AddMenuItem(nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        controls::MenuItem item("Test");
        item.AddMenuItem(nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    std::cout << "[PASS] TestDefensiveNullParameters" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Running Popup & Overlay System Tests   " << std::endl;
    std::cout << "========================================" << std::endl;

    TestPopupOpenCloseLifecycle();
    TestSmartPlacementEngineAutoFlip();
    TestLightDismissExternalPointer();
    TestLightDismissEscapeKey();
    TestComboBoxDropdownSelection();
    TestContextMenuAndMenuItemHierarchy();
    TestDefensiveNullParameters();

    std::cout << "\nAll 7 Popup & Overlay test suites PASSED successfully!\n" << std::endl;
    return 0;
}
