#include "compat/compat.hpp"
#include "ui/ioc/IServiceProvider.hpp"
#include "ui/ioc/ServiceCollection.hpp"
#include <cassert>
#include <string>
#include <thread>
#include <vector>

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

void TestGetRequiredServiceBehavior() {
    compat::println("[Test] 測試 GetRequiredService 成功與失敗行為...");
    ServiceCollection services;
    services.AddSingleton<ILogger, ConsoleLogger>();
    auto provider = services.BuildServiceProvider();

    // 成功案例
    auto logger = provider->GetRequiredService<ILogger>();
    assert(logger != nullptr);

    // 失敗案例：未註冊服務應安全回傳 nullptr（並於控制台輸出警告紀錄，不崩潰）
    auto unknown = provider->GetRequiredService<IUserService>();
    assert(unknown == nullptr);

    compat::println("  ✔ GetRequiredService 邊界驗證通過！");
}

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual std::string Query() = 0;
};

class SqlDatabase : public IDatabase {
public:
    std::string Query() override { return "SELECT *"; }
};

class IRepository {
public:
    virtual ~IRepository() = default;
    virtual std::string GetData() = 0;
};

class DataRepository : public IRepository {
public:
    explicit DataRepository(std::shared_ptr<IDatabase> db) : m_db(db) {}
    std::string GetData() override { return m_db ? m_db->Query() : ""; }
private:
    std::shared_ptr<IDatabase> m_db;
};

class IAppService {
public:
    virtual ~IAppService() = default;
    virtual std::string Execute() = 0;
};

class AppService : public IAppService {
public:
    explicit AppService(std::shared_ptr<IRepository> repo) : m_repo(repo) {}
    std::string Execute() override { return m_repo ? m_repo->GetData() : ""; }
private:
    std::shared_ptr<IRepository> m_repo;
};

void TestMultiLevelDependencyInjection() {
    compat::println("[Test] 測試多層相依性注入級聯解析 (A -> B -> C)...");
    ServiceCollection services;
    services.AddSingleton<IDatabase, SqlDatabase>();
    services.AddTransient<IRepository>([](IServiceProvider& sp) {
        return std::make_shared<DataRepository>(sp.GetRequiredService<IDatabase>());
    });
    services.AddTransient<IAppService>([](IServiceProvider& sp) {
        return std::make_shared<AppService>(sp.GetRequiredService<IRepository>());
    });

    auto provider = services.BuildServiceProvider();
    auto appSvc = provider->GetRequiredService<IAppService>();

    assert(appSvc != nullptr);
    assert(appSvc->Execute() == "SELECT *");

    compat::println("  ✔ 多層相依性注入解析驗證通過！");
}

void TestThreadSafeSingleton() {
    compat::println("[Test] 測試 Singleton 於多執行緒環境下並發解析之安全保護...");
    ServiceCollection services;
    services.AddSingleton<ILogger, ConsoleLogger>();
    auto provider = services.BuildServiceProvider();

    const int threadCount = 8;
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<ILogger>> resolvedLoggers(threadCount);

    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([&, i]() {
            resolvedLoggers[i] = provider->GetRequiredService<ILogger>();
        });
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // 驗證所有執行緒解析出同一實例指標
    for (int i = 1; i < threadCount; ++i) {
        assert(resolvedLoggers[i] != nullptr);
        assert(resolvedLoggers[i] == resolvedLoggers[0]);
    }

    compat::println("  ✔ Singleton 多執行緒並發解析執行緒安全驗證通過！");
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
    TestGetRequiredServiceBehavior();
    TestMultiLevelDependencyInjection();
    TestThreadSafeSingleton();

    compat::println("==================================================");
    compat::println(" 全部 8 項 IoC 容器單元測試通過！");
    compat::println("==================================================");
    return 0;
}
