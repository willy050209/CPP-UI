/*
 * File: Dispatcher.hpp
 * Purpose: Declares the UI thread Dispatcher for thread-safe action queuing and marshalling.
 */

#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

#include "compat/Compat.hpp"
#include "ui/core/LifetimeToken.hpp"

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
    /// 以非同步方式在 UI 執行緒上排隊並執行操作 (對標 WPF Dispatcher.InvokeAsync)。
    /// </summary>
    /// <param name="action">待排隊調度的回呼函式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 action 為空時擲出。</exception>
    void InvokeAsync(std::function<void()> action);

    /// <summary>
    /// 以非同步方式在 UI 執行緒上排隊並執行操作，並綁定生命週期權杖。
    /// 若執行時權杖已失效 (如所屬視窗或元件已解構)，則自動捨棄該任務以防 Use-After-Free。
    /// </summary>
    /// <param name="token">生命週期權杖。</param>
    /// <param name="action">待排隊調度的回呼函式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 action 為空時擲出。</exception>
    void InvokeAsync(core::LifetimeToken token, std::function<void()> action);

    /// <summary>
    /// 便捷別名：投遞任務至 UI 執行緒。
    /// </summary>
    /// <param name="action">待排隊調度的回呼函式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 action 為空時擲出。</exception>
    void Post(std::function<void()> action) {
        InvokeAsync(std::move(action));
    }

    /// <summary>
    /// 便捷別名：投遞受生命週期權杖保護之任務至 UI 執行緒。
    /// </summary>
    /// <param name="token">生命週期權杖。</param>
    /// <param name="action">待排隊調度的回呼函式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 action 為空時擲出。</exception>
    void Post(core::LifetimeToken token, std::function<void()> action) {
        InvokeAsync(token, std::move(action));
    }

    /// <summary>
    /// 檢查呼叫端是否運行於 UI 主執行緒 (對標 WPF Dispatcher.CheckAccess)。
    /// </summary>
    /// <returns>若為 UI 主執行緒傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool CheckAccess() const noexcept;

    /// <summary>
    /// 消耗並執行佇列中所有待處理的任務 (僅限 UI 執行緒呼叫)。
    /// </summary>
    void DrainTasks();

    /// <summary>
    /// 設定關聯的主執行緒 ID。
    /// </summary>
    /// <param name="threadId">主執行緒 ID。</param>
    void SetMainThreadId(std::thread::id threadId) noexcept;

    /// <summary>
    /// 清除所有待處理任務。
    /// </summary>
    void Clear();

private:
    struct TaskEntry {
        std::function<void()> action;
        core::LifetimeToken   token;
    };

    mutable std::mutex m_mutex;
    std::queue<TaskEntry> m_queue;
    std::thread::id m_mainThreadId;
    bool m_hasMainThreadId{false};
};

} // namespace app
} // namespace cppui
