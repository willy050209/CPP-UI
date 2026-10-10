# ParallelOptions 類別 (cppui::graphics::ParallelOptions)

`ParallelOptions` 結構定義於 `include/ui/graphics/ParallelOptions.hpp`，提供全域一致的影像處理、像素遍歷與卷積排程控制參數。它將單執行緒、多執行緒以及視窗切分（Window Slicing / Chunk Size）的決定權完全交由呼叫端，避免系統黑盒子隱式干預。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/ParallelOptions.hpp"`
* **特性**：
  - 輕量級 POD 結構，零記憶體配置開銷。
  - 明確支援單執行緒確定性排程與多執行緒平行運算。
  - 支援動態視窗分塊（Window Slicing），最大化快取擊中率並均衡工作負載。

---

## 2. 語法與宣告 (Syntax & Declaration)

```cpp
namespace cppui {
namespace graphics {

enum class ExecutionMode : uint8_t {
    SingleThread = 0, // 單執行緒循序執行 (保證零執行緒建立與切換開銷)
    MultiThread  = 1  // 多執行緒並行執行
};

struct ParallelOptions {
    ExecutionMode mode{ ExecutionMode::SingleThread }; // 執行模式
    uint32_t      threadCount{ 1 };                    // 工作執行緒數量 (0 代表自動採用硬體核心數)
    uint32_t      sliceHeight{ 0 };                    // 列切分視窗大小 (0 代表高度均分，>0 代表自訂固定大小區塊)

    // 便捷工廠方法
    static ParallelOptions Sequential() noexcept;
    static ParallelOptions Parallel(uint32_t threads = 0, uint32_t chunkRows = 0) noexcept;
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 欄位說明 (Field Details)

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `mode` | `ExecutionMode` | `ExecutionMode::SingleThread` | 執行緒模式。可設為 `SingleThread` 或 `MultiThread`。 |
| `threadCount` | `uint32_t` | `1` | 參與運算的最大工作執行緒數量。若設為 `0`，工廠方法將自動代入 `std::thread::hardware_concurrency()`。 |
| `sliceHeight` | `uint32_t` | `0` | 視窗分塊列高（Window Slicing）。若設為 `0`，系統將影像高度平均切分為 `threadCount` 塊；若大於 `0`，將影像切分為每塊固定 `sliceHeight` 列的 Work Chunks 由執行緒池動態領取。 |

---

## 4. 工廠方法 (Factory Methods)

### 4.1 `Sequential()`
```cpp
static ParallelOptions Sequential() noexcept;
```
* **功能**：建構標準單執行緒配置。
* **行為**：強制在當前呼叫者執行緒中同步循序執行，保證零額外執行緒建立或上下文切換開銷。
* **適用情境**：小型圖標（如 $32 \times 32$ 以下）、即時互動式單步處理、確定性單元測試。

### 4.2 `Parallel(threads, chunkRows)`
```cpp
static ParallelOptions Parallel(uint32_t threads = 0, uint32_t chunkRows = 0) noexcept;
```
* **功能**：建構多執行緒並行配置。
* **參數**：
  - `threads`：指定執行緒數（傳入 `0` 則自動採用硬體邏輯核心數）。
  - `chunkRows`：視窗切分列高（傳入 `0` 則高度均分；大於 `0` 則指定區塊列高，如 `16`、`32` 或 `64` 列）。
* **適用情境**：高解析度影像濾鏡、卷積運算、批次轉檔等高負載運算。

---

## 5. 範例程式碼 (Usage Examples)

### 5.1 單執行緒循序處理
```cpp
#include "ui/graphics/ImageFilters.hpp"
#include "ui/graphics/ParallelOptions.hpp"

using namespace cppui::graphics;

// 明確指定單執行緒處理小型頭像
Bitmap avatar(48, 48);
ImageFilters::GaussianBlur(avatar, 1.2f, 1.2f, ParallelOptions::Sequential());
```

### 5.2 多執行緒自動核心均分
```cpp
// 依本機硬體核心數均分處理 1080p 照片
Bitmap photo(1920, 1080);
ImageFilters::GaussianBlur(photo, 3.0f, 3.0f, ParallelOptions::Parallel());
```

### 5.3 指定 8 執行緒與 32 列視窗分塊 (Window Slicing)
```cpp
// 限制 8 個執行緒，每 32 列切分為一塊動態排程以最佳化快取擊中率
ParallelOptions opts = ParallelOptions::Parallel(8, 32);
ImageFilters::Canny(photo, 50, 150, 1.0f, opts);
```
