# cppui::controls 命名空間

- **標頭檔目錄**: `include/ui/controls/`

---

## 概述

`cppui::controls` 命名空間收納 CPPUI 跨平台現代桌面 GUI 控制項庫。所有控制項均繼承自 `core::UIElement`，並遵循雙階段排版管線（Measure / Arrange）與 Direct2D/DirectWrite 純量繪圖模型。

---

## 包含類別與控制項

| 類別名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| [`Control`](../classes/controls_control.md) | `Control.hpp` | 所有具樣式外觀與狀態之控制項的基礎類別（Padding, Background, Border 等）。 |
| [`ContentControl`](../classes/controls_contentcontrol.md) | `ContentControl.hpp` | 承載單一內容（文字或自訂子元件）之容器基底。 |
| [`Button`](../classes/controls_button.md) | `Button.hpp` | 具備 Hover、Pressed、Focus Ring 與鍵盤激活狀態機之互動按鈕。 |
| [`CheckBox`](../classes/controls_checkbox.md) | `CheckBox.hpp` | 具備向量勾號繪製、狀態切換、三態 (Indeterminate) 之核取方塊。 |
| [`RadioButton`](../classes/controls_radiobutton.md) | `RadioButton.hpp` | 單選按鈕控制項，支援父容器或 GroupName 互斥單選。 |
| [`TextBox`](../classes/controls_textbox.md) | `TextBox.hpp` | 單行文字編輯輸入框，支援打字、唯讀、選取、密碼遮罩與佔位符。 |
| [`TextBlock`](../classes/controls_textblock.md) | `TextBlock.hpp` | 高效能唯讀純文字/多行文字展示元件。 |
| [`Label`](../classes/controls_label.md) | `Label.hpp` | 內容標籤控制項，支援文字/內容呈現與目標控制項 (Target) 焦點導向。 |
| [`ProgressBar`](../classes/controls_progressbar.md) | `ProgressBar.hpp` | 進度指示條控制項，支援水平/垂直方向、Min/Max/Value 範圍與步進推進。 |
| [`Slider`](../classes/controls_slider.md) | `Slider.hpp` | 數值滑桿控制項，支援滑塊拖曳、軌道點選跳轉與方向鍵步進微調。 |
| [`ListBox`](../classes/controls_listbox.md) | `ListBox.hpp` | 清單方塊控制項，支援項目增刪、單選索引與鍵盤/滑鼠巡覽。 |
| [`Separator`](../classes/controls_separator.md) | `Separator.hpp` | 分隔線控制項，支援水平或垂直分隔線繪製與厚度樣式調整。 |
| [`PictureBox`](../classes/controls_picturebox.md) | `PictureBox.hpp` | 圖片展示控制項，支援 5 種尺寸縮放模式、路徑載入與 Bitmap 繫結。 |
| [`ScrollViewer`](../classes/controls_scrollviewer.md) | `ScrollViewer.hpp` | 捲動視區容器，支援滾輪、拖曳滑塊、邊界剪裁與自動捲軸呈現。 |

---

## 相關列舉型別

| 列舉名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| `CheckState` | `CheckBox.hpp` | 核取狀態 (`Unchecked`, `Checked`, `Indeterminate`)。 |
| `ScrollBarVisibility` | `ScrollViewer.hpp` | 捲軸顯示策略 (`Disabled`, `Auto`, `Hidden`, `Visible`)。 |

