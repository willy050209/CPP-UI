# ServiceCollection 類別 (cppui::ioc::ServiceCollection)

依賴反轉 (IoC / Dependency Injection) 容器中樞，提供服務工廠與實例的註冊功能，並構建執行期型別安全的 `IServiceProvider`（對標 .NET `Microsoft.Extensions.DependencyInjection`）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::ioc`](../namespaces/cppui_ioc.md)
* **標頭檔**：`#include <ui/ioc/ServiceCollection.hpp>`
* **介面標頭檔**：`#include <ui/ioc/IServiceProvider.hpp>`
* **模組**：`CPPUI IoC Container`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace ioc {

enum class ServiceLifetime {
    Transient, // 每次請求皆建立新實例
    Singleton  // 應用程式內僅建立單一共享實例
};

class IServiceProvider {
public:
    virtual ~IServiceProvider() = default;
    COMPAT_NODISCARD virtual std::shared_ptr<void> GetService(const std::type_info& type) = 0;

    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetService();

    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetRequiredService();
};

class ServiceCollection {
public:
    ServiceCollection();
    ~ServiceCollection();

    // 註冊 Singleton
    template<typename TInterface, typename TImplementation>
    ServiceCollection& AddSingleton();

    template<typename TService>
    ServiceCollection& AddSingleton();

    template<typename TService>
    ServiceCollection& AddSingleton(std::shared_ptr<TService> instance);

    template<typename TService>
    ServiceCollection& AddSingleton(std::function<std::shared_ptr<TService>(IServiceProvider&)> factory);

    // 註冊 Transient
    template<typename TInterface, typename TImplementation>
    ServiceCollection& AddTransient();

    template<typename TService>
    ServiceCollection& AddTransient();

    template<typename TService>
    ServiceCollection& AddTransient(std::function<std::shared_ptr<TService>(IServiceProvider&)> factory);

    // 構建服務提供者
    COMPAT_NODISCARD std::shared_ptr<IServiceProvider> BuildServiceProvider();
};

} // namespace ioc
} // namespace cppui
```

---

## 簡介 (Description)

`ServiceCollection` 是現代 C++ 輕量型別安全的依賴反轉容器：
1. **多重生命週期模式**：
   - `Singleton`：執行期初次請求時延遲建構，並在應用程式生命週期內保持單一共享實例。
   - `Transient`：每次解析皆重新建構全新實例，適用於無狀態服務與暫態 ViewModel。
2. **多種註冊語義**：支援「介面到實作映射」、「既有物件實例註冊」、「Lambda 工廠解析」（可於工廠內呼叫 `sp.GetRequiredService<T>()` 達成構造函數相依注入）。
3. **不可變服務提供者**：呼叫 `BuildServiceProvider()` 後產生只讀的 `IServiceProvider`，線程安全解析依賴。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/ioc/ServiceCollection.hpp"

class IRepository {
public:
    virtual ~IRepository() = default;
    virtual void Save() = 0;
};

class SqlRepository : public IRepository {
public:
    void Save() override { compat::println("儲存資料至資料庫"); }
};

void TestIoC() {
    using namespace cppui::ioc;

    ServiceCollection services;
    services.AddSingleton<IRepository, SqlRepository>();

    auto provider = services.BuildServiceProvider();
    auto repo = provider->GetRequiredService<IRepository>();
    repo->Save();
}
```

---

## 請參閱 (See Also)
* [`Application` 類別](app_application.md)
* [`ViewModelBase` 類別](mvvm_viewmodelbase.md)
* [`現代 MVVM 與 IoC 架構指南`](../guides/mvvm_and_ioc_architecture.md)
