/**
 * @file LifetimeToken.hpp
 * @brief 輕量級執行緒安全生命週期權杖 (LifetimeToken)，用於防範非同步回呼與訊號訂閱之 Use-After-Free
 */

#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include "compat/compat.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 輕量級執行緒安全生命週期權杖 (LifetimeToken)
/// 底層由 std::shared_ptr&lt;std::atomic&lt;bool&gt;&gt; 實作。
/// 當所屬物件 (如 UIElement、Form) 解構時主動呼叫 Invalidate()，
/// 任何非同步任務 (Dispatcher)、定時器 (Timer) 或響應式信號 (Signal) 於回呼前檢驗 IsAlive()，
/// 若權杖已失效則自動略過或註銷回呼，徹底杜絕 Use-After-Free 崩潰。
/// </summary>
class LifetimeToken {
public:
    /// <summary>
    /// 預設建構子，建立空權杖 (未關聯任何生命週期)。
    /// </summary>
    LifetimeToken() = default;

    /// <summary>
    /// 以現有原子旗標共享指標建構權杖。
    /// </summary>
    /// <param name="alive">持有存活狀態之 atomic bool 共享指標。</param>
    explicit LifetimeToken(std::shared_ptr<std::atomic<bool>> alive)
        : m_alive(std::move(alive)) {}

    /// <summary>
    /// 檢查所關聯之物件是否仍處於存活狀態。
    /// </summary>
    /// <returns>若關聯物件存活回傳 true；若無效或已銷毀則回傳 false。</returns>
    COMPAT_NODISCARD bool IsAlive() const noexcept {
        return m_alive && m_alive->load(std::memory_order_acquire);
    }

    /// <summary>
    /// 布林運算子，等同於 IsAlive()。
    /// </summary>
    /// <returns>若存活回傳 true。</returns>
    COMPAT_NODISCARD explicit operator bool() const noexcept {
        return IsAlive();
    }

    /// <summary>
    /// 使當前權杖失效 (標記為已銷毀)。
    /// </summary>
    void Invalidate() noexcept {
        if (m_alive) {
            m_alive->store(false, std::memory_order_release);
        }
    }

    /// <summary>
    /// 檢查當前權杖是否關聯了有效的生命週期控制代碼 (不論其是否存活)。
    /// </summary>
    /// <returns>若持有有效控制代碼回傳 true。</returns>
    COMPAT_NODISCARD bool HasHandle() const noexcept {
        return m_alive != nullptr;
    }

    /// <summary>
    /// 取得底層共享指標控制代碼。
    /// </summary>
    /// <returns>底層 std::shared_ptr 引用。</returns>
    COMPAT_NODISCARD const std::shared_ptr<std::atomic<bool>>& GetHandle() const noexcept {
        return m_alive;
    }

    /// <summary>
    /// 靜態工廠方法：建立初始狀態為存活 (true) 之全新權杖。
    /// </summary>
    /// <returns>全新 LifetimeToken 實例。</returns>
    static LifetimeToken Create() {
        return LifetimeToken(std::make_shared<std::atomic<bool>>(true));
    }

private:
    std::shared_ptr<std::atomic<bool>> m_alive;
};

} // namespace core
} // namespace cppui
