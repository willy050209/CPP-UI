/*
 * File: main.cpp
 * Purpose: Milestone 6 demo illustrating fluent RAD DSL and reactive signals.
 */

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/Visual.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../../src/rendering/backend/SoftwareRenderContext.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/FocusManager.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::core;
    using namespace cppui::reactive;
    using namespace cppui::dsl;
    using namespace cppui::rendering;
    using namespace cppui::input;

    compat::println("=================================================");
    compat::println(" [CPPUI M6] Fluent RAD DSL & Reactive UI Demo   ");
    compat::println("=================================================");

    // 1. 建立視窗
    uint32_t currentWidth = 840;
    uint32_t currentHeight = 640;
    pal::WindowDesc desc;
    desc.title = "CPPUI M6 - Fluent RAD DSL & Reactive Controls Demo";
    desc.width = currentWidth;
    desc.height = currentHeight;
    desc.resizable = true;

    auto window = pal::PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println("[Error] 無法建立平台原生視窗！");
        return 1;
    }
    window->Show();

    SoftwareRenderTarget target;
    target.Resize(currentWidth, currentHeight);
    DisplayListRenderer renderer;

    // 2. 宣告響應式狀態 (Reactive Signals)
    Signal<int32_t> clickCounter(0);
    Signal<std::string> statusMessage("歡迎使用 CPPUI M6 Fluent RAD DSL 宣告式介面！");

    // 3. 使用 Fluent RAD DSL 宣告現代化桌面 UI 階層架構 (< 60 行語意化構建)
    auto rootBorder = Border(
        VStack({
            // Header 區域
            Border(
                HStack({
                    Label("CPPUI M6")
                        .FontSize(20.0f)
                        .FontWeight(text::FontWeight::Bold)
                        .Foreground(Color(255, 255, 255, 255)),
                    Label("Fluent RAD DSL & Reactive Signals")
                        .FontSize(14.0f)
                        .Foreground(Color(130, 160, 200, 255))
                        .Margin(12.0f, 4.0f, 0.0f, 0.0f)
                })
            )
            .Background(Color(28, 32, 42, 255))
            .BorderBrush(Color(45, 52, 68, 255))
            .BorderThickness(0.0f, 0.0f, 0.0f, 1.0f)
            .Padding(20.0f, 14.0f),

            // 主工作區 (Grid 排版: 左側計數器與表單、右側即時日誌與排版展示)
            Grid({
                // [Grid Col 0] 左側卡片: 響應式計數器與文字輸入
                Border(
                    VStack({
                        Label("響應式狀態管理 (Reactive Counter)")
                            .FontSize(16.0f)
                            .FontWeight(text::FontWeight::SemiBold)
                            .Foreground(Color(230, 240, 255, 255)),

                        Label("點擊次數: 0")
                            .FontSize(18.0f)
                            .FontWeight(text::FontWeight::Bold)
                            .Foreground(Color(80, 200, 120, 255))
                            .Margin(0.0f, 6.0f)
                            .BindText(clickCounter, [](int32_t count) {
                                return "點擊次數: " + std::to_string(count);
                            }),

                        HStack({
                            Button("+1 增加")
                                .Width(90.0f)
                                .Height(34.0f)
                                .Background(Color(40, 110, 200, 255))
                                .Foreground(Color(255, 255, 255, 255))
                                .CornerRadius(4.0f)
                                .OnClick([&]() {
                                    clickCounter = clickCounter.Get() + 1;
                                    statusMessage = "點擊計數已增加至: " + std::to_string(clickCounter.Get());
                                    window->RequestRedraw();
                                }),

                            Button("-1 減少")
                                .Width(90.0f)
                                .Height(34.0f)
                                .Background(Color(60, 70, 85, 255))
                                .Foreground(Color(240, 240, 240, 255))
                                .CornerRadius(4.0f)
                                .OnClick([&]() {
                                    clickCounter = clickCounter.Get() - 1;
                                    statusMessage = "點擊計數已減少至: " + std::to_string(clickCounter.Get());
                                    window->RequestRedraw();
                                }),

                            Button("重設")
                                .Width(70.0f)
                                .Height(34.0f)
                                .Background(Color(180, 50, 50, 255))
                                .Foreground(Color(255, 255, 255, 255))
                                .CornerRadius(4.0f)
                                .OnClick([&]() {
                                    clickCounter = 0;
                                    statusMessage = "點擊計數已重設為 0！";
                                    window->RequestRedraw();
                                })
                        }).Spacing(8.0f),

                        Label("即時文字編輯 (TextBox & Focus Ring)")
                            .FontSize(15.0f)
                            .FontWeight(text::FontWeight::SemiBold)
                            .Foreground(Color(230, 240, 255, 255))
                            .Margin(0.0f, 16.0f, 0.0f, 4.0f),

                        TextBox("請輸入您的姓名或帳號...")
                            .Height(36.0f)
                            .Padding(8.0f, 6.0f)
                            .CornerRadius(4.0f)
                            .OnTextChanged([&](compat::string_view txt) {
                                std::string s(txt.data(), txt.size());
                                statusMessage = "輸入內容變更為: [" + s + "]";
                                window->RequestRedraw();
                            }),

                        CheckBox("啟用進階除錯日誌模式")
                            .Margin(0.0f, 10.0f, 0.0f, 0.0f)
                            .OnCheckedChanged([&](bool checked) {
                                statusMessage = checked ? "已開啟除錯日誌模式！" : "已關閉除錯日誌模式。";
                                window->RequestRedraw();
                            })
                    }).Spacing(10.0f)
                )
                .Row(0).Column(0)
                .Background(Color(25, 28, 36, 255))
                .BorderBrush(Color(45, 52, 68, 255))
                .BorderThickness(1.0f)
                .CornerRadius(8.0f)
                .Padding(16.0f)
                .Margin(16.0f),

                // [Grid Col 1] 右側卡片: Grid 網格佈局展示
                Border(
                    VStack({
                        Label("多儲存格網格佈局 (Grid Multi-Cell)")
                            .FontSize(16.0f)
                            .FontWeight(text::FontWeight::SemiBold)
                            .Foreground(Color(230, 240, 255, 255)),

                        Label("Grid 支援 Pixel、Auto 與比例權重 Star (*) 多維行列排版。")
                            .FontSize(12.0f)
                            .Foreground(Color(140, 155, 175, 255)),

                        Grid({
                            Border(Label("Cell [0, 0]").HorizontalAlignment(layout::HorizontalAlignment::Center))
                                .Row(0).Column(0)
                                .Background(Color(40, 50, 70, 255))
                                .CornerRadius(4.0f).Padding(6.0f),

                            Border(Label("Cell [0, 1]").HorizontalAlignment(layout::HorizontalAlignment::Center))
                                .Row(0).Column(1)
                                .Background(Color(45, 58, 82, 255))
                                .CornerRadius(4.0f).Padding(6.0f),

                            Border(Label("Cell [1, 0..1] (Span 2)").HorizontalAlignment(layout::HorizontalAlignment::Center))
                                .Row(1).Column(0).ColumnSpan(2)
                                .Background(Color(32, 70, 60, 255))
                                .CornerRadius(4.0f).Padding(6.0f)
                        })
                        .Margin(0.0f, 10.0f)
                        .Height(120.0f)
                    }).Spacing(10.0f)
                )
                .Row(0).Column(1)
                .Background(Color(25, 28, 36, 255))
                .BorderBrush(Color(45, 52, 68, 255))
                .BorderThickness(1.0f)
                .CornerRadius(8.0f)
                .Padding(16.0f)
                .Margin(0.0f, 16.0f, 16.0f, 16.0f)
            })
            .Height(460.0f),

            // Footer 狀態列 (自動綁定 Signal)
            Border(
                Label("狀態: 歡迎使用 CPPUI M6！")
                    .FontSize(12.0f)
                    .FontFamily("Consolas")
                    .Foreground(Color(130, 160, 200, 255))
                    .BindText(statusMessage, [](const std::string& msg) {
                        return "狀態: " + msg;
                    })
            )
            .Background(Color(18, 20, 26, 255))
            .BorderBrush(Color(35, 40, 52, 255))
            .BorderThickness(0.0f, 1.0f, 0.0f, 0.0f)
            .Padding(16.0f, 8.0f)
        })
    )
    .Background(Color(15, 17, 23, 255))
    .Get();

    // 設定主 Grid 的欄列定義
    if (auto* vstack = dynamic_cast<layout::StackPanel*>(rootBorder->GetChild())) {
        if (vstack->GetChildren().size() >= 2) {
            if (auto mainGrid = std::dynamic_pointer_cast<layout::Grid>(vstack->GetChildren()[1])) {
                mainGrid->AddColumn(layout::GridLength::Star(1.0f));
                mainGrid->AddColumn(layout::GridLength::Star(1.0f));
                mainGrid->AddRow(layout::GridLength::Star(1.0f));
            }
        }
    }

    // 視窗排版與繪製回呼
    auto updateLayoutAndRender = [&]() {
        Size availSize(static_cast<float>(currentWidth), static_cast<float>(currentHeight));
        layout::LayoutManager::Get().UpdateLayout(*rootBorder, availSize);

        target.Clear(Color(15, 17, 23, 255));
        DisplayList dl;
        DrawingContext dc(dl);
        rootBorder->Render(dc);
        renderer.Render(dl, &target);
    };

    window->SetPaintCallback([&]() {
        updateLayoutAndRender();
        window->PresentFramebuffer(target.GetPixelSpan(), currentWidth, currentHeight);
    });

    // 平台原生事件接駁
    window->SetEventCallback([&](const pal::RawInputEvent& e) {
        if (e.type == pal::RawEventType::WindowClose) {
            window->Close();
            return;
        }
        if (e.type == pal::RawEventType::WindowResize) {
            currentWidth = e.window.width;
            currentHeight = e.window.height;
            target.Resize(currentWidth, currentHeight);
            window->RequestRedraw();
            return;
        }

        InputManager::Get().ProcessRawEvent(*rootBorder, e);
        window->RequestRedraw();
    });

    updateLayoutAndRender();
    window->RequestRedraw();

    compat::println("M6 Fluent RAD Demo 視窗啟動成功！請體驗 Fluent DSL 控制項互動與響應式資料流。");

    // 主訊息迴圈
    while (!window->IsClosed()) {
        window->WaitEvents(16);
    }

    compat::println("M6 Fluent RAD Demo 結束執行。");
    return 0;
}
