# cppui::graphics 命名空間

- **標頭檔目錄**: `include/ui/graphics/`

---

## 概述

`cppui::graphics` 命名空間提供對標 GDI+ 的現代進階 2D 向量與點陣圖形 API。透過將畫筆 (`Pen`)、畫刷 (`Brush`)、幾何路徑 (`GraphicsPath`) 與點陣位圖 (`Bitmap`) 抽離為獨立的一級物件，由統一的排程中樞類別 `Graphics` 協調底層繪圖，並自動同步多執行緒對同一個圖形物件的存取。

後端無縫銜接 Google Skia 向量硬體加速引擎與純 CPU 軟體光柵管線，同時具備離線記憶體位圖繪製 (`Graphics::FromImage`) 與即時視窗上下文繪製 (`Graphics::FromDrawingContext`) 雙重能力。

---

## 包含類別與圖形物件

| 類別名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| [`Graphics`](../classes/graphics_graphics.md) | `Graphics.hpp` | 核心圖形排程器，支援雙模式建構、幾何圖元繪製、路徑填充、影像繪製與變換/剪裁矩陣管理。 |
| [`Pen`](../classes/graphics_objects.md#1-pen-類別) | `Pen.hpp` | 封裝描邊色彩、寬度、虛線樣式、端點與連接點樣式的一級畫筆物件，具備執行緒同步能力。 |
| [`Brush`](../classes/graphics_objects.md#2-brush-類別族) | `Brush.hpp` | 抽象畫刷基底，支援多型複製 (`Clone()`) 與執行緒同步。 |
| [`SolidBrush`](../classes/graphics_objects.md#2-brush-類別族) | `Brush.hpp` | 純色填充畫刷。 |
| [`LinearGradientBrush`](../classes/graphics_objects.md#2-brush-類別族) | `Brush.hpp` | 線性漸層填充畫刷，支援雙端點與雙色插值。 |
| [`TextureBrush`](../classes/graphics_objects.md#2-brush-類別族) | `Brush.hpp` | 紋理平鋪畫刷，以點陣圖像為填充基底。 |
| [`GraphicsPath`](../classes/graphics_objects.md#3-graphicspath-類別) | `GraphicsPath.hpp` | 複合幾何路徑，支援線段、矩形、圓角矩形、橢圓、貝茲曲線、平整化 (`Flatten`) 與射線命中測試。 |
| [`Bitmap`](../classes/graphics_objects.md#4-bitmap-類別) | `Bitmap.hpp` | 32-bit 點陣圖像緩衝區，支援像素讀寫、記憶體鎖定 (`LockBits`)、裁切複製與 90° 步進旋轉翻轉 (`RotateFlip`)。 |
| [`Pens`](../classes/graphics_objects.md#5-pens-常數集合) | `Pens.hpp` | 對標 GDI+ 的常用標準靜態畫筆集合（Black, White, Red, Blue, SteelBlue 等）。 |
| [`Brushes`](../classes/graphics_objects.md#6-brushes-常數集合) | `Brushes.hpp` | 對標 GDI+ 的常用標準靜態純色畫刷集合。 |
| [`Colors`](../classes/graphics_objects.md) | `Colors.hpp` | 對標 GDI+ 完整 140+ 種標準現代色彩常數。 |
| [`GraphicsTypes`](../classes/graphics_objects.md) | `GraphicsTypes.hpp` | 核心繪圖列舉型別（`DashStyle`, `LineCap`, `FillMode`, `SmoothingMode`, `PixelFormat` 等）。 |

---

## 相關指南
* [**GDI+ 風格圖形 API 與 PictureBox 控制項實戰指南**](../guides/gdi_plus_graphics_guide.md)
