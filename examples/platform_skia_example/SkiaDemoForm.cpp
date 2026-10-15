/*
 * File: SkiaDemoForm.cpp
 * Purpose: Implements SkiaDemoForm layout definition using fluent UI DSL.
 */

#include "SkiaDemoForm.hpp"
#include "ui/app/Application.hpp"
#include "ui/dsl/Fluent.hpp"
#include "compat/Compat.hpp"

namespace platform_skia_example {

SkiaDemoForm::SkiaDemoForm() {
    using namespace cppui::dsl;
    using cppui::app::Application;
    using cppui::app::PlatformBackend;
    using cppui::core::Color;
    using cppui::core::Thickness;

    SetTitle("CPPUI - Platform Backend Switching (Forced Skia)");
    SetFormSize(820, 520);

    const bool isSkia = (Application::GetPlatformBackend() == PlatformBackend::Skia);
    const std::string backendName = isSkia ? "Google Skia (m151) 向量核心" : "Win32 原生 GDI / 軟體後端";
    const std::string statusText  = isSkia ? "✔ 平台已成功強制切換為 Skia 向量加速引擎" : "⚠ 目前運行於非 Skia 後端";
    const Color statusColor       = isSkia ? Color(60, 210, 120, 255) : Color(255, 180, 50, 255);

    SetContent(
        Border(
            VStack({
                // 1. Header 標題橫幅
                Border(
                    VStack({
                        Label("CPPUI 平台切換實作範例")
                            .FontSize(22.0f)
                            .FontWeight(cppui::text::FontWeight::Bold)
                            .Foreground(Color(255, 255, 255, 255)),
                        Label("示範如何以 AppBuilder::UseSkia() 顯式強制切換為 Google Skia 向量渲染管線")
                            .FontSize(13.0f)
                            .Foreground(Color(160, 185, 225, 255))
                            .Margin(0.0f, 6.0f, 0.0f, 0.0f)
                    })
                )
                .Background(Color(24, 28, 38, 255))
                .BorderBrush(Color(45, 54, 75, 255))
                .BorderThickness(0.0f, 0.0f, 0.0f, 1.0f)
                .Padding(24.0f, 18.0f),

                // 2. 主卡片內容區
                VStack({
                    // 後端狀態卡片
                    Border(
                        VStack({
                            Label("渲染後端狀態監測 (Platform Backend Status)")
                                .FontSize(16.0f)
                                .FontWeight(cppui::text::FontWeight::SemiBold)
                                .Foreground(Color(240, 245, 255, 255)),

                            HStack({
                                Label("當前渲染架構：")
                                    .FontSize(14.0f)
                                    .Foreground(Color(170, 180, 200, 255)),
                                Label(backendName)
                                    .FontSize(15.0f)
                                    .FontWeight(cppui::text::FontWeight::Bold)
                                    .Foreground(statusColor)
                            }).Spacing(8.0f).Margin(0.0f, 8.0f, 0.0f, 4.0f),

                            Label(statusText)
                                .FontSize(13.0f)
                                .Foreground(statusColor)
                        })
                    )
                    .Background(Color(20, 24, 34, 255))
                    .BorderBrush(Color(42, 50, 70, 255))
                    .BorderThickness(1.0f)
                    .CornerRadius(8.0f)
                    .Padding(20.0f),

                    // 技術優勢說明卡片
                    Border(
                        VStack({
                            Label("Google Skia 向量渲染核心技術優勢")
                                .FontSize(16.0f)
                                .FontWeight(cppui::text::FontWeight::SemiBold)
                                .Foreground(Color(240, 245, 255, 255)),

                            Label("• 跨平台死局終結：擺脫 Win32 GDI 依賴，支援 Windows, Linux, macOS 與 Android。")
                                .FontSize(13.0f)
                                .Foreground(Color(170, 180, 200, 255)),

                            Label("• 向量抗鋸齒加速：支援高品質 MSAA 向量幾何抗鋸齒、貝茲曲線平滑光柵化與 GPU 著色器。")
                                .FontSize(13.0f)
                                .Foreground(Color(170, 180, 200, 255)),

                            Label("• 現代文字排版：Direct2D/HarfBuzz/Skia 複雜文字塑形與多語言排版無縫銜接。")
                                .FontSize(13.0f)
                                .Foreground(Color(170, 180, 200, 255)),

                            Label("• 雙引擎優雅回退：在無 GPU 驅動環境下，自動回退至自研零依賴 CPU 軟體光柵器。")
                                .FontSize(13.0f)
                                .Foreground(Color(170, 180, 200, 255))
                        }).Spacing(8.0f)
                    )
                    .Background(Color(20, 24, 34, 255))
                    .BorderBrush(Color(42, 50, 70, 255))
                    .BorderThickness(1.0f)
                    .CornerRadius(8.0f)
                    .Padding(20.0f),

                    // 控制按鈕
                    HStack({
                        Button("確認並關閉視窗")
                            .Width(180.0f)
                            .Height(38.0f)
                            .Background(Color(45, 115, 215, 255))
                            .Foreground(Color(255, 255, 255, 255))
                            .CornerRadius(6.0f)
                            .OnClick([this]() {
                                Close();
                            })
                    })
                })
                .Padding(24.0f)
                .Spacing(16.0f)
            })
        )
        .Background(Color(14, 17, 24, 255))
        .Get()
    );
}

void SkiaDemoForm::OnLoad() {
    Form::OnLoad();
    compat::println("[Lifecycle] SkiaDemoForm::OnLoad() 已觸發，當前 PlatformBackend: {}",
                    static_cast<int32_t>(cppui::app::Application::GetPlatformBackend()));
}

} // namespace platform_skia_example
