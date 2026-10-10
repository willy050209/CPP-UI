# FileDialog 通用檔案對話框基底

- **命名空間**: `cppui::dialogs`
- **標頭檔**: `include/ui/dialogs/FileDialog.hpp`
- **實作檔**: `src/dialogs/FileDialog.cpp`

---

## 1. CommonDialog 抽象基底類別

所有 CPPUI 對話框類別的純虛擬介面契約基底。

```cpp
namespace cppui {
namespace dialogs {

class CommonDialog {
public:
    virtual ~CommonDialog() = default;

    /// <summary>以模態方式顯示對話方塊。</summary>
    virtual app::DialogResult ShowDialog(std::shared_ptr<app::Form> owner = nullptr) = 0;

    /// <summary>重設對話框所有屬性為預設值。</summary>
    virtual void Reset() = 0;
};

} // namespace dialogs
} // namespace cppui
```

---

## 2. FileDialog 通用檔案對話框基底

`FileDialog` 繼承自 `CommonDialog`，封裝所有檔案選取對話方塊（開啟與儲存）的共同屬性、篩選器與後端模式。

### 類別定義

```cpp
namespace cppui {
namespace dialogs {

class FileDialog : public CommonDialog {
public:
    FileDialog();
    ~FileDialog() override = default;

    // --- 標題與起始路徑 ---
    COMPAT_NODISCARD compat::string_view GetTitle() const noexcept;
    void SetTitle(compat::string_view title);

    COMPAT_NODISCARD compat::string_view GetInitialDirectory() const noexcept;
    void SetInitialDirectory(compat::string_view dir);

    COMPAT_NODISCARD compat::string_view GetFileName() const noexcept;
    void SetFileName(compat::string_view name);

    // --- 檔案篩選條件 ---
    COMPAT_NODISCARD compat::string_view GetFilter() const noexcept;
    void SetFilter(compat::string_view filter);

    COMPAT_NODISCARD uint32_t GetFilterIndex() const noexcept;
    void SetFilterIndex(uint32_t index) noexcept;

    // --- 預設副檔名與驗證旗標 ---
    COMPAT_NODISCARD compat::string_view GetDefaultExt() const noexcept;
    void SetDefaultExt(compat::string_view ext);

    COMPAT_NODISCARD bool GetCheckFileExists() const noexcept;
    void SetCheckFileExists(bool check) noexcept;

    COMPAT_NODISCARD bool GetCheckPathExists() const noexcept;
    void SetCheckPathExists(bool check) noexcept;

    COMPAT_NODISCARD bool GetRestoreDirectory() const noexcept;
    void SetRestoreDirectory(bool restore) noexcept;

    // --- 後端模式 ---
    COMPAT_NODISCARD DialogBackendMode GetBackendMode() const noexcept;
    void SetBackendMode(DialogBackendMode mode) noexcept;

    void Reset() override;

protected:
    std::string m_title;
    std::string m_initialDirectory;
    std::string m_fileName;
    std::string m_filter;
    uint32_t m_filterIndex = 1;
    std::string m_defaultExt;
    bool m_checkFileExists = false;
    bool m_checkPathExists = true;
    bool m_restoreDirectory = false;
    DialogBackendMode m_backendMode = DialogBackendMode::Auto;
};

} // namespace dialogs
} // namespace cppui
```

---

## 3. 屬性詳細說明

| 屬性名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `Title` | `compat::string_view` | `""` | 視窗標題列文字。若為空，衍生類別會自動填入「開啟檔案」或「儲存檔案」。 |
| `InitialDirectory`| `compat::string_view` | `""` | 開啟時的起始目錄。若為空或不存在，自動使用目前工作目錄 (`current_path`)。 |
| `FileName` | `compat::string_view` | `""` | 預設填入或使用者選取的檔案完整路徑。 |
| `Filter` | `compat::string_view` | `""` | 篩選條件字串（例如 `"Text (*.txt)\|*.txt\|All (*.*)\|*.*"`）。 |
| `FilterIndex` | `uint32_t` | `1` | 目前選取的篩選器項目索引 (1-based)。 |
| `DefaultExt` | `compat::string_view` | `""` | 預設副檔名。若使用者輸入無副檔名，自動補齊。 |
| `CheckFileExists`| `bool` | `false` | 是否強制檢查選取之檔案必須存在（`OpenFileDialog` 預設為 `true`）。 |
| `CheckPathExists`| `bool` | `true` | 是否檢查選取之路徑資料夾必須存在。 |
| `BackendMode` | `DialogBackendMode` | `Auto` | 執行期後端模式 (`Auto`, `Native`, `BuiltIn`)。 |
