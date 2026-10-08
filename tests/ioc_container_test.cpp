#include "compat/compat.hpp"
#include "ui/ioc/IServiceProvider.hpp"
#include "ui/ioc/ServiceCollection.hpp"
#include <cassert>
#include <string>

using namespace cppui;
using namespace cppui::ioc;

// 測試介面與實作
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void Log(compat::string_view msg) = 0;
    virtual int GetCount() const = 0;
};

class ConsoleLogger : public ILogger {
public:
    void Log(compat::string_view) override {
        m_count++;
    }
    int GetCount() const override {
        return m_count;
    }
private:
    int m_count = 0;
};

class IUserService {
public:
    virtual ~IUserService() = default;
    virtual std::string GetUserName() = 0;
    virtual std::shared_ptr<ILogger> GetLogger() = 0;
};

class UserServiceImpl : public IUserService {
public:
    explicit UserServiceImpl(std::shared_ptr<ILogger> logger)
        : m_logger(logger) {}

    std::string GetUserName() override {
        if (m_logger) m_logger->Log("Fetching user name");
        return "Alice";
    }

    std::shared_ptr<ILogger> GetLogger() override {
        return m_logger;
    }

private:
    std::shared_ptr<ILogger> m_logger;
};

void TestTransientLifetime() {
    compat::println("[Test] 測試 Transient 瞬態生命週期...");
    ServiceCollection services;
    services.AddTransient<ILogger, ConsoleLogger>();

    auto provider = services.BuildServiceProvider();
    assert(provider != nullptr);

    auto logger1 = provider->GetService<ILogger>();
    auto logger2 = provider->GetService<ILogger>();

    assert(logger1 != nullptr);
    assert(logger2 != nullptr);
    assert(logger1 != logger2); // 瞬態服務每次解析應獲得獨立新實例

    logger1->Log("Test 1");
    assert(logger1->GetCount() == 1);
    assert(logger2->GetCount() == 0); // 各自獨立狀態

    compat::println("  ✔ Transient 生命週期驗證通過！");
}

void TestSingletonLifetime() {
    compat::println("[Test] 測試 Singleton 單例生命週期...");
    ServiceCollection services;
    services.AddSingleton<ILogger, ConsoleLogger>();

    auto provider = services.BuildServiceProvider();
    assert(provider != nullptr);

    auto logger1 = provider->GetService<ILogger>();
    auto logger2 = provider->GetService<ILogger>();

    assert(logger1 != nullptr);
    assert(logger2 != nullptr);
    assert(logger1 == logger2); // 單例服務解析應指向同一記憶體實例

    logger1->Log("Test 1");
    assert(logger2->GetCount() == 1); // 狀態共享

    compat::println("  ✔ Singleton 生命週期驗證通過！");
}

void TestInstanceSingleton() {
    compat::println("[Test] 測試實例單例註冊...");
    auto existingInstance = std::make_shared<ConsoleLogger>();
    existingInstance->Log("Init");

    ServiceCollection services;
    services.AddSingleton<ILogger>(existingInstance);

    auto provider = services.BuildServiceProvider();
    auto resolved = provider->GetService<ILogger>();

    assert(resolved == existingInstance);
    assert(resolved->GetCount() == 1);

    compat::println("  ✔ 實例單例註冊驗證通過！");
}

void TestFactoryRegistrationWithDependency() {
    compat::println("[Test] 測試帶有依賴注入之工廠註冊...");
    ServiceCollection services;
    services.AddSingleton<ILogger, ConsoleLogger>();
    services.AddTransient<IUserService>([](IServiceProvider& sp) -> std::shared_ptr<IUserService> {
        auto logger = sp.GetRequiredService<ILogger>();
        return std::make_shared<UserServiceImpl>(logger);
    });

    auto provider = services.BuildServiceProvider();
    auto userSvc = provider->GetService<IUserService>();

    assert(userSvc != nullptr);
    assert(userSvc->GetUserName() == "Alice");
    assert(userSvc->GetLogger()->GetCount() == 1); // Logger 成功被工廠注入並呼叫

    compat::println("  ✔ 工廠依賴注入解析驗證通過！");
}

void TestUnregisteredService() {
    compat::println("[Test] 測試未註冊服務查詢...");
    ServiceCollection services;
    auto provider = services.BuildServiceProvider();

    auto unknown = provider->GetService<ILogger>();
    assert(unknown == nullptr);

    compat::println("  ✔ 未註冊服務查詢回傳 nullptr 驗證通過！");
}

int main() {
    compat::println("==================================================");
    compat::println(" CPPUI IoC Container TDD Unit Test Suite");
    compat::println("==================================================");

    TestTransientLifetime();
    TestSingletonLifetime();
    TestInstanceSingleton();
    TestFactoryRegistrationWithDependency();
    TestUnregisteredService();

    compat::println("==================================================");
    compat::println(" 全部 5 項 IoC 容器單元測試通過！");
    compat::println("==================================================");
    return 0;
}
