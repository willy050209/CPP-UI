# Brushes 標準純色畫刷集合 (cppui::graphics::Brushes)

`Brushes` 類別借鏡 GDI+ (`System.Drawing.Brushes`)，為 CPPUI 提供 28 種最常用標準色彩的靜態唯讀純色畫刷集合（Stock Brushes）。

---

## 1. 元資料與規格 (Metadata & Lifecycle)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/Brushes.hpp"`
* **實作檔**：`src/graphics/Brushes.cpp`
* **類別特性**：純靜態工廠類別，建構子被顯式刪除（`Brushes() = delete`），無法被實例化。
* **生命週期管理**：
  - 採用 C++11 Meyer's Singleton（函數內部靜態變數）延遲初始化。
  - 回傳 `const SolidBrush&` 常數引用。
  - **零重複記憶體配置**：無論在整個應用程式中被呼叫多少次，全生命週期中每一種標準畫刷僅配置一次，並於程式退出時安全解構。
* **執行緒安全性**：所有回傳之畫刷實例皆為唯讀不變物件（Immutable），多執行緒並發呼叫安全無鎖。

---

## 2. 類別語法與宣告 (Class Syntax)

```cpp
namespace cppui {
namespace graphics {

class Brushes {
public:
    Brushes() = delete;

    COMPAT_NODISCARD static const SolidBrush& Transparent();
    COMPAT_NODISCARD static const SolidBrush& Black();
    COMPAT_NODISCARD static const SolidBrush& White();
    COMPAT_NODISCARD static const SolidBrush& Red();
    COMPAT_NODISCARD static const SolidBrush& Green();
    COMPAT_NODISCARD static const SolidBrush& Blue();
    COMPAT_NODISCARD static const SolidBrush& Yellow();
    COMPAT_NODISCARD static const SolidBrush& Cyan();
    COMPAT_NODISCARD static const SolidBrush& Magenta();
    COMPAT_NODISCARD static const SolidBrush& Gray();
    COMPAT_NODISCARD static const SolidBrush& LightGray();
    COMPAT_NODISCARD static const SolidBrush& DarkGray();
    COMPAT_NODISCARD static const SolidBrush& Orange();
    COMPAT_NODISCARD static const SolidBrush& Purple();
    COMPAT_NODISCARD static const SolidBrush& Brown();
    COMPAT_NODISCARD static const SolidBrush& SteelBlue();
    COMPAT_NODISCARD static const SolidBrush& Navy();
    COMPAT_NODISCARD static const SolidBrush& Gold();
    COMPAT_NODISCARD static const SolidBrush& Lime();
    COMPAT_NODISCARD static const SolidBrush& Pink();
    COMPAT_NODISCARD static const SolidBrush& DarkRed();
    COMPAT_NODISCARD static const SolidBrush& DarkBlue();
    COMPAT_NODISCARD static const SolidBrush& DarkGreen();
    COMPAT_NODISCARD static const SolidBrush& Olive();
    COMPAT_NODISCARD static const SolidBrush& Teal();
    COMPAT_NODISCARD static const SolidBrush& Maroon();
    COMPAT_NODISCARD static const SolidBrush& Coral();
    COMPAT_NODISCARD static const SolidBrush& Silver();
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 28 種標準畫刷對照清冊 (Stock Brushes Catalog)

| 靜態方法 | 對應色彩常數 | 十六進位 ARGB | 典型用途與場景 |
| :--- | :--- | :--- | :--- |
| `Brushes::Transparent()` | `Colors::Transparent` | `#00000000` | 透明打底、空填充區域。 |
| `Brushes::Black()` | `Colors::Black` | `#FF000000` | 標準深色文字、黑色幾何填充。 |
| `Brushes::White()` | `Colors::White` | `#FFFFFFFF` | 標準白色背景、白色卡片底色。 |
| `Brushes::Red()` | `Colors::Red` | `#FFFF0000` | 錯誤提示、警示按鈕、紅燈標記。 |
| `Brushes::Green()` | `Colors::Green` | `#FF008000` | 成功提示、確認狀態。 |
| `Brushes::Blue()` | `Colors::Blue` | `#FF0000FF` | 基礎藍色。 |
| `Brushes::Yellow()` | `Colors::Yellow` | `#FFFFFF00` | 警告提示、醒目標記。 |
| `Brushes::Cyan()` | `Colors::Cyan` | `#FF00FFFF` | 青色輔助。 |
| `Brushes::Magenta()` | `Colors::Magenta` | `#FFFF00FF` | 洋紅色標籤。 |
| `Brushes::Gray()` | `Colors::Gray` | `#FF808080` | 次要文字、未啟用 (Disabled) 遮罩。 |
| `Brushes::LightGray()` | `Colors::LightGray` | `#FFD3D3D3` | 邊界底色、分隔線背景。 |
| `Brushes::DarkGray()` | `Colors::DarkGray` | `#FFA9A9A9` | 深色輔助圖示。 |
| `Brushes::Orange()` | `Colors::Orange` | `#FFFFA500` | 橙色警告標籤、活動標題。 |
| `Brushes::Purple()` | `Colors::Purple` | `#FF800080` | 紫色標籤。 |
| `Brushes::Brown()` | `Colors::Brown` | `#FFA52A2A` | 棕色圖元。 |
| `Brushes::SteelBlue()` | `Colors::SteelBlue` | `#FF4682B4` | CPPUI 預設專業風格主色調。 |
| `Brushes::Navy()` | `Colors::Navy` | `#FF000080` | 深海軍藍、深色導覽列。 |
| `Brushes::Gold()` | `Colors::Gold` | `#FFFFD700` | 尊爵標記、評分星星。 |
| `Brushes::Lime()` | `Colors::Lime` | `#FF00FF00` | 亮綠色連線指示燈。 |
| `Brushes::Pink()` | `Colors::Pink` | `#FFFFC0CB` | 柔和粉紅。 |
| `Brushes::DarkRed()` | `Colors::DarkRed` | `#FF8B0000` | 深紅危險指示。 |
| `Brushes::DarkBlue()` | `Colors::DarkBlue` | `#FF00008B` | 深藍背景。 |
| `Brushes::DarkGreen()` | `Colors::DarkGreen` | `#FF006400` | 深綠成功標籤。 |
| `Brushes::Olive()` | `Colors::Olive` | `#FF808000` | 橄欖綠裝飾。 |
| `Brushes::Teal()` | `Colors::Teal` | `#FF008080` | 鴨翅綠資訊圖示。 |
| `Brushes::Maroon()` | `Colors::Maroon` | `#FF800000` | 栗色裝飾。 |
| `Brushes::Coral()` | `Colors::Coral` | `#FFFF7F50` | 珊瑚色標題。 |
| `Brushes::Silver()` | `Colors::Silver` | `#FFC0C0C0` | 銀灰卡片邊框底色。 |

---

## 4. 範例程式碼 (Examples)

### 4.1 快速繪製卡片與文字

```cpp
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Brushes.hpp"
#include "ui/text/Font.hpp"

void PaintDashboard(cppui::graphics::Graphics& g) {
    // 使用靜態標準畫刷填滿卡片 (零多餘物件建構開銷)
    g.FillRoundedRectangle(cppui::graphics::Brushes::White(), core::Rect(20, 20, 300, 180), 8.0f);

    // 繪製彩色頂部飾條
    g.FillRectangle(cppui::graphics::Brushes::SteelBlue(), core::Rect(20, 20, 300, 10));

    // 繪製狀態圖示圓點
    g.FillEllipse(cppui::graphics::Brushes::Lime(), core::Rect(40, 50, 12, 12));

    // 繪製標題與副標文字
    cppui::text::FontDescriptor titleFont("Segoe UI", 14.0f);
    cppui::text::FontDescriptor descFont("Segoe UI", 11.0f);
    g.DrawString("System Operational", titleFont, cppui::graphics::Brushes::Black(), core::Point(60, 48));
    g.DrawString("All services running normally.", descFont, cppui::graphics::Brushes::Gray(), core::Point(40, 80));
}
```

### 4.2 對比：Stock Brushes vs 自訂 SolidBrush

```cpp
// 推薦：標準顏色使用 Brushes，零配置、零鎖競爭
g.FillRectangle(cppui::graphics::Brushes::SteelBlue(), rect);

// 需要自訂非標準顏色或含 Alpha 半透明時才自行建構 SolidBrush
cppui::graphics::SolidBrush customSemiBrush(cppui::core::Color(70, 130, 180, 128));
g.FillRectangle(customSemiBrush, rect);
```

---

## 5. 請參閱 (See Also)

* [`Brush` 畫刷家族詳解](graphics_brush.md)
* [`Colors` 預定義常數調色盤](graphics_colors.md)
* [`Pens` 常用標準畫筆集合](graphics_pens.md)
* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
