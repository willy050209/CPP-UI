/**
 * @file FormDocumentNode.hpp
 * @brief 表單文件根節點 (FormDocumentNode) 與反應式異動匯流排 (Reactive Change Bus)
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/designer/ast/WidgetNode.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 表單元資料與生成組態 (FormSettings)
/// </summary>
struct FormSettings {
    std::string className = "MainForm";
    std::string targetNamespace = "MyAPP";
    core::Size initialSize{ 800.0f, 600.0f };
    std::string title = "MainForm";
    std::string defaultTheme = "Dark";
    std::string startPosition = "CenterScreen";
    std::string windowState = "Normal";
    bool useMvvm = true;
    bool useIoC = true;
    bool topmost = false;
    bool showInTaskbar = true;

    COMPAT_NODISCARD bool operator==(const FormSettings& other) const noexcept {
        return className == other.className &&
               targetNamespace == other.targetNamespace &&
               initialSize == other.initialSize &&
               title == other.title &&
               defaultTheme == other.defaultTheme &&
               startPosition == other.startPosition &&
               windowState == other.windowState &&
               useMvvm == other.useMvvm &&
               useIoC == other.useIoC &&
               topmost == other.topmost &&
               showInTaskbar == other.showInTaskbar;
    }

    COMPAT_NODISCARD bool operator!=(const FormSettings& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>節點新增通知載荷</summary>
struct NodeAddedPayload {
    std::string parentUuid;
    std::string childUuid;
    size_t index = 0;
};

/// <summary>節點移除通知載荷</summary>
struct NodeRemovedPayload {
    std::string parentUuid;
    std::string childUuid;
};

/// <summary>節點搬移通知載荷</summary>
struct NodeMovedPayload {
    std::string nodeUuid;
    std::string oldParentUuid;
    std::string newParentUuid;
    size_t newIndex = 0;
};

/// <summary>屬性變更通知載荷</summary>
struct PropertyChangedPayload {
    std::string nodeUuid;
    std::string propertyName;
    PropertyValue oldValue;
    PropertyValue newValue;
};

/// <summary>表單組態變更通知載荷</summary>
struct FormSettingsChangedPayload {
    FormSettings oldSettings;
    FormSettings newSettings;
};

/// <summary>
/// 反應式事件通知通道 (DesignerEvent)
/// 整合 reactive::Connection 與 core::LifetimeToken，提供無懸空回呼保證之事件推播。
/// </summary>
template <typename Payload>
class DesignerEvent {
public:
    using Callback = std::function<void(const Payload&)>;

    reactive::Connection Connect(Callback callback) {
        size_t id = ++m_nextId;
        m_listeners.emplace_back(id, std::move(callback));
        core::LifetimeToken token = m_lifetimeToken;
        DesignerEvent* self = this;
        return reactive::Connection([self, token, id]() {
            if (token.IsAlive()) {
                self->Disconnect(id);
            }
        });
    }

    reactive::Connection Connect(core::LifetimeToken token, Callback callback) {
        return Connect([token, cb = std::move(callback)](const Payload& payload) {
            if (token.IsAlive()) {
                cb(payload);
            }
        });
    }

    void Notify(const Payload& payload) {
        // 複製一份監聽器列表，防止回呼中呼叫 Disconnect 引發容器迭代器失效
        auto listenersCopy = m_listeners;
        for (const auto& entry : listenersCopy) {
            if (entry.second) {
                entry.second(payload);
            }
        }
    }

    void Disconnect(size_t id) {
        for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
            if (it->first == id) {
                m_listeners.erase(it);
                break;
            }
        }
    }

    ~DesignerEvent() {
        m_lifetimeToken.Invalidate();
    }

private:
    std::vector<std::pair<size_t, Callback>> m_listeners;
    size_t m_nextId = 0;
    core::LifetimeToken m_lifetimeToken{ core::LifetimeToken::Create() };
};

/// <summary>
/// 表單文件根節點 (FormDocumentNode)
/// 作為 Designer 的頂層領域模型與單一真實來源 (SSOT)。
/// 內建反應式變更匯流排，支援細粒度屬性與拓撲異動廣播。
/// </summary>
class CPPUI_API FormDocumentNode {
public:
    /// <summary>預設建構空白表單文件。</summary>
    FormDocumentNode();

    /// <summary>以自訂表單組態建構文件。</summary>
    explicit FormDocumentNode(FormSettings settings);

    /// <summary>解構子，於 .cpp 完整型別處實作。</summary>
    ~FormDocumentNode();

    /// <summary>移動建構子。</summary>
    FormDocumentNode(FormDocumentNode&& other) noexcept;

    /// <summary>移動賦值運算子。</summary>
    FormDocumentNode& operator=(FormDocumentNode&& other) noexcept;

    FormDocumentNode(const FormDocumentNode&) = delete;
    FormDocumentNode& operator=(const FormDocumentNode&) = delete;

    // --- 表單組態存取 ---

    COMPAT_NODISCARD const FormSettings& GetSettings() const noexcept;
    void SetSettings(FormSettings settings);

    // --- 根容器操作 ---

    COMPAT_NODISCARD WidgetNode* GetRoot() noexcept;
    COMPAT_NODISCARD const WidgetNode* GetRoot() const noexcept;
    void SetRoot(std::unique_ptr<WidgetNode> root);
    std::unique_ptr<WidgetNode> TakeRoot() noexcept;

    // --- 全域節點檢索 ---

    COMPAT_NODISCARD WidgetNode* FindByUuid(compat::string_view uuid) noexcept;
    COMPAT_NODISCARD const WidgetNode* FindByUuid(compat::string_view uuid) const noexcept;

    COMPAT_NODISCARD WidgetNode* FindByName(compat::string_view name) noexcept;
    COMPAT_NODISCARD const WidgetNode* FindByName(compat::string_view name) const noexcept;

    // --- 反應式信號通道 (Reactive Change Bus) ---

    DesignerEvent<NodeAddedPayload>& NodeAddedSignal() noexcept;
    DesignerEvent<NodeRemovedPayload>& NodeRemovedSignal() noexcept;
    DesignerEvent<NodeMovedPayload>& NodeMovedSignal() noexcept;
    DesignerEvent<PropertyChangedPayload>& PropertyChangedSignal() noexcept;
    DesignerEvent<FormSettingsChangedPayload>& SettingsChangedSignal() noexcept;

    // --- 複製 ---

    COMPAT_NODISCARD std::unique_ptr<FormDocumentNode> Clone(bool generateNewUuids = false) const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace designer
} // namespace cppui
