# cppui::graphics 命名空間

- **標頭檔目錄**: `include/ui/graphics/`

---

## 概述

`cppui::graphics` 命名空間提供對標 GDI+ 的現代進階 2D 向量與點陣圖形 API。透過將畫筆 (`Pen`)、畫刷 (`Brush`)、幾何路徑 (`GraphicsPath`) 與點陣位圖 (`Bitmap`) 抽離為獨立的一級物件，由統一的排程中樞類別 `Graphics` 協調底層繪圖，並自動同步多執行緒對同一個圖形物件的存取。

後端無縫銜接 Google Skia 向量硬體加速引擎與純 CPU 軟體光柵管線，同時具備離線記憶體位圖繪製 (`Graphics::FromImage`) 與即時視窗上下文繪製 (`Graphics::FromDrawingContext`) 雙重能力。

---

## 包含類別、常數與圖形物件

| 類別 / 命名空間 | 標頭檔 | 專屬技術文件 | 說明 |
| :--- | :--- | :--- | :--- |
| [`Graphics`](../classes/graphics_graphics.md) | `Graphics.hpp` | [**Graphics 類別**](../classes/graphics_graphics.md) | 核心圖形排程器，支援雙模式建構、幾何圖元繪製、路徑填充、影像繪製與變換/剪裁矩陣管理。 |
| [`Pen`](../classes/graphics_pen.md) | `Pen.hpp` | [**Pen 類別**](../classes/graphics_pen.md) | 封裝描邊色彩、寬度、虛線樣式、端點與連接點樣式的一級畫筆物件，具備執行緒同步能力。 |
| [`Pens`](../classes/graphics_pens.md) | `Pens.hpp` | [**Pens 標準集合**](../classes/graphics_pens.md) | 對標 GDI+ 的常用 28 種標準 1.0f 靜態畫筆集合（Black, White, Red, Blue, SteelBlue 等）。 |
| [`Brush`](../classes/graphics_brush.md) | `Brush.hpp` | [**Brush 家族**](../classes/graphics_brush.md) | 抽象畫刷基底，支援多型複製 (`Clone()`) 與執行緒同步。 |
| [`SolidBrush`](../classes/graphics_brush.md#4-純色畫刷solidbrush) | `Brush.hpp` | [**SolidBrush 類別**](../classes/graphics_brush.md#4-純色畫刷solidbrush) | 純色填充畫刷。 |
| [`LinearGradientBrush`](../classes/graphics_brush.md#5-線性漸層畫刷lineargradientbrush) | `Brush.hpp` | [**LinearGradientBrush 類別**](../classes/graphics_brush.md#5-線性漸層畫刷lineargradientbrush) | 線性漸層填充畫刷，支援雙端點、角度與空間坐標雙色插值 (`GetColorAt`)。 |
| [`TextureBrush`](../classes/graphics_brush.md#6-點陣材質畫刷texturebrush) | `Brush.hpp` | [**TextureBrush 類別**](../classes/graphics_brush.md#6-點陣材質畫刷texturebrush) | 紋理平鋪畫刷，以點陣圖像為填充基底。 |
| [`Brushes`](../classes/graphics_brushes.md) | `Brushes.hpp` | [**Brushes 標準集合**](../classes/graphics_brushes.md) | 對標 GDI+ 的常用 28 種標準靜態純色畫刷集合（零重複配置開銷）。 |
| [`GraphicsPath`](../classes/graphics_path.md) | `GraphicsPath.hpp` | [**GraphicsPath 類別**](../classes/graphics_path.md) | 複合幾何路徑，支援線段、矩形、圓角矩形、橢圓、貝茲曲線、平整化 (`Flatten`)、仿射變換與射線命中測試。 |
| [`Bitmap`](../classes/graphics_bitmap.md) | `Bitmap.hpp` | [**Bitmap 類別**](../classes/graphics_bitmap.md) | 32-bit 點陣圖像緩衝區，支援像素讀寫、記憶體鎖定 (`LockBits`)、裁切複製與 90° 步進旋轉翻轉 (`RotateFlip`)。 |
| [`Color`](../classes/graphics_color.md) | `Geometry.hpp` | [**Color 結構**](../classes/graphics_color.md) | 32-bit RGBA 色彩結構，支援 FromArgb、ToArgb、HSL 分析與 `constexpr`。 |
| [`Colors`](../classes/graphics_colors.md) | `Colors.hpp` | [**Colors 調色盤常數**](../classes/graphics_colors.md) | 對標 GDI+ 完整 140+ 種標準現代色彩常數（十六進位 ARGB 與 RGB 對照）。 |
| [`GraphicsTypes`](../classes/graphics_enums.md) | `GraphicsTypes.hpp` | [**Graphics 列舉字典**](../classes/graphics_enums.md) | 核心繪圖列舉型別（`DashStyle`, `LineCap`, `FillMode`, `SmoothingMode`, `InterpolationMode`, `PixelFormat`, `PictureBoxSizeMode` 等）。 |
| [**圖形物件核心導航**](../classes/graphics_objects.md) | `include/ui/graphics/` | [**Graphics Objects Hub**](../classes/graphics_objects.md) | 圖形一級領域物件綜述、責任劃分與多執行緒讀寫鎖同步機制。 |

---

## 相關指南
* [**GDI+ 風格圖形 API 與 PictureBox 控制項實戰指南**](../guides/gdi_plus_graphics_guide.md)
* [**PictureBox 圖片控制項規格**](../classes/controls_picturebox.md)
