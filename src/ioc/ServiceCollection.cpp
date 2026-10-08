#include "ui/ioc/ServiceCollection.hpp"
#include <mutex>
#include <utility>

namespace cppui {
namespace ioc {

namespace {

class ServiceProviderImpl : public IServiceProvider {
public:
    explicit ServiceProviderImpl(std::unordered_map<std::type_index, ServiceCollection::ServiceDescriptor> descriptors)
        : m_descriptors(std::move(descriptors)) {}

    COMPAT_NODISCARD std::shared_ptr<void> GetService(const std::type_info& type) override {
        std::type_index idx(type);
        auto it = m_descriptors.find(idx);
        if (it == m_descriptors.end()) {
            return nullptr;
        }

        auto& desc = it->second;
        if (desc.lifetime == ServiceLifetime::Singleton) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!desc.singletonInstance && desc.factory) {
                desc.singletonInstance = desc.factory(*this);
            }
            return desc.singletonInstance;
        } else {
            // Transient 每次重新建構
            if (desc.factory) {
                return desc.factory(*this);
            }
            return nullptr;
        }
    }

private:
    std::unordered_map<std::type_index, ServiceCollection::ServiceDescriptor> m_descriptors;
    std::mutex m_mutex;
};

} // anonymous namespace

ServiceCollection::ServiceCollection() = default;
ServiceCollection::~ServiceCollection() = default;

void ServiceCollection::RegisterDescriptor(const std::type_info& type, ServiceLifetime lifetime, ServiceFactory factory, std::shared_ptr<void> instance) {
    ServiceDescriptor desc;
    desc.lifetime = lifetime;
    desc.factory = std::move(factory);
    desc.singletonInstance = std::move(instance);
    m_descriptors[std::type_index(type)] = std::move(desc);
}

std::shared_ptr<IServiceProvider> ServiceCollection::BuildServiceProvider() {
    return std::make_shared<ServiceProviderImpl>(m_descriptors);
}

} // namespace ioc
} // namespace cppui
