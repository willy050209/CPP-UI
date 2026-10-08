# cppui::ioc 命名空間

- **標頭檔目錄**: `include/ui/ioc/`

---

## 概述

`cppui::ioc` 命名空間提供 CPPUI 框架的原生純 C++ 控制反轉（Inversion of Control）與依賴注入（Dependency Injection）容器。本設計完全無外部第三方庫依賴，透過 C++ 執行期型別資訊（`std::type_index`）實現強型別的服務註冊與解析，並內建執行緒安全的 Singleton 單例實例快取與 Transient 瞬態工廠委派支援。

---

## 包含型別與介面

| 型別名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| [`IServiceProvider`](file:///D:/P/CPP/CPPUI/docs/api/classes/ioc_iserviceprovider.md) | `IServiceProvider.hpp` | 服務提供者核心介面，定義 `GetService<T>()` 與 `GetRequiredService<T>()` 解析合約。 |
| [`ServiceCollection`](file:///D:/P/CPP/CPPUI/docs/api/classes/ioc_servicecollection.md) | `ServiceCollection.hpp` | 服務集合註冊表，支援鏈式註冊 Transient、Singleton、物件實例及相依工廠 Lambda。 |
| [`ServiceLifetime`](file:///D:/P/CPP/CPPUI/docs/api/classes/ioc_servicecollection.md#servicelifetime) | `ServiceCollection.hpp` | 服務生命週期模式列舉 (`Transient`, `Singleton`)。 |

---

## 典型使用模式

```cpp
#include "ui/ioc/ServiceCollection.hpp"
#include <cassert>

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void Log(compat::string_view msg) = 0;
};

class ConsoleLogger : public ILogger {
public:
    void Log(compat::string_view msg) override {
        compat::println("[Log] {}", msg);
    }
};

class OrderService {
public:
    explicit OrderService(std::shared_ptr<ILogger> logger) : m_logger(logger) {}
    void PlaceOrder() {
        if (m_logger) m_logger->Log("Order placed successfully");
    }
private:
    std::shared_ptr<ILogger> m_logger;
};

int main() {
    cppui::ioc::ServiceCollection services;
    services.AddSingleton<ILogger, ConsoleLogger>();
    services.AddTransient<OrderService>([](cppui::ioc::IServiceProvider& sp) {
        return std::make_shared<OrderService>(sp.GetRequiredService<ILogger>());
    });

    auto provider = services.BuildServiceProvider();
    auto orderSvc = provider->GetRequiredService<OrderService>();
    orderSvc->PlaceOrder();
    return 0;
}
```
