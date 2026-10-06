# DrawOp 二進位指令協定 (cppui::rendering::DrawOp)

定義 DisplayList 中高密度緊湊序列化的繪圖操作二進位標頭與各圖元負載結構體。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include <ui/rendering/DrawOp.hpp>`
* **模組**：`CPPUI Rendering`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

enum class DrawOpType : uint16_t {
    None = 0,
    Save,
    Restore,
    PushClipRect,
    PopClip,
    PushOpacity,
    PopOpacity,
    DrawRectangle,
    DrawRoundedRectangle,
    DrawLine,
    DrawTextRun
};

#pragma pack(push, 4)

struct DrawOpHeader {
    DrawOpType type;
    uint16_t   flags;
    uint32_t   size;
};

struct DrawRectOp {
    core::Rect  rect;
    core::Color color;
};

struct DrawRoundedRectOp {
    core::Rect  rect;
    float       radius = 0.0f;
    core::Color color;
};

struct DrawLineOp {
    core::Point p1;
    core::Point p2;
    core::Color color;
    float       strokeWidth = 1.0f;
};

struct PushClipRectOp {
    core::Rect rect;
};

struct PushOpacityOp {
    float opacity = 1.0f;
};

#pragma pack(pop)

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

為了達到極致的快取友善度（Cache-Friendly）與高吞吐量指令重播，CPPUI 不採用含有虛擬函式表指標 (vptr) 的物件導向類別階層，而是採用類硬體驅動程式的 **二進位 TLV (Type-Length-Value) 協議**。

所有繪圖操作皆以 8 字節的 `DrawOpHeader` 作為前導，後續緊隨具體的 Payload 結構。

---

## `DrawOpType` 列舉

| 列舉名稱 | 數值 | 說明 | 對應負載結構 |
| :--- | :--- | :--- | :--- |
| `None` | `0` | 空操作 / 無效操作。 | 無 |
| `Save` | `1` | 儲存畫布狀態。 | 無 |
| `Restore` | `2` | 回復畫布狀態。 | 無 |
| `PushClipRect` | `3` | 壓入局部剪裁矩形。 | `PushClipRectOp` |
| `PopClip` | `4` | 彈出剪裁矩形。 | 無 (Header-Only) |
| `PushOpacity` | `5` | 壓入不透明度。 | `PushOpacityOp` |
| `PopOpacity` | `6` | 彈出不透明度。 | 無 (Header-Only) |
| `DrawRectangle` | `7` | 繪製填色矩形。 | `DrawRectOp` |
| `DrawRoundedRectangle`| `8` | 繪製填色圓角矩形。 | `DrawRoundedRectOp` |
| `DrawLine` | `9` | 繪製線段。 | `DrawLineOp` |
| `DrawTextRun` | `10` | 繪製排版文字串流 (預留 M3)。| 預留 |

---

## 結構說明 (Structures)

### `DrawOpHeader`
二進位命令標頭（精確佔用 8 字節）：
* `type` (`DrawOpType`)：指令類型 (2 bytes)。
* `flags` (`uint16_t`)：保留旗標 (2 bytes)。
* `size` (`uint32_t`)：包含 Header 本身與後續 Payload 的**對齊後總長度** (4 bytes)。重播器透過此數值直接進行指標加法跳轉至下一條指令。

### 指令負載結構
* `DrawRectOp`：包含目標邊界 `core::Rect` (16 bytes) 與填色 `core::Color` (4 bytes)。
* `DrawRoundedRectOp`：包含 `core::Rect`、`radius` (float) 與 `core::Color`。
* `DrawLineOp`：包含起始點 `p1`、終點 `p2`、顏色與線寬。

---

## 記憶體佈局範例 (Memory Layout Diagram)

```
+-----------------------------------------------------------------------------------+
|               DrawOpHeader (8 bytes)             |       DrawRectOp (20 bytes)    |  Padding  |
| type: DrawRectangle (2B) | flags (2B) | size: 32B | rect: x,y,w,h (16B) | color (4B)|  (4 bytes)|
+-----------------------------------------------------------------------------------+
|<--------------------------------- 32-byte Aligned Block ------------------------->|
```

---

## 請參閱 (See Also)
* [`DisplayList` 類別參考](rendering_displaylist.md)
* [`DrawingContext` 類別參考](rendering_drawingcontext.md)
* [DisplayList 二進位錄製深度解析](../guides/display_list_recording.md)
