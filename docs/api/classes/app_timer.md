# Timer 類別

- **命名空間**: `cppui::app` (亦支援 `cppui::controls::Timer` 別名)
- **標頭檔**: [`include/ui/app/Timer.hpp`](../../../include/ui/app/Timer.hpp) / [`include/ui/controls/Timer.hpp`](../../../include/ui/controls/Timer.hpp)
- **實作檔**: `src/app/Timer.cpp`

---

## 概述

`Timer` 是在使用者定義的時間間隔內觸發週期性事件之元件，對標微軟 **`System.Windows.Forms.Timer`**。
`Timer` 具備以下核心特性：
- **UI 執行緒親和性 (UI Thread Affinity)**：所有 `Tick` 回呼事件一律排隊封送至 UI 主執行緒（透過 `Dispatcher` / `Application::Post`）執行，確保事件處理常式可直接安全存取與更新 UI 控制項、幾何屬性及表單，無需手動執行跨執行緒封送。
- **防止堆疊與合流 (Message Coalescing)**：對標 WinForms `WM_TIMER` 特性，若 UI 執行緒繁忙未及時處理 Tick，不會造成事件無限制排隊堆疊；前一次事件完成後才繼續派發下一週期事件。
- **確定性單元測試支援**：提供 `PerformTick()` 立即手動引發事件，免除異步時間差與不穩定的 sleep 測試。
- **高解析度背景排程單例 (`TimerScheduler`)**：後台採用高精度 `std::chrono::steady_clock` 與條件變數休眠，在毫秒級別精準計時，並於計時器全部停止時自動休眠或釋放線程。

---

## 建構子與解構子

```cpp
Timer();
explicit Timer(int32_t intervalMs);
~Timer();
```

- `Timer()`: 建立預設計時器（預設間隔 100 毫秒，初始處於未啟用狀態）。
- `explicit Timer(int32_t intervalMs)`: 建立指定間隔之計時器（初始未啟用）。
  - 若 `intervalMs <= 0` 立即擲出 `std::invalid_argument`。
- `~Timer()`: 解構子，自動呼叫 `Stop()` 釋放內部排程註冊，保證 RAII 生命週期安全。

---

## 核心成員方法

### 間隔與啟用控制
- `COMPAT_NODISCARD int32_t GetInterval() const noexcept`: 取得目前計時期間隔時間（毫秒）。
- `void SetInterval(int32_t intervalMs)`: 設定計時期間隔時間（毫秒）。
  - 若計時器目前處於啟用狀態，將自動以新週期重設下一次到期時間。
  - **例外**: 若 `intervalMs <= 0` 立即擲出 `std::invalid_argument`。
- `COMPAT_NODISCARD bool IsEnabled() const noexcept`: 查詢計時器是否處於啟用執行狀態。
- `void SetEnabled(bool enabled)`: 設定計時器之啟用狀態。設定為 `true` 等同於呼叫 `Start()`；設定為 `false` 等同於呼叫 `Stop()`。
- `void Start()`: 啟動計時器（對標 WinForms `Timer.Start()`）。
- `void Stop()`: 停止計時器（對標 WinForms `Timer.Stop()`）。
- `void Restart()`: 重新啟動計時器（先停止，重設倒數後重新啟動）。

### 程式化強制觸發
- `void PerformTick()`: 立即以程式碼強制同步觸發一次 `Tick` 事件。此呼叫會忽略 `IsEnabled()` 狀態強制執行所有已註冊的處理常式，極度適合單元測試或強制畫面刷新。

### 自訂標籤 (Tag)
- `COMPAT_NODISCARD void* GetTag() const noexcept`: 取得使用者自訂資料指標。
- `void SetTag(void* tag) noexcept`: 設定使用者自訂資料指標。

### 事件監聽與管理
- `void OnTick(TickHandler handler)`: 註冊無參數之 Tick 事件處理常式 `std::function<void()>`。
  - **例外**: 若 `handler` 為空立即擲出 `std::invalid_argument`。
- `void OnTick(TickWithSenderHandler handler)`: 註冊接收 Timer 引用之 Tick 事件處理常式 `std::function<void(Timer&)>`。
  - **例外**: 若 `handler` 為空立即擲出 `std::invalid_argument`。
- `void AddTickHandler(TickHandler handler)`: 註冊無參數之 Tick 事件處理常式（對標 WinForms `timer.Tick += handler`）。
- `void AddTickHandler(TickWithSenderHandler handler)`: 註冊帶發送者引用之 Tick 事件處理常式。
- `void ClearTickHandlers() noexcept`: 清除所有已註冊之 Tick 事件處理常式。
- `COMPAT_NODISCARD size_t GetTickHandlerCount() const noexcept`: 取得目前已註冊之 Tick 處理常式數量。

---

## 範例程式碼 (Usage Examples)

### 1. 表單內週期性計時與 UI 更新

```cpp
#include "ui/app/Timer.hpp"
#include "ui/app/Form.hpp"
#include "ui/controls/TextBlock.hpp"

class ClockForm : public cppui::app::Form {
public:
    ClockForm() {
        SetTitle("電子時鐘");
        m_label = std::make_shared<cppui::controls::TextBlock>("0 秒");
        SetContent(m_label);

        // 設定每秒 (1000ms) 觸發一次
        m_timer.SetInterval(1000);
        m_timer.OnTick([this]() {
            m_seconds++;
            m_label->SetText(std::to_string(m_seconds) + " 秒");
        });
        m_timer.Start();
    }

private:
    std::shared_ptr<cppui::controls::TextBlock> m_label;
    cppui::app::Timer m_timer;
    int32_t m_seconds = 0;
};
```

### 2. 接收 Sender 引用的計時器控制

```cpp
#include "ui/app/Timer.hpp"

cppui::app::Timer countdownTimer(500);
int32_t count = 10;

countdownTimer.OnTick([&count](cppui::app::Timer& sender) {
    count--;
    if (count <= 0) {
        // 到達終點，自動停止計時器
        sender.Stop();
    }
});
countdownTimer.Start();
```
