# Signal<T> 類別樣板

- **命名空間**: `cppui::reactive`
- **標頭檔**: [`include/ui/reactive/Signal.hpp`](../../../include/ui/reactive/Signal.hpp)

---

## 概述

`Signal<T>` 是一個現代化零相依、型別安全的輕量響應式狀態信號樣板。支援值變更時自動偵測（`operator!=`）並推播通知至訂閱者，並提供與 UI 元件弱引用（`std::weak_ptr`）生命週期保護的自動單向/雙向數據綁定介面（`Bind`）。

---

## 核心成員方法

### 建構與存取
- `Signal()`: 預設建構函式，初始化為 `T{}`。
- `explicit Signal(T initialValue)`: 指定初值建構函式。
- `COMPAT_NODISCARD const T& Get() const noexcept`: 取得內部儲存之常數參照。
- `COMPAT_NODISCARD const T& operator*() const noexcept`: 解引用運算子，直接存取內部值。
- `COMPAT_NODISCARD const T* operator->() const noexcept`: 成員存取運算子。

### 修改與推播
- `void Set(T newValue)`: 設定新值。若新值與舊值不同，立即觸發所有訂閱之監聽回呼。
- `Signal& operator=(T newValue)`: 賦值運算子捷徑，等同呼叫 `Set(std::move(newValue))`。

### 訂閱與生命週期
- `size_t Subscribe(ChangeListener listener)`:
  訂閱完整變更通知，傳入 `void(const T& newVal, const T& oldVal)`，回傳唯一訂閱 ID。
- `size_t Subscribe(ValueListener listener)`:
  訂閱新值變更通知，傳入 `void(const T& newVal)`。
- `void Unsubscribe(size_t id)`:
  透過訂閱 ID 取消特定監聽回呼。
- `template <typename ControlType, typename Formatter> size_t Bind(std::shared_ptr<ControlType> target, Formatter formatter)`:
  將信號綁定至目標控制項。首次呼叫立即執行一次初始化同步；當信號數值變更時，透過 `std::weak_ptr` 安全確認目標控制項存活並套用更新回呼。
