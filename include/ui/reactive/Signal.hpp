/**
 * @file Signal.hpp
 * @brief 輕量型別安全響應式屬性信號，支援自動變更推播與 UI 屬性單向/雙向綁定
 */

#pragma once

#include <vector>
#include <functional>
#include <memory>
#include "compat/compat.hpp"

namespace cppui {
namespace reactive {

/// <summary>
/// 輕量型別安全響應式屬性信號 (Signal)
/// 支援值變更時自動推播更新通知與 UI 元件單向/雙向綁定。
/// </summary>
/// <typeparam name="T">持有的數值型別。</typeparam>
template <typename T>
class Signal {
public:
    /// <summary>
    /// 完整值變更監聽器型別 (攜帶新舊值)。
    /// </summary>
    using ChangeListener = std::function<void(const T& newValue, const T& oldValue)>;

    /// <summary>
    /// 單值更新監聽器型別 (僅攜帶新值)。
    /// </summary>
    using ValueListener  = std::function<void(const T& newValue)>;

    /// <summary>
    /// 預設建構子，以 T 的預設值初始化。
    /// </summary>
    Signal() : m_value(T{}) {}

    /// <summary>
    /// 具值建構子，指定初始值。
    /// </summary>
    /// <param name="initialValue">初始數值。</param>
    explicit Signal(T initialValue) : m_value(std::move(initialValue)) {}

    /// <summary>
    /// 取得當前持有的數值常量引用。
    /// </summary>
    /// <returns>數值常量引用。</returns>
    COMPAT_NODISCARD const T& Get() const noexcept { return m_value; }

    /// <summary>
    /// 解引用運算子，等同於 Get()。
    /// </summary>
    /// <returns>數值常量引用。</returns>
    COMPAT_NODISCARD const T& operator*() const noexcept { return m_value; }

    /// <summary>
    /// 成員存取運算子。
    /// </summary>
    /// <returns>數值指標。</returns>
    COMPAT_NODISCARD const T* operator->() const noexcept { return &m_value; }

    /// <summary>
    /// 設定新數值；若新舊值不相等則廣播變更通知至所有訂閱者。
    /// </summary>
    /// <param name="newValue">欲賦予的新數值。</param>
    void Set(T newValue) {
        if (!(m_value == newValue)) {
            T old = m_value;
            m_value = std::move(newValue);
            for (auto& entry : m_listeners) {
                if (entry.second) {
                    entry.second(m_value, old);
                }
            }
        }
    }

    /// <summary>
    /// 賦值運算子，等同於 Set(newValue)。
    /// </summary>
    /// <param name="newValue">欲賦予的新數值。</param>
    /// <returns>當前信號物件引用。</returns>
    Signal& operator=(T newValue) {
        Set(std::move(newValue));
        return *this;
    }

    /// <summary>
    /// 訂閱完整變更通知 (帶有新值與舊值)。
    /// </summary>
    /// <param name="listener">變更回呼函式。</param>
    /// <returns>訂閱識別碼 (用於後續 Unsubscribe)。</returns>
    size_t Subscribe(ChangeListener listener) {
        size_t id = ++m_nextId;
        m_listeners.emplace_back(id, std::move(listener));
        return id;
    }

    /// <summary>
    /// 訂閱單一新值變更通知。
    /// </summary>
    /// <param name="listener">新值回呼函式。</param>
    /// <returns>訂閱識別碼。</returns>
    size_t Subscribe(ValueListener listener) {
        return Subscribe([listener](const T& newVal, const T&) {
            listener(newVal);
        });
    }

    /// <summary>
    /// 取消指定識別碼之通知訂閱。
    /// </summary>
    /// <param name="id">Subscribe 所回傳之訂閱識別碼。</param>
    void Unsubscribe(size_t id) {
        for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
            if (it->first == id) {
                m_listeners.erase(it);
                break;
            }
        }
    }

    /// <summary>
    /// 繫結至目標控制項的屬性更新回呼，並立即套用當前數值。
    /// </summary>
    /// <typeparam name="ControlType">目標控制項型別。</typeparam>
    /// <typeparam name="Formatter">格式化轉換與指派函式型別。</typeparam>
    /// <param name="target">目標控制項之 shared_ptr。</param>
    /// <param name="formatter">接受 (ControlType&, const T&) 的格式化器。</param>
    /// <returns>訂閱識別碼。</returns>
    template <typename ControlType, typename Formatter>
    size_t Bind(std::shared_ptr<ControlType> target, Formatter formatter) {
        std::weak_ptr<ControlType> weakTarget = target;
        // 初始套用一次
        if (target) {
            formatter(*target, m_value);
        }
        return Subscribe([weakTarget, formatter](const T& newVal, const T&) {
            if (auto locked = weakTarget.lock()) {
                formatter(*locked, newVal);
            }
        });
    }

private:
    T m_value;
    std::vector<std::pair<size_t, ChangeListener>> m_listeners;
    size_t m_nextId = 0;
};

} // namespace reactive
} // namespace cppui
