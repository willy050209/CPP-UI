# GraphicsPath 類別 (cppui::graphics::GraphicsPath)

`GraphicsPath` 類別代表任意複雜的二維向量複合幾何形狀與路徑。它封裝了由直線段、矩形、圓角矩形、橢圓、圓弧、三次貝茲曲線與平滑樣條構成的子圖形序列（Figures）。支援路徑平整化折線展開（`Flatten`）、2D 仿射矩陣變換（`Transform`）、奇偶/卷繞數射線命中測試（`IsVisible`）以及筆刷邊界碰撞測試（`IsOutlineVisible`）。

類別內部封裝讀寫鎖（`mutable std::shared_mutex`），支援多執行緒並發讀取與安全獨佔修改。

---

## 1. 元資料與規格 (Metadata)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/GraphicsPath.hpp"`
* **實作檔**：`src/graphics/GraphicsPath.cpp`
* **內部拓撲資料結構**：
  - 頂點陣列：`std::vector<core::Point> m_points`
  - 控制點型別標記陣列：`std::vector<uint8_t> m_types`（依據 [`PathPointType`](graphics_enums.md#11-路徑控制點標記-pathpointtype)）
  - 填色規則：[`FillMode`](graphics_enums.md#4-幾何路徑填色規則-fillmode)（`Alternate` 或 `Winding`）

---

## 2. 類別語法與宣告 (Class Syntax)

```cpp
namespace cppui {
namespace graphics {

class Pen;

class GraphicsPath {
public:
    // 建構子
    GraphicsPath();
    explicit GraphicsPath(FillMode fillMode);
    explicit GraphicsPath(compat::span<const core::Point> points, FillMode fillMode = FillMode::Alternate);

    // 執行緒安全拷貝與移動語意
    GraphicsPath(const GraphicsPath& other);
    GraphicsPath(GraphicsPath&& other) noexcept;
    GraphicsPath& operator=(const GraphicsPath& other);
    GraphicsPath& operator=(GraphicsPath&& other) noexcept;
    ~GraphicsPath() = default;

    // 幾何圖元建構
    void AddLine(core::Point p1, core::Point p2);
    void AddLine(float x1, float y1, float x2, float y2);
    void AddLines(compat::span<const core::Point> points);
    void AddRectangle(const core::Rect& rect);
    void AddRoundedRectangle(const core::Rect& rect, float radius);
    void AddEllipse(const core::Rect& rect);
    void AddEllipse(float x, float y, float width, float height);
    void AddArc(const core::Rect& rect, float startAngle, float sweepAngle);
    void AddBezier(core::Point p1, core::Point c1, core::Point c2, core::Point p2);
    void AddCurve(compat::span<const core::Point> points);
    void AddPolygon(compat::span<const core::Point> points);
    void AddPath(const GraphicsPath& path, bool connect = false);

    // 子圖形拓撲控制
    void StartFigure();
    void CloseFigure();
    void CloseAllFigures();
    void Reset();

    // 屬性與資料查詢
    COMPAT_NODISCARD FillMode GetFillMode() const;
    void SetFillMode(FillMode mode);
    COMPAT_NODISCARD core::Rect GetBounds() const;
    COMPAT_NODISCARD size_t GetPointCount() const;
    COMPAT_NODISCARD std::vector<core::Point> GetPathPoints() const;
    COMPAT_NODISCARD std::vector<uint8_t> GetPathTypes() const;

    // 命中測試 (Hit Testing)
    COMPAT_NODISCARD bool IsVisible(core::Point pt) const;
    COMPAT_NODISCARD bool IsVisible(float x, float y) const;
    COMPAT_NODISCARD bool IsOutlineVisible(core::Point pt, const Pen& pen) const;

    // 演算法操作：展開與仿射變換
    COMPAT_NODISCARD std::vector<core::Point> Flatten(float flatness = 0.25f) const;
    void Transform(float m11, float m12, float m21, float m22, float dx, float dy);
    COMPAT_NODISCARD GraphicsPath Clone() const;
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 圖元新增與子圖形拓撲 (Path Construction)

| 方法 | 參數 | 說明 |
| :--- | :--- | :--- |
| `AddLine` | `Point p1, Point p2` / `(x1, y1, x2, y2)` | 新增直線段。若當前已有開口子圖形且起點重合，自動延續。 |
| `AddLines` | `compat::span<const Point> points` | 新增連續多段折線。 |
| `AddRectangle` | `const core::Rect& rect` | 新增獨立閉合矩形子圖形。 |
| `AddRoundedRectangle`| `const core::Rect& rect, float radius`| 新增帶圓角的封閉矩形（由 4 段圓弧與 4 段直線平滑連接）。 |
| `AddEllipse` | `const core::Rect& rect` | 新增由 4 段三次貝茲曲線逼近之精確外接橢圓。 |
| `AddArc` | `const core::Rect& rect, float startAngle, float sweepAngle` | 新增橢圓圓弧段。 |
| `AddBezier` | `Point p1, c1, c2, p2` | 新增單條三次貝茲曲線（起點、兩控制點、終點）。 |
| `AddCurve` | `compat::span<const Point> points` | 新增通過指定點列之連續 Catmull-Rom 平滑樣條。 |
| `AddPolygon` | `compat::span<const Point> points` | 新增自動封閉之多邊形輪廓。 |
| `AddPath` | `const GraphicsPath& path, bool connect` | 附加另一路徑之全部幾何圖元。`connect = true` 時自動自前路徑終點拉一直線至新路徑起點。 |
| `StartFigure()` | 無 | 明確結束上一子圖形，下一點將視為新子圖形的起始點（`PathPointType::Start`）。 |
| `CloseFigure()` | 無 | 閉合當前開口子圖形（標記終點為 `PathPointType::CloseSubpath`）。 |
| `CloseAllFigures()`| 無 | 遍歷並閉合路徑中所有未閉合的子圖形。 |
| `Reset()` | 無 | 清空路徑所有點、型別標記與狀態。 |

---

## 4. 命中測試與幾何演算法 (Hit Testing & Algorithms)

### 4.1 內部區域命中測試 (`IsVisible`)
```cpp
COMPAT_NODISCARD bool IsVisible(core::Point pt) const;
COMPAT_NODISCARD bool IsVisible(float x, float y) const;
```
* **演算法**：射線法（Ray-Casting Algorithm）。
* **計算過程**：
  1. 首先快速檢測點是否落在 [`GetBounds()`](#43-邊界矩形查詢-getbounds) 範圍內，若在範圍外則以 $O(1)$ 時間常數快速排除。
  2. 若在邊界框內，對路徑中的曲線進行平整化（`Flatten`）展開為多邊形邊界。
  3. 依據設定之 [`FillMode`](graphics_enums.md#4-幾何路徑填色規則-fillmode)（`Alternate` 奇偶法或 `Winding` 卷繞數法）進行射線相交判定，傳回點是否位於封閉幾何圖形內部。

### 4.2 描邊邊界命中測試 (`IsOutlineVisible`)
```cpp
COMPAT_NODISCARD bool IsOutlineVisible(core::Point pt, const Pen& pen) const;
```
* **功能**：測試點是否落在沿路徑輪廓描邊、厚度為 `pen.GetWidth()` 的描邊帶寬內部。
* **應用**：圖形編輯器中的線段選取、流程圖連接線滑鼠點擊判定。

### 4.3 曲線平整化 (`Flatten`)
```cpp
COMPAT_NODISCARD std::vector<core::Point> Flatten(float flatness = 0.25f) const;
```
* **功能**：將路徑中所有的圓弧、橢圓與三次貝茲曲線，以折線段（Polyline）遞迴細分逼近展開。
* **參數 `flatness`**：折線段與真實曲線幾何表面之間容許的最大誤差距離（單位：像素）。預設值 `0.25f` 在肉眼下已達到極致平滑。

### 4.4 2D 仿射矩陣變換 (`Transform`)
```cpp
void Transform(float m11, float m12, float m21, float m22, float dx, float dy);
```
* **功能**：對路徑中的每一個頂點執行 2D 仿射變換：
  $$x' = m_{11} \cdot x + m_{21} \cdot y + dx$$
  $$y' = m_{12} \cdot x + m_{22} \cdot y + dy$$

---

## 5. 範例程式碼 (Examples)

### 5.1 生成心形參數曲線路徑

```cpp
#include <cmath>
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Brushes.hpp"
#include "ui/graphics/Pens.hpp"

cppui::graphics::GraphicsPath CreateHeartPath(core::Point center, float scale) {
    cppui::graphics::GraphicsPath path;
    const size_t steps = 100;
    std::vector<core::Point> pts;
    pts.reserve(steps);

    // 心形參數方程式:
    // x = 16 * sin^3(t)
    // y = -(13 * cos(t) - 5 * cos(2t) - 2 * cos(3t) - cos(4t))
    for (size_t i = 0; i < steps; ++i) {
        float t = static_cast<float>(i) * 2.0f * 3.14159265f / steps;
        float sinT = std::sin(t);
        float x = 16.0f * sinT * sinT * sinT;
        float y = -(13.0f * std::cos(t) - 5.0f * std::cos(2.0f * t) - 2.0f * std::cos(3.0f * t) - std::cos(4.0f * t));
        pts.emplace_back(center.x + x * scale, center.y + y * scale);
    }

    path.AddPolygon(pts);
    path.CloseFigure();
    return path;
}

void DrawHeart(cppui::graphics::Graphics& g) {
    auto heart = CreateHeartPath(core::Point(200, 200), 8.0f);
    
    // 填滿紅色並以深紅描邊
    g.FillPath(cppui::graphics::Brushes::Red(), heart);
    g.DrawPath(cppui::graphics::Pens::DarkRed(), heart);

    // 命中測試
    bool hit = heart.IsVisible(core::Point(200, 200)); // true
}
```

---

## 6. 請參閱 (See Also)

* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
* [`Pen` 畫筆類別詳解](graphics_pen.md)
* [`Brush` 畫刷家族詳解](graphics_brush.md)
* [`FillMode` 填色規則列舉](graphics_enums.md#4-幾何路徑填色規則-fillmode)
* [`PathPointType` 控制點標記型別](graphics_enums.md#11-路徑控制點標記-pathpointtype)
