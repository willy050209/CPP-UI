# Graphics 列舉型別完整字典 (Graphics Enums)

CPPUI 圖形子系統中定義的強型別列舉（`enum class`）皆繼承固定寬度整數 `uint8_t`，確保在所有 32 位元與 64 位元硬體架構中皆具備精確一致的記憶體尺寸（`sizeof == 1`），並無縫對齊底層光柵化引擎與 Skia 繪圖後端。

---

## 1. 元資料與標頭檔 (Metadata)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **核心標頭檔**：`#include "ui/graphics/GraphicsTypes.hpp"`
* **路徑標頭檔**：`#include "ui/graphics/GraphicsPath.hpp"`（包含 `PathPointType`）

---

## 2. 完整列舉型別詳細清冊 (Complete Enums Catalog)

```
+-------------------------------------------------------------------------------+
|                       CPPUI Graphics Enumerations                             |
+-------------------------------------------------------------------------------+
|  幾何描邊 (Pen)        幾何路徑 (Path)      光柵品質 (Graphics)    點陣與視圖 (Bitmap)  |
|  - DashStyle          - FillMode          - SmoothingMode      - PixelFormat  |
|  - LineCap            - PathPointType     - InterpolationMode  - PictureBox-  |
|  - LineJoin                                                    -   SizeMode   |
|                                                                - RotateFlip-  |
|  畫刷派發 (Brush)                                               -   Type       |
|  - BrushType                                                                  |
+-------------------------------------------------------------------------------+
```

---

### 2.1 線條虛線樣式：`DashStyle`

定義畫筆 [`Pen`](graphics_pen.md) 描繪線段時的虛線破折與點間距樣式。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class DashStyle : uint8_t`

| 列舉項目 | 數值 | 樣式說明 | 視覺效果示意 |
| :--- | :--- | :--- | :--- |
| `Solid` | `0` | 連續實線（預設值）。 | `─────────────` |
| `Dash` | `1` | 破折虛線（長線段與空白間隔）。 | `──  ──  ──  ──` |
| `Dot` | `2` | 點狀虛線（方點或圓點與空白間隔）。 | `•  •  •  •  •` |
| `DashDot` | `3` | 長短交替線（長線、點、長線、點）。 | `── • ── • ── •` |
| `DashDotDot`| `4` | 雙點交替線（長線、點、點、長線）。 | `── • • ── • •` |
| `Custom` | `5` | 自訂虛線樣式陣列。 | 依使用者自訂比例 |

---

### 2.2 線段端點樣式：`LineCap`

定義開口幾何線段的起始與終止端點封口幾何形狀。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class LineCap : uint8_t`

| 列舉項目 | 數值 | 幾何說明 |
| :--- | :--- | :--- |
| `Flat` | `0` | 平整端點（線段精確終止於起訖坐標，無延伸）。 |
| `Square` | `1` | 方形端點（線段兩端各向外延伸線寬一半的方形帽）。 |
| `Round` | `2` | 圓形端點（線段兩端各以半徑為半線寬的半圓封口）。 |
| `Triangle` | `3` | 三角形端點（以三角形尖端封口）。 |
| `NoAnchor` | `0x10` | 無箭頭錨點。 |
| `SquareAnchor` | `0x11` | 方形錨點。 |
| `RoundAnchor` | `0x12` | 圓形錨點。 |
| `DiamondAnchor`| `0x13` | 菱形錨點。 |
| `ArrowAnchor` | `0x14` | 箭頭形錨點（常用於方向標示與流程圖繪製）。 |

---

### 2.3 線段轉角連接樣式：`LineJoin`

定義兩條相交線段於轉角處的頂點幾何連接型態。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class LineJoin : uint8_t`

| 列舉項目 | 數值 | 幾何說明 |
| :--- | :--- | :--- |
| `Miter` | `0` | 斜接連接（延伸線條外邊緣直至相交，產生銳利的尖角）。 |
| `Bevel` | `1` | 倒角連接（切削銳角，以一直線段平坦削平轉角）。 |
| `Round` | `2` | 圓角連接（以半徑等於半線寬的圓弧平滑連接外邊緣）。 |
| `MiterClipped` | `3` | 夾止斜接（當尖角過於尖銳超過斜接限制時自動平切，避免突刺過長）。 |

---

### 2.4 幾何路徑填色規則：`FillMode`

定義當多邊形或複合幾何路徑發生自相交（Self-Intersecting）或巢狀空洞時，判定區域是否位於路徑內部需填色的演算法規則。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class FillMode : uint8_t`

| 列舉項目 | 數值 | 演算法原理 | 典型場景 |
| :--- | :--- | :--- | :--- |
| `Alternate` | `0` | **奇偶規則 (Even-Odd / Jordan Curve Theorem)**：從測試點向無限遠射出一條射線，計算射線與路徑邊緣的交點總數。若交點為**奇數**則判定在內部（填色），若為**偶數**則判定在外部（留白）。 | 經典五角星中空圖形、同心環空心效果。 |
| `Winding` | `1` | **非零卷繞數規則 (Non-Zero Winding Rule)**：從測試點向無限遠射出射線，每當路徑邊緣由左至右穿過射線時計數 $+1$，由右至左穿過時計數 $-1$。若最終總和**不為零**則判定在內部（填色）。 | 實心五角星、字型輪廓複合圖形。 |

---

### 2.5 平滑化反鋸齒模式：`SmoothingMode`

控制向量幾何邊緣的光柵化品質與反鋸齒 (Anti-Aliasing) 演算。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class SmoothingMode : uint8_t`

| 列舉項目 | 數值 | 說明 |
| :--- | :--- | :--- |
| `Default` | `0` | 系統預設模式（平衡速度與品質）。 |
| `HighSpeed` | `1` | 高速優先模式（關閉耗能的反鋸齒計算，提高幀率）。 |
| `HighQuality`| `2` | 高畫質優先模式（啟用全像素反鋸齒，平滑幾何邊緣）。 |
| `None` | `3` | 關閉反鋸齒（走樣階梯邊緣，最高光柵化速度）。 |
| `AntiAlias` | `4` | 啟用多重採樣 (MSAA) / 超採樣次像素向量反鋸齒。 |

---

### 2.6 影像縮放插值模式：`InterpolationMode`

控制點陣影像（[`Bitmap`](graphics_bitmap.md)）在執行拉伸、壓縮或任意旋轉繪製時的像素採樣過濾演算法。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class InterpolationMode : uint8_t`

| 列舉項目 | 數值 | 演算法說明 | 適用場景 |
| :--- | :--- | :--- | :--- |
| `Default` | `0` | 預設採樣模式。 | 一般介面圖片顯示。 |
| `Low` | `1` | 低品質快速採樣。 | 縮圖快速預覽。 |
| `High` | `2` | 高品質雙三次或多階採樣。 | 高解析度精緻相片顯示。 |
| `Bilinear` | `3` | **雙線性插值 (Bilinear)**：依據周圍 $2 \times 2$ 個像素進行加權平均。 | 畫面流暢、運算成本適中。 |
| `Bicubic` | `4` | **雙三次插值 (Bicubic)**：依據周圍 $4 \times 4$ 個像素進行三次多項式取樣。 | 平滑邊緣，防止模糊塊狀。 |
| `NearestNeighbor` | `5` | **最近鄰取樣 (Nearest Neighbor)**：直接取最近整數像素，不混色。 | 像素藝術 (Pixel Art)、復古遊戲、QR Code。 |
| `HighQualityBilinear` | `6` | 高精度雙線性預乘濾波。 | High-DPI 視網膜縮放。 |
| `HighQualityBicubic` | `7` | 最高精度雙三次重取樣（無鋸齒、抑制莫爾紋）。 | 專業影像輸出。 |

---

### 2.7 像素資料格式：`PixelFormat`

定義點陣影像緩衝區每個像素的色彩位元深度與通道組織格式。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class PixelFormat : uint8_t`

| 列舉項目 | 數值 | 位元深度 | 通道排列與說明 |
| :--- | :--- | :--- | :--- |
| `Format32bppArgb` | `0` | 32-bit (4 位元組) | **標準格式**：Alpha (8)、Red (8)、Green (8)、Blue (8)。支援完整半透明度。 |
| `Format32bppRgb` | `1` | 32-bit (4 位元組) | 未使用 Alpha（Alpha 固定忽略或為 255），RGB 各佔 8 位元。 |
| `Format24bppRgb` | `2` | 24-bit (3 位元組) | 緊湊 RGB 三原色（無 Alpha 通道）。 |
| `Format8bppIndexed`| `3` | 8-bit (1 位元組) | 256 色調色盤索引模式。 |

---

### 2.8 PictureBox 圖片呈現模式：`PictureBoxSizeMode`

定義 [`PictureBox`](controls_picturebox.md) 控制項在視窗尺寸改變時對目標影像的佈局與自適應縮放策略。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class PictureBoxSizeMode : uint8_t`

| 列舉項目 | 數值 | 排版與縮放行為 |
| :--- | :--- | :--- |
| `Normal` | `0` | **原寸左上對齊**：影像置於控制項左上角 `(0, 0)`，以 1:1 像素原寸呈現，超出部分自動被控制項邊界剪裁。 |
| `StretchImage` | `1` | **強制填滿拉伸**：將影像的寬度與高度完全拉伸至與 PictureBox 相同，不保持原始長寬比（可能變形）。 |
| `AutoSize` | `2` | **控制項自動適應**：控制項將自身大小（Width/Height）自動調整為影像的實際像素尺寸。 |
| `CenterImage` | `3` | **置中無縮放**：影像置於控制項正中央，以 1:1 原寸呈現，超出部分被剪裁。 |
| `Zoom` | `4` | **等比例縮放適應**：保持影像原始寬高比（Aspect Ratio），將影像最大化縮放至控制項客戶區內，並居中留白。 |

---

### 2.9 畫刷具體類型列舉：`BrushType`

提供執行期多型識別，供底層光柵器與 Skia 分發不同填色著色器。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class BrushType : uint8_t`

| 列舉項目 | 數值 | 對應具體類別 |
| :--- | :--- | :--- |
| `SolidColor` | `0` | [`SolidBrush`](graphics_brush.md#4-純色畫刷solidbrush) |
| `LinearGradient` | `1` | [`LinearGradientBrush`](graphics_brush.md#5-線性漸層畫刷lineargradientbrush) |
| `Texture` | `2` | [`TextureBrush`](graphics_brush.md#6-點陣材質畫刷texturebrush) |

---

### 2.10 影像旋轉與翻轉：`RotateFlipType`

定義點陣影像在進行 2D 矩陣旋轉與鏡像對稱變換時的 7 種正交組合。

* **定義檔**：`GraphicsTypes.hpp`
* **型別**：`enum class RotateFlipType : uint8_t`

| 列舉項目 | 數值 | 順時針旋轉角度 | 水平翻轉 (Flip X) | 垂直翻轉 (Flip Y) |
| :--- | :--- | :--- | :--- | :--- |
| `RotateNoneFlipNone` | `0` | 0° (原狀) | 否 | 否 |
| `Rotate90FlipNone` | `1` | 90° | 否 | 否 |
| `Rotate180FlipNone` | `2` | 180° | 否 | 否 |
| `Rotate270FlipNone` | `3` | 270° | 否 | 否 |
| `RotateNoneFlipX` | `4` | 0° | **是 (左右鏡像)** | 否 |
| `RotateNoneFlipY` | `5` | 0° | 否 | **是 (上下鏡像)** |
| `RotateNoneFlipXY` | `6` | 0° | **是** | **是 (等效 180° 旋轉)** |

---

### 2.11 路徑控制點標記：`PathPointType`

定義複合幾何路徑（[`GraphicsPath`](graphics_path.md)）內部頂點陣列中每個控制點的幾何意義與拓撲狀態。

* **定義檔**：`GraphicsPath.hpp`
* **型別**：`enum class PathPointType : uint8_t`

| 列舉項目 | 數值 | 意義說明 |
| :--- | :--- | :--- |
| `Start` | `0` | 新子圖形（Figure）的起始點。 |
| `Line` | `1` | 與前一點相連的直線段終點。 |
| `Bezier` | `3` | 三次貝茲曲線之控制點或終點。 |
| `CloseSubpath` | `0x80` | 位元旗標：標記此點為該子路徑的閉合點，指示光柵器自動連回該子圖形之起點。 |

---

## 3. 程式碼範例 (Examples)

### 3.1 配置高畫質反鋸齒與雙三次插值

```cpp
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

void SetupHighQualityCanvas(cppui::graphics::Graphics& g) {
    // 啟用高品質邊緣反鋸齒
    g.SetSmoothingMode(cppui::graphics::SmoothingMode::AntiAlias);

    // 影像縮放採用高品質雙三次插值濾鏡
    g.SetInterpolationMode(cppui::graphics::InterpolationMode::HighQualityBicubic);
}
```

### 3.2 自訂圓角虛線畫筆

```cpp
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Colors.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

cppui::graphics::Pen CreateBlueprintPen() {
    cppui::graphics::Pen pen(cppui::graphics::Colors::DodgerBlue, 2.0f);
    pen.SetDashStyle(cppui::graphics::DashStyle::DashDot);
    pen.SetLineCap(cppui::graphics::LineCap::Round);
    pen.SetLineJoin(cppui::graphics::LineJoin::Round);
    return pen;
}
```

---

## 4. 請參閱 (See Also)

* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
* [`Pen` 畫筆類別詳解](graphics_pen.md)
* [`Brush` 畫刷家族詳解](graphics_brush.md)
* [`Bitmap` 點陣影像詳解](graphics_bitmap.md)
* [`GraphicsPath` 複合幾何路徑](graphics_path.md)
* [`PictureBox` 圖片顯示控制項](controls_picturebox.md)
