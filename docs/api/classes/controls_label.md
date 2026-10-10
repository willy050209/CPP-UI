# Label 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Label.hpp`](../../../include/ui/controls/Label.hpp)
- **繼承關係**: 繼承自 [`controls::ContentControl`](controls_contentcontrol.md)

---

## 概述

`Label` 是文字或豐富內容標籤控制項，繼承自 `ContentControl`。除了具備承載純文字或自訂子視覺節點的能力外，亦支援目標控制項焦點導向關聯（Target Control Association，對標 WPF `Label.Target`）。
當使用者點擊標籤時，若已配置關聯之 `Target` 控制項，`Label` 將自動將輸入焦點轉移至該目標控制項。

---

## 建構子

- `Label()`: 建立預設空白標籤控制項。
- `explicit Label(compat::string_view text)`: 建立承載指定文字內容的標籤控制項。
- `explicit Label(std::shared_ptr<core::UIElement> content)`: 建立承載指定自訂子視覺節點內容的標籤控制項。若 `content == nullptr` 則擲出 `std::invalid_argument`。

---

## 核心成員方法

### 目標控制項焦點導向 (Target)
- `COMPAT_NODISCARD Control* GetTarget() const noexcept`: 取得目前關聯之目標控制項指標。若未設定則回傳 `nullptr`。
- `void SetTarget(Control* target) noexcept`: 設定關聯之目標控制項指標。當使用者以滑鼠點擊此 `Label` 時，若 `target != nullptr`，將自動呼叫 `target->Focus()` 將鍵盤輸入焦點導向該目標控制項。

---

## Fluent DSL 支援

可透過 `#include "ui/dsl/Fluent.hpp"` 鏈式建立與綁定：

```cpp
using namespace cppui::dsl;

auto nameTextBox = TextBox("請輸入姓名");
auto nameLabel = Label("姓名:")
    .Target(nameTextBox.get());
```
