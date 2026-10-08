#include "ui/app/Dispatcher.hpp"
#include "ui/app/Application.hpp"
#include <vector>

namespace cppui {
namespace app {

Dispatcher::Dispatcher()
    : m_mainThreadId(std::this_thread::get_id()), m_hasMainThreadId(true) {}

void Dispatcher::InvokeAsync(std::function<void()> action) {
    if (!action) return;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(action));
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
    std::vector<std::function<void()>> tasks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            tasks.push_back(std::move(m_queue.front()));
            m_queue.pop();
        }
    }

    for (const auto& task : tasks) {
        if (task) {
            try {
                task();
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
    std::queue<std::function<void()>> empty;
    std::swap(m_queue, empty);
}

} // namespace app
} // namespace cppui
