/**
 * @file app_form_test.cpp
 * @brief Application & Form 生命週期、事件迴圈、後端切換與契約例外單元測試
 */

#include "compat/Compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/app/AppBuilder.hpp"
#include "ui/core/Exceptions.hpp"
#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <stdexcept>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace {

// 測試用 Form 衍生類別
class LifecycleTestForm : public cppui::app::Form {
public:
    LifecycleTestForm() {
        SetTitle("Lifecycle Test Form");
        SetFormSize(640, 480);
    }

    void OnLoad() override {
        cppui::app::Form::OnLoad();
        events.push_back("Load");
    }

    void OnShown() override {
        cppui::app::Form::OnShown();
        events.push_back("Shown");
    }

    void OnFormClosing(cppui::app::FormClosingEventArgs& e) override {
        cppui::app::Form::OnFormClosing(e);
        events.push_back("Closing");
        if (cancelClose) {
            e.Cancel = true;
        }
    }

    void OnFormClosed(cppui::app::FormClosedEventArgs& e) override {
        cppui::app::Form::OnFormClosed(e);
        events.push_back("Closed");
    }

    std::vector<std::string> events;
    bool cancelClose = false;
};

class AutoCloseForm : public cppui::app::Form {
public:
    AutoCloseForm() {
        SetTitle("Auto Close Form");
    }

    void OnShown() override {
        cppui::app::Form::OnShown();
#if defined(_WIN32)
        if (GetPlatformWindow() && GetPlatformWindow()->GetNativeHandle()) {
            ::PostMessageW(static_cast<HWND>(GetPlatformWindow()->GetNativeHandle()), WM_CLOSE, 0, 0);
        } else {
            Close();
        }
#else
        Close();
#endif
    }
};

static void TestFormPropertiesAndDefaults() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Form 基本屬性與預設值...");
    auto form = std::make_shared<Form>();

    assert(form->GetTitle() == "CPPUI Form");
    assert(form->GetFormSize().width == 800.0f);
    assert(form->GetFormSize().height == 600.0f);
    assert(form->GetStartPosition() == FormStartPosition::CenterScreen);
    assert(form->GetWindowState() == WindowState::Normal);
    assert(form->GetDialogResult() == DialogResult::None);

    form->SetTitle("New Title");
    assert(form->GetTitle() == "New Title");

    form->SetFormSize(1024, 768);
    assert(form->GetFormSize().width == 1024.0f);
    assert(form->GetFormSize().height == 768.0f);

    form->SetStartPosition(FormStartPosition::Manual);
    assert(form->GetStartPosition() == FormStartPosition::Manual);

    form->SetWindowState(WindowState::Maximized);
    assert(form->GetWindowState() == WindowState::Maximized);

    compat::println("  ✔ Form 屬性讀寫驗證通過！");
}

static void TestFormLifecycle() {
    compat::println("[Test] 測試 Form 生命週期回呼與取消關閉機制...");
    auto form = std::make_shared<LifecycleTestForm>();

    form->Show();
    assert(form->events.size() == 2);
    assert(form->events[0] == "Load");
    assert(form->events[1] == "Shown");

    // 測試取消關閉
    form->cancelClose = true;
    form->Close();

    assert(form->events.size() == 3);
    assert(form->events[2] == "Closing");
    assert(form->IsDisposed() == false); // 因被取消，視窗不應關閉

    // 測試正常關閉
    form->cancelClose = false;
    form->Close();

    assert(form->events.size() == 5);
    assert(form->events[3] == "Closing");
    assert(form->events[4] == "Closed");
    assert(form->IsDisposed() == true);

    compat::println("  ✔ Form 生命週期與 FormClosing 取消機制驗證通過！");
}

static void TestFormModalDialogResult() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Form 模態對話框與 DialogResult...");
    auto form = std::make_shared<Form>();

    // 模擬在模態中設定 DialogResult 並關閉
    form->SetDialogResult(DialogResult::OK);
    assert(form->GetDialogResult() == DialogResult::OK);

    compat::println("  ✔ Form DialogResult 驗證通過！");
}

static void TestApplicationManagement() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Application 全域中樞與表單登記...");

    assert(Application::GetShutdownMode() == ShutdownMode::OnMainWindowClose);
    Application::SetShutdownMode(ShutdownMode::OnLastWindowClose);
    assert(Application::GetShutdownMode() == ShutdownMode::OnLastWindowClose);

    Application::SetProductName("CPPUI App");
    Application::SetCompanyName("Antigravity Inc.");
    assert(Application::GetProductName() == "CPPUI App");
    assert(Application::GetCompanyName() == "Antigravity Inc.");

    auto form1 = std::make_shared<Form>();
    form1->SetTitle("Form 1");
    auto form2 = std::make_shared<Form>();
    form2->SetTitle("Form 2");

    form1->Show();
    form2->Show();

    auto openForms = Application::GetOpenForms();
    assert(openForms.size() == 2);

    form1->Close();
    openForms = Application::GetOpenForms();
    assert(openForms.size() == 1);

    form2->Close();
    openForms = Application::GetOpenForms();
    assert(openForms.empty());

    // 測試 DoEvents 不崩潰且可正常處理佇列
    Application::DoEvents();

    compat::println("  ✔ Application 全域表單登記與中樞管理驗證通過！");
}

static void TestAppBuilderGenericHost() {
    using namespace cppui::app;
    compat::println("[Test] 測試 AppBuilder 與 Generic Host 啟動流...");
    auto builder = Application::CreateBuilder();

    builder.Services().AddTransient<LifecycleTestForm>();
    auto host = builder.Build();

    assert(host.Services() != nullptr);
    auto form = host.Services()->GetService<LifecycleTestForm>();
    assert(form != nullptr);
    assert(form->GetTitle() == "Lifecycle Test Form");

    compat::println("  ✔ AppBuilder 與 Host 服務解析驗證通過！");
}

static void TestApplicationExitAndIdleHandlers() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Application 全域 Exit 與 Idle 監聽回呼...");
    static bool s_idleTriggered = false;
    static bool s_exitTriggered = false;
    s_idleTriggered = false;
    s_exitTriggered = false;

    Application::AddIdleHandler([]() {
        s_idleTriggered = true;
    });

    Application::AddExitHandler([]() {
        s_exitTriggered = true;
    });

    Application::DoEvents();
    assert(s_idleTriggered == true);

    Application::Exit();
    assert(s_exitTriggered == true);

    compat::println("  ✔ Application 全域 Exit / Idle 回呼調度驗證通過！");
}

static void TestFormDelegatesAndCancel() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Form 外部委派監聽 (AddClosingHandler / AddClosedHandler)...");
    auto form = std::make_shared<Form>();
    form->SetTitle("Delegate Form");

    bool cancelRequested = true;
    bool closingInvoked = false;
    bool closedInvoked = false;

    form->AddClosingHandler([&](Form&, FormClosingEventArgs& e) {
        closingInvoked = true;
        if (cancelRequested) {
            e.Cancel = true;
        }
    });

    form->AddClosedHandler([&](Form&, FormClosedEventArgs&) {
        closedInvoked = true;
    });

    form->Show();
    assert(form->IsDisposed() == false);

    // 第一次 Close，委派攔截並取消關閉
    form->Close();
    assert(closingInvoked == true);
    assert(closedInvoked == false);
    assert(form->IsDisposed() == false);

    // 第二次 Close，允許關閉
    cancelRequested = false;
    form->Close();
    assert(closedInvoked == true);
    assert(form->IsDisposed() == true);

    compat::println("  ✔ Form 外部委派監聽與取消關閉機制驗證通過！");
}

static void TestWindowClosePlatformEventHandling() {
    compat::println("[Test] 測試原生 WM_CLOSE / WindowClose 事件分發與取消機制...");
    auto form = std::make_shared<LifecycleTestForm>();
    form->Show();
    assert(form->IsDisposed() == false);

    // 1. 模擬原生發送關閉訊息，但表單設定取消
    form->cancelClose = true;
#if defined(_WIN32)
    if (form->GetPlatformWindow() && form->GetPlatformWindow()->GetNativeHandle()) {
        ::PostMessageW(static_cast<HWND>(form->GetPlatformWindow()->GetNativeHandle()), WM_CLOSE, 0, 0);
        form->GetPlatformWindow()->PollEvents();
    } else {
        form->Close();
    }
#else
    form->Close();
#endif
    assert(form->IsDisposed() == false); // 取消成功，不應被關閉

    // 2. 模擬原生發送關閉訊息，表單允許關閉
    form->cancelClose = false;
#if defined(_WIN32)
    if (form->GetPlatformWindow() && form->GetPlatformWindow()->GetNativeHandle()) {
        ::PostMessageW(static_cast<HWND>(form->GetPlatformWindow()->GetNativeHandle()), WM_CLOSE, 0, 0);
        form->GetPlatformWindow()->PollEvents();
    } else {
        form->Close();
    }
#else
    form->Close();
#endif
    assert(form->IsDisposed() == true); // 成功銷毀
    compat::println("  ✔ 原生關閉事件分發與取消機制驗證通過！");
}

static void TestApplicationRunCleanExit() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Application::Run 於主視窗關閉時乾淨退出訊息迴圈...");
    Application::SetShutdownMode(ShutdownMode::OnMainWindowClose);
    auto form = std::make_shared<AutoCloseForm>();
    int32_t exitCode = Application::Run(form);
    assert(exitCode == 0);
    assert(Application::IsRunning() == false);
    assert(form->IsDisposed() == true);
    compat::println("  ✔ Application::Run 主視窗關閉退出迴圈驗證通過！");
}

static void TestPlatformBackendSelection() {
    using namespace cppui::app;
    compat::println("[Test] 測試 PlatformBackend 選擇與 AppBuilder 鏈式配置...");
    
    // 預設為 Win32
    assert(Application::GetPlatformBackend() == PlatformBackend::Win32);

    Application::SetPlatformBackend(PlatformBackend::Skia);
    assert(Application::GetPlatformBackend() == PlatformBackend::Skia);

    Application::SetPlatformBackend(PlatformBackend::Win32);
    assert(Application::GetPlatformBackend() == PlatformBackend::Win32);

    // 測試 AppBuilder 鏈式設定
    auto builder1 = Application::CreateBuilder();
    builder1.UseSkia();
    auto host1 = builder1.Build();
    assert(Application::GetPlatformBackend() == PlatformBackend::Skia);

    auto builder2 = Application::CreateBuilder();
    builder2.UseWin32();
    auto host2 = builder2.Build();
    assert(Application::GetPlatformBackend() == PlatformBackend::Win32);

    auto builder3 = Application::CreateBuilder();
    builder3.UsePlatformBackend(PlatformBackend::Skia);
    auto host3 = builder3.Build();
    assert(Application::GetPlatformBackend() == PlatformBackend::Skia);

    // 重設回 Win32
    Application::SetPlatformBackend(PlatformBackend::Win32);
    compat::println("  ✔ PlatformBackend 選擇與 AppBuilder 配置驗證通過！");
}

static void TestDispatcherCrossThreadExecution() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Dispatcher 跨執行緒任務投遞與 UI 執行緒執行...");

    assert(Application::CheckAccess() == true);

    std::atomic<bool> workerCheckedAccess{false};
    std::atomic<bool> taskExecuted{false};
    std::thread::id taskExecutedThreadId;

    std::thread worker([&]() {
        // 背景執行緒呼叫 CheckAccess 應回傳 false
        workerCheckedAccess = Application::CheckAccess();

        // 背景執行緒向主執行緒派發任務
        Application::Post([&]() {
            taskExecuted = true;
            taskExecutedThreadId = std::this_thread::get_id();
        });
    });

    worker.join();

    assert(workerCheckedAccess == false);
    assert(taskExecuted == false); // 尚未在主執行緒 DrainTasks

    // 在主執行緒呼叫 DoEvents 處理非同步任務
    Application::DoEvents();

    assert(taskExecuted == true);
    assert(taskExecutedThreadId == std::this_thread::get_id());

    compat::println("  ✔ Dispatcher 跨執行緒非同步任務調度與主執行緒執行驗證通過！");
}

static void TestFailFastContracts() {
    using namespace cppui::app;
    compat::println("[Test] 測試 Application 契約防禦與例外驗證 (Fail-Fast Checks)...");

    // 1. Application::Run((Form*)nullptr) 應擲出 std::invalid_argument
    bool caughtRawNullRun = false;
    try {
        cppui::app::Form* nullForm = nullptr;
        Application::Run(nullForm);
    } catch (const std::invalid_argument&) {
        caughtRawNullRun = true;
    }
    assert(caughtRawNullRun);

    // 2. Application::Run(shared_ptr<Form>(nullptr)) 應擲出 std::invalid_argument
    bool caughtSharedNullRun = false;
    try {
        std::shared_ptr<cppui::app::Form> nullSharedForm;
        Application::Run(nullSharedForm);
    } catch (const std::invalid_argument&) {
        caughtSharedNullRun = true;
    }
    assert(caughtSharedNullRun);

    // 3. Application::Post(nullptr) 應擲出 std::invalid_argument
    bool caughtNullPost = false;
    try {
        std::function<void()> emptyAction;
        Application::Post(emptyAction);
    } catch (const std::invalid_argument&) {
        caughtNullPost = true;
    }
    assert(caughtNullPost);

    // 4. Application::SetServiceProvider(nullptr) 應擲出 std::invalid_argument
    bool caughtNullProvider = false;
    try {
        Application::SetServiceProvider(nullptr);
    } catch (const std::invalid_argument&) {
        caughtNullProvider = true;
    }
    assert(caughtNullProvider);

    // 5. Application::RegisterForm(nullptr) 應擲出 std::invalid_argument
    bool caughtNullRegister = false;
    try {
        Application::RegisterForm(nullptr);
    } catch (const std::invalid_argument&) {
        caughtNullRegister = true;
    }
    assert(caughtNullRegister);

    // 6. Application::UnregisterForm(nullptr) 應擲出 std::invalid_argument
    bool caughtNullUnregister = false;
    try {
        Application::UnregisterForm(nullptr);
    } catch (const std::invalid_argument&) {
        caughtNullUnregister = true;
    }
    assert(caughtNullUnregister);

    // 7. Application::GetRequiredService<T>() 當無服務提供者時應擲出 InvalidOperationException
    bool caughtInvalidOpService = false;
    try {
        auto unused = Application::GetRequiredService<LifecycleTestForm>();
        (void)unused;
    } catch (const cppui::core::InvalidOperationException&) {
        caughtInvalidOpService = true;
    }
    assert(caughtInvalidOpService);

    // 8. AppHost Run(nullptr) 應擲出 std::invalid_argument
    bool caughtHostNullRun = false;
    try {
        auto host = Application::CreateBuilder().Build();
        std::shared_ptr<cppui::app::Form> nullForm;
        host.Run(nullForm);
    } catch (const std::invalid_argument&) {
        caughtHostNullRun = true;
    }
    assert(caughtHostNullRun);

    compat::println("  ✔ Application & AppHost 契約防禦與例外驗證通過！");
}

} // anonymous namespace

int main() {
    compat::println("==================================================");
    compat::println(" CPPUI Application & Form TDD Unit Test Suite");
    compat::println("==================================================");

    TestFormPropertiesAndDefaults();
    TestFormLifecycle();
    TestFormModalDialogResult();
    TestApplicationManagement();
    TestAppBuilderGenericHost();
    TestApplicationExitAndIdleHandlers();
    TestFormDelegatesAndCancel();
    TestWindowClosePlatformEventHandling();
    TestApplicationRunCleanExit();
    TestPlatformBackendSelection();
    TestDispatcherCrossThreadExecution();
    TestFailFastContracts();

    compat::println("==================================================");
    compat::println(" 全部 12 項 Application & Form 單元測試通過！");
    compat::println("==================================================");
    return 0;
}
