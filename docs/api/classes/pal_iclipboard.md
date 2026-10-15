# IClipboard 與 Clipboard 類別

- **命名空間**: `cppui::pal`
- **標頭檔**: [`include/ui/pal/IClipboard.hpp`](../../../include/ui/pal/IClipboard.hpp)

---

## 概述

`IClipboard` 是平台剪貼簿抽象介面，`Clipboard` 則是其全域靜態門面類別。負責與作業系統剪貼簿（Windows Win32 Clipboard、Linux X11 Selection、或通用記憶體備援）進行 UTF-8 純文字的雙向交換，為文字輸入框 [`TextBox`](../controls_textbox.md) 的複製、剪下與貼上提供原生平台橋接。

---

## IClipboard 介面方法

- `COMPAT_NODISCARD virtual bool ContainsText() const = 0`: 查詢剪貼簿目前是否包含純文字。
- `COMPAT_NODISCARD virtual std::string GetText() const = 0`: 讀取剪貼簿純文字內容（UTF-8 編碼）。
- `virtual void SetText(compat::string_view text) = 0`: 寫入純文字至系統剪貼簿。
- `virtual void Clear() = 0`: 清空剪貼簿內容。

---

## Clipboard 靜態門面方法

- `COMPAT_NODISCARD static bool ContainsText()`: 查詢目前系統剪貼簿是否包含文字。
- `COMPAT_NODISCARD static std::string GetText()`: 自系統剪貼簿取得 UTF-8 文字。
- `static void SetText(compat::string_view text)`: 設定純文字至系統剪貼簿。
- `static void Clear()`: 清空系統剪貼簿。
- `static void SetCustomClipboard(std::shared_ptr<IClipboard> customClipboard)`: 注入自訂剪貼簿實作（常用於 Headless 無頭單元測試或 Mock 驗證）。

---

## 使用範例

```cpp
#include "ui/pal/IClipboard.hpp"

// 寫入剪貼簿
cppui::pal::Clipboard::SetText("這是一段複製到系統剪貼簿的文字！");

// 檢查並讀取剪貼簿
if (cppui::pal::Clipboard::ContainsText()) {
    std::string text = cppui::pal::Clipboard::GetText();
    compat::println("剪貼簿內容：{}", text);
}
```
