# 2D 向量繪圖與影像濾鏡展示範例 (Graphics Drawing Demo)

本範例展示 **CPPUI** 的 GDI+ 風格進階繪圖引擎 [`Graphics`](../../docs/api/classes/graphics_graphics.md) 與並發影像處理管線 [`ImageFilters`](../../docs/api/classes/graphics_imagefilters.md)。涵蓋抗鋸齒幾何路徑、漸層畫刷、點陣圖檔載入、影像縮放與即時灰階（Grayscale）濾鏡處理。

---

## 1. 成果預覽 (Preview)

| 全彩向量繪圖與影像展示 | 灰階濾鏡處理後效果 (`--grayscale`) |
| :---: | :---: |
| ![全彩圖形繪製預覽](../../docs/images/graphics_drawing_preview.png) | ![灰階濾鏡處理預覽](../../docs/images/graphics_grayscale_preview.png) |

---

## 2. 範例定位與功能特色

- **直覺式 2D 向量繪圖**：提供開發者熟悉的 `Pen`, `Brush`, `LinearGradientBrush`, `GraphicsPath` 繪圖物件。
- **抗鋸齒與混合模式**：支援高精度直線、橢圓、貝茲曲線與多邊形填充。
- **高效多執行緒影像處理**：整合 `ImageFilters::Grayscale()`，使用現代 SIMD / 平行迴圈對 32-bit ARGB 點陣圖進行高精度灰階轉換。
- **動態濾鏡切換**：透過點擊按鈕或傳入 `--grayscale` 參數，即可即時在全彩與灰階模式間熱切換。

---

## 3. 核心原始碼解析

### 3.1 GDI+ 風格畫布繪製
在表單自訂畫布中透過 `Graphics` 進行向量幾何繪製：

```cpp
void DrawingDemoForm::OnPaint(graphics::Graphics& g) {
    // 1. 繪製抗鋸齒圓角卡片與漸層
    graphics::LinearGradientBrush gradient(
        core::Point(20, 20), core::Point(300, 200),
        core::Color(59, 130, 246), core::Color(147, 51, 234)
    );
    g.FillRoundedRectangle(gradient, core::Rect(20, 20, 280, 160), 12.0f);

    // 2. 幾何輪廓繪製
    graphics::Pen outlinePen(core::Color(255, 255, 255, 200), 2.0f);
    g.DrawRoundedRectangle(outlinePen, core::Rect(20, 20, 280, 160), 12.0f);

    // 3. 繪製幾何路徑 (GraphicsPath)
    graphics::GraphicsPath path;
    path.AddEllipse(core::Rect(340, 40, 100, 100));
    g.FillPath(graphics::SolidBrush(core::Color(245, 158, 11)), path);
}
```

### 3.2 影像濾鏡處理
在記憶體中對載入之點陣圖執行灰階變換：

```cpp
void DrawingDemoForm::ApplyGrayscaleFilter() {
    if (m_originalBitmap) {
        m_filteredBitmap = graphics::ImageFilters::Grayscale(*m_originalBitmap);
        InvalidateForm();
    }
}
```

---

## 4. 建置與執行方式

### 透過 CMake 建置
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target graphics_drawing_demo
```

### 執行範例
```powershell
# 1. 預設全彩模式
.\build\bin\Release\graphics_drawing_demo.exe

# 2. 啟動即套用灰階濾鏡模式
.\build\bin\Release\graphics_drawing_demo.exe --grayscale
```
