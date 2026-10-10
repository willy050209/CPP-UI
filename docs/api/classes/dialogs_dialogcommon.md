# 對話框共用型別與工具函式 (DialogCommon)

- **命名空間**: `cppui::dialogs`
- **標頭檔**: `include/ui/dialogs/DialogCommon.hpp`
- **實作檔**: `src/dialogs/DialogCommon.cpp`

---

## 1. DialogUtilities 靜態純函數工具

`DialogUtilities` 包含對話框篩選器解析、萬用字元比對、副檔名補齊與檔案大小字串化等無副作用純函式 (`static` / Pure Functions)。

### 方法宣告

```cpp
class DialogUtilities {
public:
    COMPAT_NODISCARD static std::vector<FileFilterItem> ParseFilterString(compat::string_view filter);
    COMPAT_NODISCARD static bool MatchesPattern(compat::string_view fileName, compat::string_view pattern);
    COMPAT_NODISCARD static bool MatchesFilter(compat::string_view fileName, const FileFilterItem& item);
    COMPAT_NODISCARD static std::string EnsureExtension(compat::string_view filePath, compat::string_view defaultExt);
    COMPAT_NODISCARD static std::string FormatFileSize(uint64_t bytes);
};
```

### 方法詳解

#### `ParseFilterString`
將標準對話框篩選器字串解析為結構化的 `FileFilterItem` 陣列。
- **支援格式**：`"描述文字|樣式清單|描述2|樣式2"`
- **多重副檔名**：使用分號 `;` 分隔，例如 `"Images (*.png;*.jpg)|*.png;*.jpg"`
- **容錯處理**：自動去除前後多餘空白，若格式畸形則安全略過不合法配對。

#### `MatchesPattern`
比對檔名是否符合通配符樣式。
- 支援萬用字元 `*` (匹配 0 或多個任意字元) 與 `?` (匹配 1 個字元)。
- `*.*` 與 `*` 匹配所有檔案。
- 自動執行大小寫不敏感比對（例如 `photo.JPG` 符合 `*.jpg`）。

#### `EnsureExtension`
若輸入路徑結尾無副檔名，自動拼接預設副檔名。若已存在副檔名則原樣傳回。
- 支援傳入帶點或不帶點的副檔名（如 `"txt"` 或 `".txt"`）。

#### `FormatFileSize`
將位元組數值轉換為易於人類閱讀的標準字串格式：
- `< 1024 B`: `"512 B"`
- `< 1024 KB`: `"1.5 KB"`
- `< 1024 MB`: `"2.40 MB"`
- `>= 1 GB`: `"3.50 GB"`

---

## 2. DialogManager 全域服務管理

提供全域對話框後端派送模式的查詢與設定。

```cpp
class DialogManager {
public:
    static void SetDefaultBackendMode(DialogBackendMode mode) noexcept;
    COMPAT_NODISCARD static DialogBackendMode GetDefaultBackendMode() noexcept;
};
```

---

## 3. 列舉型別清單

### 3.1 MessageBoxButtons
| 列舉值 | 基礎數值 | 顯示按鈕組合 |
| :--- | :--- | :--- |
| `OK` | `0` | 單一 [確定] 按鈕 |
| `OKCancel` | `1` | [確定] 與 [取消] 按鈕 |
| `YesNo` | `2` | [是] 與 [否] 按鈕 |
| `YesNoCancel` | `3` | [是]、[否] 與 [取消] 按鈕 |
| `RetryCancel` | `4` | [重試] 與 [取消] 按鈕 |
| `AbortRetryIgnore` | `5` | [中止]、[重試] 與 [略過] 按鈕 |

### 3.2 MessageBoxIcon
| 列舉值 | 意義 | 視覺效果 |
| :--- | :--- | :--- |
| `None` | 無圖示 | 不顯示圖示區塊 |
| `Information` | 系統資訊提示 | 藍色圓形 `[i]` 資訊徽章 |
| `Warning` | 警告通知 | 黃色三角形 `[!]` 警告徽章 |
| `Error` | 錯誤警告 | 紅色圓形 `[X]` 錯誤徽章 |
| `Question` | 詢問確認 | 青色圓形 `[?]` 詢問徽章 |

### 3.3 MessageBoxDefaultButton
| 列舉值 | 意義 |
| :--- | :--- |
| `Button1` | 第一個按鈕取得預設焦點 (Accent 強調色) |
| `Button2` | 第二個按鈕取得預設焦點 |
| `Button3` | 第三個按鈕取得預設焦點 |

### 3.4 DialogBackendMode
| 列舉值 | 運作行為 |
| :--- | :--- |
| `Auto` | 預設模式。Windows 桌面使用原生 Win32/COM 對話框；Linux/WSL 或單元測試時自動退回 BuiltIn Form。 |
| `Native` | 強制呼叫作業系統原生對話框 (Win32 COM `IFileOpenDialog`, `MessageBoxW`)。 |
| `BuiltIn` | 強制使用純 CPPUI Form 佈景主題對話框，100% 依循 `ThemeManager` 深淺色模式。 |

---

## 4. FileFilterItem 結構

```cpp
struct FileFilterItem {
    std::string description;            // 例如 "Text Files (*.txt)"
    std::vector<std::string> patterns;  // 例如 {"*.txt"} 或 {"*.jpg", "*.png"}
};
```
