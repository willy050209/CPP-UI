/**
 * @file diagnostics_test.cpp
 * @brief 診斷與 DevTools (DirtyRectTracker, VisualTreeInspector) 單元測試
 */

#include <cassert>
#include <iostream>
#include <memory>
#include "compat/compat.hpp"
#include "ui/diagnostics/DiagnosticsOptions.hpp"
#include "ui/diagnostics/DirtyRectTracker.hpp"
#include "ui/diagnostics/VisualTreeInspector.hpp"
#include "ui/controls/Button.hpp"
#include "ui/layout/StackPanel.hpp"

using namespace cppui;
using namespace cppui::diagnostics;

static void TestDirtyRectTracker() {
    auto& tracker = DirtyRectTracker::Get();

    DiagnosticsOptions opts;
    opts.enableDirtyRectOverlay = true;
    opts.showFpsCounter = true;
    opts.overlayDurationMs = 300;
    tracker.SetOptions(opts);

    assert(tracker.GetOptions().enableDirtyRectOverlay);
    assert(tracker.GetOptions().showFpsCounter);
    assert(tracker.GetOptions().overlayDurationMs == 300);

    tracker.ClearDirtyRects();
    assert(tracker.GetDirtyRectCount() == 0);

    // 記錄有效重繪區域
    tracker.RecordDirtyRect(core::Rect(10.0f, 20.0f, 200.0f, 150.0f));
    assert(tracker.GetDirtyRectCount() == 1);

    // 忽略無效尺寸
    tracker.RecordDirtyRect(core::Rect(0.0f, 0.0f, 0.0f, 0.0f));
    assert(tracker.GetDirtyRectCount() == 1);

    // 更新 FPS
    for (int32_t i = 0; i < 60; ++i) {
        tracker.UpdateFps(1.0f / 60.0f);
    }
    assert(tracker.GetCurrentFps() > 0.0f);

    // 關閉重繪高亮時應清空記錄
    opts.enableDirtyRectOverlay = false;
    tracker.SetOptions(opts);
    assert(tracker.GetDirtyRectCount() == 0);

    std::cout << "[PASS] TestDirtyRectTracker" << std::endl;
}

static void TestVisualTreeInspector() {
    auto& inspector = VisualTreeInspector::Get();

    inspector.SetVisible(false);
    assert(!inspector.IsVisible());

    inspector.Toggle();
    assert(inspector.IsVisible());

    inspector.Toggle();
    assert(!inspector.IsVisible());

    // 吸管拾取模式
    inspector.EnablePickerMode(true);
    assert(inspector.IsPickerModeEnabled());

    auto rootPanel = std::make_shared<layout::StackPanel>();
    auto btn1 = std::make_shared<controls::Button>("Click Me");
    auto btn2 = std::make_shared<controls::Button>("Cancel");
    rootPanel->AddChild(btn1);
    rootPanel->AddChild(btn2);

    inspector.SetHoveredElement(btn1.get());
    inspector.InspectElement(btn1.get());
    assert(inspector.GetInspectedElement() == btn1.get());

    // 遍歷視覺樹結構 (包含 StackPanel, Button 1, Button 1 內之 TextBlock, Button 2, Button 2 內之 TextBlock)
    auto tree = inspector.BuildVisualTree(rootPanel.get());
    assert(tree.size() == 5);
    assert(tree[0].element == rootPanel.get());
    assert(tree[0].depth == 0);
    assert(tree[1].element == btn1.get());
    assert(tree[1].depth == 1);
    assert(tree[3].element == btn2.get());
    assert(tree[3].depth == 1);

    inspector.EnablePickerMode(false);
    assert(!inspector.IsPickerModeEnabled());
    inspector.SetVisible(false);

    std::cout << "[PASS] TestVisualTreeInspector" << std::endl;
}

int main() {
    TestDirtyRectTracker();
    TestVisualTreeInspector();
    std::cout << "All Diagnostics Tests Passed Successfully!" << std::endl;
    return 0;
}
