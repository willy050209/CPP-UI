# 期望值、可空值與錯誤處理 (compat::expected & optional)

定義於標頭檔 [`include/compat/Compat.hpp`](../../../include/compat/Compat.hpp)。  
所屬命名空間：`compat`。

---

## 1. 簡介 (Description)

現代高效能 UI 框架嚴格限制 C++ 例外拋出（`throw / catch`），因例外機制會產生展開棧（Stack Unwinding）開銷、編譯器膨脹與不確定的延遲峰值。

`compat` 提供了兩大**現代詞彙型別 (Vocabulary Types)**，用以完全取代舊式 C 語言錯誤碼與異常機制：
1. **`compat::expected<T, E>`**：遵循 **ISO C++23 `std::expected`** 標準，封裝成功值 `T` 或失敗錯誤物件 `E`，支援現代函數式鏈式呼叫 (Monadic Operations)。
2. **`compat::optional<T>`**：遵循 **ISO C++17 `std::optional`** 標準，安全表述「可能存在或不存在」的數值。

---

## 2. `compat::expected<T, E>` 規格

### 2.1 宣告語法
```cpp
namespace compat {
    template <typename E>
    class unexpected;

    template <typename T, typename E>
    class expected;

    // void 特化版（無成功返回值，僅表示成功與否）
    template <typename E>
    class expected<void, E>;
}
```

### 2.2 三階層降階儲存架構 (Triple-Tier Storage)
* **C++23 模式**：直接別名原生 `std::expected<T, E>`。
* **C++17 模式**：基於 `std::variant<T, unexpected<E>>` 實作，享有移動語意與異常保證。
* **C++11/14 模式**：自研**無限制聯合體（Unrestricted Union）**。當 `T` 與 `E` 為平凡型別時，`expected` 亦自動成為平凡型別，杜絕額外空間與構造開銷。

### 2.3 狀態與生命週期不變量
* 物件永遠處於合法狀態：非持有 `T` 即持有 `E`，**不存在任何未定義的中間態或 `valueless_by_exception`**。
* 狀態切換時採用安全的 `reinit_expected` 機制，確保轉移過程遭遇異常時可自動復原或安全終止。

---

## 3. 函數式 Monadic 操作 (Chaining API)

`compat::expected` 支援現代 C++23 函數式串聯，免除層層 `if-else` 的巢狀階梯：

| Monadic 方法 | 觸發條件 | 作用與回傳 |
| :--- | :--- | :--- |
| `.and_then(func)` | 持有數值 `T` 時呼叫 | 傳遞數值給 `func`，將回傳之新 `expected` 展平串聯。 |
| `.transform(func)` | 持有數值 `T` 時呼叫 | 傳遞數值給 `func`，將計算結果包裝為相同錯誤型別的 `expected`。 |
| `.or_else(func)` | 持有錯誤 `E` 時呼叫 | 傳遞錯誤值給 `func` 進行降級修復或補償處理。 |
| `.transform_error(func)` | 持有錯誤 `E` 時呼叫 | 映射轉換錯誤型別（例如將底層 I/O 錯誤轉為 UI 呈現字串）。 |

---

## 4. UI 實戰範例 (Practical UI Examples)

### 範例 1：資源加載管線 (`expected` Monadic 鏈式呼叫)
```cpp
#include "compat/Compat.hpp"

enum class FontError { FileNotFound, CorruptedHeader, UnsupportedFormat };

struct FontData {
    std::vector<uint8_t> rawBytes;
};

struct LoadedFont {
    compat::string_view familyName;
    int glyphCount;
};

// 階段 1：讀取磁碟
compat::expected<FontData, FontError> ReadFontFile(compat::string_view path) {
    if (path.empty()) return compat::unexpected<FontError>(FontError::FileNotFound);
    return FontData{ { 0x00, 0x01, 0x00, 0x00 } };
}

// 階段 2：解析字型表
compat::expected<LoadedFont, FontError> ParseFont(const FontData& data) {
    if (data.rawBytes.size() < 4) return compat::unexpected<FontError>(FontError::CorruptedHeader);
    return LoadedFont{ "Segoe UI", 256 };
}

void LoadUIFont() {
    // 函數式管線鏈式串聯！
    auto fontResult = ReadFontFile("fonts/main.ttf")
        .and_then(ParseFont);

    if (fontResult.has_value()) {
        compat::println("字型加載成功: {}, 字符數: {}", 
            fontResult->familyName, fontResult->glyphCount);
    } else {
        compat::println("字型加載失敗，錯誤代碼: {}", static_cast<int>(fontResult.error()));
    }
}
```

### 範例 2：可空命中測試 (`optional`)
```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

struct VisualElement {
    int id;
    cppui::core::Rect bounds;
};

// 命中測試回傳 optional<VisualElement>
compat::optional<VisualElement> HitTest(const std::vector<VisualElement>& elements, 
                                        cppui::core::Point mousePt) {
    for (const auto& elem : elements) {
        if (elem.bounds.Contains(mousePt)) {
            return elem; // 命中
        }
    }
    return compat::nullopt; // 未命中任何元件
}
```

---

## 請參閱 (See Also)
* [`compat` 核心相容層總覽](compat.md)
* [`compat::views` 視圖適配器手冊](compat_views.md)
* [代碼規範與向下相容標準](../../specs/02_coding_standards_compat.md)
