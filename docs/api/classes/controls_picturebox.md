# PictureBox 圖片控制項 (PictureBox Control)

命名空間：`cppui::controls`  
標頭檔：`include/ui/controls/PictureBox.hpp`  
實作檔：`src/controls/PictureBox.cpp`  
基底類別：`cppui::controls::Control`  

---

## 1. 控制項概述 (Overview)

`PictureBox` 控制項專為影像與點陣圖像呈現設計，提供直覺的影像顯示與豐富的排版尺寸模式。
繼承自 `Control`，原生支援外觀樣式（邊框、圓角、內外距）與雙階段佈局管線（`MeasureOverride` / `ArrangeOverride`），並可無縫整合 Fluent RAD DSL 鏈式宣告與 MVVM 響應式信號綁定。

---

## 2. 尺寸呈現模式 (`PictureBoxSizeMode`)

| 模式名稱 | 列舉值 | 排版行為與影像呈現 |
| :--- | :--- | :--- |
| **Normal** | `PictureBoxSizeMode::Normal` | 影像置於左上角，若影像超出邊界則裁切超出部分。 |
| **StretchImage** | `PictureBoxSizeMode::StretchImage` | 影像強制拉伸以填滿整個控制項視口，不維持原始長寬比。 |
| **AutoSize** | `PictureBoxSizeMode::AutoSize` | 控制項尺寸自動調整至與影像的實際寬高像素完全一致。 |
| **CenterImage** | `PictureBoxSizeMode::CenterImage` | 影像置於控制項中心，超出部分裁切，不縮放影像。 |
| **Zoom** | `PictureBoxSizeMode::Zoom` | 維持原始影像長寬比，等比例縮放以完整容納於控制項內部。 |

---

## 3. API 語法與成員函數 (API Signatures)

```cpp
PictureBox();
explicit PictureBox(std::shared_ptr<graphics::Bitmap> image);

std::shared_ptr<graphics::Bitmap> GetImage() const;
void SetImage(std::shared_ptr<graphics::Bitmap> image);

graphics::PictureBoxSizeMode GetSizeMode() const;
void SetSizeMode(graphics::PictureBoxSizeMode mode);
```

---

## 4. 宣告式 Fluent DSL 語法糖 (Fluent RAD Usage)

透過 `#include "ui/dsl/Fluent.hpp"`，可使用鏈式語法極速宣告與綁定 `PictureBox`：

```cpp
using namespace cppui::dsl;
using namespace cppui::graphics;

// 1. 基本鏈式宣告
auto pic = PictureBox(myBitmap)
    .SizeMode(PictureBoxSizeMode::Zoom)
    .Width(300)
    .Height(200)
    .Background(Colors::LightGray)
    .CornerRadius(8.0f);

// 2. MVVM 響應式信號綁定
reactive::Signal<std::shared_ptr<Bitmap>> currentAvatar(defaultAvatar);

auto avatarPic = PictureBox()
    .BindImage(currentAvatar)
    .SizeMode(PictureBoxSizeMode::CenterImage)
    .Width(64)
    .Height(64);

// 當信號變更時，PictureBox 自動切換影像並觸發局部重繪
currentAvatar.Set(newAvatar);
```
