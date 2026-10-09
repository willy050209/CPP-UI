# Dispatcher 類別 (cppui::app::Dispatcher)

提供執行緒安全的非同步任務佇列，支援背景工作執行緒將回呼動作安全投遞至 UI 主執行緒消費執行（對標 WPF `System.Windows.Threading.Dispatcher`）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::app`](../namespaces/cppui_app.md)
* **標頭檔**：`#include <ui/app/Dispatcher.hpp>`
* **模組**：`CPPUI Application Framework`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace app {

class Dispatcher {
public:
    Dispatcher();
    ~Dispatcher() = default;

    // 禁止拷貝語意
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    // 非同步任務派發
    void InvokeAsync(std::function<void()> action);
    void Post(std::function<void()> action);

    // 執行緒親和性檢查
    COMPAT_NODISCARD bool CheckAccess() const noexcept;

    // 佇列任務執行 (限主執行緒)
    void DrainTasks();

    // 主執行緒 ID 配置
    void SetMainThreadId(std::thread::id threadId) noexcept;

    // 清空佇列
    void Clear();
};

} // namespace app
} // namespace cppui
```

---

## 成員方法說明 (Member Functions)

| 方法名稱 | 存取層級 | 執行緒安全性 | 說明 |
| :--- | :--- | :--- | :--- |
| `Dispatcher()` | `public` | 安全 | 預設建構子，自動將建構當下之執行緒標記為關聯主執行緒。 |
| `InvokeAsync(action)` | `public` | 執行緒安全 | 將非同步操作推入任務佇列，並呼叫底層視窗 `WakeUp()` 立即喚醒 UI 訊息泵。 |
| `Post(action)` | `public` | 執行緒安全 | `InvokeAsync` 之簡明語意別名，相容於標準訊息派發模型。 |
| `CheckAccess()` | `public` | 執行緒安全 | 判定當前呼叫端執行緒是否為關聯的 UI 主執行緒。若是傳回 `true`，背景執行緒傳回 `false`。 |
| `DrainTasks()` | `public` | 僅限主執行緒 | 擷取佇列中所有待執行任務並依序執行；具備防禦性例外處理保護，單一任務異常不中斷主迴圈。 |
| `SetMainThreadId(id)` | `public` | 執行緒安全 | 手動更新關聯之 UI 主執行緒 ID（通常由 `Application::Run` 自動設定）。 |
| `Clear()` | `public` | 執行緒安全 | 清空佇列中所有尚未執行的非同步動作。 |

---

## 使用範例 (Examples)

### 1. 背景執行緒投遞更新至 UI

```cpp
#include "ui/app/Application.hpp"
#include <thread>

void RunLongTask() {
    std::thread([=]() {
        // 耗時資料運算或網路請求
        int result = PerformCalculation();

        // 透過 Dispatcher 將結果派發回 UI 主執行緒安全更新
        cppui::app::Application::Post([=]() {
            myLabel->SetText("計算完成: " + compat::to_string(result));
        });
    }).detach();
}
```

### 2. 手動檢查執行緒親和性 (Thread Affinity)

```cpp
void UpdateData() {
    if (!cppui::app::Application::CheckAccess()) {
        // 當前位於背景執行緒，自動封送至主執行緒
        cppui::app::Application::Post([]() {
            UpdateData();
        });
        return;
    }

    // 當前已處於 UI 主執行緒，安全直接操作控制項
    myTextBox->SetText("主執行緒安全存取");
}
```

---

## 相關參考 (See Also)

* [`Application`](app_application.md) - 全域應用程式中樞與訊息迴圈調度。
* [`IPlatformWindow`](pal_iplatformwindow.md) - 底層原生視窗抽象介面與 `WakeUp()` 喚醒機制。
* [`ViewModelBase`](mvvm_viewmodelbase.md) - 具備自動執行緒封送 (Thread Marshalling) 之 ViewModel 基底類別。
