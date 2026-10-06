#include <cassert>
#include <chrono>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Visual.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Canvas.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/rendering/DrawingContext.hpp"

using namespace cppui;
using namespace cppui::core;
using namespace cppui::layout;
using namespace cppui::controls;

int main() {
    compat::println("=================================================");
    compat::println(" [CPPUI M4] Layout Engine & Visual Tree Tests    ");
    compat::println("=================================================");

    // -------------------------------------------------------------------------
    // Test 1: Measure/Arrange 幾何約束數學驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 1] Measure & Arrange Geometry Constraints...");
    {
        auto element = std::make_shared<UIElement>();
        element->SetMargin(Thickness(10.0f, 5.0f, 10.0f, 5.0f)); // H: 20, V: 10
        element->SetWidth(100.0f);
        element->SetHeight(50.0f);
        element->SetMinWidth(50.0f);
        element->SetMaxWidth(200.0f);

        // Measure with large available size
        element->Measure(Size(500.0f, 500.0f));
        Size desired = element->GetDesiredSize();
        // Width = 100 + Margin(20) = 120, Height = 50 + Margin(10) = 60
        assert(std::abs(desired.width - 120.0f) < 0.001f);
        assert(std::abs(desired.height - 60.0f) < 0.001f);

        // Arrange in a 300x200 slot
        element->Arrange(Rect(0.0f, 0.0f, 300.0f, 200.0f));
        Rect renderBounds = element->GetRenderBounds();
        // Since stretch is default, but Width/Height are explicitly set:
        assert(std::abs(renderBounds.width - 100.0f) < 0.001f);
        assert(std::abs(renderBounds.height - 50.0f) < 0.001f);
        assert(std::abs(renderBounds.x - 10.0f) < 0.001f); // margin left
        assert(std::abs(renderBounds.y - 5.0f) < 0.001f);  // margin top

        // Test MinWidth clamp
        element->SetWidth(30.0f); // below MinWidth(50)
        element->Measure(Size(500.0f, 500.0f));
        desired = element->GetDesiredSize();
        assert(std::abs(desired.width - 70.0f) < 0.001f); // 50 + 20

        compat::println("  [PASS] Element desired size and clamp constraints verified.");
    }

    // -------------------------------------------------------------------------
    // Test 2: Horizontal & Vertical Alignment 對齊求解驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 2] Alignment Solvers (Left, Center, Right, Stretch)...");
    {
        // Parent slot: 400 x 300, child: 100 x 50, margin: 0
        auto child = std::make_shared<UIElement>();
        child->SetWidth(100.0f);
        child->SetHeight(50.0f);

        // 1. Center / Center
        child->SetHorizontalAlignment(HorizontalAlignment::Center);
        child->SetVerticalAlignment(VerticalAlignment::Center);
        child->Measure(Size(400.0f, 300.0f));
        child->Arrange(Rect(0.0f, 0.0f, 400.0f, 300.0f));
        Rect b = child->GetRenderBounds();
        // X = (400 - 100) / 2 = 150, Y = (300 - 50) / 2 = 125
        assert(std::abs(b.x - 150.0f) < 0.001f);
        assert(std::abs(b.y - 125.0f) < 0.001f);

        // 2. Right / Bottom
        child->SetHorizontalAlignment(HorizontalAlignment::Right);
        child->SetVerticalAlignment(VerticalAlignment::Bottom);
        child->Arrange(Rect(0.0f, 0.0f, 400.0f, 300.0f));
        b = child->GetRenderBounds();
        // X = 400 - 100 = 300, Y = 300 - 50 = 250
        assert(std::abs(b.x - 300.0f) < 0.001f);
        assert(std::abs(b.y - 250.0f) < 0.001f);

        // 3. Stretch without fixed dimensions
        auto stretchChild = std::make_shared<UIElement>();
        stretchChild->SetMargin(Thickness(10.0f, 10.0f, 10.0f, 10.0f));
        stretchChild->SetHorizontalAlignment(HorizontalAlignment::Stretch);
        stretchChild->SetVerticalAlignment(VerticalAlignment::Stretch);
        stretchChild->Measure(Size(400.0f, 300.0f));
        stretchChild->Arrange(Rect(0.0f, 0.0f, 400.0f, 300.0f));
        b = stretchChild->GetRenderBounds();
        // X = 10, Y = 10, Width = 380, Height = 280
        assert(std::abs(b.x - 10.0f) < 0.001f);
        assert(std::abs(b.y - 10.0f) < 0.001f);
        assert(std::abs(b.width - 380.0f) < 0.001f);
        assert(std::abs(b.height - 280.0f) < 0.001f);

        compat::println("  [PASS] Alignment offsets and stretch boundaries accurately calculated.");
    }

    // -------------------------------------------------------------------------
    // Test 3: StackPanel 線性流式佈局驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 3] StackPanel Linear Flow Layout & Spacing...");
    {
        auto stack = std::make_shared<StackPanel>();
        stack->SetOrientation(Orientation::Vertical);
        stack->SetSpacing(10.0f);

        auto c1 = std::make_shared<UIElement>(); c1->SetHeight(30.0f); c1->SetWidth(100.0f);
        auto c2 = std::make_shared<UIElement>(); c2->SetHeight(40.0f); c2->SetWidth(150.0f);
        auto c3 = std::make_shared<UIElement>(); c3->SetHeight(50.0f); c3->SetWidth(120.0f);

        stack->AddChild(c1);
        stack->AddChild(c2);
        stack->AddChild(c3);

        stack->Measure(Size(500.0f, 500.0f));
        Size desired = stack->GetDesiredSize();
        // Total Height = 30 + 10 + 40 + 10 + 50 = 140
        // Max Width = 150
        assert(std::abs(desired.height - 140.0f) < 0.001f);
        assert(std::abs(desired.width - 150.0f) < 0.001f);

        stack->Arrange(Rect(0.0f, 0.0f, 200.0f, 300.0f));
        // Verify child positions
        assert(std::abs(c1->GetVisualOffset().y - 0.0f) < 0.001f);
        assert(std::abs(c2->GetVisualOffset().y - 40.0f) < 0.001f); // 30 + 10
        assert(std::abs(c3->GetVisualOffset().y - 90.0f) < 0.001f); // 40 + 40 + 10

        // Test Collapsed Child
        auto cCollapsed = std::make_shared<UIElement>();
        cCollapsed->SetHeight(100.0f);
        cCollapsed->SetVisibility(Visibility::Collapsed);
        stack->AddChild(cCollapsed);

        stack->Measure(Size(500.0f, 500.0f));
        assert(std::abs(stack->GetDesiredSize().height - 140.0f) < 0.001f);

        // Test Horizontal StackPanel
        auto hstack = std::make_shared<StackPanel>();
        hstack->SetOrientation(Orientation::Horizontal);
        hstack->SetSpacing(5.0f);
        auto hc1 = std::make_shared<UIElement>(); hc1->SetWidth(40.0f); hc1->SetHeight(20.0f);
        auto hc2 = std::make_shared<UIElement>(); hc2->SetWidth(60.0f); hc2->SetHeight(25.0f);
        hstack->AddChild(hc1);
        hstack->AddChild(hc2);

        hstack->Measure(Size(500.0f, 500.0f));
        assert(std::abs(hstack->GetDesiredSize().width - 105.0f) < 0.001f); // 40 + 5 + 60
        assert(std::abs(hstack->GetDesiredSize().height - 25.0f) < 0.001f);

        compat::println("  [PASS] StackPanel vertical/horizontal flow with spacing verified.");
    }

    // -------------------------------------------------------------------------
    // Test 4: Border 裝飾容器邊界驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 4] Border Container & Content Insetting...");
    {
        auto border = std::make_shared<Border>();
        border->SetBorderThickness(Thickness(2.0f, 2.0f, 2.0f, 2.0f));
        border->SetPadding(Thickness(8.0f, 8.0f, 8.0f, 8.0f));
        border->SetCornerRadius(4.0f);
        border->SetBackground(Color(240, 240, 240, 255));
        border->SetBorderBrush(Color(60, 60, 60, 255));

        auto child = std::make_shared<UIElement>();
        child->SetWidth(80.0f);
        child->SetHeight(40.0f);
        border->SetChild(child);

        border->Measure(Size(500.0f, 500.0f));
        Size desired = border->GetDesiredSize();
        // Extra H = 2*2 + 8*2 = 20; Extra V = 2*2 + 8*2 = 20
        // Desired = (80 + 20, 40 + 20) = (100, 60)
        assert(std::abs(desired.width - 100.0f) < 0.001f);
        assert(std::abs(desired.height - 60.0f) < 0.001f);

        border->Arrange(Rect(10.0f, 10.0f, 120.0f, 80.0f));
        // Child should be arranged inside (border + padding) = (10, 10) offset
        Point childOffset = child->GetVisualOffset();
        assert(std::abs(childOffset.x - 10.0f) < 0.001f);
        assert(std::abs(childOffset.y - 10.0f) < 0.001f);

        // Verify OnRender produces DrawRoundedRect ops
        rendering::DisplayList dl;
        rendering::DrawingContext dc(dl);
        border->Render(dc);
        assert(dl.GetOpCount() >= 2); // Background and Border strokes

        compat::println("  [PASS] Border padding insets and render command generation verified.");
    }

    // -------------------------------------------------------------------------
    // Test 5: Canvas 絕對定位驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 5] Canvas Absolute Positioning...");
    {
        auto canvas = std::make_shared<Canvas>();
        auto c1 = std::make_shared<UIElement>();
        c1->SetWidth(50.0f); c1->SetHeight(50.0f);
        Canvas::SetLeft(*c1, 30.0f);
        Canvas::SetTop(*c1, 40.0f);

        auto c2 = std::make_shared<UIElement>();
        c2->SetWidth(80.0f); c2->SetHeight(60.0f);
        Canvas::SetLeft(*c2, 120.0f);
        Canvas::SetTop(*c2, 200.0f);

        canvas->AddChild(c1);
        canvas->AddChild(c2);

        canvas->Measure(Size(500.0f, 500.0f));
        canvas->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));

        assert(std::abs(c1->GetVisualOffset().x - 30.0f) < 0.001f);
        assert(std::abs(c1->GetVisualOffset().y - 40.0f) < 0.001f);
        assert(std::abs(c2->GetVisualOffset().x - 120.0f) < 0.001f);
        assert(std::abs(c2->GetVisualOffset().y - 200.0f) < 0.001f);

        compat::println("  [PASS] Canvas attached properties Left and Top verified.");
    }

    // -------------------------------------------------------------------------
    // Test 6: Dirty Tracking 隔離性斷言 (Dirty Isolation Assert)
    // -------------------------------------------------------------------------
    compat::println("[Test 6] Dirty Tracking Isolation Invariant...");
    {
        auto root = std::make_shared<Border>();
        auto stack = std::make_shared<StackPanel>();
        auto textBlock = std::make_shared<TextBlock>("Clean State");

        stack->AddChild(textBlock);
        root->SetChild(stack);

        LayoutManager& lm = LayoutManager::Get();
        lm.ResetStats();
        lm.UpdateLayout(*root, Size(600.0f, 400.0f));

        // Initial pass completed
        assert(root->IsMeasureValid());
        assert(root->IsArrangeValid());
        assert(textBlock->IsMeasureValid());
        assert(textBlock->IsArrangeValid());
        assert(lm.GetStats().totalMeasures == 1);
        assert(lm.GetStats().totalArranges == 1);

        // 1. Invalidate Visual ONLY by changing Foreground color
        textBlock->SetForeground(Color(255, 0, 0, 255));
        // Strict Invariant: Measure & Arrange MUST remain valid!
        assert(textBlock->IsMeasureValid());
        assert(textBlock->IsArrangeValid());
        assert(!textBlock->IsVisualValid());
        assert(root->IsMeasureValid());
        assert(root->IsArrangeValid());

        // UpdateLayout should NOT trigger Measure or Arrange!
        lm.UpdateLayout(*root, Size(600.0f, 400.0f));
        assert(lm.GetStats().totalMeasures == 1);
        assert(lm.GetStats().totalArranges == 1);

        // 2. Invalidate Measure by changing Text content
        textBlock->SetText("Dirty State Triggered");
        // Must invalidate TextBlock and bubble up to Stack and Root!
        assert(!textBlock->IsMeasureValid());
        assert(!textBlock->IsArrangeValid());
        assert(!stack->IsMeasureValid());
        assert(!root->IsMeasureValid());

        // UpdateLayout now runs Measure & Arrange pass
        lm.UpdateLayout(*root, Size(600.0f, 400.0f));
        assert(lm.GetStats().totalMeasures == 2);
        assert(lm.GetStats().totalArranges == 2);
        assert(root->IsMeasureValid());
        assert(root->IsArrangeValid());

        compat::println("  [PASS] Dirty tracking isolation: Color changes never trigger layout recalc.");
    }

    // -------------------------------------------------------------------------
    // Test 7: DisplayList 遞迴渲染與 HitTest 幾何命中驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 7] DisplayList Hierarchical Rendering & HitTest...");
    {
        auto root = std::make_shared<StackPanel>();
        root->SetOrientation(Orientation::Vertical);

        auto item1 = std::make_shared<UIElement>();
        item1->SetWidth(100.0f); item1->SetHeight(50.0f);

        auto item2 = std::make_shared<UIElement>();
        item2->SetWidth(100.0f); item2->SetHeight(50.0f);

        root->AddChild(item1);
        root->AddChild(item2);

        root->Measure(Size(400.0f, 400.0f));
        root->Arrange(Rect(10.0f, 20.0f, 400.0f, 400.0f));

        // Coordinate transforms
        Point ptInItem2(10.0f, 15.0f);
        Point screenPt = item2->PointToScreen(ptInItem2);
        // Root is at (10, 20). item2 is at Y = 50 relative to root.
        // Screen = (10 + 0 + 10, 20 + 50 + 15) = (20, 85)
        assert(std::abs(screenPt.x - 20.0f) < 0.001f);
        assert(std::abs(screenPt.y - 85.0f) < 0.001f);

        // Hit Testing in local coordinates of root
        Visual* hitResult = root->HitTest(Point(50.0f, 25.0f));
        assert(hitResult == item1.get());

        hitResult = root->HitTest(Point(50.0f, 75.0f));
        assert(hitResult == item2.get());

        // Hit inside root container but outside items
        hitResult = root->HitTest(Point(300.0f, 300.0f));
        assert(hitResult == root.get());

        // Hit completely outside root
        hitResult = root->HitTest(Point(500.0f, 500.0f));
        assert(hitResult == nullptr);

        compat::println("  [PASS] Coordinate mapping and reverse Z-order HitTesting verified.");
    }

    // -------------------------------------------------------------------------
    // Test 8: 排版引擎壓力測試 (Stress Benchmark 10,000 Elements)
    // -------------------------------------------------------------------------
    compat::println("[Test 8] Layout Engine Stress Benchmark (10,000 Elements)...");
    {
        const size_t branchCount = 100;
        const size_t itemsPerBranch = 100; // 100 * 100 = 10,000 elements
        auto mainContainer = std::make_shared<StackPanel>();
        mainContainer->SetOrientation(Orientation::Vertical);

        for (size_t i = 0; i < branchCount; ++i) {
            auto branch = std::make_shared<StackPanel>();
            branch->SetOrientation(Orientation::Horizontal);
            for (size_t j = 0; j < itemsPerBranch; ++j) {
                auto leaf = std::make_shared<UIElement>();
                leaf->SetWidth(10.0f);
                leaf->SetHeight(5.0f);
                branch->AddChild(leaf);
            }
            mainContainer->AddChild(branch);
        }

        auto start = std::chrono::high_resolution_clock::now();
        mainContainer->Measure(Size(1920.0f, 1080.0f));
        mainContainer->Arrange(Rect(0.0f, 0.0f, 1920.0f, 1080.0f));
        auto end = std::chrono::high_resolution_clock::now();
        double initialMs = std::chrono::duration<double, std::milli>(end - start).count();

        // Cached pass
        start = std::chrono::high_resolution_clock::now();
        mainContainer->Measure(Size(1920.0f, 1080.0f));
        mainContainer->Arrange(Rect(0.0f, 0.0f, 1920.0f, 1080.0f));
        end = std::chrono::high_resolution_clock::now();
        double cachedMs = std::chrono::duration<double, std::milli>(end - start).count();

        compat::println("  [PASS] 10,000 Elements Initial Layout: {:.2f} ms", initialMs);
        compat::println("  [PASS] 10,000 Elements Cached Pass:     {:.4f} ms", cachedMs);
        assert(cachedMs < initialMs);
    }

    compat::println("=================================================");
    compat::println(" ALL M4 LAYOUT ENGINE TESTS PASSED!              ");
    compat::println("=================================================");
    return 0;
}
