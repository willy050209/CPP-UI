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
template <typename T>
class Signal {
public:
    using ChangeListener = std::function<void(const T& newValue, const T& oldValue)>;
    using ValueListener  = std::function<void(const T& newValue)>;

    Signal() : m_value(T{}) {}
    explicit Signal(T initialValue) : m_value(std::move(initialValue)) {}

    COMPAT_NODISCARD const T& Get() const noexcept { return m_value; }
    COMPAT_NODISCARD const T& operator*() const noexcept { return m_value; }
    COMPAT_NODISCARD const T* operator->() const noexcept { return &m_value; }

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

    Signal& operator=(T newValue) {
        Set(std::move(newValue));
        return *this;
    }

    // 訂閱完整變更通知 (帶新舊值)
    size_t Subscribe(ChangeListener listener) {
        size_t id = ++m_nextId;
        m_listeners.emplace_back(id, std::move(listener));
        return id;
    }

    // 訂閱單一新值通知
    size_t Subscribe(ValueListener listener) {
        return Subscribe([listener](const T& newVal, const T&) {
            listener(newVal);
        });
    }

    void Unsubscribe(size_t id) {
        for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
            if (it->first == id) {
                m_listeners.erase(it);
                break;
            }
        }
    }

    // 綁定至目標控制項的屬性更新回呼
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
