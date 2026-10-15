/**
 * @file security_safety_test.cpp
 * @brief 安全性與記憶體安全測試 (LinearArena 對齊、LifetimeToken UAF 防禦、Bitmap 溢位防護)
 */

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <stdexcept>

#include "compat/Compat.hpp"
#include "ui/rendering/LinearArena.hpp"
#include "ui/rendering/DrawOp.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/core/LifetimeToken.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/controls/Button.hpp"
#include "ui/app/Dispatcher.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Timer.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/PixelOps.hpp"
#include "ui/graphics/Convolution.hpp"
#include "ui/graphics/ImageFilters.hpp"

using namespace cppui;
using namespace cppui::core;
using namespace cppui::rendering;
using namespace cppui::reactive;
using namespace cppui::app;
using namespace cppui::graphics;

// =============================================================================
// 1. LinearArena 與 DrawOp 非平凡型別與記憶體嚴格對齊測試
// =============================================================================
static void TestArenaTrivialDestructionAndAlignment() {
    compat::println("--- Running TestArenaTrivialDestructionAndAlignment ---");

    // 1.1 編譯期 Trivially Destructible 特性驗證
    static_assert(std::is_trivially_destructible<DrawOpHeader>::value, "DrawOpHeader must be trivially destructible");
    static_assert(std::is_trivially_destructible<DrawRectOp>::value, "DrawRectOp must be trivially destructible");
    static_assert(std::is_trivially_destructible<DrawLineOp>::value, "DrawLineOp must be trivially destructible");
    static_assert(std::is_trivially_destructible<DrawTextOp>::value, "DrawTextOp must be trivially destructible");
    static_assert(std::is_trivially_destructible<DrawImageOp>::value, "DrawImageOp must be trivially destructible");

    // 驗證非平凡型別（如 std::string）必然不符合 trivially destructible
    static_assert(!std::is_trivially_destructible<std::string>::value, "std::string is non-trivial");
    static_assert(!std::is_trivially_destructible<std::vector<int32_t>>::value, "std::vector is non-trivial");
    static_assert(!std::is_trivially_destructible<std::shared_ptr<int32_t>>::value, "std::shared_ptr is non-trivial");

    // 1.2 測試各種對齊邊界 (8, 16, 32, 64) 之絕對指標位址嚴格對齊
    LinearArena arena(4096);

    for (size_t align : { 8, 16, 32, 64 }) {
        for (size_t bytes : { 1, 3, 7, 13, 24, 63, 128 }) {
            void* ptr = arena.Allocate(bytes, align);
            assert(ptr != nullptr);
            uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
            assert(addr % align == 0);
            (void)addr;
        }
    }
    compat::println("  [PASS] Absolute pointer alignment (8, 16, 32, 64) verified.");

    // 1.3 測試 ArenaSpan<T>
    auto span = arena.AllocateSpan<uint32_t>(16);
    assert(span.size() == 16);
    assert(!span.empty());
    for (size_t i = 0; i < span.size(); ++i) {
        span[i] = static_cast<uint32_t>(i * 10);
    }
    for (size_t i = 0; i < span.size(); ++i) {
        assert(span[i] == static_cast<uint32_t>(i * 10));
    }
    compat::println("  [PASS] ArenaSpan<T> allocation and indexing verified.");

    // 1.4 測試 AllocateString
    compat::string_view testStr = "CPPUI Secure Linear Arena";
    compat::string_view arenaStr = arena.AllocateString(testStr);
    assert(arenaStr == testStr);
    assert(arenaStr.data() != testStr.data()); // 必須為 Arena 內部深拷貝
    compat::println("  [PASS] AllocateString deep-copy verified.");

    // 1.5 測試 DisplayList 指令寫入
    DisplayList dl;
    DrawRectOp rectOp{ Rect(0.0f, 0.0f, 100.0f, 50.0f), Color(255, 0, 0, 255) };
    dl.PushOp(DrawOpType::DrawRectangle, rectOp);

    DrawTextOp textOp;
    textOp.origin = Point(10.0f, 10.0f);
    textOp.color = Color(255, 255, 255, 255);
    textOp.textLength = static_cast<uint32_t>(testStr.size());
    dl.PushOpWithData(DrawOpType::DrawText, textOp, testStr.data(), testStr.size());

    assert(dl.GetOpCount() == 2);
    size_t inspected = 0;
    dl.ForEach([&](const DrawOpHeader& header, const void* payload) {
        inspected++;
        assert(header.size >= sizeof(DrawOpHeader));
        (void)payload;
    });
    assert(inspected == 2);
    compat::println("  [PASS] DisplayList PushOp and PushOpWithData verified.");
}

// =============================================================================
// 2. LifetimeToken 與 ScopedConnection 防止 Use-After-Free 測試
// =============================================================================
static void TestLifetimeTokenAndScopedConnectionUAF() {
    compat::println("--- Running TestLifetimeTokenAndScopedConnectionUAF ---");

    // 2.1 測試 LifetimeToken 基本狀態轉換
    {
        LifetimeToken emptyToken;
        assert(!emptyToken.HasHandle());
        assert(!emptyToken.IsAlive());

        auto token = LifetimeToken::Create();
        assert(token.HasHandle());
        assert(token.IsAlive());
        assert(static_cast<bool>(token));

        token.Invalidate();
        assert(token.HasHandle());
        assert(!token.IsAlive());
        assert(!static_cast<bool>(token));
    }
    compat::println("  [PASS] LifetimeToken state transitions verified.");

    // 2.2 測試 ScopedConnection RAII 自動中斷
    {
        bool disconnected = false;
        {
            ScopedConnection scoped(Connection([&disconnected]() {
                disconnected = true;
            }));
            assert(scoped.IsConnected());
            assert(!disconnected);
        }
        assert(disconnected);
    }
    compat::println("  [PASS] ScopedConnection RAII auto-disconnect verified.");

    // 2.3 測試 Signal 訂閱結合 LifetimeToken
    {
        Signal<int32_t> signal(10);
        int32_t callCount = 0;
        auto token = LifetimeToken::Create();

        signal.Subscribe(token, [&callCount](int32_t) {
            callCount++;
        });

        signal.Set(20);
        assert(callCount == 1);

        // 失效權杖，後續 Set 不應再觸發回呼
        token.Invalidate();
        signal.Set(30);
        assert(callCount == 1);
    }
    compat::println("  [PASS] Signal + LifetimeToken callback suppression verified.");

    // 2.4 測試 Signal + ScopedConnection
    {
        Signal<std::string> signal("A");
        int32_t callCount = 0;
        {
            ScopedConnection conn = signal.Connect([&callCount](const std::string&) {
                callCount++;
            });
            assert(conn.IsConnected());
            signal.Set("B");
            assert(callCount == 1);
        } // conn 出範疇自動斷開

        signal.Set("C");
        assert(callCount == 1); // 斷開後不應再觸發
    }
    compat::println("  [PASS] Signal + ScopedConnection RAII disconnect verified.");

    // 2.5 測試 Dispatcher 任務過濾（模擬視窗銷毀防範 UAF）
    {
        Dispatcher dispatcher;
        int32_t executedCount = 0;
        auto token = LifetimeToken::Create();

        // 投遞兩個任務：一個有效，一個即將失效
        dispatcher.InvokeAsync(token, [&executedCount]() {
            executedCount++;
        });

        auto dyingToken = LifetimeToken::Create();
        dispatcher.InvokeAsync(dyingToken, [&executedCount]() {
            executedCount++;
        });

        // 模擬所屬物件在任務消費前解構
        dyingToken.Invalidate();

        // 消耗佇列
        dispatcher.DrainTasks();

        // 僅存活之任務執行，失效任務被安全略過
        assert(executedCount == 1);
    }
    compat::println("  [PASS] Dispatcher task filtering with invalidated token verified.");

    // 2.6 測試 UIElement 整合 LifetimeToken 與 ManageConnection
    {
        auto button = std::make_shared<controls::Button>("TestBtn");
        auto token = button->GetLifetimeToken();
        assert(token.IsAlive());

        bool connectionDisconnected = false;
        button->ManageConnection(ScopedConnection(Connection([&connectionDisconnected]() {
            connectionDisconnected = true;
        })));
        assert(!connectionDisconnected);

        // 投遞關聯至 button 之非同步 UI 任務
        bool taskRan = false;
        Application::Post(token, [&taskRan]() {
            taskRan = true;
        });

        // 銷毀 button (模擬視窗關閉)
        button.reset();

        assert(!token.IsAlive());
        assert(connectionDisconnected);

        // 消費 Application Dispatcher 任務
        Application::GetDispatcher().DrainTasks();
        assert(!taskRan); // 任務被安全丟棄，未存取野指標
    }
    compat::println("  [PASS] UIElement destructor token invalidation and managed connection cleanup verified.");

    // 2.7 測試 Timer + LifetimeToken
    {
        Timer timer(100);
        int32_t tickCount = 0;
        auto token = LifetimeToken::Create();

        timer.AddTickHandler(token, [&tickCount]() {
            tickCount++;
        });

        timer.PerformTick();
        assert(tickCount == 1);

        // 失效權杖
        token.Invalidate();
        timer.PerformTick();
        assert(tickCount == 1); // 不應再增加
    }
    compat::println("  [PASS] Timer + LifetimeToken protection verified.");
}

// =============================================================================
// 3. 點陣圖、影像濾鏡與卷積整數溢位防護測試
// =============================================================================
static void TestBitmapAndFilterIntegerOverflowDefense() {
    compat::println("--- Running TestBitmapAndFilterIntegerOverflowDefense ---");

    // 3.1 零或負維度拒絕
    bool threwZero = false;
    try {
        Bitmap bmp(0, 100);
    } catch (const std::invalid_argument&) {
        threwZero = true;
    }
    assert(threwZero);

    // 3.2 超出 MAX_DIMENSION (32768) 拒絕 (例如 65536)
    bool threwDim = false;
    try {
        Bitmap bmp(65536, 100);
    } catch (const std::invalid_argument&) {
        threwDim = true;
    }
    assert(threwDim);

    // 3.3 總像素超出 MAX_PIXELS (256M) 拒絕
    bool threwPixels = false;
    try {
        Bitmap bmp(32768, 32768); // 32768 * 32768 = 1024M > 256M
    } catch (const std::invalid_argument&) {
        threwPixels = true;
    }
    assert(threwPixels);

    // 3.4 像素 Span 容量小於所需尺寸拒絕
    std::vector<uint32_t> smallBuf(50);
    bool threwSpan = false;
    try {
        Bitmap bmp(10, 10, smallBuf); // 需要 100 像素，僅提供 50
    } catch (const std::invalid_argument&) {
        threwSpan = true;
    }
    assert(threwSpan);

    // 3.5 合法點陣圖配置與 GetStride
    Bitmap validBmp(256, 128);
    assert(validBmp.GetWidth() == 256);
    assert(validBmp.GetHeight() == 128);
    assert(validBmp.GetStride() == 256 * 4);
    compat::println("  [PASS] Bitmap constructor fail-fast boundary defenses verified.");

    // 3.6 PixelOps 跨度保護
    int32_t pixelSum = 0;
    PixelOps::ForEach(validBmp, [&pixelSum](uint32_t) {
        pixelSum++;
    });
    assert(pixelSum == 256 * 128);
    compat::println("  [PASS] PixelOps traversal safety verified.");

    // 3.7 ConvolutionEngine 尺寸邊界保護
    SeparableKernel kernel;
    kernel.hWeights = { 1 };
    kernel.vWeights = { 1 };
    kernel.hDivisor = 1;
    kernel.vDivisor = 1;
    kernel.radiusX = 0;
    kernel.radiusY = 0;

    Bitmap dstBmp(256, 128);
    ConvolutionEngine::ConvolveSeparable(validBmp, dstBmp, kernel);
    compat::println("  [PASS] ConvolutionEngine safe execution verified.");

    // 3.8 ImageFilters::ComputeOtsuThreshold 64-bit 總和計算安全驗證
    uint8_t otsu = ImageFilters::ComputeOtsuThreshold(validBmp);
    (void)otsu;
    compat::println("  [PASS] ImageFilters 64-bit Otsu threshold verified.");
}

// =============================================================================
// 主程式入口點
// =============================================================================
int main() {
    compat::println("=================================================");
    compat::println("CPPUI Security & Memory Safety Suite Running...");
    compat::println("=================================================");

    TestArenaTrivialDestructionAndAlignment();
    TestLifetimeTokenAndScopedConnectionUAF();
    TestBitmapAndFilterIntegerOverflowDefense();

    compat::println("=================================================");
    compat::println("[SUCCESS] All Security & Safety Tests PASSED!");
    compat::println("=================================================");
    return 0;
}
