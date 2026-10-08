#pragma once

#include "compat/compat.hpp"
#include "IServiceProvider.hpp"
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace cppui {
namespace ioc {

/// <summary>
/// 服務生命週期模式
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
    using ServiceFactory = std::function<std::shared_ptr<void>(IServiceProvider&)>;

    struct ServiceDescriptor {
        ServiceLifetime lifetime;
        ServiceFactory factory;
        std::shared_ptr<void> singletonInstance;
    };

    ServiceCollection();
    ~ServiceCollection();

    // 註冊 Singleton
    template<typename TInterface, typename TImplementation>
    ServiceCollection& AddSingleton() {
        auto factory = [](IServiceProvider&) -> std::shared_ptr<void> {
            return std::make_shared<TImplementation>();
        };
        RegisterDescriptor(typeid(TInterface), ServiceLifetime::Singleton, factory, nullptr);
        return *this;
    }

    template<typename TService>
    ServiceCollection& AddSingleton() {
        return AddSingleton<TService, TService>();
    }

    template<typename TService>
    ServiceCollection& AddSingleton(std::shared_ptr<TService> instance) {
        auto factory = [instance](IServiceProvider&) -> std::shared_ptr<void> {
            return instance;
        };
        RegisterDescriptor(typeid(TService), ServiceLifetime::Singleton, factory, instance);
        return *this;
    }

    template<typename TService>
    ServiceCollection& AddSingleton(std::function<std::shared_ptr<TService>(IServiceProvider&)> factory) {
        auto erasedFactory = [factory](IServiceProvider& sp) -> std::shared_ptr<void> {
            return factory(sp);
        };
        RegisterDescriptor(typeid(TService), ServiceLifetime::Singleton, erasedFactory, nullptr);
        return *this;
    }

    // 註冊 Transient
    template<typename TInterface, typename TImplementation>
    ServiceCollection& AddTransient() {
        auto factory = [](IServiceProvider&) -> std::shared_ptr<void> {
            return std::make_shared<TImplementation>();
        };
        RegisterDescriptor(typeid(TInterface), ServiceLifetime::Transient, factory, nullptr);
        return *this;
    }

    template<typename TService>
    ServiceCollection& AddTransient() {
        return AddTransient<TService, TService>();
    }

    template<typename TService>
    ServiceCollection& AddTransient(std::function<std::shared_ptr<TService>(IServiceProvider&)> factory) {
        auto erasedFactory = [factory](IServiceProvider& sp) -> std::shared_ptr<void> {
            return factory(sp);
        };
        RegisterDescriptor(typeid(TService), ServiceLifetime::Transient, erasedFactory, nullptr);
        return *this;
    }

    /// <summary>
    /// 構建並回傳不可變的服務提供者
    /// </summary>
    COMPAT_NODISCARD std::shared_ptr<IServiceProvider> BuildServiceProvider();

private:
    void RegisterDescriptor(const std::type_info& type, ServiceLifetime lifetime, ServiceFactory factory, std::shared_ptr<void> instance);

    std::unordered_map<std::type_index, ServiceDescriptor> m_descriptors;
};

} // namespace ioc
} // namespace cppui
