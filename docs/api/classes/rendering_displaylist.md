# DisplayList 類別 (cppui::rendering::DisplayList)

二進位繪圖命令串流容器，封裝平坦記憶體池，提供高速二進位指令序列化與遍歷重播。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include <ui/rendering/DisplayList.hpp>`
* **模組**：`CPPUI Rendering`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

class DisplayList {
public:
    explicit DisplayList(size_t initialCapacity = LinearArena::DEFAULT_CAPACITY);
    ~DisplayList() = default;

    DisplayList(const DisplayList&) = delete;
    DisplayList& operator=(const DisplayList&) = delete;

    DisplayList(DisplayList&& other) noexcept;
    DisplayList& operator=(DisplayList&& other) noexcept;

    template <typename PayloadType>
    void PushOp(DrawOpType type, const PayloadType& payload, uint16_t flags = 0);

    void PushHeaderOnly(DrawOpType type, uint16_t flags = 0);

    template <typename Visitor>
    void ForEach(Visitor&& visitor) const;

    void Clear() noexcept;

    COMPAT_NODISCARD size_t GetOpCount() const noexcept;
    COMPAT_NODISCARD size_t GetTotalBytes() const noexcept;
    COMPAT_NODISCARD bool IsEmpty() const noexcept;
};

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

`DisplayList` 類似現代圖形 API（如 Vulkan 的 Command Buffer 或 Skia 的 `SkPicture`），它將一幀的所有繪圖操作打包儲存在一塊連續的記憶體池（[`LinearArena`](rendering_lineararena.md)）中。

### 核心特性
* **移動語意 (Move-Only)**：禁止昂貴的拷貝，支援零拷貝移動傳遞。
* **8 字節硬體對齊**：所有寫入的指令長度皆向上對齊至 8 字節倍數，提升 CPU 向量化存取效能。
* **$O(1)$ 幀末重設**：呼叫 `Clear()` 僅將內部指標歸零，實體記憶體緩衝區保留供下一幀複用，杜絕記憶體抖動與 GC 停頓。

---

## 方法說明 (Methods)

### `PushOp<PayloadType>(type, payload, flags)`
將帶有資料結構的繪圖操作寫入命令串流。
```cpp
template <typename PayloadType>
void PushOp(DrawOpType type, const PayloadType& payload, uint16_t flags = 0);
```
* **參數**：
  * `type`：[`DrawOpType`](rendering_drawop.md#drawoptype-列舉) 操作列舉。
  * `payload`：指令專屬負載結構體（如 `DrawRectOp`）。
  * `flags`：保留旗標（預設 `0`）。

### `PushHeaderOnly(type, flags)`
寫入僅有標頭的操作（例如 `PopClip`、`PopOpacity`），總字節長度僅為 8 字節。

### `ForEach(visitor)`
高效率遍歷串流中的所有繪圖操作。
```cpp
template <typename Visitor>
void ForEach(Visitor&& visitor) const;
```
* **Visitor 簽章**：`void(const DrawOpHeader& header, const void* payload)`
* **備註**：直接透過記憶體指標連續位移進行無虛擬函式、無鎖定的高速解包。

### `Clear()`
```cpp
void Clear() noexcept;
```
$O(1)$ 清空命令清單，保留已分配記憶體供下一幀重用。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/rendering/DisplayList.hpp"

void InspectDisplayList(const cppui::rendering::DisplayList& dl) {
    using namespace cppui::rendering;

    compat::println("DisplayList 包含 {} 個操作，總佔用 {} 字節", 
        dl.GetOpCount(), dl.GetTotalBytes());

    // 遍歷指令
    dl.ForEach([](const DrawOpHeader& header, const void* payload) {
        if (header.type == DrawOpType::DrawRectangle) {
            const auto* op = static_cast<const DrawRectOp*>(payload);
            compat::println("發現矩形繪製指令: 色彩 A={}", op->color.a);
        }
    });
}
```

---

## 請參閱 (See Also)
* [`DrawingContext` 錄製類別](rendering_drawingcontext.md)
* [`LinearArena` 記憶體池](rendering_lineararena.md)
* [`DisplayListRenderer` 重播器](rendering_displaylistrenderer.md)
