/*
 * File: Application.cpp
 * Purpose: Implements the Application static hub managing application lifecycle,
 *          event message loops, active forms, dispatching tasks, and fail-fast validation.
 */

#include "ui/app/Application.hpp"
#include "ui/app/AppBuilder.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace cppui {
namespace app {

namespace {

bool s_isRunning = false;
bool s_exitRequested = false;
ShutdownMode s_shutdownMode = ShutdownMode::OnMainWindowClose;
std::vector<std::shared_ptr<Form>> s_openForms;
std::weak_ptr<Form> s_activeForm;
std::shared_ptr<Form> s_mainForm;

std::string s_productName = "CPPUI Application";
std::string s_companyName = "CPPUI";
std::string s_executablePath = "";
std::string s_startupPath = "";

std::vector<Application::ExitCallback> s_exitHandlers;
std::vector<Application::IdleCallback> s_idleHandlers;
std::shared_ptr<ioc::IServiceProvider> s_serviceProvider;

PlatformBackend s_platformBackend = PlatformBackend::Win32;
Dispatcher s_dispatcher;

static int32_t RunInternal(std::shared_ptr<Form> mainForm) {
    s_mainForm = mainForm;
    s_isRunning = true;
    s_exitRequested = false;
    s_dispatcher.SetMainThreadId(std::this_thread::get_id());

    if (mainForm) {
        mainForm->Show();
        Application::SetActiveForm(mainForm);
    }

    // 主訊息迴圈
    while (s_isRunning && !s_exitRequested) {
        // 先行處理並執行跨執行緒投遞的非同步任務
        s_dispatcher.DrainTasks();

        if (s_shutdownMode == ShutdownMode::OnMainWindowClose) {
            if (s_mainForm && s_mainForm->IsDisposed()) {
                break;
            }
        } else if (s_shutdownMode == ShutdownMode::OnLastWindowClose) {
            if (s_openForms.empty()) {
                break;
            }
        }

        auto currentForms = s_openForms;
        for (auto& f : currentForms) {
            if (f && !f->IsDisposed()) {
                if (auto win = f->GetPlatformWindow()) {
                    if (!win->IsClosed()) {
                        win->PollEvents();
                    }
                }
            }
        }

        s_dispatcher.DrainTasks();

        if (s_openForms.empty()) {
            break;
        }

        for (const auto& idleHandler : s_idleHandlers) {
            if (idleHandler) idleHandler();
        }

        if (s_mainForm && s_mainForm->GetPlatformWindow() && !s_mainForm->GetPlatformWindow()->IsClosed()) {
            s_mainForm->GetPlatformWindow()->WaitEvents(16);
        } else if (!s_openForms.empty()) {
            bool waited = false;
            for (auto& f : s_openForms) {
                if (f && !f->IsDisposed() && f->GetPlatformWindow() && !f->GetPlatformWindow()->IsClosed()) {
                    f->GetPlatformWindow()->WaitEvents(16);
                    waited = true;
                    break;
                }
            }
            if (!waited) {
                break;
            }
        } else {
            break;
        }
    }

    Application::Exit();
    return 0;
}

} // anonymous namespace

int32_t Application::Run() {
    return RunInternal(nullptr);
}

int32_t Application::Run(Form* mainForm) {
    if (!mainForm) {
        throw std::invalid_argument("Application::Run: mainForm cannot be null.");
    }
    std::shared_ptr<Form> sharedForm(mainForm);
    return RunInternal(sharedForm);
}

int32_t Application::Run(std::shared_ptr<Form> mainForm) {
    if (!mainForm) {
        throw std::invalid_argument("Application::Run: mainForm cannot be null.");
    }
    return RunInternal(mainForm);
}

void Application::Exit() {
    if (s_exitRequested) return;
    s_exitRequested = true;
    s_isRunning = false;

    auto formsCopy = s_openForms;
    for (auto& f : formsCopy) {
        if (f && !f->IsDisposed()) {
            f->Close();
        }
    }
    s_openForms.clear();
    s_mainForm.reset();

    auto exitHandlers = std::move(s_exitHandlers);
    s_exitHandlers.clear();
    for (const auto& handler : exitHandlers) {
        if (handler) handler();
    }
    s_idleHandlers.clear();
    s_dispatcher.Clear();
}

void Application::ExitThread() {
    Exit();
}

void Application::DoEvents() {
    s_dispatcher.DrainTasks();
    auto currentForms = s_openForms;
    for (auto& f : currentForms) {
        if (f && !f->IsDisposed()) {
            if (auto win = f->GetPlatformWindow()) {
                if (!win->IsClosed()) {
                    win->PollEvents();
                }
            }
        }
    }
    s_dispatcher.DrainTasks();
    for (const auto& idleHandler : s_idleHandlers) {
        if (idleHandler) idleHandler();
    }
}

bool Application::IsRunning() noexcept {
    return s_isRunning;
}

std::shared_ptr<Form> Application::GetActiveForm() noexcept {
    return s_activeForm.lock();
}

std::vector<std::shared_ptr<Form>> Application::GetOpenForms() {
    return s_openForms;
}

ShutdownMode Application::GetShutdownMode() noexcept {
    return s_shutdownMode;
}

void Application::SetShutdownMode(ShutdownMode mode) noexcept {
    s_shutdownMode = mode;
}

compat::string_view Application::GetExecutablePath() {
#if defined(_WIN32)
    if (s_executablePath.empty()) {
        char buffer[MAX_PATH] = { 0 };
        if (GetModuleFileNameA(nullptr, buffer, MAX_PATH) > 0) {
            s_executablePath = buffer;
        }
    }
#endif
    return s_executablePath;
}

compat::string_view Application::GetStartupPath() {
    if (s_startupPath.empty()) {
        auto exePath = GetExecutablePath();
        if (!exePath.empty()) {
            std::string path(exePath.data(), exePath.size());
            auto lastSlash = path.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                s_startupPath = path.substr(0, lastSlash);
            }
        }
    }
    return s_startupPath;
}

compat::string_view Application::GetProductName() {
    return s_productName;
}

void Application::SetProductName(compat::string_view name) {
    s_productName = std::string(name.data(), name.size());
}

compat::string_view Application::GetCompanyName() {
    return s_companyName;
}

void Application::SetCompanyName(compat::string_view name) {
    s_companyName = std::string(name.data(), name.size());
}

void Application::AddExitHandler(ExitCallback handler) {
    if (handler) {
        s_exitHandlers.push_back(std::move(handler));
    }
}

void Application::AddIdleHandler(IdleCallback handler) {
    if (handler) {
        s_idleHandlers.push_back(std::move(handler));
    }
}

void Application::SetServiceProvider(std::shared_ptr<ioc::IServiceProvider> provider) {
    if (!provider) {
        throw std::invalid_argument("Application::SetServiceProvider: provider cannot be null.");
    }
    s_serviceProvider = std::move(provider);
}

std::shared_ptr<ioc::IServiceProvider> Application::GetServices() noexcept {
    return s_serviceProvider;
}

void Application::RegisterForm(std::shared_ptr<Form> form) {
    if (!form) {
        throw std::invalid_argument("Application::RegisterForm: form cannot be null.");
    }
    for (const auto& existing : s_openForms) {
        if (existing.get() == form.get()) return;
    }
    s_openForms.push_back(form);
    s_activeForm = form;
}

void Application::UnregisterForm(Form* form) {
    if (!form) {
        throw std::invalid_argument("Application::UnregisterForm: form cannot be null.");
    }
    s_openForms.erase(
        std::remove_if(s_openForms.begin(), s_openForms.end(),
            [form](const std::shared_ptr<Form>& item) {
                return item.get() == form;
            }),
        s_openForms.end());

    if (auto active = s_activeForm.lock()) {
        if (active.get() == form) {
            s_activeForm.reset();
            if (!s_openForms.empty()) {
                s_activeForm = s_openForms.back();
            }
        }
    }
}

void Application::SetActiveForm(std::shared_ptr<Form> form) {
    s_activeForm = form;
}

AppBuilder Application::CreateBuilder() {
    return AppBuilder();
}

PlatformBackend Application::GetPlatformBackend() noexcept {
    return s_platformBackend;
}

void Application::SetPlatformBackend(PlatformBackend backend) noexcept {
    s_platformBackend = backend;
}

void Application::Post(std::function<void()> action) {
    if (!action) {
        throw std::invalid_argument("Application::Post: action cannot be empty.");
    }
    s_dispatcher.Post(std::move(action));
}

bool Application::CheckAccess() noexcept {
    return s_dispatcher.CheckAccess();
}

Dispatcher& Application::GetDispatcher() noexcept {
    return s_dispatcher;
}

void Application::WakeUpUIThread() {
    for (auto& f : s_openForms) {
        if (f && !f->IsDisposed() && f->GetPlatformWindow() && !f->GetPlatformWindow()->IsClosed()) {
            f->GetPlatformWindow()->WakeUp();
            return;
        }
    }
    if (s_mainForm && s_mainForm->GetPlatformWindow() && !s_mainForm->GetPlatformWindow()->IsClosed()) {
        s_mainForm->GetPlatformWindow()->WakeUp();
    }
}

} // namespace app
} // namespace cppui
