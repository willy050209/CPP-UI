/**
 * @file example_features_test.cpp
 * @brief 平台切換 (強制 Skia)、非同步事件 (Dispatcher) 與心臟線漸層繪圖全自動單元測試
 */

#include "compat/compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/AppBuilder.hpp"
#include "ui/app/Form.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Brush.hpp"
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Colors.hpp"
#include "ui/reactive/Signal.hpp"
#include <cassert>
#include <cmath>
#include <thread>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <chrono>

namespace {

constexpr float PI = 3.14159265358979323846f;

// 測試用 Form
class DummyTestForm : public cppui::app::Form {
public:
    DummyTestForm() {
        SetTitle("Dummy Test Form");
        SetFormSize(400, 300);
    }
};

// =============================================================================
// 1. 平台切換與強制 Skia 後端測試
// =============================================================================

static void TestPlatformSwitchForcedSkia() {
    using namespace cppui::app;
    compat::println("[Test] 測試 1: 平台切換 (強制使用 Skia) 與 AppBuilder 配置...");

    // 保存原後端
    PlatformBackend original = Application::GetPlatformBackend();

    // 顯式切換為 Skia
    Application::SetPlatformBackend(PlatformBackend::Skia);
    assert(Application::GetPlatformBackend() == PlatformBackend::Skia);

    // 顯式切換回 Win32
    Application::SetPlatformBackend(PlatformBackend::Win32);
    assert(Application::GetPlatformBackend() == PlatformBackend::Win32);

    // 測試 AppBuilder::UseSkia() 鏈式調用
    auto builder = Application::CreateBuilder();
    builder.UseSkia();
    auto host = builder.Build();
    assert(Application::GetPlatformBackend() == PlatformBackend::Skia);

    // 測試在強制 Skia 後端下 Form 實例化與屬性存取
    auto form = std::make_shared<DummyTestForm>();
    assert(form->GetTitle() == "Dummy Test Form");
    assert(form->GetFormSize().width == 400.0f);
    assert(form->GetFormSize().height == 300.0f);

    // 測試 AppBuilder::UseWin32() 復原
    auto builder2 = Application::CreateBuilder();
    builder2.UseWin32();
    auto host2 = builder2.Build();
    assert(Application::GetPlatformBackend() == PlatformBackend::Win32);

    // 復原環境
    Application::SetPlatformBackend(original);
    compat::println("  ✔ 平台切換 (強制使用 Skia) 驗證通過！");
}

// =============================================================================
// 2. 非同步事件與 Dispatcher 調度測試
// =============================================================================

static void TestAsyncEventsAndDispatcher() {
    using namespace cppui::app;
    using namespace cppui::reactive;
    compat::println("[Test] 測試 2: 非同步事件調度、執行緒親和性與高壓並發投遞...");

    // 驗證主執行緒 CheckAccess
    assert(Application::CheckAccess() == true);

    // 驗證背景執行緒 CheckAccess 為 false
    std::atomic<bool> workerCheckedAccess{true};
    std::thread checkThread([&]() {
        workerCheckedAccess = Application::CheckAccess();
    });
    checkThread.join();
    assert(workerCheckedAccess == false);

    // 驗證單次非同步事件投遞
    std::atomic<bool> taskExecuted{false};
    std::thread worker1([&]() {
        Application::Post([&]() {
            taskExecuted = true;
        });
    });
    worker1.join();

    assert(taskExecuted == false); // 尚未 Drain
    Application::DoEvents();
    assert(taskExecuted == true);

    // 驗證 Reactive Signal 跨執行緒安全更新
    Signal<int32_t> counterSignal{0};
    std::atomic<int32_t> observedValue{0};

    counterSignal.Subscribe([&](int32_t val) {
        assert(Application::CheckAccess() == true); // 訂閱者必須於主執行緒觸發
        observedValue = val;
    });

    std::thread worker2([&]() {
        for (int32_t i = 1; i <= 5; ++i) {
            Application::Post([&counterSignal, i]() {
                counterSignal = i;
            });
        }
    });
    worker2.join();

    Application::DoEvents();
    assert(counterSignal.Get() == 5);
    assert(observedValue == 5);

    // 高壓並發測試：10 個背景執行緒同時向 Application::Post 投遞任務
    constexpr size_t THREAD_COUNT = 10;
    constexpr size_t TASKS_PER_THREAD = 20;
    std::atomic<size_t> totalTasksProcessed{0};
    std::vector<std::thread> threadPool;
    threadPool.reserve(THREAD_COUNT);

    for (size_t t = 0; t < THREAD_COUNT; ++t) {
        threadPool.emplace_back([&totalTasksProcessed]() {
            for (size_t k = 0; k < TASKS_PER_THREAD; ++k) {
                Application::Post([&totalTasksProcessed]() {
                    totalTasksProcessed++;
                });
            }
        });
    }

    for (auto& th : threadPool) {
        if (th.joinable()) th.join();
    }

    Application::DoEvents();
    assert(totalTasksProcessed.load() == THREAD_COUNT * TASKS_PER_THREAD);

    // 契約防禦：傳入空 action 應擲出 std::invalid_argument
    bool caughtNullPost = false;
    try {
        std::function<void()> emptyFunc;
        Application::Post(emptyFunc);
    } catch (const std::invalid_argument&) {
        caughtNullPost = true;
    }
    assert(caughtNullPost);

    compat::println("  ✔ 非同步事件調度、Signal 綁定與並發高壓測試 ({} 任務) 驗證通過！",
                    THREAD_COUNT * TASKS_PER_THREAD);
}

// =============================================================================
// 3. 心臟線幾何演算法測試 (Classical Cardioid & Heart Curve)
// =============================================================================

static void TestCardioidGeometryAndMath() {
    using namespace cppui::core;
    using namespace cppui::graphics;
    compat::println("[Test] 測試 3: 心臟線幾何演算法 (Cardioid & Heart Curve) 與路徑命中...");

    const Point center(300.0f, 300.0f);
    const float scale = 60.0f;
    const uint32_t samples = 180;

    // 1. 古典心臟線生成
    GraphicsPath cardioid;
    std::vector<Point> pts;
    pts.reserve(samples + 1);
    for (uint32_t i = 0; i <= samples; ++i) {
        float theta = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(samples);
        float x = center.x + scale * (2.0f * std::cos(theta) - std::cos(2.0f * theta));
        float y = center.y - scale * (2.0f * std::sin(theta) - std::sin(2.0f * theta));
        pts.push_back(Point(x, y));
    }
    cardioid.AddLines(pts);
    cardioid.CloseFigure();

    assert(cardioid.GetPointCount() >= samples);
    Rect cBounds = cardioid.GetBounds();
    assert(!cBounds.IsEmpty());
    assert(cBounds.width > 100.0f && cBounds.height > 100.0f);

    // 射線點命中測試：內部點應命中，遠處外部點應不命中
    assert(cardioid.IsVisible(center.x, center.y) == true);
    assert(cardioid.IsVisible(0.0f, 0.0f) == false);
    assert(cardioid.IsVisible(1000.0f, 1000.0f) == false);

    // 2. 浪漫心形線生成
    GraphicsPath heart;
    std::vector<Point> hpts;
    hpts.reserve(samples + 1);
    for (uint32_t i = 0; i <= samples; ++i) {
        float t = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(samples);
        float sinT = std::sin(t);
        float x = center.x + 10.0f * 16.0f * sinT * sinT * sinT;
        float y = center.y - 10.0f * (13.0f * std::cos(t) - 5.0f * std::cos(2.0f * t) -
                                      2.0f * std::cos(3.0f * t) - std::cos(4.0f * t));
        hpts.push_back(Point(x, y));
    }
    heart.AddLines(hpts);
    heart.CloseFigure();

    assert(heart.GetPointCount() >= samples);
    Rect hBounds = heart.GetBounds();
    assert(!hBounds.IsEmpty());
    assert(heart.IsVisible(center.x, center.y) == true);
    assert(heart.IsVisible(0.0f, 0.0f) == false);

    compat::println("  ✔ 心臟線與心形幾何路徑生成、閉合與射線點命中測試驗證通過！");
}

// =============================================================================
// 4. 線性漸層色彩插值與心臟線點陣繪圖測試
// =============================================================================

static void TestLinearGradientAndRasterization() {
    using namespace cppui::core;
    using namespace cppui::graphics;
    using namespace cppui::controls;
    compat::println("[Test] 測試 4: LinearGradientBrush 色彩插值與 Graphics 光柵化繪製...");

    // 1. 測試 LinearGradientBrush::GetColorAt 數值精度
    Point p1(0.0f, 0.0f);
    Point p2(100.0f, 0.0f);
    Color red(255, 0, 0, 255);
    Color blue(0, 0, 255, 255);

    LinearGradientBrush brush(p1, p2, red, blue);
    assert(brush.GetStartColor() == red);
    assert(brush.GetEndColor() == blue);

    compat::println("    [Debug] Checking GetColorAt...");
    Color atStart = brush.GetColorAt(Point(0.0f, 0.0f));
    assert(atStart.r == 255 && atStart.g == 0 && atStart.b == 0);

    Color atEnd = brush.GetColorAt(Point(100.0f, 0.0f));
    assert(atEnd.r == 0 && atEnd.g == 0 && atEnd.b == 255);

    Color atMid = brush.GetColorAt(Point(50.0f, 0.0f));
    assert(std::abs(static_cast<int32_t>(atMid.r) - 127) <= 2);
    assert(atMid.g == 0);
    assert(std::abs(static_cast<int32_t>(atMid.b) - 127) <= 2);

    Color atBefore = brush.GetColorAt(Point(-50.0f, 0.0f));
    assert(atBefore.r == 255 && atBefore.b == 0);
    Color atAfter = brush.GetColorAt(Point(150.0f, 0.0f));
    assert(atAfter.r == 0 && atAfter.b == 255);

    compat::println("    [Debug] Rendering rasterization...");
    auto bmp = std::make_shared<Bitmap>(200, 200);
    Graphics g = Graphics::FromImage(bmp);

    Color bg(10, 10, 10, 255);
    g.Clear(bg);

    GraphicsPath path;
    const Point center(100.0f, 100.0f);
    std::vector<Point> pts;
    for (uint32_t i = 0; i <= 90; ++i) {
        float theta = (2.0f * PI * static_cast<float>(i)) / 90.0f;
        float x = center.x + 20.0f * (2.0f * std::cos(theta) - std::cos(2.0f * theta));
        float y = center.y - 20.0f * (2.0f * std::sin(theta) - std::sin(2.0f * theta));
        pts.push_back(Point(x, y));
    }
    path.AddLines(pts);
    path.CloseFigure();

    LinearGradientBrush pathBrush(Point(100.0f, 50.0f), Point(100.0f, 150.0f), red, blue);
    g.FillPath(pathBrush, path);

    Pen outlinePen(cppui::graphics::Colors::White, 2.0f);
    g.DrawPath(outlinePen, path);
    g.Flush();

    Color innerColor = bmp->GetPixel(80, 100);
    compat::println("    [Debug] innerColor at (80, 100): R={}, G={}, B={}, A={}",
                    innerColor.r, innerColor.g, innerColor.b, innerColor.a);
    Color innerColor100 = bmp->GetPixel(100, 100);
    compat::println("    [Debug] innerColor at (100, 100): R={}, G={}, B={}, A={}",
                    innerColor100.r, innerColor100.g, innerColor100.b, innerColor100.a);

    assert(innerColor != bg);
    assert(innerColor.r > 0 || innerColor.b > 0);

    Color cornerColor = bmp->GetPixel(5, 5);
    compat::println("    [Debug] cornerColor at (5, 5): R={}, G={}, B={}, A={}",
                    cornerColor.r, cornerColor.g, cornerColor.b, cornerColor.a);
    assert(cornerColor == bg);

    auto pictureBox = std::make_shared<PictureBox>(bmp);
    assert(pictureBox->GetImage() == bmp);
    pictureBox->SetSizeMode(PictureBoxSizeMode::Zoom);
    assert(pictureBox->GetSizeMode() == PictureBoxSizeMode::Zoom);

    pictureBox->Measure(Size(400.0f, 400.0f));
    Size desired = pictureBox->GetDesiredSize();
    compat::println("    [Debug] PictureBox desired size: {}x{}", desired.width, desired.height);
    assert(desired.width > 0.0f && desired.height > 0.0f);

    compat::println("  ✔ LinearGradientBrush 色彩插值與心臟線點陣繪圖光柵化驗證通過！");
}

} // namespace

int main() {
    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Example Features & Drawing Verification     ");
    compat::println("==========================================================");

    TestPlatformSwitchForcedSkia();
    TestAsyncEventsAndDispatcher();
    TestCardioidGeometryAndMath();
    TestLinearGradientAndRasterization();

    compat::println("==========================================================");
    compat::println(" ✔ 全部範例功能與繪圖演算法單元測試 100% 順利通過！       ");
    compat::println("==========================================================");
    return 0;
}
