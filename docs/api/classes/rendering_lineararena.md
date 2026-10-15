# LinearArena 類別 (cppui::rendering::LinearArena)

高效連續平坦記憶體池 (Arena Allocator)，保證 8 字節硬體對齊、$O(1)$ 指標分配與零開銷幀末重設。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include <ui/rendering/LinearArena.hpp>`
* **模組**：`CPPUI Rendering`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

class LinearArena {
public:
    static constexpr size_t DEFAULT_CAPACITY = 256 * 1024; // 預設 256 KB

    explicit LinearArena(size_t initialCapacity = DEFAULT_CAPACITY);
    ~LinearArena() = default;

    LinearArena(const LinearArena&) = delete;
    LinearArena& operator=(const LinearArena&) = delete;

    LinearArena(LinearArena&& other) noexcept;
    LinearArena& operator=(LinearArena&& other) noexcept;

    void* Allocate(size_t bytes, size_t alignment = 8);

    template <typename T>
    T* Allocate(size_t count = 1);

    void Reset() noexcept;

    COMPAT_NODISCARD size_t GetUsedBytes() const noexcept;
    COMPAT_NODISCARD size_t GetCapacity() const noexcept;
    COMPAT_NODISCARD const uint8_t* GetData() const noexcept;
    COMPAT_NODISCARD uint8_t* GetData() noexcept;
};

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

在每秒 60~120 幀的現代 UI 渲染中，若每幀為成千上萬個指令個別呼叫標準函式庫的 `new` 或 `malloc`，將產生嚴重的記憶體碎片與執行期鎖定開銷。

`LinearArena` 透過在底層維護一塊平坦連續的 `std::vector<uint8_t>` 緩衝區，將記憶體分配簡化為單純的**位移指標累加** ($O(1)$)。當一幀渲染結束時，呼叫 `Reset()` 僅需重置偏移指標為 0，記憶體空間完全保留供下一幀重複使用。

---

## 常數與屬性 (Constants & Properties)

| 識別名稱 | 型別 | 數值 | 說明 |
| :--- | :--- | :--- | :--- |
| `DEFAULT_CAPACITY` | `size_t` | `262,144` (256 KB) | 預設初始容量，足以容納數千個繪圖圖元。 |

---

## 方法說明 (Methods)

### `Allocate(bytes, alignment)`
在連續記憶體池中分配指定的位元組數，保證硬體位址符合 `alignment` 對齊要求。
```cpp
void* Allocate(size_t bytes, size_t alignment = 8);
```
* **參數**：
  * `bytes`：欲分配的位元組數量。
  * `alignment`：對齊字節數（預設為 8 字節）。
* **傳回值**：指向分配記憶體區塊起始處的指標。
* **備註**：若分配超出當前容量，將自動擴充平坦緩衝區以容納資料（使用 `COMPAT_UNLIKELY` 進行微架構分支預測）。

### `Allocate<T>(count)`
型別化分配樣板輔助函式。自動依據 `sizeof(T)` 與 `alignof(T)` 計算長度與對齊。

### `Reset()`
```cpp
void Reset() noexcept;
```
將內部偏移量 `m_offset` 歸零。$O(1)$ 極速完成，不釋放實體記憶體。

### `GetUsedBytes()` / `GetCapacity()` / `GetData()`
* `GetUsedBytes()`：取得當前已使用的位元組數。
* `GetCapacity()`：取得當前平坦記憶體池的總物理容量。
* `GetData()`：取得平坦緩衝區的起始記憶體指標。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/rendering/LinearArena.hpp"

struct Particle {
    float x, y;
    float vx, vy;
};

void ExampleArena() {
    using namespace cppui::rendering;

    LinearArena arena(1024 * 1024); // 預先配置 1 MB

    // 高速連續分配 1000 個粒子
    Particle* particles = arena.Allocate<Particle>(1000);
    for (int i = 0; i < 1000; ++i) {
        particles[i] = { 0.0f, 0.0f, 1.0f, 1.0f };
    }

    compat::println("已使用記憶體: {} 字節", arena.GetUsedBytes());

    // 幀末 O(1) 歸零
    arena.Reset();
    compat::println("重設後已使用: {} 字節", arena.GetUsedBytes());
}
```

---

## 請參閱 (See Also)
* [`DisplayList` 命令串流](rendering_displaylist.md)
* [DisplayList 二進位錄製深度解析](../guides/display_list_recording.md)
