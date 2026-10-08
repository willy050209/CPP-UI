# Graphics 類別 (Graphics Class)

命名空間：`cppui::graphics`  
標頭檔：`include/ui/graphics/Graphics.hpp`  
實作檔：`src/graphics/Graphics.cpp`  

---

## 1. 類別概述 (Class Overview)

`Graphics` 類別借鏡經典 GDI+ (`System.Drawing.Graphics`) 與現代向量繪圖庫設計，扮演 CPPUI 2D 繪圖操作的中樞排程器。
它將畫筆（`Pen`）、畫刷（`Brush`）、幾何路徑（`GraphicsPath`）與位圖影像（`Bitmap`）抽離為獨立領域物件，統一調度幾何圖元、曲線、文字與影像繪製。

### 核心特性
- **雙模式架構**：
  - **UI 繪製模式 (`FromDrawingContext`)**：在任何控制項的 `OnRender(DrawingContext& dc)` 中調用，將指令無鎖錄製至 `DisplayList`，配合 Skia GPU 向量加速管線。
  - **離線影像模式 (`FromImage`)**：在背景執行緒中對 `std::shared_ptr<Bitmap>` 進行記憶體離線光柵繪圖，呼叫 `Flush()` 或解構時自動將畫面輸出至點陣緩衝區。
- **階層式狀態管理與變換**：
  - 支援平移 (`TranslateTransform`)、縮放 (`ScaleTransform`)、旋轉 (`RotateTransform`) 與重設 (`ResetTransform`)。
  - 支援矩形與路徑剪裁 (`SetClip`, `ResetClip`)。
  - 提供 `Save()` / `Restore()` 與 RAII `Graphics::StateScope` 狀態保護守衛。

---

## 2. API 語法與成員函數 (API Signatures)

### 工廠與建構子
```cpp
static Graphics FromDrawingContext(rendering::DrawingContext& dc);
static Graphics FromImage(std::shared_ptr<Bitmap> image);
explicit Graphics(rendering::DrawingContext& dc);
explicit Graphics(std::shared_ptr<Bitmap> image);
~Graphics();
```

### 狀態管理與變換矩陣
```cpp
uint32_t Save();
void Restore(uint32_t stateId);
void TranslateTransform(float dx, float dy);
void ScaleTransform(float sx, float sy);
void RotateTransform(float angleDegrees);
void ResetTransform();

void SetClip(const core::Rect& rect);
void SetClip(const GraphicsPath& path);
void ResetClip();
core::Rect GetClipBounds() const;

void SetSmoothingMode(SmoothingMode mode);
SmoothingMode GetSmoothingMode() const;
void SetInterpolationMode(InterpolationMode mode);
InterpolationMode GetInterpolationMode() const;

void Clear(core::Color color);
void Flush();
```

### 幾何描邊與填滿 (對標 GDI+)
```cpp
// 直線與折線
void DrawLine(const Pen& pen, core::Point pt1, core::Point pt2);
void DrawLine(const Pen& pen, float x1, float y1, float x2, float y2);
void DrawLines(const Pen& pen, compat::span<const core::Point> points);

// 矩形
void DrawRectangle(const Pen& pen, const core::Rect& rect);
void DrawRectangle(const Pen& pen, float x, float y, float width, float height);
void DrawRectangles(const Pen& pen, compat::span<const core::Rect> rects);
void FillRectangle(const Brush& brush, const core::Rect& rect);
void FillRectangle(const Brush& brush, float x, float y, float width, float height);
void FillRectangles(const Brush& brush, compat::span<const core::Rect> rects);

// 圓角矩形
void DrawRoundedRectangle(const Pen& pen, const core::Rect& rect, float radius);
void FillRoundedRectangle(const Brush& brush, const core::Rect& rect, float radius);

// 橢圓與圓弧
void DrawEllipse(const Pen& pen, const core::Rect& rect);
void DrawEllipse(const Pen& pen, float x, float y, float width, float height);
void FillEllipse(const Brush& brush, const core::Rect& rect);
void FillEllipse(const Brush& brush, float x, float y, float width, float height);
void DrawArc(const Pen& pen, const core::Rect& rect, float startAngle, float sweepAngle);

// 扇形
void DrawPie(const Pen& pen, const core::Rect& rect, float startAngle, float sweepAngle);
void FillPie(const Brush& brush, const core::Rect& rect, float startAngle, float sweepAngle);

// 多邊形
void DrawPolygon(const Pen& pen, compat::span<const core::Point> points);
void FillPolygon(const Brush& brush, compat::span<const core::Point> points, FillMode fillMode = FillMode::Alternate);

// 貝茲曲線與平滑曲線
void DrawBezier(const Pen& pen, core::Point pt1, core::Point pt2, core::Point pt3, core::Point pt4);
void DrawBeziers(const Pen& pen, compat::span<const core::Point> points);
void DrawCurve(const Pen& pen, compat::span<const core::Point> points);

// 幾何路徑
void DrawPath(const Pen& pen, const GraphicsPath& path);
void FillPath(const Brush& brush, const GraphicsPath& path);
```

### 影像繪製
```cpp
void DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect);
void DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect, const core::Rect& srcRect);
void DrawImage(std::shared_ptr<Bitmap> image, float x, float y);
void DrawImage(std::shared_ptr<Bitmap> image, float x, float y, float width, float height);
void DrawImageUnscaled(std::shared_ptr<Bitmap> image, core::Point origin);
```

### 文字排版與度量
```cpp
void DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, core::Point origin);
void DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, const core::Rect& layoutRect);
core::Size MeasureString(compat::string_view text, const text::FontDescriptor& font);
```

---

## 3. 範例代碼 (Examples)

### 在自訂控制項中即時繪製 (UI Mode)
```cpp
void MyCustomControl::OnRender(rendering::DrawingContext& dc) {
    Control::OnRender(dc);

    graphics::Graphics g(dc);

    // 使用 RAII StateScope 自動管理畫布矩陣
    {
        graphics::Graphics::StateScope scope(g);
        g.TranslateTransform(20.0f, 20.0f);
        g.RotateTransform(15.0f);

        // 填滿圓角矩形並描邊
        g.FillRoundedRectangle(graphics::Brushes::SteelBlue(), core::Rect(0, 0, 120, 60), 8.0f);
        g.DrawRoundedRectangle(graphics::Pens::White(), core::Rect(0, 0, 120, 60), 8.0f);

        // 繪製文字
        text::FontDescriptor font("Segoe UI", 14.0f);
        g.DrawString("CPPUI Graphics", font, graphics::Brushes::White(), core::Point(10, 20));
    }
}
```

### 離線點陣影像生成 (Offscreen Image Mode)
```cpp
auto bmp = std::make_shared<graphics::Bitmap>(400, 300);
bmp->Clear(graphics::Colors::White);

{
    graphics::Graphics g = graphics::Graphics::FromImage(bmp);
    g.FillEllipse(graphics::Brushes::Red(), core::Rect(50, 50, 100, 100));
    g.DrawRectangle(graphics::Pens::Black(), core::Rect(200, 50, 150, 100));
    g.Flush(); // 自動或於解構時光柵化至 bmp 像素緩衝區中
}
```
