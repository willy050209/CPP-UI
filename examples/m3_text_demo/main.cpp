/*
 * File: main.cpp
 * Purpose: Milestone 3 demo illustrating text pipeline, font selection, and typography metrics.
 */

#include <chrono>
#include <cmath>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/FontManager.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "../../src/rendering/backend/SoftwareRenderContext.hpp"

int main() {
    using namespace cppui::core;
    using namespace cppui::pal;
    using namespace cppui::rendering;
    using namespace cppui::text;

    compat::println("================================================================");
    compat::println(" CPPUI Milestone 3: Text Pipeline, Typography & Hit-Testing Demo");
    compat::println("================================================================");

    // 1. 建立視窗
    WindowDesc desc;
    desc.title = "CPPUI M3 - Typography, Wrapping & Hit-Testing Demo";
    desc.width = 1000;
    desc.height = 700;
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

    // 3. 取得系統字型與排版引擎
    FontManager& fm = FontManager::Get();
    FontDescriptor titleFont = fm.GetDefaultChineseFont(24.0f).WithWeight(FontWeight::Bold);
    FontDescriptor headingFont = fm.GetDefaultFont(16.0f).WithWeight(FontWeight::SemiBold);
    FontDescriptor bodyFont = fm.GetDefaultChineseFont(14.0f);
    FontDescriptor italicFont = fm.GetDefaultFont(13.0f).WithStyle(FontStyle::Italic);
    FontDescriptor monoFont = fm.GetDefaultMonospaceFont(13.0f);

    TextLayoutEngine& textEngine = TextLayoutEngine::Get();

    // 互動狀態
    float dpiScale = window->GetDpiScale();
    Point mousePos(0.0f, 0.0f);
    bool isMouseDown = false;
    uint32_t currentWidth = static_cast<uint32_t>(fbSize.width);
    uint32_t currentHeight = static_cast<uint32_t>(fbSize.height);

    // 響應式排版段落與自適應寬度快取
    compat::string_view card1Text = "CPPUI 工業級文字管線已全面上線！支援 DirectWrite 原生字形塑形、Unicode UAX #14 斷行規範與多字節 UTF-8 編碼。";
    compat::string_view centerText = "這是一段置中對齊的多行段落 (Center Aligned)，能夠自適應各種控制項寬度約束。";
    compat::string_view trimSample = "這是一段超長文字將會在此邊界被截斷並呈現省略號 (TextTrimming::CharacterEllipsis) 無論長度多長...";
    compat::string_view bidiSample = "Bidirectional Text: Hello مرحبا (Arabic) and שלום (Hebrew) in one line!";
    compat::string_view interactiveText = "【滑鼠命中測試區】請將滑鼠移入此段文字上，或點擊任意字元：CPPUI 會精準計算出當前字元的索引位置，並繪製即時跟隨的插入游標 (Cursor Rect)！Try clicking anywhere inside this box.";

    float lastContentW = -1.0f;
    TextLayout card1Layout;
    TextLayout centerLayout;
    TextLayout trimLayout;
    TextLayout bidiLayout;
    TextLayout interactiveLayout;

    auto updateLayouts = [&](float contentW) {
        if (std::abs(contentW - lastContentW) < 1.0f) return;
        lastContentW = contentW;

        card1Layout = textEngine.CreateLayout(card1Text, bodyFont, contentW, 400.0f, TextAlignment::Left, TextWrapping::WordWrap);
        centerLayout = textEngine.CreateLayout(centerText, bodyFont, contentW, 400.0f, TextAlignment::Center, TextWrapping::WordWrap);
        trimLayout = textEngine.CreateLayout(trimSample, bodyFont, contentW, 30.0f, TextAlignment::Left, TextWrapping::NoWrap, TextTrimming::CharacterEllipsis);
        bidiLayout = textEngine.CreateLayout(bidiSample, headingFont, contentW, 60.0f, TextAlignment::Left, TextWrapping::NoWrap);
        interactiveLayout = textEngine.CreateLayout(interactiveText, bodyFont, contentW, 400.0f, TextAlignment::Left, TextWrapping::WordWrap);
    };

    RunLoop loop;
    loop.AttachWindow(window);

    window->SetEventCallback([&](const RawInputEvent& e) {
        switch (e.type) {
            case RawEventType::MouseMove:
                mousePos = e.mouse.position;
                loop.RequestRedraw();
                break;
            case RawEventType::MouseDown:
                isMouseDown = true;
                mousePos = e.mouse.position;
                loop.RequestRedraw();
                break;
            case RawEventType::MouseUp:
                isMouseDown = false;
                mousePos = e.mouse.position;
                loop.RequestRedraw();
                break;
            case RawEventType::WindowResize:
                currentWidth = e.window.width;
                currentHeight = e.window.height;
                dpiScale = e.window.dpiScale;
                renderContext->Resize(currentWidth, currentHeight);
                loop.RequestRedraw();
                break;
            case RawEventType::KeyDown:
                if (e.key.virtualKey == 27) {
                    window->Close();
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

    compat::println("Entering RunLoop. Move mouse over the interactive text area to inspect hit-testing!");

    auto startTime = std::chrono::high_resolution_clock::now();

    loop.SetRenderCallback([&]() {
        if (currentWidth == 0 || currentHeight == 0) return;

        auto now = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(now - startTime).count();

        // 1. 動態計算響應式雙欄佈局 (Responsive Layout)
        float margin = 24.0f;
        float gap = 20.0f;
        float availW = (std::max)(500.0f, static_cast<float>(currentWidth) - margin * 2.0f);
        float colWidth = (std::max)(240.0f, (availW - gap) * 0.5f);
        float cardContentW = (std::max)(100.0f, colWidth - 32.0f);

        updateLayouts(cardContentW);

        float col1X = margin;
        float col2X = col1X + colWidth + gap;

        // 2. 重設 DisplayList 記憶體池
        displayList.Clear();

        // 3. 繪製深色現代背景
        dc.DrawRectangle(Rect(0.0f, 0.0f, static_cast<float>(currentWidth), static_cast<float>(currentHeight)), Color(28, 30, 36, 255));

        // 4. 繪製頂部導航列 / 標題
        dc.DrawRectangle(Rect(0.0f, 0.0f, static_cast<float>(currentWidth), 60.0f), Color(38, 41, 50, 255));
        dc.DrawText("CPPUI M3 - 文字排版與雙向命中測試展示", Point(24.0f, 16.0f), titleFont, Colors::White);
        dc.DrawText("DirectWrite Native Engine (60 FPS)", Point(static_cast<float>(currentWidth) - 280.0f, 22.0f), monoFont, Color(160, 180, 210, 255));

        // 卡片 1：自動折行與基礎樣式 (高度自適應文字行數)
        float card1H = card1Layout.GetSize().height + 56.0f;
        Rect card1(col1X, 80.0f, colWidth, card1H);
        dc.DrawRoundedRectangle(card1, 8.0f, Color(35, 38, 46, 255));
        dc.DrawText("1. 多行自動折行 (WordWrap)", Point(card1.x + 16.0f, card1.y + 14.0f), headingFont, Colors::SteelBlue);
        dc.DrawTextLayout(card1Layout, Point(card1.x + 16.0f, card1.y + 44.0f), Color(220, 220, 220, 255));

        // 卡片 2：置中對齊
        float card2H = centerLayout.GetSize().height + 56.0f;
        Rect card2(col1X, card1.y + card1.height + 16.0f, colWidth, card2H);
        dc.DrawRoundedRectangle(card2, 8.0f, Color(35, 38, 46, 255));
        dc.DrawText("2. 文字置中對齊 (TextAlignment::Center)", Point(card2.x + 16.0f, card2.y + 14.0f), headingFont, Colors::SteelBlue);
        dc.DrawTextLayout(centerLayout, Point(card2.x + 16.0f, card2.y + 44.0f), Color(220, 220, 220, 255));

        // 卡片 3：省略號修剪與雙向文字
        float card3H = trimLayout.GetSize().height + bidiLayout.GetSize().height + 72.0f;
        Rect card3(col1X, card2.y + card2.height + 16.0f, colWidth, card3H);
        dc.DrawRoundedRectangle(card3, 8.0f, Color(35, 38, 46, 255));
        dc.DrawText("3. 省略號裁切與雙向文字 (BiDi)", Point(card3.x + 16.0f, card3.y + 14.0f), headingFont, Colors::SteelBlue);
        dc.DrawTextLayout(trimLayout, Point(card3.x + 16.0f, card3.y + 44.0f), Color(255, 200, 100, 255));
        dc.DrawTextLayout(bidiLayout, Point(card3.x + 16.0f, card3.y + 44.0f + trimLayout.GetSize().height + 10.0f), Color(130, 220, 180, 255));

        // 卡片 4：右側互動式命中測試專案 (高度自適應左欄或視窗客戶區)
        float col1Bottom = card3.y + card3.height;
        float rightH = (std::max)(col1Bottom - 80.0f, static_cast<float>(currentHeight) - 80.0f - 48.0f);
        Rect cardInteractive(col2X, 80.0f, colWidth, rightH);
        dc.DrawRoundedRectangle(cardInteractive, 8.0f, Color(35, 38, 46, 255));
        dc.DrawText("4. 即時雙向命中測試 (Hit-Testing Demo)", Point(cardInteractive.x + 16.0f, cardInteractive.y + 14.0f), headingFont, Color(255, 140, 0, 255));

        Point textOrigin(cardInteractive.x + 16.0f, cardInteractive.y + 50.0f);
        dc.DrawTextLayout(interactiveLayout, textOrigin, Color(230, 230, 230, 255));

        // 計算滑鼠相對於文字起點的局部座標
        Point localMouse(mousePos.x - textOrigin.x, mousePos.y - textOrigin.y);
        TextHitTestResult hit = interactiveLayout.HitTestPoint(localMouse);

        float infoY = textOrigin.y + interactiveLayout.GetSize().height + 24.0f;

        // 若滑鼠在文字區域內，繪製高亮反饋與動態游標
        if (hit.isInside) {
            // 字元邊界高亮
            Rect charBox = interactiveLayout.HitTestTextPosition(hit.textPosition);
            dc.DrawRectangle(Rect(textOrigin.x + charBox.x, textOrigin.y + charBox.y, charBox.width, charBox.height), Color(0, 150, 255, 50));

            // 插入游標
            Rect globalCursor(textOrigin.x + hit.cursorRect.x, textOrigin.y + hit.cursorRect.y, 2.0f, hit.cursorRect.height);

            // 閃爍游標動畫 (每秒閃爍 2 次)
            bool cursorVisible = (std::fmod(elapsed * 2.0f, 1.0f) < 0.5f) || isMouseDown;
            if (cursorVisible) {
                dc.DrawRectangle(globalCursor, Color(0, 180, 255, 255));
            }

            // 顯示即時命中度量資訊
            std::string hitInfo = "命中字元索引: " + compat::to_string(hit.textPosition)
                                + " | 後半字元: " + (hit.isTrailingHit ? "TRUE" : "FALSE");
            dc.DrawText(hitInfo, Point(cardInteractive.x + 16.0f, infoY), monoFont, Colors::SteelBlue);
        } else {
            dc.DrawText("請將滑鼠移入上方文字段落內測試...", Point(cardInteractive.x + 16.0f, infoY), italicFont, Colors::Gray);
        }

        // 底部狀態列
        dc.DrawRectangle(Rect(0.0f, static_cast<float>(currentHeight) - 32.0f, static_cast<float>(currentWidth), 32.0f), Color(20, 22, 26, 255));
        dc.DrawText("CPPUI M3 Architecture | 0 GC | Zero Memory Leaks | Cross-Compiler Ready", Point(24.0f, static_cast<float>(currentHeight) - 24.0f), monoFont, Color(140, 140, 140, 255));

        // 5. 重播繪製至軟體畫布並呈現
        IRenderTarget* target = renderContext->BeginFrame();
        renderer.Render(displayList, target);
        renderContext->EndFrame();

        // 持續請求重繪以維持 60 FPS 動畫 (光標閃爍與動態度量)
        loop.RequestRedraw();
    });

    loop.Run();
    return 0;
}
