/**
 * @file ServiceCollection.hpp
 * @brief IoC 服務集合類別，用於註冊應用程式相依性、工廠與生命週期管理
 */

#pragma once

#include "compat/Compat.hpp"
#include "IServiceProvider.hpp"
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

namespace cppui {
namespace ioc {

/// <summary>
/// 服務生命週期模式。
/// </summary>
enum class ServiceLifetime {
    Transient, // 每次請求皆建立新實例
    Singleton  // 應用程式內僅建立單一實例
};

/// <summary>
/// 服務集合 (ServiceCollection)
/// 用於註冊應用程式依賴項目與服務工廠，並構建不可變之 IServiceProvider。
/// </summary>
class ServiceCollection {
public:
    /// <summary>
    /// 服務工廠函數類型別名。
    /// </summary>
    using ServiceFactory = std::function<std::shared_ptr<void>(IServiceProvider&)>;

    /// <summary>
    /// 服務註冊描述項。
    /// </summary>
    struct ServiceDescriptor {
        ServiceLifetime lifetime;
        ServiceFactory factory;
        std::shared_ptr<void> singletonInstance;
    };

    /// <summary>
    /// 建構空白服務集合。
    /// </summary>
    ServiceCollection();

    /// <summary>
    /// 解構服務集合。
    /// </summary>
    ~ServiceCollection();

    /// <summary>
    /// 註冊單例服務 (Singleton)，將介面 TInterface 映射至具體實作 TImplementation。
    /// </summary>
    /// <typeparam name="TInterface">目標介面型別。</typeparam>
    /// <typeparam name="TImplementation">目標實作型別，必須具備無參數預設建構子。</typeparam>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    template<typename TInterface, typename TImplementation>
    ServiceCollection& AddSingleton() {
        auto factory = [](IServiceProvider&) -> std::shared_ptr<void> {
            return std::make_shared<TImplementation>();
        };
        RegisterDescriptor(typeid(TInterface), ServiceLifetime::Singleton, factory, nullptr);
        return *this;
    }

    /// <summary>
    /// 註冊具體類別單例服務 (Singleton)。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    template<typename TService>
    ServiceCollection& AddSingleton() {
        return AddSingleton<TService, TService>();
    }

    /// <summary>
    /// 註冊現有實例單例服務 (Singleton)。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <param name="instance">預先建構之服務實例智慧指標。</param>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    /// <exception cref="std::invalid_argument">當 instance 為空指標時擲出。</exception>
    template<typename TService>
    ServiceCollection& AddSingleton(std::shared_ptr<TService> instance) {
        if (!instance) {
            throw std::invalid_argument("ServiceCollection::AddSingleton: instance cannot be null.");
        }
        auto factory = [instance](IServiceProvider&) -> std::shared_ptr<void> {
            return instance;
        };
        RegisterDescriptor(typeid(TService), ServiceLifetime::Singleton, factory, instance);
        return *this;
    }

    /// <summary>
    /// 透過工廠函數註冊單例服務 (Singleton)。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <param name="factory">建立服務之自訂工廠函數。</param>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    /// <exception cref="std::invalid_argument">當 factory 未持有有效回呼時擲出。</exception>
    template<typename TService>
    ServiceCollection& AddSingleton(std::function<std::shared_ptr<TService>(IServiceProvider&)> factory) {
        if (!factory) {
            throw std::invalid_argument("ServiceCollection::AddSingleton: factory cannot be null.");
        }
        auto erasedFactory = [factory](IServiceProvider& sp) -> std::shared_ptr<void> {
            return factory(sp);
        };
        RegisterDescriptor(typeid(TService), ServiceLifetime::Singleton, erasedFactory, nullptr);
        return *this;
    }

    /// <summary>
    /// 註冊瞬態服務 (Transient)，每次解析皆透過預設建構子產生全新實例。
    /// </summary>
    /// <typeparam name="TInterface">目標介面型別。</typeparam>
    /// <typeparam name="TImplementation">目標實作型別。</typeparam>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    template<typename TInterface, typename TImplementation>
    ServiceCollection& AddTransient() {
        auto factory = [](IServiceProvider&) -> std::shared_ptr<void> {
            return std::make_shared<TImplementation>();
        };
        RegisterDescriptor(typeid(TInterface), ServiceLifetime::Transient, factory, nullptr);
        return *this;
    }

    /// <summary>
    /// 註冊具體類別瞬態服務 (Transient)。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    template<typename TService>
    ServiceCollection& AddTransient() {
        return AddTransient<TService, TService>();
    }

    /// <summary>
    /// 透過工廠函數註冊瞬態服務 (Transient)。
    /// </summary>
    /// <typeparam name="TService">目標服務型別。</typeparam>
    /// <param name="factory">建立服務之自訂工廠函數。</param>
    /// <returns>ServiceCollection 引用，支援鏈式呼叫。</returns>
    /// <exception cref="std::invalid_argument">當 factory 未持有有效回呼時擲出。</exception>
    template<typename TService>
    ServiceCollection& AddTransient(std::function<std::shared_ptr<TService>(IServiceProvider&)> factory) {
        if (!factory) {
            throw std::invalid_argument("ServiceCollection::AddTransient: factory cannot be null.");
        }
        auto erasedFactory = [factory](IServiceProvider& sp) -> std::shared_ptr<void> {
            return factory(sp);
        };
        RegisterDescriptor(typeid(TService), ServiceLifetime::Transient, erasedFactory, nullptr);
        return *this;
    }

    /// <summary>
    /// 構建並回傳不可變之服務提供者 (ServiceProvider)。
    /// </summary>
    /// <returns>不可變的 IServiceProvider 智慧指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<IServiceProvider> BuildServiceProvider();

private:
    void RegisterDescriptor(const std::type_info& type, ServiceLifetime lifetime, ServiceFactory factory, std::shared_ptr<void> instance);

    std::unordered_map<std::type_index, ServiceDescriptor> m_descriptors;
};

} // namespace ioc
} // namespace cppui
