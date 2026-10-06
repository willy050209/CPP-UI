# CPPUI 設計規範 02：代碼規範與向下相容標準 (Coding Standards & compat.hpp)

## 1. 核心相容性哲學

現代 C++ 提供了豐富的語法與標準庫功能（如 C++17 `std::string_view`、`std::optional`，C++20 `std::format`、`std::span`，C++23 `std::expected`），但在許多工業環境與既有編譯器上，無法保證支援最新標準。

CPPUI 專案全面引入自行研發的 [`compat.hpp`](file:///D:/P/CPP/CPPUI/include/compat/compat.hpp) 作為基礎相容層：
1. **在新版編譯器（如 C++20/23）下**：`compat::` 自動無縫 alias 到原生 `std::` 實作，享有零開銷（Zero-Overhead）與極致編譯器最佳化。
2. **在舊版編譯器（如 C++14/17）下**：`compat::` 自動提供完整的 Polyfill 退回實作，確保相同語法能無痛通過編譯。

---

## 2. 強制性相容型別使用規範

在 `cppui` 命名空間內編寫所有公共頭文件與核心邏輯時，**一律遵循以下型別映射表**：

| 標準庫概念 | 禁用寫法 | 強制使用寫法 | 說明 |
| :--- | :--- | :--- | :--- |
| **字串視圖** | `const char*` + `size_t` | `compat::string_view` | 避免多餘字串拷貝，唯讀參數首選。 |
| **陣列切片視圖**| `const T*` + `size_t` | `compat::span<T>` / `compat::span<const T>` | 記憶體連續緩衝區（如 DisplayList 指令流、Glyph 陣列）首選。 |
| **可空值語意** | 裸指標 `nullptr` 代表無值 | `compat::optional<T>` | 清楚表述數值可能不存在（如文字命中測試結果）。 |
| **型別安全聯合**| `union` 或 `void*` | `compat::variant<Ts...>` | 用於多型事件參數、屬性值包裝。 |
| **錯誤返回** | C 風格錯誤代碼 / 拋出異常 | `compat::expected<T, E>` | 適用於可預期的管線失敗（如字型加載、檔案解碼），不產生異常棧開銷。 |
| **字串格式化** | `sprintf` / `std::stringstream` | `compat::format(...)` | 型別安全且高效的文字格式化。 |
| **終端除錯輸出**| `printf` / `std::cout` | `compat::print(...)` | 跨平台 UTF-8 友善終端輸出。 |

---

## 3. 代碼風格與命名約定 (Naming Conventions)

全專案採用具備高度辨識度與現代感的命名風格：

1. **命名空間 (Namespaces)**：
   - 頂層：`cppui`
   - 子模組：`cppui::core`, `cppui::pal`, `cppui::rendering`, `cppui::text`, `cppui::layout`, `cppui::input`, `cppui::controls`
   - 私有細節：`cppui::detail`
   - **禁止** 在任何標頭檔中使用 `using namespace ...;`！

2. **型別命名 (Types)**：
   - 類別、結構、列舉：大駝峰 `PascalCase`（例如 `DrawingContext`, `DisplayList`, `VisualTree`）。
   - 介面：以 `I` 開頭的大駝峰（例如 `IPlatformWindow`, `IRenderer`）。
   - 列舉成員：大駝峰 `PascalCase` 或強型別列舉 `enum class Alignment { Left, Center, Right };`。

3. **函式與方法 (Functions & Methods)**：
   - 公共方法與 Fluent API：大駝峰 `PascalCase`（例如 `Measure()`, `Arrange()`, `InvalidateVisual()`, `Background(Color)`）。
   - 私有/內部輔助函式：小駝峰 `camelCase`（例如 `calculateDesiredSize()`, `rebuildDirtyRegions()`）。

4. **變數與成員 (Variables & Fields)**：
   - 類別成員變數：必須加上 `m_` 前綴（例如 `m_bounds`, `m_parent`, `m_isDirty`）。
   - 區域變數與參數：小駝峰 `camelCase`（例如 `availableSize`, `finalRect`）。
   - 常數與靜態唯讀：大駝峰或全大寫蛇形（例如 `DefaultFontSize`, `MAX_ARENA_BLOCK_SIZE`）。

---

## 4. 記憶體管理與安全準則

1. **嚴禁隨意使用裸 `new` 與 `delete`**：
   - 動態所有權必須使用 `std::unique_ptr` 或 `std::shared_ptr`（或封裝為 `RefPtr<T>`）。
   - 樹狀結構（Visual/UIElement）所有權遵循 **Parent owns Children**：
     - 父節點透過 `std::vector<std::unique_ptr<Visual>>` 持有子節點。
     - 子節點指向父節點一律使用裸觀察指標 `Visual* m_parent = nullptr;`，避免循環引用與引用計數負擔。
2. **生命週期與移動語意 (Move Semantics)**：
   - Fluent DSL 鏈式構建器廣泛使用移動語意（`std::move`）傳遞子樹，消除額外堆積分配。
