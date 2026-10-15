/**
 * @file Signal.hpp
 * @brief 輕量型別安全響應式屬性信號，支援自動變更推播與 UI 屬性單向/雙向綁定
 */

#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <utility>
#include "compat/Compat.hpp"
#include "ui/core/LifetimeToken.hpp"

namespace cppui {
namespace reactive {

/// <summary>
/// 響應式信號訂閱連線句柄 (Connection)
/// 封裝中斷回呼函式，支援手動 Disconnect() 與連線狀態查詢。
/// </summary>
class Connection {
public:
    Connection() = default;

    explicit Connection(std::function<void()> disconnect)
        : m_disconnect(std::move(disconnect)), m_connected(true) {}

    /// <summary>
    /// 主動中斷信號訂閱。多次呼叫具備等冪性 (Idempotent)。
    /// </summary>
    void Disconnect() {
        if (m_connected && m_disconnect) {
            m_disconnect();
            m_disconnect = nullptr;
            m_connected = false;
        }
    }

    /// <summary>
    /// 查詢當前連線是否處於活躍狀態。
    /// </summary>
    /// <returns>若連線活躍傳回 true。</returns>
    COMPAT_NODISCARD bool IsConnected() const noexcept {
        return m_connected;
    }

private:
    std::function<void()> m_disconnect;
    bool m_connected = false;
};

/// <summary>
/// RAII 作用域信號訂閱連線管理員 (ScopedConnection)
/// 在其生命週期結束 (解構) 時自動中斷訂閱，禁止拷貝，支援移動語意。
/// </summary>
class ScopedConnection {
public:
    ScopedConnection() = default;

    /// <summary>
    /// 從 Connection 轉換建構，接管連線生命週期管理。
    /// </summary>
    /// <param name="connection">欲接管之連線。</param>
    ScopedConnection(Connection connection)
        : m_connection(std::move(connection)) {}

    /// <summary>
    /// 從 Connection 賦值接管連線。
    /// </summary>
    ScopedConnection& operator=(Connection connection) noexcept {
        m_connection.Disconnect();
        m_connection = std::move(connection);
        return *this;
    }

    ~ScopedConnection() {
        m_connection.Disconnect();
    }

    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;

    ScopedConnection(ScopedConnection&& other) noexcept
        : m_connection(std::move(other.m_connection)) {}

    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            m_connection.Disconnect();
            m_connection = std::move(other.m_connection);
        }
        return *this;
    }

    /// <summary>
    /// 手動提前中斷連線。
    /// </summary>
    void Disconnect() {
        m_connection.Disconnect();
    }

    /// <summary>
    /// 查詢當前連線是否處於活躍狀態。
    /// </summary>
    /// <returns>若連線活躍傳回 true。</returns>
    COMPAT_NODISCARD bool IsConnected() const noexcept {
        return m_connection.IsConnected();
    }

    /// <summary>
    /// 釋出底層 Connection 物件並解除 RAII 自動中斷責任。
    /// </summary>
    /// <returns>底層 Connection 物件。</returns>
    Connection Release() noexcept {
        Connection conn = std::move(m_connection);
        return conn;
    }

private:
    Connection m_connection;
};

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
    /// 解構子，自動失效內部 LifetimeToken，使外部持有的 Connection 斷開時不會存取野指標。
    /// </summary>
    ~Signal() {
        m_lifetimeToken.Invalidate();
    }

    /// <summary>
    /// 具值建構子，指定初始值。
    /// </summary>
    /// <param name="initialValue">初始數值。</param>
    explicit Signal(T initialValue) : m_value(std::move(initialValue)) {}

    // 拷貝與移動建構子
    Signal(const Signal& other) : m_value(other.m_value) {}
    Signal& operator=(const Signal& other) {
        if (this != &other) {
            Set(other.m_value);
        }
        return *this;
    }

    Signal(Signal&& other) noexcept
        : m_value(std::move(other.m_value)),
          m_listeners(std::move(other.m_listeners)),
          m_nextId(other.m_nextId) {
        other.m_lifetimeToken.Invalidate();
        other.m_nextId = 0;
    }

    Signal& operator=(Signal&& other) noexcept {
        if (this != &other) {
            m_lifetimeToken.Invalidate();
            m_lifetimeToken = core::LifetimeToken::Create();
            m_value = std::move(other.m_value);
            m_listeners = std::move(other.m_listeners);
            m_nextId = other.m_nextId;
            other.m_lifetimeToken.Invalidate();
            other.m_nextId = 0;
        }
        return *this;
    }

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
    /// 訂閱完整變更通知 (受 LifetimeToken 保護，目標物件銷毀時回呼自動失效)。
    /// </summary>
    /// <param name="token">目標物件生命週期權杖。</param>
    /// <param name="listener">變更回呼函式。</param>
    /// <returns>訂閱識別碼。</returns>
    size_t Subscribe(core::LifetimeToken token, ChangeListener listener) {
        return Subscribe([token, listener](const T& newVal, const T& oldVal) {
            if (token.IsAlive()) {
                listener(newVal, oldVal);
            }
        });
    }

    /// <summary>
    /// 訂閱單一新值變更通知 (受 LifetimeToken 保護)。
    /// </summary>
    /// <param name="token">目標物件生命週期權杖。</param>
    /// <param name="listener">新值回呼函式。</param>
    /// <returns>訂閱識別碼。</returns>
    size_t Subscribe(core::LifetimeToken token, ValueListener listener) {
        return Subscribe(token, [listener](const T& newVal, const T&) {
            listener(newVal);
        });
    }

    /// <summary>
    /// 建立 RAII 訂閱連線 (完整變更通知)。
    /// </summary>
    /// <param name="listener">變更回呼函式。</param>
    /// <returns>Connection 實例。</returns>
    COMPAT_NODISCARD Connection Connect(ChangeListener listener) {
        size_t id = Subscribe(std::move(listener));
        core::LifetimeToken token = m_lifetimeToken;
        Signal* self = this;
        return Connection([self, token, id]() {
            if (token.IsAlive()) {
                self->Unsubscribe(id);
            }
        });
    }

    /// <summary>
    /// 建立 RAII 訂閱連線 (單一新值變更通知)。
    /// </summary>
    /// <param name="listener">新值回呼函式。</param>
    /// <returns>Connection 實例。</returns>
    COMPAT_NODISCARD Connection Connect(ValueListener listener) {
        return Connect([listener](const T& newVal, const T&) {
            listener(newVal);
        });
    }

    /// <summary>
    /// 建立受 LifetimeToken 保護之 RAII 訂閱連線 (完整變更通知)。
    /// </summary>
    /// <param name="token">生命週期權杖。</param>
    /// <param name="listener">變更回呼函式。</param>
    /// <returns>Connection 實例。</returns>
    COMPAT_NODISCARD Connection Connect(core::LifetimeToken token, ChangeListener listener) {
        size_t id = Subscribe(token, std::move(listener));
        core::LifetimeToken signalToken = m_lifetimeToken;
        Signal* self = this;
        return Connection([self, signalToken, id]() {
            if (signalToken.IsAlive()) {
                self->Unsubscribe(id);
            }
        });
    }

    /// <summary>
    /// 建立受 LifetimeToken 保護之 RAII 訂閱連線 (單一新值變更通知)。
    /// </summary>
    /// <param name="token">生命週期權杖。</param>
    /// <param name="listener">新值回呼函式。</param>
    /// <returns>Connection 實例。</returns>
    COMPAT_NODISCARD Connection Connect(core::LifetimeToken token, ValueListener listener) {
        return Connect(token, [listener](const T& newVal, const T&) {
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
    core::LifetimeToken m_lifetimeToken{ core::LifetimeToken::Create() };
};

} // namespace reactive
} // namespace cppui
