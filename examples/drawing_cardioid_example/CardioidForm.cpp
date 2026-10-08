/*
 * File: CardioidForm.cpp
 * Purpose: Implements CardioidMath geometric curves and CardioidForm interactive rendering.
 */

#include "CardioidForm.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Colors.hpp"
#include "ui/dsl/Fluent.hpp"
#include "compat/compat.hpp"
#include <cmath>

namespace drawing_cardioid_example {

namespace {
constexpr float PI = 3.14159265358979323846f;
} // namespace

// =============================================================================
// CardioidMath 純函數幾何演算法實作
// =============================================================================

cppui::graphics::GraphicsPath CardioidMath::GenerateClassicalCardioid(
    cppui::core::Point center, float scale, uint32_t samples) {

    if (samples < 3) samples = 360;
    std::vector<cppui::core::Point> points;
    points.reserve(samples + 1);

    for (uint32_t i = 0; i <= samples; ++i) {
        float theta = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(samples);
        // 古典心臟線參數方程 (蚶線，由兩等圓滾動軌跡衍生)
        float x = center.x + scale * (2.0f * std::cos(theta) - std::cos(2.0f * theta));
        float y = center.y - scale * (2.0f * std::sin(theta) - std::sin(2.0f * theta));
        points.push_back(cppui::core::Point(x, y));
    }

    cppui::graphics::GraphicsPath path;
    path.AddLines(points);
    path.CloseFigure();
    return path;
}

cppui::graphics::GraphicsPath CardioidMath::GenerateHeartCurve(
    cppui::core::Point center, float scale, uint32_t samples) {

    if (samples < 3) samples = 360;
    std::vector<cppui::core::Point> points;
    points.reserve(samples + 1);

    for (uint32_t i = 0; i <= samples; ++i) {
        float t = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(samples);
        // 經典心形線參數方程式 (Taubin / Heart Formula)
        float sinT = std::sin(t);
        float x = center.x + scale * 16.0f * sinT * sinT * sinT;
        float y = center.y - scale * (13.0f * std::cos(t) - 5.0f * std::cos(2.0f * t) -
                                      2.0f * std::cos(3.0f * t) - std::cos(4.0f * t));
        points.push_back(cppui::core::Point(x, y));
    }

    cppui::graphics::GraphicsPath path;
    path.AddLines(points);
    path.CloseFigure();
    return path;
}

// =============================================================================
// CardioidForm 視窗與繪圖互動實作
// =============================================================================

CardioidForm::CardioidForm() {
    using namespace cppui::dsl;
    using cppui::core::Color;

    SetTitle("CPPUI - GDI+ Graphics Demo (Cardioid & Linear Gradient)");
    SetFormSize(1024, 680);

    // 建立 600x520 繪圖緩衝區
    m_bitmap = std::make_shared<cppui::graphics::Bitmap>(600, 520);

    // 建立 PictureBox 並設置為等比例居中縮放 (Zoom)
    m_pictureBox = std::make_shared<cppui::controls::PictureBox>(m_bitmap);
    m_pictureBox->SetSizeMode(cppui::graphics::PictureBoxSizeMode::Zoom);

    // 初始繪製藝術圖元
    RefreshArtwork();

    SetContent(
        Border(
            VStack({
                // Header 橫幅
                Border(
                    VStack({
                        Label("CPPUI GDI+ 風格圖形繪圖範例 (Cardioid & Linear Gradient)")
                            .FontSize(20.0f)
                            .FontWeight(cppui::text::FontWeight::Bold)
                            .Foreground(Color(255, 255, 255, 255)),
                        Label("展示 GraphicsPath 幾何路徑生成、LinearGradientBrush 雙色漸層、Pen 描邊與 PictureBox 高清渲染")
                            .FontSize(13.0f)
                            .Foreground(Color(160, 185, 230, 255))
                            .Margin(0.0f, 6.0f, 0.0f, 0.0f)
                    })
                )
                .Background(Color(20, 24, 34, 255))
                .BorderBrush(Color(42, 50, 72, 255))
                .BorderThickness(0.0f, 0.0f, 0.0f, 1.0f)
                .Padding(24.0f, 16.0f),

                // 主工作區 (水平分割: 左側 PictureBox 展示，右側控制面板)
                HStack({
                    // 左側: PictureBox 畫布容器
                    Border(m_pictureBox)
                        .Width(620.0f)
                        .Height(540.0f)
                        .Background(Color(12, 14, 20, 255))
                        .BorderBrush(Color(45, 55, 80, 255))
                        .BorderThickness(1.0f)
                        .CornerRadius(8.0f)
                        .Padding(10.0f),

                    // 右側: 參數調校面板
                    Border(
                        VStack({
                            Label("幾何圖形與調色盤控制")
                                .FontSize(16.0f)
                                .FontWeight(cppui::text::FontWeight::SemiBold)
                                .Foreground(Color(230, 240, 255, 255)),

                            Label("選擇曲線模型 (Curve Type):")
                                .FontSize(13.0f)
                                .Foreground(Color(160, 175, 200, 255))
                                .Margin(0.0f, 6.0f, 0.0f, 2.0f),

                            HStack({
                                Button("古典心臟線")
                                    .Width(130.0f)
                                    .Height(34.0f)
                                    .Background(Color(45, 115, 215, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_curveType = CurveType::ClassicalCardioid;
                                        RefreshArtwork();
                                    }),

                                Button("浪漫心形線")
                                    .Width(130.0f)
                                    .Height(34.0f)
                                    .Background(Color(215, 60, 100, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_curveType = CurveType::RomanticHeart;
                                        RefreshArtwork();
                                    })
                            }).Spacing(10.0f),

                            Label("選擇漸層配色 (Color Palette):")
                                .FontSize(13.0f)
                                .Foreground(Color(160, 175, 200, 255))
                                .Margin(0.0f, 10.0f, 0.0f, 2.0f),

                            VStack({
                                Button("日落暮色 (Crimson -> RoyalBlue)")
                                    .Width(270.0f)
                                    .Height(34.0f)
                                    .Background(Color(180, 50, 80, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_gradientScheme = GradientScheme::SunsetCrimson;
                                        RefreshArtwork();
                                    }),

                                Button("蔚藍海岸 (Cyan -> Deep Indigo)")
                                    .Width(270.0f)
                                    .Height(34.0f)
                                    .Background(Color(30, 130, 190, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_gradientScheme = GradientScheme::OceanBreeze;
                                        RefreshArtwork();
                                    }),

                                Button("翡翠森林 (Mint -> Dark Green)")
                                    .Width(270.0f)
                                    .Height(34.0f)
                                    .Background(Color(35, 140, 85, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_gradientScheme = GradientScheme::EmeraldForest;
                                        RefreshArtwork();
                                    })
                            }).Spacing(8.0f),

                            Label("線條外框樣式 (Stroke Width):")
                                .FontSize(13.0f)
                                .Foreground(Color(160, 175, 200, 255))
                                .Margin(0.0f, 10.0f, 0.0f, 2.0f),

                            HStack({
                                Button("纖細外框 (1.5px)")
                                    .Width(130.0f)
                                    .Height(34.0f)
                                    .Background(Color(50, 60, 80, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_strokeWidth = 1.5f;
                                        RefreshArtwork();
                                    }),

                                Button("醒目厚框 (3.5px)")
                                    .Width(130.0f)
                                    .Height(34.0f)
                                    .Background(Color(50, 60, 80, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .OnClick([this]() {
                                        m_strokeWidth = 3.5f;
                                        RefreshArtwork();
                                    })
                            }).Spacing(10.0f),

                            // 狀態即時標籤
                            Label("")
                                .FontSize(12.0f)
                                .FontFamily("Consolas")
                                .Foreground(Color(140, 210, 160, 255))
                                .Margin(0.0f, 12.0f, 0.0f, 0.0f)
                                .BindText(m_infoText, [](const std::string& t) {
                                    return t;
                                })
                        }).Spacing(6.0f)
                    )
                    .Background(Color(18, 22, 32, 255))
                    .BorderBrush(Color(40, 48, 68, 255))
                    .BorderThickness(1.0f)
                    .CornerRadius(8.0f)
                    .Padding(20.0f)
                })
                .Padding(20.0f)
                .Spacing(16.0f)
            })
        )
        .Background(Color(13, 15, 22, 255))
        .Get()
    );
}

void CardioidForm::RefreshArtwork() {
    using namespace cppui::core;
    using namespace cppui::graphics;

    if (!m_bitmap) return;

    // 建立排程畫布
    Graphics g = Graphics::FromImage(m_bitmap);

    // 清空背景為深色雅致底色
    g.Clear(Color(15, 18, 26, 255));

    // 依據選取型態生成幾何路徑
    GraphicsPath path;
    const Point center(300.0f, 260.0f);
    std::string curveName;

    if (m_curveType == CurveType::ClassicalCardioid) {
        path = CardioidMath::GenerateClassicalCardioid(center, 70.0f, 360);
        curveName = "古典心臟線 (Classical Cardioid)";
    } else {
        path = CardioidMath::GenerateHeartCurve(center, 12.0f, 360);
        curveName = "浪漫心形線 (Heart Curve)";
    }

    // 依據配色方案建立線性漸層畫刷 (從頂部漸變至底部)
    Point gradStart(300.0f, 60.0f);
    Point gradEnd(300.0f, 460.0f);
    Color c1, c2;
    std::string schemeName;

    switch (m_gradientScheme) {
    case GradientScheme::SunsetCrimson:
        c1 = Color(255, 75, 110, 255);  // Crimson
        c2 = Color(40, 115, 235, 255);  // RoyalBlue
        schemeName = "日落暮色 (Crimson -> RoyalBlue)";
        break;
    case GradientScheme::OceanBreeze:
        c1 = Color(0, 215, 255, 255);   // DeepSkyBlue / Cyan
        c2 = Color(75, 45, 195, 255);   // Indigo
        schemeName = "蔚藍海岸 (Cyan -> Indigo)";
        break;
    case GradientScheme::EmeraldForest:
        c1 = Color(60, 235, 145, 255);  // SpringGreen / Mint
        c2 = Color(25, 95, 70, 255);    // DarkSlateGreen
        schemeName = "翡翠森林 (Mint -> DarkSlateGreen)";
        break;
    }

    LinearGradientBrush brush(gradStart, gradEnd, c1, c2);

    // 1. 執行線性漸層路徑填色
    g.FillPath(brush, path);

    // 2. 執行高反差發光輪廓描邊
    Pen pen(Color(255, 255, 255, 220), m_strokeWidth);
    g.DrawPath(pen, path);

    // 3. 提交光柵化至 Bitmap 記憶體緩衝區
    g.Flush();

    // 更新資訊狀態文字
    m_infoText = "已生成：" + curveName + " | 漸層：" + schemeName;

    // 重新指派至 PictureBox 觸發排版與重繪
    if (m_pictureBox) {
        m_pictureBox->SetImage(m_bitmap);
        m_pictureBox->InvalidateVisual();
    }
}

void CardioidForm::OnLoad() {
    Form::OnLoad();
    compat::println("[Lifecycle] CardioidForm::OnLoad() 觸發，心臟線繪圖已完成初始化！");
}

} // namespace drawing_cardioid_example
