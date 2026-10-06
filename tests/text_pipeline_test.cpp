#include <cassert>
#include <chrono>
#include "compat/compat.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/FontManager.hpp"
#include "ui/text/TextShaper.hpp"
#include "ui/text/BiDiAnalyzer.hpp"
#include "ui/text/LineBreaker.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../src/rendering/backend/SoftwareRenderContext.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::text;

    compat::println("=================================================");
    compat::println(" [CPPUI M3] Text Pipeline Full Automated Tests   ");
    compat::println("=================================================");

    // -------------------------------------------------------------------------
    // 1. FontManager 測試
    // -------------------------------------------------------------------------
    compat::println("[Test 1] FontManager Query & Metrics...");
    FontManager& fm = FontManager::Get();
    FontDescriptor defFont = fm.GetDefaultFont(16.0f);
    assert(!defFont.GetFamily().empty());
    assert(defFont.GetSize() == 16.0f);

    FontDescriptor zhFont = fm.GetDefaultChineseFont(16.0f);
    assert(!zhFont.GetFamily().empty());

    FontDescriptor monoFont = fm.GetDefaultMonospaceFont(14.0f);
    assert(!monoFont.GetFamily().empty());

    FontMetrics metrics = fm.GetFontMetrics(defFont);
    assert(metrics.ascent > 0.0f);
    assert(metrics.descent > 0.0f);
    assert(metrics.GetLineHeight() > 0.0f);
    compat::println("  [PASS] Default font: {}, Chinese font: {}, Monospace: {}",
                    defFont.GetFamily(), zhFont.GetFamily(), monoFont.GetFamily());
    compat::println("  [PASS] Metrics: ascent={:.1f}, descent={:.1f}, lineHeight={:.1f}",
                    metrics.ascent, metrics.descent, metrics.GetLineHeight());

    // -------------------------------------------------------------------------
    // 2. TextShaper 字符塑形測試
    // -------------------------------------------------------------------------
    compat::println("[Test 2] TextShaper Shaping (Latin, CJK, Emoji)...");
    TextShaper shaper;
    compat::string_view sampleText = "Hello 世界! 🚀";
    std::vector<ShapedRun> runs = shaper.Shape(sampleText, defFont);
    assert(!runs.empty());

    size_t totalGlyphs = 0;
    float shapedWidth = 0.0f;
    for (const auto& run : runs) {
        totalGlyphs += run.GetGlyphCount();
        shapedWidth += run.GetTotalWidth();
        compat::span<const ShapedGlyph> glyphSpan = run.GetGlyphs();
        assert(glyphSpan.size() == run.GetGlyphCount());
    }
    assert(totalGlyphs > 0);
    assert(shapedWidth > 0.0f);
    compat::println("  [PASS] Shaped '{}': {} runs, {} glyphs, totalWidth={:.2f}",
                    sampleText, runs.size(), totalGlyphs, shapedWidth);

    // -------------------------------------------------------------------------
    // 3. BiDiAnalyzer 雙向文字分析測試
    // -------------------------------------------------------------------------
    compat::println("[Test 3] BiDiAnalyzer (LTR, RTL, Mixed)...");
    BiDiAnalyzer bidi;
    compat::string_view ltrText = "Hello World 123";
    std::vector<BiDiRun> ltrRuns = bidi.Analyze(ltrText);
    assert(!ltrRuns.empty());
    assert(ltrRuns[0].isRtl == false);
    assert(bidi.DetectBaseDirection(ltrText) == BiDiDirection::LeftToRight);

    // 阿拉伯文 (مرحبا)
    compat::string_view rtlText = "\xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7";
    std::vector<BiDiRun> rtlRuns = bidi.Analyze(rtlText);
    assert(!rtlRuns.empty());
    assert(rtlRuns[0].isRtl == true);
    assert(bidi.DetectBaseDirection(rtlText) == BiDiDirection::RightToLeft);

    // 混排
    compat::string_view mixedText = "Welcome مرحبا to CPPUI";
    std::vector<BiDiRun> mixedRuns = bidi.Analyze(mixedText);
    assert(mixedRuns.size() >= 3);
    assert(bidi.DetectBaseDirection(mixedText) == BiDiDirection::Mixed);
    compat::println("  [PASS] BiDi analysis detected {} runs in mixed text", mixedRuns.size());

    // -------------------------------------------------------------------------
    // 4. LineBreaker 換行斷詞測試
    // -------------------------------------------------------------------------
    compat::println("[Test 4] LineBreaker Unicode UAX #14 Rules...");
    LineBreaker breaker;
    compat::string_view breakSample = "Hello World\n這是測試，標點符號禁則。";
    std::vector<LineBreak> breaks = breaker.FindBreaks(breakSample);
    assert(!breaks.empty());

    bool foundMustBreak = false;
    for (const auto& lb : breaks) {
        if (lb.opportunity == BreakOpportunity::MustBreak) {
            foundMustBreak = true;
            break;
        }
    }
    assert(foundMustBreak);
    compat::println("  [PASS] LineBreaker identified {} breakpoints (hard break confirmed)", breaks.size());

    // -------------------------------------------------------------------------
    // 5. TextLayoutEngine & TextLayout 多行排版與度量測試
    // -------------------------------------------------------------------------
    compat::println("[Test 5] TextLayoutEngine Layout Creation & Multiline Wrapping...");
    TextLayoutEngine& engine = TextLayoutEngine::Get();
    compat::string_view longText = "CPPUI is a modern C++ cross-platform UI framework designed with high performance and zero memory fragmentation in mind. 它具備工業級文字排版能力。";

    // 限制寬度 200px 自動折行
    TextLayout wrapLayout = engine.CreateLayout(longText, defFont, 200.0f, 1000.0f, TextAlignment::Left, TextWrapping::WordWrap, TextTrimming::None);
    assert(wrapLayout.GetLineCount() > 1);
    assert(wrapLayout.GetSize().width <= 205.0f);
    assert(wrapLayout.GetSize().height > 0.0f);
    compat::println("  [PASS] Wrapped layout: {} lines, size=({:.1f}, {:.1f})",
                    wrapLayout.GetLineCount(), wrapLayout.GetSize().width, wrapLayout.GetSize().height);

    // 省略號修剪
    TextLayout trimLayout = engine.CreateLayout(longText, defFont, 100.0f, 30.0f, TextAlignment::Left, TextWrapping::NoWrap, TextTrimming::CharacterEllipsis);
    assert(trimLayout.GetSize().width > 0.0f);
    compat::println("  [PASS] Trimming layout created successfully");

    // -------------------------------------------------------------------------
    // 6. Hit-Testing 雙向命中測試
    // -------------------------------------------------------------------------
    compat::println("[Test 6] Hit-Testing (Point -> Position & Position -> Rect)...");
    TextHitTestResult hit = wrapLayout.HitTestPoint(core::Point(10.0f, 10.0f));
    assert(hit.cursorRect.height > 0.0f);
    assert(hit.cursorRect.width > 0.0f);

    core::Rect charRect = wrapLayout.HitTestTextPosition(0);
    assert(charRect.height > 0.0f);
    assert(charRect.width > 0.0f);
    compat::println("  [PASS] HitTestPoint(10,10): pos={}, isInside={}, cursorRect=({:.1f},{:.1f},{:.1f},{:.1f})",
                    hit.textPosition, hit.isInside, hit.cursorRect.x, hit.cursorRect.y, hit.cursorRect.width, hit.cursorRect.height);
    compat::println("  [PASS] HitTestTextPosition(0): rect=({:.1f},{:.1f},{:.1f},{:.1f})",
                    charRect.x, charRect.y, charRect.width, charRect.height);

    // -------------------------------------------------------------------------
    // 7. MeasureText 效能壓測 (10,000 次查詢)
    // -------------------------------------------------------------------------
    compat::println("[Test 7] MeasureText Performance Benchmark...");
    const int BENCH_ITERATIONS = 10000;
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < BENCH_ITERATIONS; ++i) {
        core::Size sz = engine.MeasureText("Performance Measurement String 測試", defFont, 300.0f);
        (void)sz;
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double totalMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    compat::println("  [PASS] 10,000 MeasureText operations executed in {:.2f} ms ({:.3f} us/op)",
                    totalMs, (totalMs * 1000.0) / BENCH_ITERATIONS);

    // -------------------------------------------------------------------------
    // 8. DisplayList 渲染整合測試
    // -------------------------------------------------------------------------
    compat::println("[Test 8] DisplayList Text Recording & Playback...");
    rendering::DisplayList dl;
    rendering::DrawingContext dc(dl);

    dc.DrawRectangle(core::Rect(0.0f, 0.0f, 400.0f, 300.0f), core::Colors::DarkSlateGray);
    dc.DrawText("CPPUI Rendering Text", core::Point(20.0f, 20.0f), defFont, core::Colors::White);
    dc.DrawTextLayout(wrapLayout, core::Point(20.0f, 60.0f), core::Colors::Yellow);

    assert(dl.GetOpCount() >= 3);

    rendering::SoftwareRenderTarget target;
    target.Resize(400, 300);
    rendering::DisplayListRenderer renderer;
    rendering::RenderStats stats = renderer.Render(dl, &target);
    assert(stats.renderedOps >= 3);

    // 驗證 DrawTextLayout 實際光柵化出非背景像素 (黃色文字 R, G 較高)
    compat::span<const uint32_t> pixels = target.GetPixelSpan();
    size_t yellowTextPixels = 0;
    for (size_t y = 60; y < 200; ++y) {
        for (size_t x = 20; x < 250; ++x) {
            uint32_t p = pixels[y * 400 + x];
            uint8_t r = static_cast<uint8_t>((p >> 16) & 0xFF);
            uint8_t b = static_cast<uint8_t>(p & 0xFF);
            // 黃色文字混色後 R 會明顯高於背景且高於 B
            if (r > 150 && b < 100) {
                yellowTextPixels++;
            }
        }
    }
    assert(yellowTextPixels > 0);
    compat::println("  [PASS] DisplayList rendered {} ops successfully to target ({} text pixels rasterized)",
                    stats.renderedOps, yellowTextPixels);

    compat::println("=================================================");
    compat::println(" [CPPUI M3] All Text Pipeline Tests PASSED!     ");
    compat::println("=================================================");
    return 0;
}
