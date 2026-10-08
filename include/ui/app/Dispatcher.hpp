#pragma once

#include "compat/compat.hpp"
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <utility>

namespace cppui {
namespace app {

/// <summary>
/// UI 執行緒非同步事件調度器 (Dispatcher)
/// 提供執行緒安全的任務佇列，支援跨執行緒投遞動作至主執行緒消費執行。
/// </summary>
class Dispatcher {
public:
    Dispatcher();
    ~Dispatcher() = default;

    // 禁止拷貝
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    /// <summary>
    /// 以非同步方式在 UI 執行緒上排隊並執行操作 (對標 WPF Dispatcher.InvokeAsync)
    /// </summary>
    void InvokeAsync(std::function<void()> action);

    /// <summary>
    /// 便捷別名：投遞任務至 UI 執行緒
    /// </summary>
    void Post(std::function<void()> action) {
        InvokeAsync(std::move(action));
    }

    /// <summary>
    /// 檢查呼叫端是否運行於 UI 主執行緒 (對標 WPF Dispatcher.CheckAccess)
    /// </summary>
    COMPAT_NODISCARD bool CheckAccess() const noexcept;

    /// <summary>
    /// 消耗並執行佇列中所有待處理的任務 (僅限 UI 執行緒呼叫)
    /// </summary>
    void DrainTasks();

    /// <summary>
    /// 設定關聯的主執行緒 ID
    /// </summary>
    void SetMainThreadId(std::thread::id threadId) noexcept;

    /// <summary>
    /// 清除所有待處理任務
    /// </summary>
    void Clear();

private:
    mutable std::mutex m_mutex;
    std::queue<std::function<void()>> m_queue;
    std::thread::id m_mainThreadId;
    bool m_hasMainThreadId = false;
};

} // namespace app
} // namespace cppui
