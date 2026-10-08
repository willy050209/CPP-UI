#include "compat/compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/app/AppBuilder.hpp"
#include <cassert>
#include <vector>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

using namespace cppui;
using namespace cppui::app;

// 測試用 Form 衍生類別
class LifecycleTestForm : public Form {
public:
    LifecycleTestForm() {
        SetTitle("Lifecycle Test Form");
        SetFormSize(640, 480);
    }

    void OnLoad() override {
        Form::OnLoad();
        events.push_back("Load");
    }

    void OnShown() override {
        Form::OnShown();
        events.push_back("Shown");
    }

    void OnFormClosing(FormClosingEventArgs& e) override {
        Form::OnFormClosing(e);
        events.push_back("Closing");
        if (cancelClose) {
            e.Cancel = true;
        }
    }

    void OnFormClosed(FormClosedEventArgs& e) override {
        Form::OnFormClosed(e);
        events.push_back("Closed");
    }

    std::vector<std::string> events;
    bool cancelClose = false;
};

void TestFormPropertiesAndDefaults() {
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

void TestFormLifecycle() {
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

void TestFormModalDialogResult() {
    compat::println("[Test] 測試 Form 模態對話框與 DialogResult...");
    auto form = std::make_shared<Form>();

    // 模擬在模態中設定 DialogResult 並關閉
    form->SetDialogResult(DialogResult::OK);
    assert(form->GetDialogResult() == DialogResult::OK);

    compat::println("  ✔ Form DialogResult 驗證通過！");
}

void TestApplicationManagement() {
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

void TestAppBuilderGenericHost() {
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

void TestApplicationExitAndIdleHandlers() {
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

void TestFormDelegatesAndCancel() {
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

class AutoCloseForm : public Form {
public:
    AutoCloseForm() {
        SetTitle("Auto Close Form");
    }

    void OnShown() override {
        Form::OnShown();
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

void TestWindowClosePlatformEventHandling() {
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

void TestApplicationRunCleanExit() {
    compat::println("[Test] 測試 Application::Run 於主視窗關閉時乾淨退出訊息迴圈...");
    Application::SetShutdownMode(ShutdownMode::OnMainWindowClose);
    auto form = std::make_shared<AutoCloseForm>();
    int exitCode = Application::Run(form);
    assert(exitCode == 0);
    assert(Application::IsRunning() == false);
    assert(form->IsDisposed() == true);
    compat::println("  ✔ Application::Run 主視窗關閉退出迴圈驗證通過！");
}

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

    compat::println("==================================================");
    compat::println(" 全部 9 項 Application & Form 單元測試通過！");
    compat::println("==================================================");
    return 0;
}
