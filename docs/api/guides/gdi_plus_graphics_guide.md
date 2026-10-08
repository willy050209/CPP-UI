# GDI+ 風格 2D 圖形、Skia 加速與非同步影像開發指南 (GDI+ 2D Graphics Guide)

歡迎閱讀 CPPUI 2D 向量繪圖與影像子系統指南。本指南將帶領您了解如何使用 CPPUI 提供的 GDI+ 物件導向圖形 API、結合 Skia 向量硬體加速與純 CPU 軟體光柵、多執行緒自動同步機制以及 `PictureBox` 控制項。

---

## 1. 核心概念與架構分離 (Separation of Concerns)

傳統 Windows GDI 的缺點在於所有狀態綁定於單一的裝置上下文（Device Context, DC），容易引發狀態洩漏、色彩污染與難以跨執行緒操作的問題。
CPPUI 借鏡 GDI+ (`System.Drawing`) 的設計，貫徹關注點分離 (SoC)：
- **`Pen`（畫筆）**：定義輪廓描邊樣式。
- **`Brush`（畫刷）**：定義填滿樣式。
- **`GraphicsPath`（幾何路徑）**：定義由線段、曲線、圓弧構成的複合向量輪廓。
- **`Bitmap`（點陣影像）**：可獨立於顯示裝置存在之記憶體點陣緩衝區。
- **`Graphics`（繪圖排程器）**：負責接收繪圖物件並統一分發繪圖命令。

```
+--------------------+        +---------------------+
| Pen / Brush / Path | -----> |      Graphics       |
+--------------------+        +----------+----------+
                                         |
               +-------------------------+-------------------------+
               |                                                   |
       [ UI 即時模式 ]                                     [ 離線點陣模式 ]
               |                                                   |
  FromDrawingContext(dc)                                    FromImage(bitmap)
               |                                                   |
    DisplayList 指令串流                                 記憶體光柵化引擎
               |                                                   |
   Skia GPU 向量加速管線                                 Bitmap 像素緩衝區
```

---

## 2. 跨執行緒自動同步機制 (Automatic Multi-Threading Synchronization)

在傳統圖形庫中，跨執行緒存取同一個 Bitmap 或 Path 極易發生 Data Race 或 Torn Read。
CPPUI 的圖形物件內建 `std::shared_mutex` 讀寫鎖：
1. **多讀取者無鎖阻塞**：`bmp->GetPixel(...)`、`path->GetBounds()` 等查詢操作自動獲取共用讀鎖 (`std::shared_lock`)，多個背景執行緒與 UI 渲染執行緒可同時採樣。
2. **排他安全寫入**：`bmp->SetPixel(...)`、`path->AddLine(...)` 等寫入操作自動獲取獨佔寫鎖 (`std::unique_lock`)。
3. **區塊極速存取**：需要每秒處理數百萬像素的影像濾鏡或演算法，可使用 `bmp->LockBits(...)` 與 `bmp->UnlockBits()` 在區塊範圍內進行指標級別極速操作。

### 非同步影像生成範例
```cpp
auto sharedBmp = std::make_shared<graphics::Bitmap>(800, 600);
sharedBmp->Clear(graphics::Colors::White);

// 背景執行緒非同步生成碎形或圖形
std::thread worker([sharedBmp]() {
    graphics::Graphics g = graphics::Graphics::FromImage(sharedBmp);
    for (int32_t i = 0; i < 100; ++i) {
        g.DrawEllipse(graphics::Pens::SteelBlue(), core::Rect(i * 5, i * 5, 100, 100));
    }
    g.Flush();
});

// UI 執行緒即時顯示 PictureBox (安全並發讀取，無崩潰與競爭)
auto pic = PictureBox(sharedBmp)
    .SizeMode(graphics::PictureBoxSizeMode::Zoom)
    .Width(400)
    .Height(300);

worker.join();
```

---

## 3. 雙引擎架構：Skia 向量加速與 CPU 軟體備援

CPPUI 原生支援雙渲染後端：
- **Google Skia 向量加速**：
  - 偵測到 Skia 庫時（`CPPUI_HAS_SKIA=1`），繪圖指令直通 `SkCanvas::drawOval`、`SkCanvas::drawPath` 與 `SkCanvas::drawImageRect`。
  - 提供 GPU 頂點處理、MSAA 多重取樣反鋸齒與 High-DPI 視網膜螢幕超取樣。
- **純 CPU 軟體光柵備援 (Software Fallback)**：
  - 在無顯卡驅動、Headless 伺服器或未編譯 Skia 靜態庫的環境下 100% 獨立運行。
  - 實作中點橢圓演算法、邊緣列表 (Edge Table) 掃描線多邊形填充、雙線性插值 (Bilinear Interpolation) 圖像拉伸縮放與 Alpha 混色合成。

---

## 4. 相關第三方函式庫授權宣告 (Licenses & Acknowledgments)

CPPUI 圖形子系統整合與參考了以下開源專案之設計與實作：

### 1. Google Skia
- **授權條款**：BSD 3-Clause License
- **版權宣告**：Copyright (c) 2011 Google Inc. All rights reserved.
- **說明**：用於提供高效能 2D 向量繪圖、GPU 渲染與進階影像合成管線。

### 2. Microsoft DirectWrite
- **授權條款**：Microsoft Windows SDK License
- **說明**：用於在 Windows 平台提供原生硬體加速文字排版與塑形。

### 3. HarfBuzz & ICU
- **授權條款**：Old MIT License / Unicode License
- **說明**：用於跨平台雙向文字 (BiDi) 分析與 OpenType 複合字形排版。
