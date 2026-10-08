# cppui::reactive 命名空間

- **標頭檔**: [`include/ui/reactive/Signal.hpp`](../../../include/ui/reactive/Signal.hpp)

---

## 概述

`cppui::reactive` 命名空間提供 CPPUI 之響應式資料流與狀態管理機制。藉由型別安全、零外部相依之 `Signal<T>` 物件，在資料數值改變時自動通知訂閱者與驅動 UI 視圖刷新，達成資料與介面的解耦與單向/雙向數據綁定。

---

## 包含型別

| 型別名稱 | 說明 |
| :--- | :--- |
| [`Signal<T>`](../classes/reactive_signal.md) | 響應式屬性狀態信號樣板，支援值變更推播與控制項弱引用綁定。 |
