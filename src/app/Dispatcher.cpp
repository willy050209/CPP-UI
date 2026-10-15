/*
 * File: Dispatcher.cpp
 * Purpose: Implements the UI thread Dispatcher with fail-fast validation and thread-safe execution.
 */

#include "ui/app/Dispatcher.hpp"
#include "ui/app/Application.hpp"
#include <stdexcept>
#include <vector>

namespace cppui {
namespace app {

Dispatcher::Dispatcher()
    : m_mainThreadId(std::this_thread::get_id()), m_hasMainThreadId(true) {}

void Dispatcher::InvokeAsync(std::function<void()> action) {
    InvokeAsync(core::LifetimeToken{}, std::move(action));
}

void Dispatcher::InvokeAsync(core::LifetimeToken token, std::function<void()> action) {
    if (!action) {
        throw std::invalid_argument("Dispatcher::InvokeAsync: action cannot be empty.");
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(TaskEntry{ std::move(action), std::move(token) });
    }

    Application::WakeUpUIThread();
}

bool Dispatcher::CheckAccess() const noexcept {
    if (!m_hasMainThreadId) {
        return true;
    }
    return std::this_thread::get_id() == m_mainThreadId;
}

void Dispatcher::DrainTasks() {
    std::vector<TaskEntry> tasks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            tasks.push_back(std::move(m_queue.front()));
            m_queue.pop();
        }
    }

    for (const auto& task : tasks) {
        if (task.action) {
            // 若任務關聯了生命週期權杖，檢查其所屬物件是否仍存活；若已失效則安全略過以防 Use-After-Free
            if (task.token.HasHandle() && !task.token.IsAlive()) {
                continue;
            }
            try {
                task.action();
            } catch (...) {
                // 防禦性捕捉任務異常，防止中斷 UI 訊息迴圈
            }
        }
    }
}

void Dispatcher::SetMainThreadId(std::thread::id threadId) noexcept {
    m_mainThreadId = threadId;
    m_hasMainThreadId = true;
}

void Dispatcher::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<TaskEntry> empty;
    std::swap(m_queue, empty);
}

} // namespace app
} // namespace cppui
