# CPPUI 圖形一級物件核心綜述與導航中心 (Core Graphics Objects Hub)

命名空間：[`cppui::graphics`](../namespaces/cppui_graphics.md)  
目錄：`include/ui/graphics/`  

---

## 1. 架構設計哲學 (Design Philosophy)

在傳統 Windows GDI 中，畫筆、畫刷、字型與位圖皆為系統核心資源（GDI Handles），操作極易發生洩漏或狀態污染。CPPUI 借鏡 GDI+ (`System.Drawing`) 的設計，將繪圖要素徹底解耦為獨立、可重用且型別安全的一級領域物件（First-Class Citizens）：

```
                       +---------------------------------------+
                       |   Graphics 繪圖中樞 (排程器)          |
                       +-------------------+-------------------+
                                           | 調度繪製
         +-----------------+---------------+---------------+-----------------+
         |                 |                               |                 |
         v                 v                               v                 v
+-----------------+ +-----------------+             +-----------------+ +-----------------+
|  Pen / Pens     | |  Brush / Brushes|             |  GraphicsPath   | |  Bitmap         |
|  (輪廓描邊樣式) | |  (區域填滿樣式) |             |  (複合幾何路徑) | |  (32-bit 點陣)  |
+-----------------+ +-----------------+             +-----------------+ +-----------------+
         |                 |                               |                 |
         +-----------------+---------------+---------------+-----------------+
                                           | 色彩依賴
                                           v
                                +---------------------+
                                |   Color / Colors    |
                                | (32-bit ARGB 色彩)  |
                                +---------------------+
```

---

## 2. 跨執行緒自動同步模型 (Thread Safety Architecture)

所有 CPPUI 圖形一級物件（`Pen`, `SolidBrush`, `LinearGradientBrush`, `TextureBrush`, `Bitmap`, `GraphicsPath`）內部均封裝 `mutable std::shared_mutex`：

1. **並行讀取（Shared Read, 零阻塞）**：
   - 包含色彩查詢（`GetColor`）、像素採樣（`GetPixel`）、邊界計算（`GetBounds`）、命中測試（`IsVisible`）與幾何複製（`Clone`）。
   - 自動獲取 `std::shared_lock`，多個背景執行緒與主 UI 渲染執行緒可同時並發存取。
2. **排他修改（Unique Write, 互斥防衛）**：
   - 包含屬性修改（`SetColor`, `SetWidth`, `SetPixel`）、路徑圖元追加（`AddLine`, `AddBezier`）等變更狀態操作。
   - 自動獲取 `std::unique_lock`，杜絕任何 Data Race 與記憶體撕裂（Torn Read）。
3. **防死鎖賦值協議（Deadlock-Free Assignment）**：
   - 拷貝賦值運算子（`operator=`）採用 `std::lock(m_mutex, other.m_mutex)` 雙重鎖定協議，徹底杜絕多執行緒交互拷貝時的死鎖隱患。

---

## 3. 獨立物件專屬技術文件索引 (Dedicated Documentation Index)

為了提供最詳盡的 API 參數規範、異常行為、記憶體規格與使用範例，每個一級物件已拆分為獨立專屬技術文件：

| 領域物件 | 類別 / 命名空間 | 標頭檔 | 專屬技術文件連結 | 核心亮點 |
| :--- | :--- | :--- | :--- | :--- |
| **色彩資料結構** | `cppui::core::Color` | `Geometry.hpp` | [**Color 結構詳解**](graphics_color.md) | 32-bit 緊湊佈局、`constexpr`、ARGB 打包、HSL 明度/色相/飽和度分析。 |
| **預定義色盤** | `cppui::graphics::Colors` | `Colors.hpp` | [**Colors 調色盤常數**](graphics_colors.md) | 140+ GDI+ 標準現代色碼表、十六進位與 RGB 對照。 |
| **畫筆類別** | `cppui::graphics::Pen` | `Pen.hpp` | [**Pen 類別詳解**](graphics_pen.md) | 描邊線寬、虛線樣式、端點封口、轉角連接、執行緒安全。 |
| **標準畫筆集合** | `cppui::graphics::Pens` | `Pens.hpp` | [**Pens 集合詳解**](graphics_pens.md) | 28 種標準 1.0f 寬度靜態唯讀畫筆單例（Meyer's Singleton）。 |
| **畫刷家族** | `Brush` / `SolidBrush` / `LinearGradientBrush` / `TextureBrush` | `Brush.hpp` | [**Brush 家族詳解**](graphics_brush.md) | 純色填滿、雙色空間線性漸層插值 (`GetColorAt`)、紋理貼圖、讀寫鎖防護。 |
| **標準畫刷集合** | `cppui::graphics::Brushes` | `Brushes.hpp` | [**Brushes 集合詳解**](graphics_brushes.md) | 28 種標準靜態純色畫刷單例，零重複配置開銷。 |
| **複合幾何路徑** | `cppui::graphics::GraphicsPath` | `GraphicsPath.hpp`| [**GraphicsPath 類別詳解**](graphics_path.md) | 貝茲曲線、樣條、路徑展開平整化 (`Flatten`)、仿射變換 (`Transform`)、射線命中測試。 |
| **點陣影像** | `cppui::graphics::Bitmap` | `Bitmap.hpp` | [**Bitmap 類別詳解**](graphics_bitmap.md) | 32-bit 點陣、邊界越界防護、`LockBits` 高性能直讀緩衝區、90° 步進旋轉翻轉。 |
| **列舉型別字典** | `DashStyle`, `LineCap`, `FillMode`, `SmoothingMode`, `PixelFormat` 等 | `GraphicsTypes.hpp` | [**Graphics 列舉字典**](graphics_enums.md) | 11 種核心列舉型別規格、數值與演算法原理詳解。 |
| **繪圖中樞排程器** | `cppui::graphics::Graphics` | `Graphics.hpp` | [**Graphics 類別詳解**](graphics_graphics.md) | 雙模式（UI vs 離線影像）、狀態矩陣管理、幾何繪製/填滿、文字排版、圖像渲染。 |

---

## 4. 相關指南與控制項

* [**GDI+ 2D 圖形、Skia 加速與非同步影像開發指南**](../guides/gdi_plus_graphics_guide.md)
* [**PictureBox 圖片控制項規格**](controls_picturebox.md)
* [**cppui::graphics 命名空間總覽**](../namespaces/cppui_graphics.md)
