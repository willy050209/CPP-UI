# ICommand 介面

- **命名空間**: `cppui::mvvm`
- **標頭檔**: `include/ui/mvvm/ICommand.hpp`

---

## 概述

`ICommand` 為 CPPUI MVVM 架構中命令模式（Command Pattern）的核心抽象介面（對標 WPF `System.Windows.Input.ICommand`）。

它將使用者的互動動作（如按鈕點擊）與具體業務邏輯完全解耦，並透過 `CanExecute` 與 `CanExecuteChanged` 提供動態的執行條件校驗與 UI 控制項狀態聯動（如按鈕自動反灰啟用/禁用）。

---

## 介面宣告

```cpp
namespace cppui {
namespace mvvm {

class ICommand {
public:
    virtual ~ICommand() = default;

    using CanExecuteChangedHandler = std::function<void()>;

    // 檢查當前命令是否處於可執行狀態
    COMPAT_NODISCARD virtual bool CanExecute(void* parameter = nullptr) const = 0;

    // 執行命令邏輯
    virtual void Execute(void* parameter = nullptr) = 0;

    // 註冊命令可執行狀態變更回呼
    virtual void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) = 0;

    // 廣播可執行狀態變更通知
    virtual void NotifyCanExecuteChanged() = 0;
};

} // namespace mvvm
} // namespace cppui
```

---

## 方法說明

### `CanExecute(void* parameter)`
- **參數**: `parameter`（可選傳入之參數指標）。
- **回傳值**: `bool`。若可執行回傳 `true`；否則回傳 `false`。
- **說明**: 當綁定至按鈕時，`Button` 會依據此方法回傳值自動同步 `IsEnabled()` 狀態。

### `Execute(void* parameter)`
- **參數**: `parameter`（可選傳入之參數指標）。
- **說明**: 觸發具體業務動作。實作應在內部再次調用 `CanExecute` 確保條件滿足。

### `NotifyCanExecuteChanged()`
- **說明**: 當 ViewModel 中的條件發生改變時（例如計數器達到上限），呼叫此方法會通知所有綁定該命令的 UI 元件重新計算啟用/禁用狀態。

---

## 衍生實作

- 輕量委派實作：參見 [`RelayCommand`](mvvm_relaycommand.md)。
