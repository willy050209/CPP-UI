# 格式化、終端輸出與數值解析 (compat::format, print & parse)

定義於標頭檔 [`include/compat/compat.hpp`](file:///D:/P/CPP/CPPUI/include/compat/compat.hpp)。  
所屬命名空間：`compat`。

---

## 1. 簡介 (Description)

在傳統 C++ 開發中，終端除錯與字串排版長期存在兩大痛點：
1. **Windows 終端亂碼問題**：Windows 繁體中文終端預設編碼為 CP950，使用 `std::cout` 或 `printf` 印出 UTF-8 中文字串時必定產生嚴重亂碼。
2. **型別不安全與低效**：`sprintf` 缺少型別檢查易觸發 Buffer Overflow，而 `std::stringstream` 則存在大量暫存記憶體分配。

`compat` 導入了三大標準化設施：
* **`compat::format`**：完全對齊 **ISO C++20 `std::format`** 規範，提供型別安全且高效的字串格式化。
* **`compat::print` / `compat::println`**：完全對齊 **ISO C++23 `std::print`** 規範，並在 Windows 底層採用 `WriteConsoleW` 直寫技術，徹底終結亂碼。
* **`compat::parse<T>` / `from_chars`**：完全對齊 **ISO C++17 `std::from_chars`**，提供零動態配置的高速數字解析。

---

## 2. 格式化規範語法 (`compat::format`)

格式規格遵循標準微語法：
```text
[[fill]align][sign][#][0][width][.precision][type]
```

### 對齊與欄寬 (Alignment & Width)
* `<`：靠左對齊。
* `>`：靠右對齊。
* `^`：置中對齊。
* `fill`：自訂填充字元（預設為空白）。

```cpp
#include "compat/compat.hpp"

void FormatExamples() {
    // 置中對齊，總寬度 10，填充 '*'
    std::string s1 = compat::format("{:*^10}", "CPPUI"); // 輸出: "**CPPUI***"

    // 靠右對齊整數，補零
    std::string s2 = compat::format("{:08d}", 42);       // 輸出: "00000042"

    // 浮點數保留 2 位小數
    std::string s3 = compat::format("{:.2f}", 3.14159);  // 輸出: "3.14"
}
```

### 東亞字符寬度 (East Asian Width / UAX #11)
`compat::format` 具備完整的 Unicode 東亞寬度感知：繁體中文字元自動判定佔用 2 個欄位寬度，使得表格對齊排版在混合中英文時依然能保持工整。

---

## 3. 終端輸出：`compat::print` 與 `compat::println`

### 3.1 Windows UTF-8 直寫技術 (Bypass ANSI Code Page)
在 Windows 平台上，`compat::print` 會主動偵測標準輸出是否為終端主控台 Handle：
* 若為主控台，內部**繞過 C Runtime 與系統預設字碼頁 (CP950)**，直接將 UTF-8 解碼並透過 Win32 API `WriteConsoleW` 輸出寬字元。
* 任何繁體中文、日文、特殊符號均能清晰、無亂碼地呈現於 Windows 終端中。
* 若輸出被重導向至檔案或管道，則自動退回標準位元組串流。

### 3.2 使用範例
```cpp
#include "compat/compat.hpp"

void PrintExamples() {
    // 不換行輸出
    compat::print("處理中: [{:3d}%]", 75);

    // 自動換行輸出
    compat::println("任務完成！耗時: {:.3f} 秒", 0.125);
}
```

---

## 4. 數值解析：`compat::parse<T>` 與 `from_chars`

提供比 `std::stoi` / `std::atof` 快上數倍、且**無任何堆積配置、無異常拋出**的高速解析原語：

```cpp
#include "compat/compat.hpp"

void ParseExamples() {
    // 1. 高階 parse 介面 (回傳 optional<T>)
    compat::optional<int> num = compat::parse<int>("12345");
    if (num) {
        compat::println("解析整數: {}", *num);
    }

    // 2. 底層 from_chars (支援 2~36 進位)
    const char hexStr[] = "1A3F";
    int hexVal = 0;
    auto [ptr, ec] = compat::from_chars(hexStr, hexStr + 4, hexVal, 16);
    if (ec == std::errc{}) {
        compat::println("16 進位解析數值: {}", hexVal); // 6719
    }
}
```

* **安全保證**：當解析失敗時，傳入的輸出目標數值**保證絕不被修改**，防止髒數據殘留。

---

## 請參閱 (See Also)
* [`compat` 核心相容層總覽](compat.md)
* [`compat::views` 視圖適配器手冊](compat_views.md)
* [代碼規範與向下相容標準](../../specs/02_coding_standards_compat.md)
