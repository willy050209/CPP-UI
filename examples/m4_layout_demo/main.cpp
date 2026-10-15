/*
 * File: main.cpp
 * Purpose: Milestone 4 demo illustrating the visual tree and two-pass layout engine.
 */

#include <chrono>
#include <cmath>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/Visual.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Canvas.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../../src/rendering/backend/SoftwareRenderContext.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::core;
    using namespace cppui::layout;
    using namespace cppui::controls;
    using namespace cppui::pal;
    using namespace cppui::rendering;

    compat::println("================================================================");
    compat::println(" CPPUI Milestone 4: Visual Tree & Two-Pass Layout Engine Demo   ");
    compat::println("================================================================");

    // 1. 建立視窗
    WindowDesc desc;
    desc.title = "CPPUI M4 - Visual Tree & Two-Pass Layout Engine Demo";
    desc.width = 1160;
    desc.height = 760;
    desc.resizable = true;

    auto window = PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println(stderr, "Failed to create platform window!");
        return -1;
    }

    // 2. 建立軟體渲染 Context
    auto renderContext = std::make_unique<SoftwareRenderContext>();
    Size fbSize = window->GetFramebufferSize();
    if (!renderContext->Init(window->GetNativeHandle(), static_cast<uint32_t>(fbSize.width), static_cast<uint32_t>(fbSize.height))) {
        compat::println(stderr, "Failed to initialize render context!");
        return -1;
    }

    uint32_t currentWidth = static_cast<uint32_t>(fbSize.width);
    uint32_t currentHeight = static_cast<uint32_t>(fbSize.height);

    // 3. 建立 3 層巢狀視覺排版樹
    // 根節點: Border
    auto rootBorder = std::make_shared<Border>();
    rootBorder->SetBackground(Color(24, 26, 32, 255));
    rootBorder->SetBorderBrush(Color(45, 50, 62, 255));
    rootBorder->SetBorderThickness(Thickness(1.0f));
    rootBorder->SetPadding(Thickness(20.0f));

    // 主排版容器: 垂直 StackPanel
    auto mainStack = std::make_shared<StackPanel>();
    mainStack->SetOrientation(Orientation::Vertical);
    mainStack->SetSpacing(16.0f);
    rootBorder->SetChild(mainStack);

    // --- 頂部標題區 ---
    auto headerPanel = std::make_shared<StackPanel>();
    headerPanel->SetOrientation(Orientation::Horizontal);
    headerPanel->SetSpacing(12.0f);

    auto titleBlock = std::make_shared<TextBlock>("CPPUI M4 - 視覺樹與雙階段排版引擎");
    titleBlock->SetFontSize(22.0f);
    titleBlock->SetFontWeight(text::FontWeight::Bold);
    titleBlock->SetForeground(Color(255, 255, 255, 255));

    auto subTitleBlock = std::make_shared<TextBlock>("(Measure -> Arrange Pipeline | 0 GC | Dirty Tracking)");
    subTitleBlock->SetFontSize(13.0f);
    subTitleBlock->SetVerticalAlignment(VerticalAlignment::Center);
    subTitleBlock->SetForeground(Color(130, 150, 180, 255));

    headerPanel->AddChild(titleBlock);
    headerPanel->AddChild(subTitleBlock);
    mainStack->AddChild(headerPanel);

    // --- 操作說明工具提示列 ---
    auto toolbarBorder = std::make_shared<Border>();
    toolbarBorder->SetBackground(Color(32, 36, 46, 255));
    toolbarBorder->SetBorderBrush(Color(55, 62, 78, 255));
    toolbarBorder->SetBorderThickness(Thickness(1.0f));
    toolbarBorder->SetCornerRadius(6.0f);
    toolbarBorder->SetPadding(Thickness(14.0f, 10.0f, 14.0f, 10.0f));

    auto toolbarText = std::make_shared<TextBlock>("【快捷鍵控制】[A] 新增動態卡片 | [D] 刪除卡片 | [C] 切換標題色彩 (純 Visual 刷新，零排版計算) | [H] 切換對齊 | 拖曳視窗即時響應式排版");
    toolbarText->SetFontSize(13.0f);
    toolbarText->SetForeground(Color(140, 200, 255, 255));
    toolbarBorder->SetChild(toolbarText);
    mainStack->AddChild(toolbarBorder);

    // --- 內容展示區: 水平雙欄佈局 ---
    auto contentPanel = std::make_shared<StackPanel>();
    contentPanel->SetOrientation(Orientation::Horizontal);
    contentPanel->SetSpacing(20.0f);
    mainStack->AddChild(contentPanel);

    // [左欄]: 垂直 StackPanel，包含動態新增的卡片
    auto leftColumnBorder = std::make_shared<Border>();
    leftColumnBorder->SetBackground(Color(28, 31, 39, 255));
    leftColumnBorder->SetBorderBrush(Color(48, 54, 68, 255));
    leftColumnBorder->SetBorderThickness(Thickness(1.0f));
    leftColumnBorder->SetCornerRadius(8.0f);
    leftColumnBorder->SetPadding(Thickness(16.0f));

    auto cardStack = std::make_shared<StackPanel>();
    cardStack->SetOrientation(Orientation::Vertical);
    cardStack->SetSpacing(12.0f);
    leftColumnBorder->SetChild(cardStack);

    auto leftTitle = std::make_shared<TextBlock>("動態線性列表 (StackPanel 垂直流式排列)");
    leftTitle->SetFontSize(15.0f);
    leftTitle->SetFontWeight(text::FontWeight::SemiBold);
    leftTitle->SetForeground(Color(255, 180, 80, 255));
    cardStack->AddChild(leftTitle);

    contentPanel->AddChild(leftColumnBorder);

    // 輔助函式: 建立卡片元件 (黃金比例：寬敞內距、清晰字體層級、良好行距)
    int32_t cardCounter = 0;
    auto createCard = [&](const std::string& title, const std::string& descText, Color accent) {
        auto card = std::make_shared<Border>();
        card->SetBackground(Color(36, 40, 52, 255));
        card->SetBorderBrush(accent);
        card->SetBorderThickness(Thickness(1.5f));
        card->SetCornerRadius(8.0f);
        card->SetPadding(Thickness(16.0f, 12.0f, 16.0f, 12.0f));

        auto cStack = std::make_shared<StackPanel>();
        cStack->SetOrientation(Orientation::Vertical);
        cStack->SetSpacing(6.0f);

        auto tBlock = std::make_shared<TextBlock>(title);
        tBlock->SetFontSize(15.0f);
        tBlock->SetFontWeight(text::FontWeight::SemiBold);
        tBlock->SetForeground(Color(245, 245, 250, 255));

        auto dBlock = std::make_shared<TextBlock>(descText);
        dBlock->SetFontSize(13.0f);
        dBlock->SetForeground(Color(175, 185, 200, 255));

        cStack->AddChild(tBlock);
        cStack->AddChild(dBlock);
        card->SetChild(cStack);
        return card;
    };

    // 加入初始卡片
    cardStack->AddChild(createCard("Card #1: Measure 階段", "父節點傳遞 AvailableSize，子元件計算 Content DesiredSize 並快取。", Color(80, 160, 255, 255)));
    cardStack->AddChild(createCard("Card #2: Arrange 階段", "父節點配置 FinalRect，子元件定位 RenderBounds 與 VisualOffset。", Color(80, 220, 140, 255)));
    cardStack->AddChild(createCard("Card #3: 髒標記傳播 (Dirty Tracking)", "局部變更時向上冒泡 InvalidateMeasure，未改動子樹 100% 快取命中。", Color(240, 120, 80, 255)));
    cardCounter = 3;

    // [右欄]: Canvas 絕對定位畫布
    auto rightColumnBorder = std::make_shared<Border>();
    rightColumnBorder->SetBackground(Color(28, 31, 39, 255));
    rightColumnBorder->SetBorderBrush(Color(48, 54, 68, 255));
    rightColumnBorder->SetBorderThickness(Thickness(1.0f));
    rightColumnBorder->SetCornerRadius(8.0f);
    rightColumnBorder->SetPadding(Thickness(16.0f));

    auto canvasLayout = std::make_shared<Canvas>();
    rightColumnBorder->SetChild(canvasLayout);

    auto canvasTitle = std::make_shared<TextBlock>("絕對定位畫布 (Canvas Absolute Coordinates)");
    canvasTitle->SetFontSize(15.0f);
    canvasTitle->SetFontWeight(text::FontWeight::SemiBold);
    canvasTitle->SetForeground(Color(180, 140, 255, 255));
    Canvas::SetLeft(canvasTitle, 0.0f);
    Canvas::SetTop(canvasTitle, 0.0f);
    canvasLayout->AddChild(canvasTitle);

    auto floatCard1 = createCard("Canvas Item A", "位置: Left=24, Top=40 (浮動元件)", Color(255, 100, 150, 255));
    floatCard1->SetWidth(260.0f);
    Canvas::SetLeft(floatCard1, 24.0f);
    Canvas::SetTop(floatCard1, 40.0f);
    canvasLayout->AddChild(floatCard1);

    auto floatCard2 = createCard("Canvas Item B", "位置: Left=190, Top=155 (二維圖層)", Color(255, 210, 80, 255));
    floatCard2->SetWidth(260.0f);
    Canvas::SetLeft(floatCard2, 190.0f);
    Canvas::SetTop(floatCard2, 155.0f);
    canvasLayout->AddChild(floatCard2);

    auto floatCard3 = createCard("Canvas Item C", "位置: Left=60, Top=270 (自由定位)", Color(60, 220, 220, 255));
    floatCard3->SetWidth(260.0f);
    Canvas::SetLeft(floatCard3, 60.0f);
    Canvas::SetTop(floatCard3, 270.0f);
    canvasLayout->AddChild(floatCard3);

    contentPanel->AddChild(rightColumnBorder);

    // 依據視窗尺寸即時計算雙欄佈局尺寸 (響應式均分)
    auto updateColumnLayout = [&](uint32_t w, uint32_t h) {
        float availableColWidth = std::max(320.0f, (static_cast<float>(w) - 42.0f - 20.0f) * 0.5f);
        leftColumnBorder->SetWidth(availableColWidth);
        rightColumnBorder->SetWidth(availableColWidth);
        rightColumnBorder->SetHeight(std::max(420.0f, static_cast<float>(h) - 250.0f));
    };
    updateColumnLayout(currentWidth, currentHeight);

    // --- 底部即時狀態監控欄 ---
    auto footerBorder = std::make_shared<Border>();
    footerBorder->SetBackground(Color(18, 20, 24, 255));
    footerBorder->SetBorderBrush(Color(36, 40, 50, 255));
    footerBorder->SetBorderThickness(Thickness(1.0f));
    footerBorder->SetCornerRadius(4.0f);
    footerBorder->SetPadding(Thickness(14.0f, 8.0f, 14.0f, 8.0f));

    auto statusText = std::make_shared<TextBlock>("準備中...");
    statusText->SetFontSize(12.0f);
    statusText->SetFontFamily("Consolas");
    statusText->SetForeground(Color(140, 160, 180, 255));
    footerBorder->SetChild(statusText);
    mainStack->AddChild(footerBorder);

    // 狀態管理
    int32_t colorIdx = 0;
    std::vector<Color> titleColors = {
        Color(255, 255, 255, 255),
        Color(100, 220, 255, 255),
        Color(255, 215, 80, 255),
        Color(140, 255, 140, 255),
        Color(255, 130, 200, 255)
    };

    int32_t alignIdx = 0;
    std::vector<HorizontalAlignment> aligns = {
        HorizontalAlignment::Left,
        HorizontalAlignment::Center,
        HorizontalAlignment::Right,
        HorizontalAlignment::Stretch
    };
    std::vector<const char*> alignNames = { "Left", "Center", "Right", "Stretch" };

    LayoutManager& layoutMgr = LayoutManager::Get();

    RunLoop loop;
    loop.AttachWindow(window);

    window->SetEventCallback([&](const RawInputEvent& e) {
        switch (e.type) {
            case RawEventType::WindowResize:
                currentWidth = e.window.width;
                currentHeight = e.window.height;
                renderContext->Resize(currentWidth, currentHeight);
                updateColumnLayout(currentWidth, currentHeight);
                layoutMgr.UpdateLayout(*rootBorder, Size(static_cast<float>(currentWidth), static_cast<float>(currentHeight)));
                loop.RequestRedraw();
                break;

            case RawEventType::KeyDown:
                if (e.key.virtualKey == 27) { // ESC
                    window->Close();
                } else if (e.key.virtualKey == 'A') { // 新增卡片
                    cardCounter++;
                    std::string t = "動態卡片 #" + compat::to_string(cardCounter);
                    std::string d = "觸發 InvalidateMeasure 向上冒泡重排版";
                    Color accent = titleColors[cardCounter % titleColors.size()];
                    cardStack->AddChild(createCard(t, d, accent));
                    loop.RequestRedraw();
                } else if (e.key.virtualKey == 'D') { // 刪除卡片
                    if (cardStack->GetChildrenCount() > 1) {
                        const auto& children = cardStack->GetChildren();
                        cardStack->RemoveChild(children.back());
                        loop.RequestRedraw();
                    }
                } else if (e.key.virtualKey == 'C') { // 切換色彩 (純 Visual 髒標記)
                    colorIdx = (colorIdx + 1) % titleColors.size();
                    titleBlock->SetForeground(titleColors[colorIdx]);
                    loop.RequestRedraw();
                } else if (e.key.virtualKey == 'H') { // 切換對齊
                    alignIdx = (alignIdx + 1) % aligns.size();
                    headerPanel->SetHorizontalAlignment(aligns[alignIdx]);
                    loop.RequestRedraw();
                }
                break;

            case RawEventType::WindowClose:
                window->Close();
                break;

            default:
                break;
        }
    });

    DisplayList displayList;
    DrawingContext dc(displayList);
    DisplayListRenderer renderer;

    auto updateLayoutAndRender = [&]() {
        if (currentWidth == 0 || currentHeight == 0) return;

        // 1. 更新狀態字串 (於排版更新前執行，確保狀態字串之度量與邊界配置與當前幀同步)
        auto stats = layoutMgr.GetStats();
        std::string statStr = compat::format("【排版引擎即時度量】 累計 Measure: {} 次 | 累計 Arrange: {} 次 | 視窗解析度: {}x{} | 標題對齊: {} | 卡片總數: {}",
            stats.totalMeasures, stats.totalArranges, currentWidth, currentHeight, alignNames[alignIdx], cardStack->GetChildrenCount() - 1);
        statusText->SetText(statStr);

        // 2. 執行排版更新 (雙階段管線)
        updateColumnLayout(currentWidth, currentHeight);
        layoutMgr.UpdateLayout(*rootBorder, Size(static_cast<float>(currentWidth), static_cast<float>(currentHeight)));

        // 3. 錄製視覺樹繪製指令流
        displayList.Clear();
        rootBorder->Render(dc);

        // 4. 重播繪製至軟體後端
        IRenderTarget* target = renderContext->BeginFrame();
        renderer.Render(displayList, target);
        renderContext->EndFrame();
    };

    loop.SetRenderCallback(updateLayoutAndRender);

    // 視窗 WM_SIZE / WM_PAINT 即時回呼
    window->SetPaintCallback(updateLayoutAndRender);

    compat::println("M4 Layout Demo running! Press [A] Add, [D] Remove, [C] Color, [H] Align, or resize window.");
    loop.Run();
    return 0;
}
