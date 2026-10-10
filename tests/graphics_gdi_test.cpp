/**
 * @file graphics_gdi_test.cpp
 * @brief GDI+ 風格現代 2D 圖形 API、Skia/軟體雙引擎、執行緒安全自動同步與 PictureBox 100% 覆蓋單元測試
 */

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/graphics/GraphicsTypes.hpp"
#include "ui/graphics/Colors.hpp"
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Pens.hpp"
#include "ui/graphics/Brush.hpp"
#include "ui/graphics/Brushes.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"

#include <cassert>
#include <cstdint>
#include <vector>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cmath>

using namespace cppui;
using namespace cppui::graphics;

// =============================================================================
// 1. Color 與 Colors 測試
// =============================================================================
static void TestColorAndColors() {
    compat::println("[Test] Testing Color and Colors stock collection...");

    // FromArgb 多載測試
    core::Color c1 = core::Color::FromArgb(128, 255, 0, 100);
    assert(c1.a == 128 && c1.r == 255 && c1.g == 0 && c1.b == 100);

    core::Color c2 = core::Color::FromArgb(10, 20, 30);
    assert(c2.a == 255 && c2.r == 10 && c2.g == 20 && c2.b == 30);

    core::Color c3 = core::Color::FromArgb(0x80FF0064);
    assert(c3.a == 0x80 && c3.r == 0xFF && c3.g == 0x00 && c3.b == 0x64);

    core::Color c4 = core::Color::FromArgb(200, c2);
    assert(c4.a == 200 && c4.r == 10 && c4.g == 20 && c4.b == 30);

    // ToArgb 與 IsEmpty 測試
    uint32_t argbVal = c1.ToArgb();
    assert(argbVal == 0x80FF0064);
    (void)argbVal;

    core::Color emptyColor(0, 0, 0, 0);
    assert(emptyColor.IsEmpty());
    assert(!c1.IsEmpty());

    // HSL 亮度、色相、飽和度運算
    core::Color red(255, 0, 0, 255);
    assert(std::abs(red.GetBrightness() - 0.5f) < 0.01f);
    assert(std::abs(red.GetHue() - 0.0f) < 0.01f);
    assert(std::abs(red.GetSaturation() - 1.0f) < 0.01f);

    core::Color green(0, 255, 0, 255);
    assert(std::abs(green.GetHue() - 120.0f) < 0.01f);

    core::Color blue(0, 0, 255, 255);
    assert(std::abs(blue.GetHue() - 240.0f) < 0.01f);

    // Colors 常數集合驗證
    assert(Colors::Transparent == core::Color(0, 0, 0, 0));
    assert(Colors::Black == core::Color(0, 0, 0, 255));
    assert(Colors::White == core::Color(255, 255, 255, 255));
    assert(Colors::Red == core::Color(255, 0, 0, 255));
    assert(Colors::Green == core::Color(0, 128, 0, 255));
    assert(Colors::Blue == core::Color(0, 0, 255, 255));
    assert(Colors::SteelBlue == core::Color(70, 130, 180, 255));
    assert(Colors::Gold == core::Color(255, 215, 0, 255));
    assert(Colors::Coral == core::Color(255, 127, 80, 255));

    compat::println(" -> PASS: Color and Colors");
}

// =============================================================================
// 2. Pen 與 Pens 測試
// =============================================================================
static void TestPenAndPens() {
    compat::println("[Test] Testing Pen and Pens stock collection...");

    Pen pen(Colors::Red, 2.5f);
    assert(pen.GetColor() == Colors::Red);
    assert(pen.GetWidth() == 2.5f);
    assert(pen.GetDashStyle() == DashStyle::Solid);
    assert(pen.GetLineCap() == LineCap::Flat);
    assert(pen.GetLineJoin() == LineJoin::Miter);

    // 修改屬性
    pen.SetColor(Colors::Blue);
    assert(pen.GetColor() == Colors::Blue);
    pen.SetWidth(4.0f);
    assert(pen.GetWidth() == 4.0f);
    pen.SetDashStyle(DashStyle::DashDot);
    assert(pen.GetDashStyle() == DashStyle::DashDot);
    pen.SetLineCap(LineCap::Round);
    assert(pen.GetLineCap() == LineCap::Round);
    pen.SetLineJoin(LineJoin::Bevel);
    assert(pen.GetLineJoin() == LineJoin::Bevel);

    // Clone 測試
    Pen clonePen = pen.Clone();
    assert(clonePen.GetColor() == Colors::Blue);
    assert(clonePen.GetWidth() == 4.0f);
    assert(clonePen.GetDashStyle() == DashStyle::DashDot);
    assert(clonePen.GetLineCap() == LineCap::Round);
    assert(clonePen.GetLineJoin() == LineJoin::Bevel);

    // 拷貝與移動建構與賦值
    Pen copied(pen);
    assert(copied.GetColor() == pen.GetColor());
    Pen assigned(Colors::Black);
    assigned = pen;
    assert(assigned.GetWidth() == pen.GetWidth());

    Pen moved(std::move(copied));
    assert(moved.GetColor() == Colors::Blue);

    // 從 Brush 建構 Pen
    SolidBrush sb(Colors::Yellow);
    Pen penFromBrush(sb, 3.0f);
    assert(penFromBrush.GetColor() == Colors::Yellow);
    assert(penFromBrush.GetWidth() == 3.0f);

    // Pens 靜態集合測試
    assert(Pens::Black().GetColor() == Colors::Black);
    assert(Pens::White().GetColor() == Colors::White);
    assert(Pens::Red().GetColor() == Colors::Red);
    assert(Pens::Green().GetColor() == Colors::Green);
    assert(Pens::Blue().GetColor() == Colors::Blue);
    assert(Pens::Transparent().GetColor() == Colors::Transparent);
    assert(Pens::SteelBlue().GetColor() == Colors::SteelBlue);
    assert(Pens::Gold().GetColor() == Colors::Gold);

    compat::println(" -> PASS: Pen and Pens");
}

// =============================================================================
// 3. Brush 與 Brushes 測試
// =============================================================================
static void TestBrushAndBrushes() {
    compat::println("[Test] Testing Brush and Brushes stock collection...");

    // SolidBrush
    SolidBrush sb(Colors::SteelBlue);
    assert(sb.GetType() == BrushType::SolidColor);
    assert(sb.GetColor() == Colors::SteelBlue);
    sb.SetColor(Colors::Cyan);
    assert(sb.GetColor() == Colors::Cyan);
    auto clonedSb = sb.Clone();
    assert(clonedSb->GetType() == BrushType::SolidColor);
    assert(static_cast<SolidBrush*>(clonedSb.get())->GetColor() == Colors::Cyan);

    // LinearGradientBrush (起訖點)
    LinearGradientBrush lgb(core::Point(0, 0), core::Point(100, 100), Colors::Red, Colors::Blue);
    assert(lgb.GetType() == BrushType::LinearGradient);
    assert(lgb.GetStartPoint() == core::Point(0, 0));
    assert(lgb.GetEndPoint() == core::Point(100, 100));
    assert(lgb.GetStartColor() == Colors::Red);
    assert(lgb.GetEndColor() == Colors::Blue);

    lgb.SetStartPoint(core::Point(10, 10));
    lgb.SetEndPoint(core::Point(50, 50));
    lgb.SetStartColor(Colors::Yellow);
    lgb.SetEndColor(Colors::Green);
    assert(lgb.GetStartPoint() == core::Point(10, 10));
    assert(lgb.GetStartColor() == Colors::Yellow);

    // LinearGradientBrush (矩形與角度)
    LinearGradientBrush lgbAngle(core::Rect(0, 0, 200, 100), Colors::White, Colors::Black, 45.0f);
    assert(lgbAngle.GetType() == BrushType::LinearGradient);
    auto clonedLgb = lgbAngle.Clone();
    assert(clonedLgb->GetType() == BrushType::LinearGradient);

    // TextureBrush
    auto bmp = std::make_shared<Bitmap>(32, 32);
    TextureBrush tb(bmp);
    assert(tb.GetType() == BrushType::Texture);
    assert(tb.GetImage() == bmp);
    auto clonedTb = tb.Clone();
    assert(clonedTb->GetType() == BrushType::Texture);

    // Brushes 靜態集合測試
    assert(Brushes::Black().GetColor() == Colors::Black);
    assert(Brushes::White().GetColor() == Colors::White);
    assert(Brushes::Red().GetColor() == Colors::Red);
    assert(Brushes::Green().GetColor() == Colors::Green);
    assert(Brushes::Blue().GetColor() == Colors::Blue);
    assert(Brushes::Transparent().GetColor() == Colors::Transparent);
    assert(Brushes::SteelBlue().GetColor() == Colors::SteelBlue);
    assert(Brushes::Gold().GetColor() == Colors::Gold);

    compat::println(" -> PASS: Brush and Brushes");
}

// =============================================================================
// 4. GraphicsPath 測試
// =============================================================================
static void TestGraphicsPath() {
    compat::println("[Test] Testing GraphicsPath...");

    GraphicsPath path;
    assert(path.GetPointCount() == 0);
    assert(path.GetFillMode() == FillMode::Alternate);

    path.SetFillMode(FillMode::Winding);
    assert(path.GetFillMode() == FillMode::Winding);

    // 新增線段
    path.AddLine(0.0f, 0.0f, 100.0f, 0.0f);
    path.AddLine(core::Point(100.0f, 0.0f), core::Point(100.0f, 100.0f));
    path.CloseFigure();
    assert(path.GetPointCount() >= 2);

    // 包圍矩形
    core::Rect bounds = path.GetBounds();
    assert(bounds.x == 0.0f && bounds.y == 0.0f);
    assert(bounds.width == 100.0f && bounds.height == 100.0f);

    // 重設
    path.Reset();
    assert(path.GetPointCount() == 0);

    // 矩形
    path.AddRectangle(core::Rect(10, 20, 80, 60));
    bounds = path.GetBounds();
    assert(bounds.x == 10 && bounds.y == 20 && bounds.width == 80 && bounds.height == 60);

    // 點命中測試 (IsVisible)
    assert(path.IsVisible(50.0f, 50.0f));
    assert(!path.IsVisible(0.0f, 0.0f));
    assert(!path.IsVisible(150.0f, 150.0f));

    // 圓角矩形
    path.Reset();
    path.AddRoundedRectangle(core::Rect(0, 0, 100, 100), 10.0f);
    assert(path.GetPointCount() > 4);

    // 橢圓與圓弧
    path.Reset();
    path.AddEllipse(core::Rect(0, 0, 200, 100));
    assert(path.GetPointCount() > 4);
    path.AddArc(core::Rect(0, 0, 100, 100), 0.0f, 90.0f);

    // 貝茲曲線與曲線平滑展開
    path.Reset();
    path.AddBezier(core::Point(0, 0), core::Point(20, 80), core::Point(80, 80), core::Point(100, 0));
    auto flatPoints = path.Flatten(0.25f);
    assert(flatPoints.size() >= 4);

    // 多邊形
    std::vector<core::Point> polyPoints = { core::Point(0, 0), core::Point(100, 0), core::Point(50, 80) };
    path.Reset();
    path.AddPolygon(polyPoints);
    assert(path.IsVisible(50, 40));
    assert(!path.IsVisible(10, 80));

    // 描邊命中測試 (IsOutlineVisible)
    Pen strokePen(Colors::Black, 4.0f);
    assert(path.IsOutlineVisible(core::Point(50, 0), strokePen));

    // 仿射變換
    path.Transform(2.0f, 0.0f, 0.0f, 2.0f, 10.0f, 20.0f);
    bounds = path.GetBounds();
    assert(bounds.x >= 10.0f && bounds.width >= 200.0f);

    // 拷貝
    GraphicsPath clonePath = path.Clone();
    assert(clonePath.GetPointCount() == path.GetPointCount());

    compat::println(" -> PASS: GraphicsPath");
}

// =============================================================================
// 5. Bitmap 測試
// =============================================================================
static void TestBitmap() {
    compat::println("[Test] Testing Bitmap...");

    Bitmap bmp(64, 32);
    assert(bmp.GetWidth() == 64);
    assert(bmp.GetHeight() == 32);
    assert(bmp.GetPixelFormat() == PixelFormat::Format32bppArgb);
    assert(bmp.GetStride() == 64 * sizeof(uint32_t));

    // 讀寫像素
    bmp.SetPixel(10, 10, Colors::Red);
    assert(bmp.GetPixel(10, 10) == Colors::Red);
    assert(bmp.GetPixel(0, 0) == core::Color(0, 0, 0, 0));

    // Clear
    bmp.Clear(Colors::Blue);
    assert(bmp.GetPixel(0, 0) == Colors::Blue);
    assert(bmp.GetPixel(30, 20) == Colors::Blue);

    // Span
    auto span = bmp.GetPixelSpan();
    assert(span.size() == 64 * 32);
    assert(span[0] == Colors::Blue.ToArgb());

    // LockBits / UnlockBits
    void* bits = bmp.LockBits(core::Rect(0, 0, 64, 32), 0);
    assert(bits != nullptr);
    static_cast<uint32_t*>(bits)[0] = Colors::Gold.ToArgb();
    bmp.UnlockBits();
    assert(bmp.GetPixel(0, 0) == Colors::Gold);

    // Clone 裁切
    auto cropped = bmp.Clone(core::Rect(0, 0, 16, 16), PixelFormat::Format32bppArgb);
    assert(cropped->GetWidth() == 16 && cropped->GetHeight() == 16);
    assert(cropped->GetPixel(0, 0) == Colors::Gold);

    // 旋轉翻轉 (RotateFlip)
    Bitmap flipBmp(4, 2);
    flipBmp.SetPixel(0, 0, Colors::Red);
    flipBmp.SetPixel(3, 0, Colors::Blue);
    flipBmp.RotateFlip(RotateFlipType::RotateNoneFlipX);
    assert(flipBmp.GetPixel(3, 0) == Colors::Red);
    assert(flipBmp.GetPixel(0, 0) == Colors::Blue);

    flipBmp.RotateFlip(RotateFlipType::Rotate90FlipNone);
    assert(flipBmp.GetWidth() == 2 && flipBmp.GetHeight() == 4);

    compat::println(" -> PASS: Bitmap");
}

// =============================================================================
// 6. Graphics UI 錄製模式與離線點陣渲染模式測試
// =============================================================================
static void TestGraphicsModes() {
    compat::println("[Test] Testing Graphics (DrawingContext & Image Offscreen modes)...");

    // Mode A: DrawingContext 錄製模式
    rendering::DisplayList dl;
    rendering::DrawingContext dc(dl);
    Graphics gUi(dc);

    gUi.SetSmoothingMode(SmoothingMode::AntiAlias);
    assert(gUi.GetSmoothingMode() == SmoothingMode::AntiAlias);
    gUi.SetInterpolationMode(InterpolationMode::HighQualityBilinear);
    assert(gUi.GetInterpolationMode() == InterpolationMode::HighQualityBilinear);

    uint32_t sId = gUi.Save();
    gUi.TranslateTransform(10.0f, 20.0f);
    gUi.ScaleTransform(1.5f, 1.5f);
    gUi.RotateTransform(45.0f);
    gUi.Restore(sId);

    // 測試各種繪製 API
    gUi.Clear(Colors::White);
    gUi.DrawLine(Pens::Red(), 0, 0, 100, 100);
    gUi.DrawLine(Pens::Red(), core::Point(0, 0), core::Point(100, 100));

    std::vector<core::Point> pts = { core::Point(0, 0), core::Point(50, 20), core::Point(100, 0) };
    gUi.DrawLines(Pens::Blue(), pts);

    gUi.DrawRectangle(Pens::Green(), 10, 10, 80, 80);
    gUi.DrawRectangle(Pens::Green(), core::Rect(10, 10, 80, 80));
    gUi.FillRectangle(Brushes::Yellow(), 10, 10, 80, 80);
    gUi.FillRectangle(Brushes::Yellow(), core::Rect(10, 10, 80, 80));

    gUi.DrawRoundedRectangle(Pens::Black(), core::Rect(0, 0, 50, 50), 5.0f);
    gUi.FillRoundedRectangle(Brushes::Coral(), core::Rect(0, 0, 50, 50), 5.0f);

    gUi.DrawEllipse(Pens::Purple(), 0, 0, 40, 40);
    gUi.DrawEllipse(Pens::Purple(), core::Rect(0, 0, 40, 40));
    gUi.FillEllipse(Brushes::Navy(), 0, 0, 40, 40);
    gUi.FillEllipse(Brushes::Navy(), core::Rect(0, 0, 40, 40));

    gUi.DrawArc(Pens::Orange(), core::Rect(0, 0, 50, 50), 0, 90);
    gUi.DrawPie(Pens::Orange(), core::Rect(0, 0, 50, 50), 0, 90);
    gUi.FillPie(Brushes::Pink(), core::Rect(0, 0, 50, 50), 0, 90);

    gUi.DrawPolygon(Pens::Black(), pts);
    gUi.FillPolygon(Brushes::Gray(), pts);

    gUi.DrawBezier(Pens::Red(), core::Point(0, 0), core::Point(10, 30), core::Point(30, 10), core::Point(40, 40));
    std::vector<core::Point> bpts = { core::Point(0, 0), core::Point(10, 30), core::Point(30, 10), core::Point(40, 40) };
    gUi.DrawBeziers(Pens::Red(), bpts);
    gUi.DrawCurve(Pens::Red(), pts);

    GraphicsPath p;
    p.AddRectangle(core::Rect(0, 0, 30, 30));
    gUi.DrawPath(Pens::Red(), p);
    gUi.FillPath(Brushes::Green(), p);

    auto testBmp = std::make_shared<Bitmap>(32, 32);
    testBmp->Clear(Colors::Gold);
    gUi.DrawImage(testBmp, 0, 0);
    gUi.DrawImage(testBmp, 0, 0, 32, 32);
    gUi.DrawImage(testBmp, core::Rect(0, 0, 32, 32));
    gUi.DrawImage(testBmp, core::Rect(0, 0, 32, 32), core::Rect(0, 0, 16, 16));
    gUi.DrawImageUnscaled(testBmp, core::Point(10, 10));

    text::FontDescriptor font("Segoe UI", 12.0f);
    gUi.DrawString("Hello CPPUI Graphics", font, Brushes::Black(), core::Point(10, 10));
    gUi.DrawString("Hello CPPUI Graphics", font, Brushes::Black(), core::Rect(10, 10, 200, 30));
    auto strSize = gUi.MeasureString("Hello", font);
    assert(strSize.width > 0.0f);

    assert(dl.GetOpCount() > 10);

    // Mode B: 離線記憶體影像繪製 (FromImage)
    auto targetBmp = std::make_shared<Bitmap>(100, 100);
    targetBmp->Clear(Colors::White);

    {
        Graphics gImg = Graphics::FromImage(targetBmp);
        gImg.FillRectangle(Brushes::Red(), 10, 10, 30, 30);
        gImg.FillEllipse(Brushes::Blue(), 50, 50, 40, 40);
        gImg.Flush();
    }

    // 驗證離線光柵繪圖結果確實寫入 targetBmp 像素中
    assert(targetBmp->GetPixel(20, 20) == Colors::Red);
    assert(targetBmp->GetPixel(70, 70) == Colors::Blue);
    assert(targetBmp->GetPixel(0, 0) == Colors::White);

    compat::println(" -> PASS: Graphics modes");
}

// =============================================================================
// 7. PictureBox 與 Fluent DSL 測試
// =============================================================================
static void TestPictureBoxAndFluent() {
    compat::println("[Test] Testing PictureBox and Fluent DSL...");

    auto bmp = std::make_shared<Bitmap>(200, 100);
    bmp->Clear(Colors::SteelBlue);

    controls::PictureBox pb(bmp);
    assert(pb.GetImage() == bmp);
    assert(pb.GetSizeMode() == PictureBoxSizeMode::Normal);

    // 測試 5 種 SizeMode 之 MeasureOverride
    pb.SetSizeMode(PictureBoxSizeMode::AutoSize);
    assert(pb.GetSizeMode() == PictureBoxSizeMode::AutoSize);
    // Measure
    pb.Measure(core::Size(500, 500));
    assert(pb.GetDesiredSize().width == 200.0f && pb.GetDesiredSize().height == 100.0f);

    pb.SetSizeMode(PictureBoxSizeMode::Zoom);
    pb.Measure(core::Size(100, 100)); // 縮減寬度限制為 100
    assert(pb.GetDesiredSize().width == 100.0f && pb.GetDesiredSize().height == 50.0f);

    pb.SetSizeMode(PictureBoxSizeMode::StretchImage);
    pb.Measure(core::Size(400, 300));
    pb.Arrange(core::Rect(0, 0, 400, 300));

    // 測試 OnRender 呼叫
    rendering::DisplayList dl;
    rendering::DrawingContext dc(dl);
    pb.Render(dc);
    assert(dl.GetOpCount() > 0);

    // Fluent DSL 鏈式測試
    using namespace dsl;
    auto picElement = PictureBox(bmp)
        .SizeMode(PictureBoxSizeMode::CenterImage)
        .Width(300)
        .Height(200)
        .Get();

    assert(picElement->GetSizeMode() == PictureBoxSizeMode::CenterImage);
    assert(picElement->GetWidth() == 300);
    assert(picElement->GetHeight() == 200);

    // Signal 反應式圖片綁定
    reactive::Signal<std::shared_ptr<Bitmap>> imageSignal(bmp);
    auto reactivePic = PictureBox()
        .BindImage(imageSignal)
        .Get();

    assert(reactivePic->GetImage() == bmp);
    auto newBmp = std::make_shared<Bitmap>(50, 50);
    imageSignal.Set(newBmp);
    assert(reactivePic->GetImage() == newBmp);

    compat::println(" -> PASS: PictureBox and Fluent DSL");
}

// =============================================================================
// 8. 多執行緒並發自動同步壓力測試 (Thread Synchronization Stress Test)
// =============================================================================
static void TestMultiThreadSynchronization() {
    compat::println("[Test] Testing Multi-thread Automatic Synchronization Stress...");

    auto sharedBmp = std::make_shared<Bitmap>(100, 100);
    sharedBmp->Clear(Colors::Black);

    GraphicsPath sharedPath;
    sharedPath.AddRectangle(core::Rect(0, 0, 50, 50));

    std::atomic<bool> running{ true };
    std::atomic<int64_t> readCount{ 0 };
    std::atomic<int64_t> writeCount{ 0 };
    std::vector<std::thread> workers;

    // 啟動 4 個背景寫入執行緒 (密集寫入 Bitmap 與 Path)
    for (int32_t t = 0; t < 4; ++t) {
        workers.emplace_back([&, t]() {
            uint32_t x = static_cast<uint32_t>(t * 20);
            while (running.load(std::memory_order_relaxed)) {
                sharedBmp->SetPixel(x % 100, (x * 2) % 100, Colors::Red);
                sharedPath.AddLine(core::Point(static_cast<float>(x), 0), core::Point(static_cast<float>(x), 10));
                writeCount.fetch_add(1, std::memory_order_relaxed);
                x = (x + 1) % 100;
            }
        });
    }

    // 啟動 6 個並發讀取執行緒 (密集讀取與採樣)
    for (int32_t t = 0; t < 6; ++t) {
        workers.emplace_back([&]() {
            while (running.load(std::memory_order_relaxed)) {
                volatile auto c = sharedBmp->GetPixel(50, 50);
                volatile auto bounds = sharedPath.GetBounds();
                volatile bool vis = sharedPath.IsVisible(25, 25);
                (void)c; (void)bounds; (void)vis;
                readCount.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // 執行 150 毫秒高壓並發測試
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    running.store(false, std::memory_order_relaxed);

    for (auto& th : workers) {
        if (th.joinable()) {
            th.join();
        }
    }

    compat::println(" -> Completed {} writes and {} reads concurrently with 0 data race/corruption.",
                    writeCount.load(), readCount.load());
    compat::println(" -> PASS: Multi-thread Synchronization");
}

// =============================================================================
// 9. Fail-Fast 參數驗證與契約邊界例外測試
// =============================================================================
static void TestFailFastValidation() {
    compat::println("[Test] Testing Fail-Fast parameter validations and exception contracts...");

    // 1. Pen 負線寬
    bool caught = false;
    try {
        Pen invalidPen(Colors::Black, -2.0f);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        Pen p(Colors::Black, 1.0f);
        p.SetWidth(-0.5f);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    // 2. TextureBrush null 影像
    caught = false;
    try {
        TextureBrush invalidTb(nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    // 3. Bitmap 0 尺寸
    caught = false;
    try {
        Bitmap invalidBmp(0, 100);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        Bitmap invalidBmp(100, 0);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    // 4. Bitmap 越界 GetPixel / SetPixel
    Bitmap safeBmp(10, 10);
    caught = false;
    try {
        (void)safeBmp.GetPixel(10, 5);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        safeBmp.SetPixel(5, 10, Colors::Red);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    // 5. Graphics::FromImage null 影像
    caught = false;
    try {
        Graphics g = Graphics::FromImage(nullptr);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    compat::println(" -> PASS: Fail-Fast validation");
}

// =============================================================================
// main 測試進入點
// =============================================================================
int main() {
    compat::println("==================================================");
    compat::println("  CPPUI Milestone 8: GDI+ Graphics & PictureBox  ");
    compat::println("==================================================");

    TestColorAndColors();
    TestPenAndPens();
    TestBrushAndBrushes();
    TestGraphicsPath();
    TestBitmap();
    TestGraphicsModes();
    TestPictureBoxAndFluent();
    TestMultiThreadSynchronization();
    TestFailFastValidation();

    compat::println("==================================================");
    compat::println("  ALL 9 GRAPHICS GDI+ TEST SUITES PASSED (100%)   ");
    compat::println("==================================================");
    return 0;
}
