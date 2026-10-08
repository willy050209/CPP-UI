# 核心圖形一級物件與標準集合 (Core Graphics Objects & Stock Collections)

命名空間：`cppui::graphics`  
目錄：`include/ui/graphics/`  

---

## 1. 概觀與設計哲學 (Overview)

CPPUI 圖形子系統將繪圖要素徹底解耦為獨立的一級領域物件：
- **畫筆 (`Pen`)**：定義線條粗細、色彩、虛線樣式與端點連接規格。
- **畫刷 (`Brush`)**：定義區域填充樣式（`SolidBrush`, `LinearGradientBrush`, `TextureBrush`）。
- **幾何路徑 (`GraphicsPath`)**：定義任意複雜的 2D 向量形狀、曲線序列與多邊形輪廓。
- **點陣影像 (`Bitmap`)**：32-bit ARGB/BGRA 影像記憶體緩衝區。
- **標準集合與常數**：`Pens`、`Brushes`、`Colors` 提供常用單例與豐富預定義調色盤。

### 執行緒安全保證 (Automatic Thread-Safety)
所有圖形物件內部均封裝 `mutable std::shared_mutex`：
- **並行讀取**：`GetPixel`、`GetBounds`、`IsVisible`、渲染取樣等自動套用 `std::shared_lock`，多執行緒同時查詢零阻塞。
- **排他寫入**：`SetPixel`、`AddLine`、`SetColor` 等自動套用 `std::unique_lock`，保證不會發生 Data Race 或 Torn Read。
- **防死鎖賦值**：拷貝賦值使用 `std::lock` 雙重鎖定預防死鎖。

---

## 2. 畫筆類別 (`Pen`) 與標準集合 (`Pens`)

### `Pen` API
```cpp
explicit Pen(core::Color color, float width = 1.0f);
explicit Pen(const Brush& brush, float width = 1.0f);
Pen(const Pen& other);
Pen& operator=(const Pen& other);

core::Color GetColor() const;
void SetColor(core::Color color);
float GetWidth() const;
void SetWidth(float width); // 若 < 0 擲出 std::invalid_argument

DashStyle GetDashStyle() const;
void SetDashStyle(DashStyle style);
LineCap GetLineCap() const;
void SetLineCap(LineCap cap);
LineJoin GetLineJoin() const;
void SetLineJoin(LineJoin join);

Pen Clone() const;
```

### `Pens` 靜態集合
提供常用色彩且線寬為 1.0f 的標準靜態畫筆引用：
- `Pens::Black()`, `Pens::White()`, `Pens::Red()`, `Pens::Green()`, `Pens::Blue()`, `Pens::Transparent()`, `Pens::SteelBlue()`, `Pens::Gold()` 等。

---

## 3. 畫刷家族 (`Brush`) 與標準集合 (`Brushes`)

### `Brush` 基底與衍生類別
```cpp
class Brush {
public:
    virtual ~Brush() = default;
    virtual BrushType GetType() const noexcept = 0;
    virtual std::unique_ptr<Brush> Clone() const = 0;
};

// 1. 純色畫刷
class SolidBrush : public Brush {
    explicit SolidBrush(core::Color color);
    core::Color GetColor() const;
    void SetColor(core::Color color);
};

// 2. 線性漸層畫刷
class LinearGradientBrush : public Brush {
    LinearGradientBrush(core::Point p1, core::Point p2, core::Color c1, core::Color c2);
    LinearGradientBrush(const core::Rect& rect, core::Color c1, core::Color c2, float angleDegrees);
    core::Point GetStartPoint() const;
    void SetStartPoint(core::Point p);
    core::Point GetEndPoint() const;
    void SetEndPoint(core::Point p);
    core::Color GetStartColor() const;
    void SetStartColor(core::Color c);
    core::Color GetEndColor() const;
    void SetEndColor(core::Color c);
};

// 3. 點陣材質畫刷
class TextureBrush : public Brush {
    explicit TextureBrush(std::shared_ptr<Bitmap> image); // 若 image 為 null 擲出 std::invalid_argument
    std::shared_ptr<Bitmap> GetImage() const;
};
```

### `Brushes` 靜態集合
提供常用色彩的標準靜態純色畫刷引用：
- `Brushes::Black()`, `Brushes::White()`, `Brushes::Red()`, `Brushes::Green()`, `Brushes::Blue()`, `Brushes::Transparent()`, `Brushes::SteelBlue()`, `Brushes::Gold()` 等。

---

## 4. 幾何路徑類別 (`GraphicsPath`)

### 路徑建構與操作 API
```cpp
GraphicsPath();
explicit GraphicsPath(FillMode fillMode);
explicit GraphicsPath(compat::span<const core::Point> points, FillMode fillMode = FillMode::Alternate);

void AddLine(core::Point p1, core::Point p2);
void AddLine(float x1, float y1, float x2, float y2);
void AddLines(compat::span<const core::Point> points);
void AddRectangle(const core::Rect& rect);
void AddRoundedRectangle(const core::Rect& rect, float radius);
void AddEllipse(const core::Rect& rect);
void AddArc(const core::Rect& rect, float startAngle, float sweepAngle);
void AddBezier(core::Point p1, core::Point c1, core::Point c2, core::Point p2);
void AddCurve(compat::span<const core::Point> points);
void AddPolygon(compat::span<const core::Point> points);
void AddPath(const GraphicsPath& path, bool connect = false);

void CloseFigure();
void CloseAllFigures();
void StartFigure();
void Reset();

FillMode GetFillMode() const;
void SetFillMode(FillMode mode);
core::Rect GetBounds() const;
size_t GetPointCount() const;
std::vector<core::Point> GetPathPoints() const;
std::vector<uint8_t> GetPathTypes() const;

// 命中測試
bool IsVisible(core::Point pt) const;
bool IsVisible(float x, float y) const;
bool IsOutlineVisible(core::Point pt, const Pen& pen) const;

// 曲線平整化與矩陣變換
std::vector<core::Point> Flatten(float flatness = 0.25f) const;
void Transform(float m11, float m12, float m21, float m22, float dx, float dy);
GraphicsPath Clone() const;
```

---

## 5. 點陣位圖類別 (`Bitmap`)

### 影像緩衝區 API
```cpp
Bitmap(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::Format32bppArgb);
Bitmap(uint32_t width, uint32_t height, compat::span<const uint32_t> pixels, PixelFormat format = PixelFormat::Format32bppArgb);
explicit Bitmap(compat::string_view filename);

uint32_t GetWidth() const noexcept;
uint32_t GetHeight() const noexcept;
PixelFormat GetPixelFormat() const noexcept;
uint32_t GetStride() const noexcept;

core::Color GetPixel(uint32_t x, uint32_t y) const; // 越界擲出 std::out_of_range
void SetPixel(uint32_t x, uint32_t y, core::Color color); // 越界擲出 std::out_of_range
void Clear(core::Color color);

compat::span<const uint32_t> GetPixelSpan() const;
compat::span<uint32_t> GetPixelSpanMut();

void* LockBits(const core::Rect& rect, uint32_t flags);
void UnlockBits();

std::shared_ptr<Bitmap> Clone(const core::Rect& rect, PixelFormat format) const;
void RotateFlip(RotateFlipType rotateFlipType);
```

---

## 6. 色彩與常數集合 (`Color` & `Colors`)

### `Color` 結構
```cpp
struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    static Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept;
    static Color FromArgb(uint8_t r, uint8_t g, uint8_t b) noexcept;
    static Color FromArgb(uint32_t argb) noexcept;
    static Color FromArgb(uint8_t a, const Color& baseColor) noexcept;

    uint32_t ToArgb() const noexcept;
    bool IsEmpty() const noexcept;
    float GetBrightness() const noexcept;
    float GetHue() const noexcept;
    float GetSaturation() const noexcept;
};
```

### `Colors` 命名空間
涵蓋 GDI+ 完整色盤常數：`Transparent`, `AliceBlue`, `Black`, `White`, `Red`, `Green`, `Blue`, `SteelBlue`, `Gold`, `Coral` 等 140+ 個標準顏色。
