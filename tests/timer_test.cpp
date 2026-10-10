/**
 * @file timer_test.cpp
 * @brief Timer 計時器元件 (對標 System.Windows.Forms.Timer) 完整功能與契約驗證
 */

#include <cassert>
#include <cstdint>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <stdexcept>

#include "compat/compat.hpp"
#include "ui/app/Timer.hpp"
#include "ui/controls/Timer.hpp"
#include "ui/app/Application.hpp"

#define TEST_ASSERT(cond) do { \
    if (!(cond)) { \
        compat::println("[FAIL] Assertion failed: " #cond " at line ", __LINE__); \
        assert(cond); \
    } \
} while(0)

namespace {

void Test1_DefaultConstructorAndInitialState() {
    compat::println("--- Running Test1_DefaultConstructorAndInitialState ---");
    cppui::app::Timer timer;
    TEST_ASSERT(timer.GetInterval() == 100);
    TEST_ASSERT(!timer.IsEnabled());
    TEST_ASSERT(timer.GetTag() == nullptr);
    TEST_ASSERT(timer.GetTickHandlerCount() == 0);
    compat::println("[PASS] Test1 passed.");
}

void Test2_CustomIntervalAndProperties() {
    compat::println("--- Running Test2_CustomIntervalAndProperties ---");
    cppui::app::Timer timer(250);
    TEST_ASSERT(timer.GetInterval() == 250);
    TEST_ASSERT(!timer.IsEnabled());

    timer.SetInterval(500);
    TEST_ASSERT(timer.GetInterval() == 500);

    uintptr_t dummyTag = 0xABCD;
    timer.SetTag(reinterpret_cast<void*>(dummyTag));
    TEST_ASSERT(timer.GetTag() == reinterpret_cast<void*>(dummyTag));

    timer.SetTag(nullptr);
    TEST_ASSERT(timer.GetTag() == nullptr);
    compat::println("[PASS] Test2 passed.");
}

void Test3_FailFastParameterValidation() {
    compat::println("--- Running Test3_FailFastParameterValidation ---");
    // 驗證非法間隔時間 (<= 0)
    bool threwZeroConstructor = false;
    try {
        cppui::app::Timer t(0);
    } catch (const std::invalid_argument&) {
        threwZeroConstructor = true;
    }
    TEST_ASSERT(threwZeroConstructor);

    bool threwNegativeConstructor = false;
    try {
        cppui::app::Timer t(-50);
    } catch (const std::invalid_argument&) {
        threwNegativeConstructor = true;
    }
    TEST_ASSERT(threwNegativeConstructor);

    cppui::app::Timer timer(100);
    bool threwSetIntervalZero = false;
    try {
        timer.SetInterval(0);
    } catch (const std::invalid_argument&) {
        threwSetIntervalZero = true;
    }
    TEST_ASSERT(threwSetIntervalZero);

    bool threwSetIntervalNegative = false;
    try {
        timer.SetInterval(-10);
    } catch (const std::invalid_argument&) {
        threwSetIntervalNegative = true;
    }
    TEST_ASSERT(threwSetIntervalNegative);

    // 驗證空回呼函式
    bool threwEmptyOnTick0 = false;
    try {
        timer.OnTick(cppui::app::Timer::TickHandler{});
    } catch (const std::invalid_argument&) {
        threwEmptyOnTick0 = true;
    }
    TEST_ASSERT(threwEmptyOnTick0);

    bool threwEmptyOnTick1 = false;
    try {
        timer.OnTick(cppui::app::Timer::TickWithSenderHandler{});
    } catch (const std::invalid_argument&) {
        threwEmptyOnTick1 = true;
    }
    TEST_ASSERT(threwEmptyOnTick1);

    bool threwEmptyAddHandler0 = false;
    try {
        timer.AddTickHandler(cppui::app::Timer::TickHandler{});
    } catch (const std::invalid_argument&) {
        threwEmptyAddHandler0 = true;
    }
    TEST_ASSERT(threwEmptyAddHandler0);

    bool threwEmptyAddHandler1 = false;
    try {
        timer.AddTickHandler(cppui::app::Timer::TickWithSenderHandler{});
    } catch (const std::invalid_argument&) {
        threwEmptyAddHandler1 = true;
    }
    TEST_ASSERT(threwEmptyAddHandler1);

    compat::println("[PASS] Test3 passed.");
}

void Test4_StartStopRestartStateTransitions() {
    compat::println("--- Running Test4_StartStopRestartStateTransitions ---");
    cppui::app::Timer timer;
    TEST_ASSERT(!timer.IsEnabled());

    timer.Start();
    TEST_ASSERT(timer.IsEnabled());

    timer.Stop();
    TEST_ASSERT(!timer.IsEnabled());

    timer.SetEnabled(true);
    TEST_ASSERT(timer.IsEnabled());

    timer.SetEnabled(false);
    TEST_ASSERT(!timer.IsEnabled());

    timer.Restart();
    TEST_ASSERT(timer.IsEnabled());

    timer.Stop();
    TEST_ASSERT(!timer.IsEnabled());
    compat::println("[PASS] Test4 passed.");
}

void Test5_ProgrammaticPerformTick() {
    compat::println("--- Running Test5_ProgrammaticPerformTick ---");
    cppui::app::Timer timer(100);
    int32_t count1 = 0;
    int32_t count2 = 0;
    cppui::app::Timer* senderReceived = nullptr;

    timer.OnTick([&count1]() {
        count1++;
    });

    timer.OnTick([&count2, &senderReceived](cppui::app::Timer& sender) {
        count2++;
        senderReceived = &sender;
    });

    TEST_ASSERT(timer.GetTickHandlerCount() == 2);
    TEST_ASSERT(count1 == 0 && count2 == 0);

    timer.PerformTick();
    TEST_ASSERT(count1 == 1);
    TEST_ASSERT(count2 == 1);
    TEST_ASSERT(senderReceived == &timer);

    timer.PerformTick();
    TEST_ASSERT(count1 == 2);
    TEST_ASSERT(count2 == 2);
    compat::println("[PASS] Test5 passed.");
}

void Test6_MultipleHandlersAndClearing() {
    compat::println("--- Running Test6_MultipleHandlersAndClearing ---");
    cppui::app::Timer timer;
    TEST_ASSERT(timer.GetTickHandlerCount() == 0);

    timer.AddTickHandler([]() {});
    timer.AddTickHandler([]() {});
    timer.AddTickHandler([](cppui::app::Timer&) {});
    TEST_ASSERT(timer.GetTickHandlerCount() == 3);

    timer.ClearTickHandlers();
    TEST_ASSERT(timer.GetTickHandlerCount() == 0);
    compat::println("[PASS] Test6 passed.");
}

void Test7_MoveSemantics() {
    compat::println("--- Running Test7_MoveSemantics ---");
    cppui::app::Timer t1(350);
    t1.Start();
    TEST_ASSERT(t1.GetInterval() == 350);
    TEST_ASSERT(t1.IsEnabled());

    cppui::app::Timer t2 = std::move(t1);
    TEST_ASSERT(t2.GetInterval() == 350);
    TEST_ASSERT(t2.IsEnabled());

    cppui::app::Timer t3;
    t3 = std::move(t2);
    TEST_ASSERT(t3.GetInterval() == 350);
    TEST_ASSERT(t3.IsEnabled());

    t3.Stop();
    TEST_ASSERT(!t3.IsEnabled());
    compat::println("[PASS] Test7 passed.");
}

void Test8_RealTimeIntervalAndMessagePump() {
    compat::println("--- Running Test8_RealTimeIntervalAndMessagePump ---");
    cppui::app::Timer timer(20);
    std::atomic<int32_t> tickCount{ 0 };

    timer.OnTick([&tickCount]() {
        tickCount++;
    });

    timer.Start();

    // 等待計時器到期並透過 DoEvents 派發
    for (int32_t i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cppui::app::Application::DoEvents();
        if (tickCount.load() >= 1) {
            break;
        }
    }

    int32_t capturedTicks = tickCount.load();
    TEST_ASSERT(capturedTicks >= 1);

    // 停止計時器後確認不會再觸發
    timer.Stop();
    for (int32_t i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cppui::app::Application::DoEvents();
    }

    TEST_ASSERT(tickCount.load() == capturedTicks);
    compat::println("[PASS] Test8 passed.");
}

void Test9_UIThreadAffinity() {
    compat::println("--- Running Test9_UIThreadAffinity ---");
    cppui::app::Timer timer(15);
    bool checkAccessResult = false;
    bool handlerRan = false;

    timer.OnTick([&checkAccessResult, &handlerRan]() {
        handlerRan = true;
        checkAccessResult = cppui::app::Application::CheckAccess();
    });

    timer.Start();
    for (int32_t i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cppui::app::Application::DoEvents();
        if (handlerRan) {
            break;
        }
    }
    timer.Stop();

    TEST_ASSERT(handlerRan);
    TEST_ASSERT(checkAccessResult);
    compat::println("[PASS] Test9 passed.");
}

void Test10_ControlsNamespaceAliasAndRAIIDestruction() {
    compat::println("--- Running Test10_ControlsNamespaceAliasAndRAIIDestruction ---");
    // 驗證 cppui::controls::Timer 命名空間別名正常運作
    {
        cppui::controls::Timer ctrlTimer(20);
        ctrlTimer.Start();
        TEST_ASSERT(ctrlTimer.IsEnabled());
        // 離開區域作用域自動解構
    }

    // 解構後等待一段時間，確認背景排程器無任何違規存取或記憶體崩潰
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    cppui::app::Application::DoEvents();

    compat::println("[PASS] Test10 passed.");
}

} // anonymous namespace

int main() {
    compat::println("====================================================");
    compat::println("       CPPUI Timer Unit & Integration Test Suite    ");
    compat::println("====================================================");

    Test1_DefaultConstructorAndInitialState();
    Test2_CustomIntervalAndProperties();
    Test3_FailFastParameterValidation();
    Test4_StartStopRestartStateTransitions();
    Test5_ProgrammaticPerformTick();
    Test6_MultipleHandlersAndClearing();
    Test7_MoveSemantics();
    Test8_RealTimeIntervalAndMessagePump();
    Test9_UIThreadAffinity();
    Test10_ControlsNamespaceAliasAndRAIIDestruction();

    compat::println("====================================================");
    compat::println("   ALL 10 TIMER UNIT TESTS PASSED SUCCESSFULLY!     ");
    compat::println("====================================================");
    return 0;
}
