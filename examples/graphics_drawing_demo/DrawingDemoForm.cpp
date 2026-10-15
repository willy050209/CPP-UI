/*
 * File: DrawingDemoForm.cpp
 * Purpose: Implements 2D graphics vector drawing, bitmap composition, and Grayscale filters.
 */

#include "DrawingDemoForm.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Brush.hpp"
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Colors.hpp"
#include "compat/Compat.hpp"
#include <cmath>
#include <vector>

namespace graphics_drawing_demo {

/// <summary>
/// 建立正五角星幾何路徑 (GraphicsPath)。
/// </summary>
cppui::graphics::GraphicsPath DrawingDemoMath::CreateStarPath(
    cppui::core::Point center,
    float outerRadius,
    float innerRadius,
    uint32_t points) {
    if (points < 3) {
        points = 3;
    }

    cppui::graphics::GraphicsPath path;
    const float pi = 3.14159265358979323846f;
    const float step = pi / static_cast<float>(points);
    float angle = -pi / 2.0f;

    std::vector<cppui::core::Point> pts;
    pts.reserve(static_cast<size_t>(points) * 2);

    for (uint32_t i = 0; i < points * 2; ++i) {
        const float r = (i % 2 == 0) ? outerRadius : innerRadius;
        const float x = center.x + r * std::cos(angle);
        const float y = center.y + r * std::sin(angle);
        pts.push_back(cppui::core::Point(x, y));
        angle += step;
    }

    path.AddLines(compat::span<const cppui::core::Point>(pts.data(), pts.size()));
    path.CloseFigure();
    return path;
}

/// <summary>
/// 生成展示用之印記徽章點陣圖，示範動態繪製與圖片載入。
/// </summary>
std::shared_ptr<cppui::graphics::Bitmap> DrawingDemoMath::CreateSampleBadge(uint32_t size) {
    if (size == 0) {
        size = 64;
    }

    auto bmp = std::make_shared<cppui::graphics::Bitmap>(size, size);
    cppui::graphics::Graphics g(bmp);
    g.Clear(cppui::core::Color(0, 0, 0, 0));

    const float s = static_cast<float>(size);

    cppui::graphics::LinearGradientBrush ringBrush(
        cppui::core::Point(0.0f, 0.0f),
        cppui::core::Point(s, s),
        cppui::core::Color(255, 60, 100, 255),
        cppui::core::Color(255, 180, 0, 255));
    g.FillEllipse(ringBrush, cppui::core::Rect(2.0f, 2.0f, s - 4.0f, s - 4.0f));

    cppui::graphics::SolidBrush innerBrush(cppui::core::Color(35, 42, 58, 255));
    g.FillEllipse(innerBrush, cppui::core::Rect(s * 0.18f, s * 0.18f, s * 0.64f, s * 0.64f));

    cppui::graphics::Pen whitePen(cppui::core::Color(255, 255, 255, 220), 2.0f);
    g.DrawEllipse(whitePen, cppui::core::Rect(s * 0.18f, s * 0.18f, s * 0.64f, s * 0.64f));

    cppui::graphics::SolidBrush diamondBrush(cppui::core::Color(255, 215, 0, 255));
    const float mid = s * 0.5f;
    const float d = s * 0.18f;
    std::vector<cppui::core::Point> diamond = {
        cppui::core::Point(mid, mid - d),
        cppui::core::Point(mid + d, mid),
        cppui::core::Point(mid, mid + d),
        cppui::core::Point(mid - d, mid)
    };
    g.FillPolygon(diamondBrush, compat::span<const cppui::core::Point>(diamond.data(), diamond.size()));

    g.Flush();
    return bmp;
}

/// <summary>
/// 建構 DrawingDemoForm 並繪製初始畫布與介面元件。
/// </summary>
DrawingDemoForm::DrawingDemoForm(bool startGrayscale) {
    using namespace cppui::dsl;
    using namespace cppui::core;

    SetTitle("CPPUI - 2D 圖形繪製與灰階濾鏡示範 (Graphics & ImageFilters)");
    SetFormSize(580, 520);

    RenderMasterScene();

    if (startGrayscale && m_originalBitmap) {
        m_currentBitmap = cppui::graphics::ImageFilters::ToGrayscale(
            *m_originalBitmap, cppui::graphics::GrayscaleMethod::BT601);
    }

    m_pictureBox = std::make_shared<cppui::controls::PictureBox>(m_currentBitmap);
    m_pictureBox->SetWidth(530.0f);
    m_pictureBox->SetHeight(330.0f);
    m_pictureBox->SetSizeMode(cppui::graphics::PictureBoxSizeMode::Zoom);
    m_pictureBox->SetBackground(Color(20, 24, 32, 255));

    m_statusLabel = std::make_shared<cppui::controls::Label>(
        startGrayscale ? "當前狀態：已套用 ITU-R BT.601 標準灰階" : "當前狀態：原始 32-bit ARGB 彩色畫布");
    m_statusLabel->SetFontSize(13.0f);
    m_statusLabel->SetForeground(
        startGrayscale ? Color(255, 200, 80, 255) : Color(80, 200, 255, 255));

    SetContent(
        VStack({
            Label("2D 圖形函數、圖片繪製與灰階濾鏡")
                .FontSize(18.0f)
                .FontWeight(cppui::text::FontWeight::Bold)
                .Foreground(Colors::White),

            Label("示範 Graphics 幾何圖元、自訂路徑、漸層畫刷、DrawImage 與 ImageFilters 灰階轉換")
                .FontSize(12.0f)
                .Foreground(Color(160, 165, 175, 255)),

            Border(
                UIBuilder<cppui::controls::PictureBox>(m_pictureBox)
            )
            .BorderBrush(Color(50, 58, 72, 255))
            .BorderThickness(1.0f)
            .CornerRadius(6.0f)
            .Margin(0.0f, 6.0f, 0.0f, 4.0f),

            UIBuilder<cppui::controls::Label>(m_statusLabel)
                .Margin(0.0f, 2.0f, 0.0f, 6.0f),

            HStack({
                Button("彩色原圖 (Reset)")
                    .Width(125.0f)
                    .Height(34.0f)
                    .Background(Color(45, 120, 220, 255))
                    .Foreground(Colors::White)
                    .CornerRadius(4.0f)
                    .OnClick([this]() {
                        ResetOriginal();
                    }),

                Button("BT.601 灰階")
                    .Width(115.0f)
                    .Height(34.0f)
                    .Background(Color(60, 70, 85, 255))
                    .Foreground(Colors::White)
                    .CornerRadius(4.0f)
                    .OnClick([this]() {
                        ApplyGrayscale(cppui::graphics::GrayscaleMethod::BT601, "ITU-R BT.601 標準灰階");
                    }),

                Button("BT.709 灰階")
                    .Width(115.0f)
                    .Height(34.0f)
                    .Background(Color(60, 70, 85, 255))
                    .Foreground(Colors::White)
                    .CornerRadius(4.0f)
                    .OnClick([this]() {
                        ApplyGrayscale(cppui::graphics::GrayscaleMethod::BT709, "ITU-R BT.709 高清灰階");
                    }),

                Button("平均值灰階")
                    .Width(115.0f)
                    .Height(34.0f)
                    .Background(Color(60, 70, 85, 255))
                    .Foreground(Colors::White)
                    .CornerRadius(4.0f)
                    .OnClick([this]() {
                        ApplyGrayscale(cppui::graphics::GrayscaleMethod::Average, "RGB 算術平均灰階");
                    })
            }).Spacing(8.0f)
        })
        .Padding(Thickness(20.0f))
        .Spacing(4.0f)
    );
}

/// <summary>
/// 繪製彩色主畫布內容。
/// </summary>
void DrawingDemoForm::RenderMasterScene() {
    constexpr uint32_t canvasW = 530;
    constexpr uint32_t canvasH = 330;

    m_originalBitmap = std::make_shared<cppui::graphics::Bitmap>(canvasW, canvasH);
    cppui::graphics::Graphics g(m_originalBitmap);

    // 1. 深色畫布背景
    g.Clear(cppui::core::Color(26, 30, 40, 255));

    // 2. 漸層標題橫幅 (LinearGradientBrush + FillRectangle + DrawRectangle)
    cppui::graphics::LinearGradientBrush headerBrush(
        cppui::core::Point(15.0f, 12.0f),
        cppui::core::Point(515.0f, 58.0f),
        cppui::core::Color(30, 80, 180, 255),
        cppui::core::Color(140, 40, 160, 255));
    g.FillRectangle(headerBrush, 15.0f, 12.0f, 500.0f, 46.0f);
    g.DrawRectangle(cppui::graphics::Pen(cppui::core::Color(100, 180, 255, 200), 1.5f), 15.0f, 12.0f, 500.0f, 46.0f);

    // 3. 第一排幾何圖元：矩形、圓角矩形、漸層橢圓、扇形與圓弧
    // 矩形輪廓 (DrawRectangle)
    g.DrawRectangle(cppui::graphics::Pen(cppui::core::Color(0, 200, 255, 255), 2.0f), 20.0f, 74.0f, 85.0f, 60.0f);

    // 圓角矩形填色 (FillRoundedRectangle & DrawRoundedRectangle)
    cppui::graphics::SolidBrush orangeBrush(cppui::core::Color(255, 120, 50, 255));
    g.FillRoundedRectangle(orangeBrush, cppui::core::Rect(125.0f, 74.0f, 90.0f, 60.0f), 8.0f);
    g.DrawRoundedRectangle(cppui::graphics::Pen(cppui::core::Colors::White, 1.5f), cppui::core::Rect(125.0f, 74.0f, 90.0f, 60.0f), 8.0f);

    // 漸層橢圓 (FillEllipse)
    cppui::graphics::LinearGradientBrush ellipseBrush(
        cppui::core::Point(235.0f, 74.0f),
        cppui::core::Point(335.0f, 134.0f),
        cppui::core::Color(255, 220, 50, 255),
        cppui::core::Color(220, 60, 80, 255));
    g.FillEllipse(ellipseBrush, 235.0f, 74.0f, 95.0f, 60.0f);

    // 扇形 (FillPie) 與圓弧 (DrawArc)
    cppui::graphics::SolidBrush pieBrush(cppui::core::Color(160, 90, 240, 255));
    g.FillPie(pieBrush, cppui::core::Rect(350.0f, 74.0f, 75.0f, 60.0f), 30.0f, 260.0f);
    g.DrawArc(cppui::graphics::Pen(cppui::core::Color(80, 230, 160, 255), 2.5f), cppui::core::Rect(445.0f, 74.0f, 65.0f, 60.0f), 45.0f, 270.0f);

    // 分隔線 (DrawLine)
    g.DrawLine(cppui::graphics::Pen(cppui::core::Color(60, 70, 90, 255), 1.0f), 15.0f, 150.0f, 515.0f, 150.0f);

    // 4. 第二排進階幾何：三次貝茲曲線、自訂 GraphicsPath (五角星)、與載入/繪製點陣影像 (DrawImage)
    // 三次貝茲曲線 (DrawBezier)
    cppui::graphics::Pen bezierPen(cppui::core::Color(255, 90, 160, 255), 3.0f);
    g.DrawBezier(bezierPen,
        cppui::core::Point(20.0f, 240.0f),
        cppui::core::Point(65.0f, 165.0f),
        cppui::core::Point(115.0f, 305.0f),
        cppui::core::Point(165.0f, 225.0f));

    // 五角星 GraphicsPath (FillPath & DrawPath)
    auto starPath = DrawingDemoMath::CreateStarPath(cppui::core::Point(245.0f, 235.0f), 46.0f, 20.0f, 5);
    cppui::graphics::SolidBrush starBrush(cppui::core::Color(255, 215, 0, 255));
    g.FillPath(starBrush, starPath);
    g.DrawPath(cppui::graphics::Pen(cppui::core::Color(255, 140, 0, 255), 2.0f), starPath);

    // 繪製動態生成的徽章影像 (DrawImage)
    auto badge = DrawingDemoMath::CreateSampleBadge(80);
    g.DrawImage(badge, cppui::core::Rect(330.0f, 195.0f, 80.0f, 80.0f));

    // 多尺度縮放印記 (DrawImage)
    g.DrawImage(badge, cppui::core::Rect(435.0f, 210.0f, 54.0f, 54.0f));

    // 底部裝飾線
    g.DrawLine(cppui::graphics::Pen(cppui::core::Color(100, 200, 255, 160), 2.0f), 15.0f, 310.0f, 515.0f, 310.0f);

    g.Flush();
    m_currentBitmap = m_originalBitmap->Clone();
}

/// <summary>
/// 套用指定灰階轉換演算法。
/// </summary>
void DrawingDemoForm::ApplyGrayscale(cppui::graphics::GrayscaleMethod method, compat::string_view description) {
    if (!m_originalBitmap) {
        return;
    }

    m_currentBitmap = cppui::graphics::ImageFilters::ToGrayscale(*m_originalBitmap, method);
    m_pictureBox->SetImage(m_currentBitmap);
    m_statusLabel->SetText(compat::format("當前狀態：已套用 {}", description));
    m_statusLabel->SetForeground(cppui::core::Color(255, 200, 80, 255));
}

/// <summary>
/// 還原為原始彩色影像。
/// </summary>
void DrawingDemoForm::ResetOriginal() {
    if (!m_originalBitmap) {
        return;
    }

    m_currentBitmap = m_originalBitmap->Clone();
    m_pictureBox->SetImage(m_currentBitmap);
    m_statusLabel->SetText("當前狀態：原始 32-bit ARGB 彩色畫布");
    m_statusLabel->SetForeground(cppui::core::Color(80, 200, 255, 255));
}

} // namespace graphics_drawing_demo
