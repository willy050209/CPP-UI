# IServiceProvider 介面

- **命名空間**: `cppui::ioc`
- **標頭檔**: `include/ui/ioc/IServiceProvider.hpp`

---

## 概述

`IServiceProvider` 為 CPPUI 控制反轉（IoC）體系中的唯讀服務定位核心介面（對標 .NET `System.IServiceProvider`）。它提供強型別樣板函式 `GetService<T>()` 與 `GetRequiredService<T>()`，用於在執行期依型別獲取已註冊之相依服務實例。

---

## 介面宣告

```cpp
namespace cppui {
namespace ioc {

class IServiceProvider {
public:
    virtual ~IServiceProvider() = default;

    // 底層擦除型別查詢介面
    COMPAT_NODISCARD virtual std::shared_ptr<void> GetService(const std::type_info& type) = 0;

    // 強型別安全樣板方法
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetService();

    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetRequiredService();
};

} // namespace ioc
} // namespace cppui
```

---

## 方法說明

### `GetService<T>()`
- **樣板參數**: `T`（目標介面或實作型別）。
- **回傳值**: `std::shared_ptr<T>`。若該型別未註冊則回傳 `nullptr`。
- **說明**: 適用於可選依賴項目的查詢。

### `GetRequiredService<T>()`
- **樣板參數**: `T`（必須存在的相依服務型別）。
- **回傳值**: `std::shared_ptr<T>`。若該型別未註冊，則回傳 `nullptr` 並於控制台輸出警告紀錄。
- **說明**: 適用於關鍵相依項目的強烈契約查詢。

---

## 使用範例

```cpp
std::shared_ptr<cppui::ioc::IServiceProvider> sp = /* 由 ServiceCollection 建置 */;

// 解析必要之日誌服務
auto logger = sp->GetRequiredService<ILogger>();
if (logger) {
    logger->Log("應用程式正常初始化");
}
```
