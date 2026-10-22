/**
 * @file designer_surface_test.cpp
 * @brief CPPUI Designer 階段 3 視覺化畫布設計表面 (Design Surface) 單元測試
 */

#include <cassert>
#include <iostream>
#include <string>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"
#include "ui/input/PointerEventArgs.hpp"

using namespace cppui;
using namespace cppui::designer;
using namespace cppui::designer::surface;

static void TestSelectionModel() {
    compat::println("[Test 1] SelectionModel State & Events...");

    SelectionModel sm;
    assert(sm.GetSelectionCount() == 0);
    assert(!sm.GetPrimarySelection().has_value());

    size_t signalCallCount = 0;
    std::vector<std::string> lastSelection;

    core::LifetimeToken token = core::LifetimeToken::Create();
    auto conn = sm.SelectionChangedSignal().Connect([&](const std::vector<std::string>& sel) {
        signalCallCount++;
        lastSelection = sel;
    });

    // 1. 單選
    sm.SetSelection("node-1");
    assert(sm.GetSelectionCount() == 1);
    assert(sm.IsSelected("node-1"));
    assert(sm.GetPrimarySelection().value_or("") == "node-1");
    assert(signalCallCount == 1);
    assert(lastSelection.size() == 1 && lastSelection[0] == "node-1");

    // 2. 多選追加
    sm.AddToSelection("node-2");
    assert(sm.GetSelectionCount() == 2);
    assert(sm.IsSelected("node-1"));
    assert(sm.IsSelected("node-2"));
    assert(sm.GetPrimarySelection().value_or("") == "node-2");
    assert(signalCallCount == 2);

    // 3. 移除指定選取
    sm.RemoveFromSelection("node-1");
    assert(sm.GetSelectionCount() == 1);
    assert(!sm.IsSelected("node-1"));
    assert(sm.IsSelected("node-2"));
    assert(signalCallCount == 3);

    // 4. 清空
    sm.ClearSelection();
    assert(sm.GetSelectionCount() == 0);
    assert(!sm.GetPrimarySelection().has_value());
    assert(signalCallCount == 4);

    compat::println("  [PASS] SelectionModel single/multi-selection & signals verified.");
}

static void TestDesignerHitTest() {
    compat::println("[Test 2] DesignerHitTest Handles & Cursors...");

    core::Rect rect(100.0f, 100.0f, 200.0f, 80.0f);

    // 八向 Handle 命中測試
    assert(DesignerHitTest::HitTestHandle(rect, core::Point(100.0f, 100.0f)) == HandleType::TopLeft);
    assert(GetCursorForHandle(HandleType::TopLeft) == controls::CursorType::SizeNWSE);

    assert(DesignerHitTest::HitTestHandle(rect, core::Point(300.0f, 180.0f)) == HandleType::BottomRight);
    assert(GetCursorForHandle(HandleType::BottomRight) == controls::CursorType::SizeNWSE);

    assert(DesignerHitTest::HitTestHandle(rect, core::Point(200.0f, 100.0f)) == HandleType::TopCenter);
    assert(GetCursorForHandle(HandleType::TopCenter) == controls::CursorType::SizeNS);

    assert(DesignerHitTest::HitTestHandle(rect, core::Point(300.0f, 140.0f)) == HandleType::MiddleRight);
    assert(GetCursorForHandle(HandleType::MiddleRight) == controls::CursorType::SizeWE);

    assert(DesignerHitTest::HitTestHandle(rect, core::Point(200.0f, 140.0f)) == HandleType::None);

    // 階層節點命中測試
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "canvasRoot");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "testBtn");
    std::string btnUuid = btn->GetUuid();

    auto addRes = root->AddChild(std::move(btn));
    assert(addRes.has_value());

    std::map<std::string, core::Rect> boundsMap;
    boundsMap[root->GetUuid()] = core::Rect(0.0f, 0.0f, 800.0f, 600.0f);
    boundsMap[btnUuid] = core::Rect(50.0f, 50.0f, 120.0f, 40.0f);

    // 命中按鈕內部
    WidgetNode* hit = DesignerHitTest::HitTestNode(root.get(), boundsMap, core::Point(60.0f, 60.0f));
    assert(hit != nullptr);
    assert(hit->GetUuid() == btnUuid);

    // 命中 Canvas 空白處
    WidgetNode* hitCanvas = DesignerHitTest::HitTestNode(root.get(), boundsMap, core::Point(300.0f, 300.0f));
    assert(hitCanvas != nullptr);
    assert(hitCanvas->GetUuid() == root->GetUuid());

    // 畫布外部未命中
    WidgetNode* hitNone = DesignerHitTest::HitTestNode(root.get(), boundsMap, core::Point(900.0f, 900.0f));
    assert(hitNone == nullptr);

    compat::println("  [PASS] Handle hit detection, cursors and hierarchical node hit tested.");
}

static void TestSnapEngine() {
    compat::println("[Test 3] SnapEngine Pure Functional Snapping & Smart Guides...");

    // 1. 網格吸附測試 (gridSize = 8.0, threshold = 6.0)
    assert(SnapEngine::SnapToGrid(15.0f, 8.0f, 6.0f) == 16.0f);
    assert(SnapEngine::SnapToGrid(17.0f, 8.0f, 6.0f) == 16.0f);
    assert(SnapEngine::SnapToGrid(12.0f, 8.0f, 2.0f) == 12.0f); // 超出 threshold 不吸附

    // 2. 智慧兄弟元件導引線吸附 (左緣對齊)
    core::Rect target(98.0f, 50.0f, 100.0f, 40.0f); // X=98，差 2px 與 sibling 左緣 100 對齊
    core::Rect sibling(100.0f, 150.0f, 120.0f, 40.0f);

    SurfaceOptions opts;
    opts.snapThreshold = 5.0f;
    opts.snapToGuides = true;
    opts.snapToGrid = false; // 單獨測試導引線

    std::vector<core::Rect> sibList = { sibling };
    SnapResult res = SnapEngine::CalculateSnap(target, sibList, opts);

    assert(res.snappedX == true);
    assert(res.snappedPos.x == 100.0f);
    assert(res.activeGuides.size() == 1);
    assert(res.activeGuides[0].orientation == SnapOrientation::Vertical);
    assert(res.activeGuides[0].position == 100.0f);

    // 3. 垂直中心線 (Center-to-Center) 吸附 (使用不同高度確保僅 Center 對齊，頂底邊緣不吸附)
    core::Rect target2(200.0f, 156.0f, 80.0f, 30.0f); // CenterY = 171
    core::Rect sibling2(50.0f, 140.0f, 80.0f, 60.0f); // CenterY = 170 (差 1px), X 軸相距 70px 不吸附
    std::vector<core::Rect> sibList2 = { sibling2 };

    SnapResult res2 = SnapEngine::CalculateSnap(target2, sibList2, opts);
    assert(res2.snappedX == false);
    assert(res2.snappedY == true);
    assert(res2.snappedPos.x == 200.0f);
    assert(res2.snappedPos.y == 155.0f);
    assert(res2.activeGuides.size() == 1);
    assert(res2.activeGuides[0].orientation == SnapOrientation::Horizontal);
    assert(res2.activeGuides[0].position == 170.0f);

    compat::println("  [PASS] SnapEngine grid and smart guide calculations verified.");
}

static void TestDesignSurfacePointerDragAndUndo() {
    compat::println("[Test 4] DesignSurface Pointer Drag & Command Undo/Redo...");

    FormDocumentNode doc;
    auto rootCanvas = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "myButton");
    btn->SetProperty("Width", 120.0);
    btn->SetProperty("Height", 40.0);
    btn->SetAttachedProperty("Canvas.Left", 50.0);
    btn->SetAttachedProperty("Canvas.Top", 50.0);
    std::string btnUuid = btn->GetUuid();

    auto addRes = rootCanvas->AddChild(std::move(btn));
    assert(addRes.has_value());
    doc.SetRoot(std::move(rootCanvas));

    CommandHistoryManager history;
    DesignSurface surface(&doc, &history);
    surface.SetBounds(0.0f, 0.0f, 800.0f, 600.0f);
    surface.SetNodeBounds(btnUuid, core::Rect(50.0f, 50.0f, 120.0f, 40.0f));

    SurfaceOptions opts = surface.GetOptions();
    opts.snapToGrid = false;
    surface.SetOptions(opts);

    // 1. 滑鼠按下按鈕
    input::PointerEventArgs pressEvt(
        input::StandardEvents::PointerPressed,
        &surface,
        core::Point(60.0f, 60.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerPressed(pressEvt);

    assert(surface.GetCurrentDragMode() == DragMode::Move);
    assert(surface.GetSelectionModel().IsSelected(btnUuid));

    // 2. 拖曳位移 (+40, +50)
    input::PointerEventArgs moveEvt(
        input::StandardEvents::PointerMoved,
        &surface,
        core::Point(100.0f, 110.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerMoved(moveEvt);

    core::Rect movedBounds = surface.GetNodeBounds(btnUuid);
    assert(movedBounds.x == 90.0f);
    assert(movedBounds.y == 100.0f);

    // 3. 釋放滑鼠完成拖曳
    input::PointerEventArgs releaseEvt(
        input::StandardEvents::PointerReleased,
        &surface,
        core::Point(100.0f, 110.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerReleased(releaseEvt);
    assert(surface.GetCurrentDragMode() == DragMode::None);

    // 4. 驗證 Undo 回溯原始位置
    assert(surface.Undo() == true);
    WidgetNode* btnNode = doc.FindByUuid(btnUuid);
    assert(btnNode != nullptr);
    auto leftVal = btnNode->GetAttachedProperty("Canvas.Left");
    assert(leftVal.has_value() && leftVal->Is<double>());
    assert(leftVal->Get<double>().value() == 50.0);

    // 5. 驗證 Redo 重做
    assert(surface.Redo() == true);
    auto redoneLeft = btnNode->GetAttachedProperty("Canvas.Left");
    assert(redoneLeft.has_value() && redoneLeft->Is<double>());
    assert(redoneLeft->Get<double>().value() == 90.0);

    compat::println("  [PASS] DesignSurface drag and drop with full Undo/Redo verified.");
}

static void TestDesignSurfaceResizeHandle() {
    compat::println("[Test 5] DesignSurface 8-Handle Resizing...");

    FormDocumentNode doc;
    auto rootCanvas = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "myButton");
    btn->SetProperty("Width", 100.0);
    btn->SetProperty("Height", 50.0);
    btn->SetAttachedProperty("Canvas.Left", 100.0);
    btn->SetAttachedProperty("Canvas.Top", 100.0);
    std::string btnUuid = btn->GetUuid();

    auto addRes = rootCanvas->AddChild(std::move(btn));
    assert(addRes.has_value());
    doc.SetRoot(std::move(rootCanvas));

    CommandHistoryManager history;
    DesignSurface surface(&doc, &history);
    surface.SetBounds(0.0f, 0.0f, 800.0f, 600.0f);
    surface.SetNodeBounds(btnUuid, core::Rect(100.0f, 100.0f, 100.0f, 50.0f));

    // 先選取該元件
    surface.GetSelectionModel().SetSelection(btnUuid);

    // 點擊右下角 (BottomRight) 控制點 (200, 150)
    input::PointerEventArgs pressEvt(
        input::StandardEvents::PointerPressed,
        &surface,
        core::Point(200.0f, 150.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerPressed(pressEvt);

    assert(surface.GetCurrentDragMode() == DragMode::Resize);
    assert(surface.GetActiveHandle() == HandleType::BottomRight);

    // 拖曳向右下擴展 (+50, +30) -> 新尺寸應為 (150, 80)
    input::PointerEventArgs moveEvt(
        input::StandardEvents::PointerMoved,
        &surface,
        core::Point(250.0f, 180.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerMoved(moveEvt);

    core::Rect resized = surface.GetNodeBounds(btnUuid);
    assert(resized.width == 150.0f);
    assert(resized.height == 80.0f);

    // 釋放
    input::PointerEventArgs releaseEvt(
        input::StandardEvents::PointerReleased,
        &surface,
        core::Point(250.0f, 180.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerReleased(releaseEvt);
    assert(surface.GetCurrentDragMode() == DragMode::None);

    compat::println("  [PASS] 8-Handle resizing correctly updates width/height in AST.");
}

static void TestRubberBandAndDeleteSelected() {
    compat::println("[Test 6] Rubber-band Box Selection & DeleteSelected...");

    FormDocumentNode doc;
    auto rootCanvas = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");

    auto btn1 = std::make_unique<WidgetNode>(NodeType::Button, "btn1");
    std::string id1 = btn1->GetUuid();
    auto btn2 = std::make_unique<WidgetNode>(NodeType::Button, "btn2");
    std::string id2 = btn2->GetUuid();

    auto r1 = rootCanvas->AddChild(std::move(btn1));
    assert(r1.has_value());
    auto r2 = rootCanvas->AddChild(std::move(btn2));
    assert(r2.has_value());
    doc.SetRoot(std::move(rootCanvas));

    CommandHistoryManager history;
    DesignSurface surface(&doc, &history);
    surface.SetBounds(0.0f, 0.0f, 800.0f, 600.0f);
    surface.SetNodeBounds(id1, core::Rect(100.0f, 100.0f, 80.0f, 30.0f));
    surface.SetNodeBounds(id2, core::Rect(200.0f, 100.0f, 80.0f, 30.0f));

    // 框選涵蓋兩個按鈕之矩形 (50, 50) -> (300, 200)
    input::PointerEventArgs pressEvt(
        input::StandardEvents::PointerPressed,
        &surface,
        core::Point(50.0f, 50.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerPressed(pressEvt);
    assert(surface.GetCurrentDragMode() == DragMode::RubberBand);

    input::PointerEventArgs moveEvt(
        input::StandardEvents::PointerMoved,
        &surface,
        core::Point(300.0f, 200.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerMoved(moveEvt);

    assert(surface.GetSelectionModel().IsSelected(id1));
    assert(surface.GetSelectionModel().IsSelected(id2));
    assert(surface.GetSelectionModel().GetSelectionCount() == 2);

    input::PointerEventArgs releaseEvt(
        input::StandardEvents::PointerReleased,
        &surface,
        core::Point(300.0f, 200.0f),
        pal::RawMouseButton::Left
    );
    surface.OnPointerReleased(releaseEvt);

    // 刪除選取之元件
    assert(surface.DeleteSelected() == true);
    assert(doc.FindByUuid(id1) == nullptr);
    assert(doc.FindByUuid(id2) == nullptr);
    assert(surface.GetSelectionModel().GetSelectionCount() == 0);

    // 撤銷刪除 (Undo)
    assert(surface.Undo() == true); // 還原第二個
    assert(surface.Undo() == true); // 還原第一個
    assert(doc.FindByUuid(id1) != nullptr);
    assert(doc.FindByUuid(id2) != nullptr);

    compat::println("  [PASS] Rubber-band selection & DeleteSelected with subtree Undo verified.");
}

static void TestDesignSurfaceViewportScrolling() {
    compat::println("[Test 7] Design Surface Viewport Scrolling & Typography Initialization...");

    FormDocumentNode doc;
    doc.SetRoot(std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas"));
    CommandHistoryManager history;
    DesignSurface surface(&doc, &history);
    surface.SetBounds(0.0f, 0.0f, 800.0f, 600.0f);

    // 1. 預設滾動偏移量為 (0, 0)
    core::Point offset = surface.GetScrollOffset();
    assert(offset.x == 0.0f);
    assert(offset.y == 0.0f);

    // 2. 預設內容尺寸至少涵蓋 FormSettings (800, 600)
    core::Size extent = surface.GetContentExtent();
    assert(extent.width >= 800.0f);
    assert(extent.height >= 600.0f);

    // 3. 新增越界元件以擴充內容尺寸
    std::string btnId = surface.AddNewWidget(NodeType::Button, core::Point(1200.0f, 900.0f));
    assert(!btnId.empty());

    core::Size extentAfter = surface.GetContentExtent();
    assert(extentAfter.width >= 1320.0f);
    assert(extentAfter.height >= 932.0f);

    // 4. 驗證 AddNewWidget 預設 Typography 與外觀屬性
    WidgetNode* btnNode = doc.FindByUuid(btnId);
    assert(btnNode != nullptr);
    assert(btnNode->GetProperty("FontSize").has_value());
    assert(btnNode->GetProperty("FontSize")->Get<double>().value() == 13.0);
    assert(btnNode->GetProperty("FontFamily").has_value());
    assert(btnNode->GetProperty("FontFamily")->Get<std::string>().value() == "Segoe UI");
    assert(btnNode->GetProperty("FontWeight").has_value());
    assert(btnNode->GetProperty("FontWeight")->Get<int32_t>().value() == 400);
    assert(btnNode->GetProperty("Visibility").has_value());
    assert(btnNode->GetProperty("Visibility")->Get<layout::Visibility>().value() == layout::Visibility::Visible);

    // 5. 設定滾動偏移量並驗證 clamping
    surface.SetScrollOffset(120.0f, 80.0f);
    assert(surface.GetScrollOffset().x == 120.0f);
    assert(surface.GetScrollOffset().y == 80.0f);

    // 負數邊界 clamp 至 0
    surface.SetScrollOffset(-50.0f, -30.0f);
    assert(surface.GetScrollOffset().x == 0.0f);
    assert(surface.GetScrollOffset().y == 0.0f);

    // 6. 模擬滾輪事件 (PointerWheel)
    input::PointerEventArgs wheelDown(
        input::StandardEvents::PointerWheel,
        &surface,
        core::Point(200.0f, 200.0f),
        pal::RawMouseButton::None,
        pal::KeyModifiers::None,
        0, // clickCount
        0, // wheelDeltaX
        -120 // wheelDeltaY: 往下滾
    );
    surface.OnPointerWheel(wheelDown);
    assert(surface.GetScrollOffset().y > 0.0f);

    float scrolledY = surface.GetScrollOffset().y;

    // Shift 鍵橫向滾輪
    input::PointerEventArgs wheelShift(
        input::StandardEvents::PointerWheel,
        &surface,
        core::Point(200.0f, 200.0f),
        pal::RawMouseButton::None,
        pal::KeyModifiers::Shift,
        0, // clickCount
        0, // wheelDeltaX
        -120 // wheelDeltaY
    );
    surface.OnPointerWheel(wheelShift);
    assert(surface.GetScrollOffset().x > 0.0f);
    assert(surface.GetScrollOffset().y == scrolledY); // Y 軸不變

    compat::println("  [PASS] Viewport scrolling and typography defaults verified.");
}

int main() {
    compat::println("=================================================");
    compat::println(" [CPPUI Designer Phase 3] Design Surface Tests");
    compat::println("=================================================");

    TestSelectionModel();
    TestDesignerHitTest();
    TestSnapEngine();
    TestDesignSurfacePointerDragAndUndo();
    TestDesignSurfaceResizeHandle();
    TestRubberBandAndDeleteSelected();
    TestDesignSurfaceViewportScrolling();

    compat::println("=================================================");
    compat::println(" All 7 Design Surface Test Suites Passed!");
    compat::println("=================================================");

    return 0;
}
