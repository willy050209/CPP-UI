/*
 * File: Timer.cpp
 * Purpose: Implements Timer component and background scheduler adhering to System.Windows.Forms.Timer.
 */

#include "ui/app/Timer.hpp"
#include "ui/app/Application.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace cppui {
namespace app {

struct Timer::Impl : public std::enable_shared_from_this<Timer::Impl> {
    mutable std::mutex m_mutex;
    int32_t m_intervalMs = 100;
    bool m_enabled = false;
    void* m_tag = nullptr;
    std::atomic<bool> m_tickPending{ false };
    std::vector<std::function<void(Timer&)>> m_handlers;
    Timer* m_owner = nullptr;

    void RaiseTick(bool force = false) {
        std::vector<std::function<void(Timer&)>> handlersCopy;
        Timer* owner = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!force && !m_enabled) {
                return;
            }
            handlersCopy = m_handlers;
            owner = m_owner;
        }

        for (const auto& handler : handlersCopy) {
            if (handler) {
                if (owner) {
                    handler(*owner);
                } else {
                    Timer dummy;
                    handler(dummy);
                }
            }
        }
    }
};

namespace {

class TimerScheduler {
public:
    static TimerScheduler& GetInstance() {
        static TimerScheduler s_instance;
        return s_instance;
    }

    TimerScheduler() = default;

    ~TimerScheduler() {
        Shutdown();
    }

    void Register(const std::shared_ptr<Timer::Impl>& impl, int32_t intervalMs) {
        if (!impl) return;
        std::unique_lock<std::mutex> lock(m_mutex);
        EnsureRunning(lock);

        RemoveItemInternal(impl);

        ScheduledItem item;
        item.impl = impl;
        item.intervalMs = intervalMs;
        item.nextDue = std::chrono::steady_clock::now() + std::chrono::milliseconds(intervalMs);
        m_items.push_back(item);

        m_cv.notify_one();
    }

    void Unregister(const std::shared_ptr<Timer::Impl>& impl) {
        if (!impl) return;
        std::unique_lock<std::mutex> lock(m_mutex);
        RemoveItemInternal(impl);
        m_cv.notify_one();
    }

    void Shutdown() {
        std::thread toJoin;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_running && !m_worker.joinable()) {
                return;
            }
            m_stopRequested = true;
            m_items.clear();
            m_cv.notify_all();
            if (m_worker.joinable()) {
                toJoin = std::move(m_worker);
            }
            m_running = false;
        }
        if (toJoin.joinable()) {
            toJoin.join();
        }
    }

private:
    struct ScheduledItem {
        std::weak_ptr<Timer::Impl> impl;
        std::chrono::steady_clock::time_point nextDue;
        int32_t intervalMs = 100;
    };

    void RemoveItemInternal(const std::shared_ptr<Timer::Impl>& target) {
        m_items.erase(
            std::remove_if(m_items.begin(), m_items.end(),
                [&target](const ScheduledItem& item) {
                    auto sp = item.impl.lock();
                    return !sp || sp == target;
                }),
            m_items.end());
    }

    void EnsureRunning(std::unique_lock<std::mutex>& lock) {
        if (m_stopRequested) {
            if (m_worker.joinable()) {
                lock.unlock();
                m_worker.join();
                lock.lock();
            }
            m_stopRequested = false;
            m_running = false;
        }

        if (!m_running) {
            m_running = true;
            m_worker = std::thread([this]() { ThreadProc(); });
        }
    }

    void ThreadProc() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_stopRequested) {
            if (m_items.empty()) {
                m_cv.wait(lock, [this]() { return m_stopRequested || !m_items.empty(); });
                continue;
            }

            auto now = std::chrono::steady_clock::now();
            auto earliestIt = std::min_element(
                m_items.begin(), m_items.end(),
                [](const ScheduledItem& a, const ScheduledItem& b) {
                    return a.nextDue < b.nextDue;
                });

            if (earliestIt == m_items.end()) {
                continue;
            }

            if (earliestIt->nextDue <= now) {
                auto item = *earliestIt;
                auto sp = item.impl.lock();

                if (!sp) {
                    m_items.erase(earliestIt);
                    continue;
                }

                earliestIt->nextDue = now + std::chrono::milliseconds(earliestIt->intervalMs);

                bool enabled = false;
                {
                    std::lock_guard<std::mutex> itemLock(sp->m_mutex);
                    enabled = sp->m_enabled;
                }

                if (enabled) {
                    bool expected = false;
                    if (sp->m_tickPending.compare_exchange_strong(expected, true)) {
                        std::weak_ptr<Timer::Impl> weakImpl = sp;
                        try {
                            Application::Post([weakImpl]() {
                                if (auto live = weakImpl.lock()) {
                                    live->m_tickPending = false;
                                    live->RaiseTick();
                                }
                            });
                        } catch (...) {
                            sp->m_tickPending = false;
                        }
                    }
                } else {
                    m_items.erase(earliestIt);
                }
            } else {
                auto wakeTime = earliestIt->nextDue;
                m_cv.wait_until(lock, wakeTime);
            }
        }
        m_running = false;
    }

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_worker;
    std::vector<ScheduledItem> m_items;
    bool m_running = false;
    bool m_stopRequested = false;
};

static std::once_flag s_exitHookOnce;

static void EnsureExitHook() {
    std::call_once(s_exitHookOnce, []() {
        Application::AddExitHandler([]() {
            TimerScheduler::GetInstance().Shutdown();
        });
    });
}

} // anonymous namespace

Timer::Timer()
    : m_impl(std::make_shared<Impl>()) {
    EnsureExitHook();
    m_impl->m_owner = this;
}

Timer::Timer(int32_t intervalMs)
    : m_impl(std::make_shared<Impl>()) {
    if (intervalMs <= 0) {
        throw std::invalid_argument("Timer::Timer: interval must be greater than 0.");
    }
    EnsureExitHook();
    m_impl->m_owner = this;
    m_impl->m_intervalMs = intervalMs;
}

Timer::~Timer() {
    if (m_impl) {
        Stop();
        std::lock_guard<std::mutex> lock(m_impl->m_mutex);
        m_impl->m_owner = nullptr;
    }
}

Timer::Timer(Timer&& other) noexcept
    : m_impl(std::move(other.m_impl)) {
    if (m_impl) {
        std::lock_guard<std::mutex> lock(m_impl->m_mutex);
        m_impl->m_owner = this;
    }
}

Timer& Timer::operator=(Timer&& other) noexcept {
    if (this != &other) {
        Stop();
        m_impl = std::move(other.m_impl);
        if (m_impl) {
            std::lock_guard<std::mutex> lock(m_impl->m_mutex);
            m_impl->m_owner = this;
        }
    }
    return *this;
}

int32_t Timer::GetInterval() const noexcept {
    if (!m_impl) return 100;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    return m_impl->m_intervalMs;
}

void Timer::SetInterval(int32_t intervalMs) {
    if (intervalMs <= 0) {
        throw std::invalid_argument("Timer::SetInterval: interval must be greater than 0.");
    }
    if (!m_impl) return;

    bool wasEnabled = false;
    {
        std::lock_guard<std::mutex> lock(m_impl->m_mutex);
        m_impl->m_intervalMs = intervalMs;
        wasEnabled = m_impl->m_enabled;
    }

    if (wasEnabled) {
        TimerScheduler::GetInstance().Register(m_impl, intervalMs);
    }
}

bool Timer::IsEnabled() const noexcept {
    if (!m_impl) return false;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    return m_impl->m_enabled;
}

void Timer::SetEnabled(bool enabled) {
    if (!m_impl) return;

    int32_t interval = 100;
    {
        std::lock_guard<std::mutex> lock(m_impl->m_mutex);
        if (m_impl->m_enabled == enabled) {
            return;
        }
        m_impl->m_enabled = enabled;
        interval = m_impl->m_intervalMs;
    }

    if (enabled) {
        TimerScheduler::GetInstance().Register(m_impl, interval);
    } else {
        TimerScheduler::GetInstance().Unregister(m_impl);
    }
}

void Timer::Start() {
    SetEnabled(true);
}

void Timer::Stop() {
    SetEnabled(false);
}

void Timer::Restart() {
    Stop();
    Start();
}

void Timer::PerformTick() {
    if (m_impl) {
        m_impl->RaiseTick(true);
    }
}

void* Timer::GetTag() const noexcept {
    if (!m_impl) return nullptr;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    return m_impl->m_tag;
}

void Timer::SetTag(void* tag) noexcept {
    if (!m_impl) return;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    m_impl->m_tag = tag;
}

void Timer::OnTick(TickHandler handler) {
    if (!handler) {
        throw std::invalid_argument("Timer::OnTick: handler cannot be empty.");
    }
    AddTickHandler(std::move(handler));
}

void Timer::OnTick(TickWithSenderHandler handler) {
    if (!handler) {
        throw std::invalid_argument("Timer::OnTick: handler cannot be empty.");
    }
    AddTickHandler(std::move(handler));
}

void Timer::AddTickHandler(TickHandler handler) {
    if (!handler) {
        throw std::invalid_argument("Timer::AddTickHandler: handler cannot be empty.");
    }
    if (!m_impl) return;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    m_impl->m_handlers.push_back([h = std::move(handler)](Timer&) {
        h();
    });
}

void Timer::AddTickHandler(TickWithSenderHandler handler) {
    if (!handler) {
        throw std::invalid_argument("Timer::AddTickHandler: handler cannot be empty.");
    }
    if (!m_impl) return;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    m_impl->m_handlers.push_back(std::move(handler));
}

void Timer::ClearTickHandlers() noexcept {
    if (!m_impl) return;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    m_impl->m_handlers.clear();
}

size_t Timer::GetTickHandlerCount() const noexcept {
    if (!m_impl) return 0;
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    return m_impl->m_handlers.size();
}

} // namespace app
} // namespace cppui
