/**
 * @file Control.hpp
 * @brief 基礎控制項基底類別，完整對標 Windows Forms / WPF 規格
 *        提供識別、樣式、幾何座標、排版約束 (Anchor/Dock)、字型繼承、焦點與 Z-Order 管理、
 *        佈局生命週期掛起/恢復、滑鼠捕獲、以及 GDI+ 統一繪圖互操作介面。
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/text/Font.hpp"

namespace cppui {

namespace graphics {
class Graphics;
} // namespace graphics

namespace app {
class Form;
} // namespace app

namespace theme {
class Theme;
} // namespace theme

namespace controls {

/// <summary>
/// 控制項邊界錨定樣式 (AnchorStyles，位元遮罩列舉)
/// </summary>
enum class AnchorStyles : uint32_t {
    None   = 0,
    Top    = 1 << 0,
    Bottom = 1 << 1,
    Left   = 1 << 2,
    Right  = 1 << 3
};

COMPAT_CONSTEXPR_14 inline AnchorStyles operator|(AnchorStyles a, AnchorStyles b) noexcept {
    return static_cast<AnchorStyles>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

COMPAT_CONSTEXPR_14 inline AnchorStyles operator&(AnchorStyles a, AnchorStyles b) noexcept {
    return static_cast<AnchorStyles>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

COMPAT_CONSTEXPR_14 inline AnchorStyles operator^(AnchorStyles a, AnchorStyles b) noexcept {
    return static_cast<AnchorStyles>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

COMPAT_CONSTEXPR_14 inline AnchorStyles operator~(AnchorStyles a) noexcept {
    return static_cast<AnchorStyles>(~static_cast<uint32_t>(a));
}

/// <summary>
/// 控制項邊界停靠模式 (DockStyle)
/// </summary>
enum class DockStyle : uint32_t {
    None   = 0,
    Top    = 1,
    Bottom = 2,
    Left   = 3,
    Right  = 4,
    Fill   = 5
};

/// <summary>
/// 滑鼠游標類型樣式 (CursorType)
/// </summary>
enum class CursorType : uint32_t {
    Default  = 0,
    Arrow    = 1,
    IBeam    = 2,
    Wait     = 3,
    Cross    = 4,
    Hand     = 5,
    No       = 6,
    SizeAll  = 7,
    SizeNS   = 8,
    SizeWE   = 9,
    SizeNESW = 10,
    SizeNWSE = 11,
    UpArrow  = 12
};

/// <summary>
/// 基礎控制項類別 (Control)
/// 完整對標 Windows Forms Control 與 WPF Control 之核心規格，
/// 為所有具備視覺外觀、排版幾何、事件互動與繪圖能力之 UI 控制項的基礎基底。
/// </summary>
class Control : public core::UIElement {
public:
    /// <summary>
    /// 建構基礎控制項實例。
    /// </summary>
    Control();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Control() override;

    // --- 1. 識別與自訂資料 (Identity & Tag) ---

    /// <summary>
    /// 取得控制項名稱識別字串視圖。
    /// </summary>
    /// <returns>名稱字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetName() const noexcept { return m_name; }

    /// <summary>
    /// 設定控制項名稱識別字串。
    /// </summary>
    /// <param name="name">名稱字串視圖。</param>
    void SetName(compat::string_view name);

    /// <summary>
    /// 取得附加於控制項的使用者自訂資料指標 (Tag)。
    /// </summary>
    /// <returns>自訂資料 shared_ptr。</returns>
    COMPAT_NODISCARD std::shared_ptr<void> GetTag() const noexcept { return m_tag; }

    /// <summary>
    /// 設定附加於控制項的使用者自訂資料指標。
    /// </summary>
    /// <param name="tag">自訂資料 shared_ptr。</param>
    void SetTag(std::shared_ptr<void> tag) noexcept { m_tag = std::move(tag); }

    /// <summary>
    /// 依型別 T 取得並轉型自訂資料 (Tag)。
    /// </summary>
    /// <typeparam name="T">目標資料型別。</typeparam>
    /// <returns>轉型後之 std::shared_ptr&lt;T&gt;。</returns>
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetTag() const {
        return std::static_pointer_cast<T>(m_tag);
    }

    // --- 2. 文字協定與同步 (Text Protocol) ---

    /// <summary>
    /// 取得控制項顯示之標題或文字內容。
    /// </summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD virtual compat::string_view GetText() const noexcept;

    /// <summary>
    /// 設定控制項顯示之標題或文字內容。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    virtual void SetText(compat::string_view text);

    // --- 3. 可用狀態與可見性 (State & Availability) ---

    /// <summary>
    /// 檢查控制項是否處於可用啟用狀態 (對標 WinForms Enabled)。
    /// </summary>
    /// <returns>若可用傳回 true，停用傳回 false。</returns>
    COMPAT_NODISCARD virtual bool IsEnabled() const noexcept { return m_enabled; }

    /// <summary>
    /// 設定控制項是否可用。
    /// </summary>
    /// <param name="enabled">true 為啟用；false 為禁用。</param>
    virtual void SetEnabled(bool enabled);

    /// <summary>
    /// 檢查控制項是否可見 (包含層級與 Visibility 狀態)。
    /// </summary>
    /// <returns>若可見傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsVisible() const noexcept;

    /// <summary>
    /// 設定控制項是否可見。
    /// </summary>
    /// <param name="visible">true 為可見；false 為隱藏。</param>
    void SetVisible(bool visible);

    /// <summary>
    /// 顯示控制項 (快捷函式)。
    /// </summary>
    void Show() { SetVisible(true); }

    /// <summary>
    /// 隱藏控制項 (快捷函式)。
    /// </summary>
    void Hide() { SetVisible(false); }

    // --- 4. 幾何座標、邊界與客戶區 (Bounds & Client Area) ---

    /// <summary>
    /// 取得控制項相對於父容器之 X 軸座標 (Left)。
    /// </summary>
    /// <returns>X 軸像素值。</returns>
    COMPAT_NODISCARD float GetLeft() const noexcept { return GetLocation().x; }

    /// <summary>
    /// 設定控制項相對於父容器之 X 軸座標。
    /// </summary>
    /// <param name="left">X 軸像素值。</param>
    void SetLeft(float left) { SetLocation(left, GetTop()); }

    /// <summary>
    /// 取得控制項相對於父容器之 Y 軸座標 (Top)。
    /// </summary>
    /// <returns>Y 軸像素值。</returns>
    COMPAT_NODISCARD float GetTop() const noexcept { return GetLocation().y; }

    /// <summary>
    /// 設定控制項相對於父容器之 Y 軸座標。
    /// </summary>
    /// <param name="top">Y 軸像素值。</param>
    void SetTop(float top) { SetLocation(GetLeft(), top); }

    /// <summary>
    /// 取得控制項右側邊界相對於父容器之座標 (Right = Left + Width)。
    /// </summary>
    /// <returns>右邊界像素值。</returns>
    COMPAT_NODISCARD float GetRight() const noexcept;

    /// <summary>
    /// 取得控制項底部邊界相對於父容器之座標 (Bottom = Top + Height)。
    /// </summary>
    /// <returns>下邊界像素值。</returns>
    COMPAT_NODISCARD float GetBottom() const noexcept;

    /// <summary>
    /// 取得控制項左上角相對於父容器之座標點 (Location)。
    /// </summary>
    /// <returns>座標點。</returns>
    COMPAT_NODISCARD core::Point GetLocation() const noexcept;

    /// <summary>
    /// 設定控制項左上角相對於父容器之座標點。
    /// </summary>
    /// <param name="location">座標點。</param>
    void SetLocation(core::Point location);

    /// <summary>
    /// 設定控制項左上角相對於父容器之座標點。
    /// </summary>
    /// <param name="x">X 軸像素座標。</param>
    /// <param name="y">Y 軸像素座標。</param>
    void SetLocation(float x, float y) { SetLocation(core::Point(x, y)); }

    /// <summary>
    /// 取得控制項之當前尺寸 (Size)。
    /// </summary>
    /// <returns>尺寸結構。</returns>
    COMPAT_NODISCARD core::Size GetSize() const noexcept;

    /// <summary>
    /// 設定控制項尺寸。
    /// </summary>
    /// <param name="size">尺寸結構。</param>
    void SetSize(core::Size size);

    /// <summary>
    /// 設定控制項寬度與高度尺寸。
    /// </summary>
    /// <param name="width">寬度像素。</param>
    /// <param name="height">高度像素。</param>
    void SetSize(float width, float height) { SetSize(core::Size(width, height)); }

    /// <summary>
    /// 取得控制項相對於父容器之外圍邊界矩形 (Bounds)。
    /// </summary>
    /// <returns>矩形結構。</returns>
    COMPAT_NODISCARD core::Rect GetBounds() const noexcept;

    /// <summary>
    /// 設定控制項之邊界矩形。
    /// </summary>
    /// <param name="x">X 軸像素。</param>
    /// <param name="y">Y 軸像素。</param>
    /// <param name="width">寬度像素。</param>
    /// <param name="height">高度像素。</param>
    void SetBounds(float x, float y, float width, float height);

    /// <summary>
    /// 設定控制項之邊界矩形。
    /// </summary>
    /// <param name="bounds">邊界矩形引用。</param>
    void SetBounds(const core::Rect& bounds);

    /// <summary>
    /// 取得控制項扣除邊框與內距後之繪圖客戶區矩形 (ClientRectangle)。
    /// </summary>
    /// <returns>以 (0, 0) 為原點之客戶區矩形。</returns>
    COMPAT_NODISCARD core::Rect GetClientRectangle() const noexcept;

    /// <summary>
    /// 取得控制項之客戶區尺寸 (ClientSize)。
    /// </summary>
    /// <returns>客戶區寬度與高度。</returns>
    COMPAT_NODISCARD core::Size GetClientSize() const noexcept;

    // --- 5. 排版約束 (Anchor & Dock) ---

    /// <summary>
    /// 取得邊界錨定樣式 (Anchor)。
    /// </summary>
    /// <returns>AnchorStyles 列舉值。</returns>
    COMPAT_NODISCARD AnchorStyles GetAnchor() const noexcept { return m_anchor; }

    /// <summary>
    /// 設定邊界錨定樣式。
    /// </summary>
    /// <param name="anchor">AnchorStyles 列舉值。</param>
    void SetAnchor(AnchorStyles anchor);

    /// <summary>
    /// 取得邊界停靠模式 (Dock)。
    /// </summary>
    /// <returns>DockStyle 列舉值。</returns>
    COMPAT_NODISCARD DockStyle GetDock() const noexcept { return m_dock; }

    /// <summary>
    /// 設定邊界停靠模式。
    /// </summary>
    /// <param name="dock">DockStyle 列舉值。</param>
    void SetDock(DockStyle dock);

    // --- 6. 字型與環境屬性 (Font & Style Hierarchy) ---

    /// <summary>
    /// 取得字型家族名稱 (若未顯式設定則向上繼承)。
    /// </summary>
    /// <returns>字型家族字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetFontFamily() const noexcept;

    /// <summary>
    /// 設定字型家族名稱。
    /// </summary>
    /// <param name="family">字型家族字串視圖。</param>
    void SetFontFamily(compat::string_view family);

    /// <summary>
    /// 取得字型大小 (若未顯式設定則向上繼承)。
    /// </summary>
    /// <returns>字型大小像素值。</returns>
    COMPAT_NODISCARD float GetFontSize() const noexcept;

    /// <summary>
    /// 設定字型大小。
    /// </summary>
    /// <param name="size">字型大小像素值。</param>
    void SetFontSize(float size);

    /// <summary>
    /// 取得字型粗細 (若未顯式設定則向上繼承)。
    /// </summary>
    /// <returns>FontWeight 列舉值。</returns>
    COMPAT_NODISCARD text::FontWeight GetFontWeight() const noexcept;

    /// <summary>
    /// 設定字型粗細。
    /// </summary>
    /// <param name="weight">FontWeight 列舉值。</param>
    void SetFontWeight(text::FontWeight weight);

    /// <summary>
    /// 取得字型傾斜度 (若未顯式設定則向上繼承)。
    /// </summary>
    /// <returns>FontStyle 列舉值。</returns>
    COMPAT_NODISCARD text::FontStyle GetFontStyle() const noexcept;

    /// <summary>
    /// 設定字型傾斜度。
    /// </summary>
    /// <param name="style">FontStyle 列舉值。</param>
    void SetFontStyle(text::FontStyle style);

    /// <summary>
    /// 取得完整之有效字型描述項 (包含繼承解析)。
    /// </summary>
    /// <returns>FontDescriptor 物件。</returns>
    COMPAT_NODISCARD text::FontDescriptor GetFont() const;

    /// <summary>
    /// 設定完整字型描述項。
    /// </summary>
    /// <param name="font">FontDescriptor 物件。</param>
    void SetFont(const text::FontDescriptor& font);

    /// <summary>
    /// 檢查本控制項是否具備顯式自訂之字型設定。
    /// </summary>
    /// <returns>若有顯式設定傳回 true。</returns>
    COMPAT_NODISCARD bool HasExplicitFont() const noexcept { return m_hasExplicitFont; }

    // --- 7. 滑鼠游標 (Cursor) ---

    /// <summary>
    /// 取得滑鼠懸停於此控制項時之游標樣式。
    /// </summary>
    /// <returns>CursorType 列舉值。</returns>
    COMPAT_NODISCARD CursorType GetCursor() const noexcept { return m_cursor; }

    /// <summary>
    /// 設定滑鼠懸停於此控制項時之游標樣式。
    /// </summary>
    /// <param name="cursor">CursorType 列舉值。</param>
    void SetCursor(CursorType cursor) noexcept { m_cursor = cursor; }

    // --- 8. 外觀色彩與邊框 (Appearance & Padding) ---

    /// <summary>取得內部留白厚度 (Padding)。</summary>
    /// <returns>Thickness 物件。</returns>
    COMPAT_NODISCARD core::Thickness GetPadding() const noexcept { return m_padding; }

    /// <summary>設定內部留白厚度。</summary>
    /// <param name="padding">四方向內距厚度。</param>
    void SetPadding(const core::Thickness& padding);

    /// <summary>設定四邊等寬的內部留白厚度。</summary>
    /// <param name="uniform">均勻內距像素值。</param>
    void SetPadding(float uniform) { SetPadding(core::Thickness(uniform)); }

    /// <summary>設定水平與垂直對稱的內部留白厚度。</summary>
    /// <param name="h">水平內距像素值。</param>
    /// <param name="v">垂直內距像素值。</param>
    void SetPadding(float h, float v) { SetPadding(core::Thickness(h, v, h, v)); }

    /// <summary>取得背景填色色彩。</summary>
    /// <returns>背景色彩。</returns>
    COMPAT_NODISCARD core::Color GetBackground() const noexcept { return m_background; }

    /// <summary>設定背景填色色彩。</summary>
    /// <param name="bg">背景色彩。</param>
    void SetBackground(core::Color bg);

    /// <summary>取得前景筆刷色彩。</summary>
    /// <returns>前景色彩。</returns>
    COMPAT_NODISCARD core::Color GetForeground() const noexcept { return m_foreground; }

    /// <summary>設定前景筆刷色彩。</summary>
    /// <param name="fg">前景色彩。</param>
    void SetForeground(core::Color fg);

    /// <summary>取得邊框筆刷色彩。</summary>
    /// <returns>邊框筆刷色彩。</returns>
    COMPAT_NODISCARD core::Color GetBorderBrush() const noexcept { return m_borderBrush; }

    /// <summary>設定邊框筆刷色彩。</summary>
    /// <param name="brush">邊框筆刷色彩。</param>
    void SetBorderBrush(core::Color brush);

    /// <summary>取得邊框厚度。</summary>
    /// <returns>Thickness 物件。</returns>
    COMPAT_NODISCARD core::Thickness GetBorderThickness() const noexcept { return m_borderThickness; }

    /// <summary>設定邊框厚度。</summary>
    /// <param name="thickness">邊框四邊厚度。</param>
    void SetBorderThickness(const core::Thickness& thickness);

    /// <summary>設定四邊等寬之邊框厚度。</summary>
    /// <param name="uniform">均勻邊框寬度。</param>
    void SetBorderThickness(float uniform) { SetBorderThickness(core::Thickness(uniform)); }

    /// <summary>取得邊框圓角半徑。</summary>
    /// <returns>圓角半徑像素值。</returns>
    COMPAT_NODISCARD float GetCornerRadius() const noexcept { return m_cornerRadius; }

    /// <summary>設定邊框圓角半徑。</summary>
    /// <param name="radius">圓角半徑像素值。</param>
    void SetCornerRadius(float radius);

    /// <summary>WinForms 色彩別名：取得背景色 (BackColor)。</summary>
    /// <returns>背景色。</returns>
    COMPAT_NODISCARD core::Color GetBackColor() const noexcept { return GetBackground(); }

    /// <summary>WinForms 色彩別名：設定背景色 (BackColor)。</summary>
    /// <param name="bg">背景色。</param>
    void SetBackColor(core::Color bg) { SetBackground(bg); }

    /// <summary>WinForms 色彩別名：取得前景色 (ForeColor)。</summary>
    /// <returns>前景色。</returns>
    COMPAT_NODISCARD core::Color GetForeColor() const noexcept { return GetForeground(); }

    /// <summary>WinForms 色彩別名：設定前景色 (ForeColor)。</summary>
    /// <param name="fg">前景色。</param>
    void SetForeColor(core::Color fg) { SetForeground(fg); }

    /// <summary>
    /// 套用主題規格代幣至控制項 (若屬性未被顯式覆寫則更新)。
    /// </summary>
    /// <param name="theme">欲套用之 Theme 常數引用。</param>
    virtual void ApplyTheme(const theme::Theme& theme);

    /// <summary>檢查控制項是否被使用者顯式自訂背景色。</summary>
    /// <returns>若有顯式設定傳回 true。</returns>
    COMPAT_NODISCARD bool HasExplicitBackground() const noexcept { return m_hasExplicitBackground; }

    /// <summary>檢查控制項是否被使用者顯式自訂前景文字色。</summary>
    /// <returns>若有顯式設定傳回 true。</returns>
    COMPAT_NODISCARD bool HasExplicitForeground() const noexcept { return m_hasExplicitForeground; }

    /// <summary>檢查控制項是否被使用者顯式自訂邊框色。</summary>
    /// <returns>若有顯式設定傳回 true。</returns>
    COMPAT_NODISCARD bool HasExplicitBorder() const noexcept { return m_hasExplicitBorder; }

    // --- 9. 視覺樹與階層導航 (Hierarchy Navigation) ---

    /// <summary>
    /// 取得視覺樹之父控制項 (若父節點為 Control)。
    /// </summary>
    /// <returns>父 Control 指標，若無則傳回 nullptr。</returns>
    COMPAT_NODISCARD Control* GetParent() const noexcept;

    /// <summary>
    /// 沿視覺樹向上搜尋承載本控制項之最頂層視窗表單 (Form)。
    /// </summary>
    /// <returns>找到之 Form 指標，若未掛載於 Form 則傳回 nullptr。</returns>
    COMPAT_NODISCARD app::Form* FindForm() const noexcept;

    /// <summary>
    /// 沿視覺樹向上搜尋最頂層之根控制項。
    /// </summary>
    /// <returns>頂層根 Control 指標。</returns>
    COMPAT_NODISCARD Control* GetTopLevelControl() const noexcept;

    /// <summary>
    /// 檢查指定之控制項是否為本控制項之直接或間接子節點。
    /// </summary>
    /// <param name="control">欲檢查之控制項指標。</param>
    /// <returns>若為子元件傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool Contains(const Control* control) const noexcept;

    /// <summary>
    /// 取得本控制項之所有直接子控制項清單。
    /// </summary>
    /// <returns>子控制項指標陣列。</returns>
    COMPAT_NODISCARD std::vector<Control*> GetChildControls() const;

    // --- 10. Z-Order 圖層堆疊管理 (Z-Order Management) ---

    /// <summary>
    /// 將本控制項移至父容器子節點的最上層 (最前景/繪製順序末端)。
    /// </summary>
    void BringToFront();

    /// <summary>
    /// 將本控制項移至父容器子節點的最底層 (最背景/繪製順序開頭)。
    /// </summary>
    void SendToBack();

    // --- 11. 焦點與互動選擇 (Focus & Selection) ---

    /// <summary>
    /// 使本控制項獲得輸入焦點 (對標 WinForms Focus)。
    /// </summary>
    /// <returns>若成功獲得焦點傳回 true，否則傳回 false。</returns>
    bool Focus();

    /// <summary>
    /// 選取控制項並使其獲焦 (對標 WinForms Select)。
    /// </summary>
    void Select() { Focus(); }

    /// <summary>
    /// 檢查本控制項或其任何子節點目前是否正擁有鍵盤焦點。
    /// </summary>
    /// <returns>若自身或子節點獲焦傳回 true。</returns>
    COMPAT_NODISCARD bool ContainsFocus() const noexcept;

    // --- 12. 滑鼠指標捕獲 (Mouse Capture) ---

    /// <summary>
    /// 要求捕獲全域滑鼠指標訊息。
    /// </summary>
    /// <returns>若捕獲成功傳回 true。</returns>
    bool CaptureMouse();

    /// <summary>
    /// 釋放滑鼠指標捕獲。
    /// </summary>
    void ReleaseMouseCapture();

    /// <summary>
    /// 檢查本控制項當前是否正捕獲滑鼠指標。
    /// </summary>
    /// <returns>若擁有捕獲傳回 true。</returns>
    COMPAT_NODISCARD bool HasMouseCapture() const noexcept;

    // --- 13. 佈局生命週期管理 (Layout Lifecycle) ---

    /// <summary>
    /// 暫停控制項的排版計算 (對標 WinForms SuspendLayout)。
    /// 適用於批量新增/配置子控制項時避免重複觸發度量與排列。
    /// </summary>
    void SuspendLayout() noexcept;

    /// <summary>
    /// 恢復控制項排版計算並選擇性立即執行排版 (對標 WinForms ResumeLayout)。
    /// </summary>
    /// <param name="performLayout">若為 true 且掛起計數歸零，立即觸發重新排版。</param>
    void ResumeLayout(bool performLayout = true);

    /// <summary>
    /// 強制立即執行排版佈局計算 (對標 WinForms PerformLayout)。
    /// </summary>
    void PerformLayout();

    /// <summary>
    /// 檢查控制項目前是否處於佈局掛起狀態。
    /// </summary>
    /// <returns>若處於掛起狀態傳回 true。</returns>
    COMPAT_NODISCARD bool IsLayoutSuspended() const noexcept { return m_layoutSuspendCount > 0; }

    // --- 14. 重繪與即時更新 (Repaint & Refresh) ---

    /// <summary>
    /// 標記整片控制項區域為髒區並請求重繪 (對標 WinForms Invalidate)。
    /// </summary>
    void Invalidate();

    /// <summary>
    /// 標記指定之局部矩形區域為髒區並請求重繪。
    /// </summary>
    /// <param name="rect">局部髒區矩形。</param>
    void Invalidate(const core::Rect& rect);

    /// <summary>
    /// 即時更新控制項渲染管線 (對標 WinForms Update)。
    /// </summary>
    void Update();

    /// <summary>
    /// 強制控制項立即失效並重繪 (對標 WinForms Refresh)。
    /// </summary>
    void Refresh();

    // --- 15. 圖形互操作與自訂繪製 (Graphics Interop) ---

    /// <summary>
    /// 為本控制項建立獨立的 GDI+ 風格 Graphics 繪圖排程實例。
    /// </summary>
    /// <returns>Graphics 獨占指標。</returns>
    COMPAT_NODISCARD std::unique_ptr<graphics::Graphics> CreateGraphics();

    /// <summary>
    /// GDI+ 風格自訂繪圖虛擬進入點，衍生控制項可直接覆寫此函式進行繪製。
    /// </summary>
    /// <param name="g">Graphics 繪圖排程器引用。</param>
    virtual void OnPaint(graphics::Graphics& g);

    /// <summary>
    /// UI 渲染實作函式，負責繪製背景邊框並將繪圖上下文轉發至 OnPaint。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    // --- 16. 座標與矩形轉換 (Coordinate Conversion) ---

    /// <summary>
    /// 將本控制項客戶區矩形轉換為螢幕/全域座標矩形。
    /// </summary>
    /// <param name="clientRect">客戶區局部矩形。</param>
    /// <returns>全域座標矩形。</returns>
    COMPAT_NODISCARD core::Rect RectangleToScreen(const core::Rect& clientRect) const;

    /// <summary>
    /// 將螢幕/全域座標矩形轉換為本控制項之客戶區局部矩形。
    /// </summary>
    /// <param name="screenRect">全域座標矩形。</param>
    /// <returns>客戶區局部矩形。</returns>
    COMPAT_NODISCARD core::Rect RectangleToClient(const core::Rect& screenRect) const;

    // --- 17. 常用事件監聽器掛載 (Event Listeners) ---

    /// <summary>掛載按鈕/控制項點擊 (Click) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnClick(std::function<void()> handler);

    /// <summary>掛載雙擊 (DoubleClick) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnDoubleClick(std::function<void()> handler);

    /// <summary>掛載文字內容變更 (TextChanged) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnTextChanged(std::function<void(compat::string_view)> handler);

    /// <summary>掛載可用性變更 (EnabledChanged) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnEnabledChanged(std::function<void(bool)> handler);

    /// <summary>掛載可見性變更 (VisibleChanged) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnVisibleChanged(std::function<void(bool)> handler);

    /// <summary>掛載尺寸大小變更 (SizeChanged / Resize) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnSizeChanged(std::function<void(core::Size)> handler);

    /// <summary>掛載位置座標變更 (LocationChanged / Move) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnLocationChanged(std::function<void(core::Point)> handler);

    /// <summary>掛載滑鼠進入 (MouseEnter) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnMouseEnter(std::function<void()> handler);

    /// <summary>掛載滑鼠離開 (MouseLeave) 事件處理常式。</summary>
    /// <param name="handler">事件回呼函式。</param>
    void OnMouseLeave(std::function<void()> handler);

    /// <summary>掛載自訂 Paint 繪圖回呼函式。</summary>
    /// <param name="handler">繪圖回呼函式。</param>
    void OnPaintHandler(std::function<void(graphics::Graphics&)> handler);

protected:
    // --- 衍生類別虛擬回呼點 (Protected Virtual Hooks) ---

    /// <summary>點擊事件內部虛擬分發點。</summary>
    virtual void OnClickCore();

    /// <summary>雙擊事件內部虛擬分發點。</summary>
    virtual void OnDoubleClickCore();

    /// <summary>文字變更內部虛擬分發點。</summary>
    /// <param name="newText">新文字內容視圖。</param>
    virtual void OnTextChangedCore(compat::string_view newText);

    /// <summary>可用性變更內部虛擬分發點。</summary>
    /// <param name="enabled">新可用狀態。</param>
    virtual void OnEnabledChangedCore(bool enabled);

    /// <summary>可見性變更內部虛擬分發點。</summary>
    /// <param name="visible">新可見狀態。</param>
    virtual void OnVisibleChangedCore(bool visible);

    /// <summary>尺寸變更內部虛擬分發點。</summary>
    /// <param name="newSize">新尺寸。</param>
    virtual void OnResizeCore(core::Size newSize);

    /// <summary>位置變更內部虛擬分發點。</summary>
    /// <param name="newLocation">新座標點。</param>
    virtual void OnMoveCore(core::Point newLocation);

    /// <summary>繪製背景與外圍邊框之標準輔助函式。</summary>
    /// <param name="dc">繪圖上下文。</param>
    /// <param name="bounds">繪製矩形邊界。</param>
    void RenderBackgroundAndBorder(rendering::DrawingContext& dc, const core::Rect& bounds);

    /// <summary>計算扣除 Padding 與 BorderThickness 後可供子元件佈局的尺寸。</summary>
    /// <param name="size">外圍原始尺寸。</param>
    /// <returns>扣除邊框與內距後之內容尺寸。</returns>
    COMPAT_NODISCARD core::Size DeflateSize(core::Size size) const noexcept;

    /// <summary>計算加上 Padding 與 BorderThickness 後的外圍尺寸。</summary>
    /// <param name="size">內部內容尺寸。</param>
    /// <returns>加上邊框與內距後之外圍尺寸。</returns>
    COMPAT_NODISCARD core::Size InflateSize(core::Size size) const noexcept;

    // --- 內部成員欄位 ---
    std::string             m_name;
    std::shared_ptr<void>   m_tag;
    std::string             m_text;
    bool                    m_enabled = true;

    AnchorStyles            m_anchor = AnchorStyles::Top | AnchorStyles::Left;
    DockStyle               m_dock = DockStyle::None;
    CursorType              m_cursor = CursorType::Default;

    // 字型設定 (空字串 / 負數大小表示繼承自父層)
    std::string             m_fontFamily;
    float                   m_fontSize = -1.0f;
    text::FontWeight        m_fontWeight = text::FontWeight::Normal;
    text::FontStyle         m_fontStyle = text::FontStyle::Normal;
    bool                    m_hasExplicitFont = false;

    // 樣式與外觀
    core::Thickness         m_padding{ 0.0f };
    core::Color             m_background{ 0, 0, 0, 0 };
    core::Color             m_foreground{ 255, 255, 255, 255 };
    core::Color             m_borderBrush{ 0, 0, 0, 0 };
    core::Thickness         m_borderThickness{ 0.0f };
    float                   m_cornerRadius = 0.0f;
    core::Point             m_location{ 0.0f, 0.0f };
    bool                    m_hasExplicitBackground = false;
    bool                    m_hasExplicitForeground = false;
    bool                    m_hasExplicitBorder = false;

    // 佈局掛起計數
    uint32_t                m_layoutSuspendCount = 0;
    bool                    m_isLayoutPending = false;

    // 常用事件函式物件
    std::function<void()>                         m_clickHandler;
    std::function<void()>                         m_doubleClickHandler;
    std::function<void(compat::string_view)>      m_textChangedHandler;
    std::function<void(bool)>                     m_enabledChangedHandler;
    std::function<void(bool)>                     m_visibleChangedHandler;
    std::function<void(core::Size)>               m_sizeChangedHandler;
    std::function<void(core::Point)>              m_locationChangedHandler;
    std::function<void()>                         m_mouseEnterHandler;
    std::function<void()>                         m_mouseLeaveHandler;
    std::function<void(graphics::Graphics&)>      m_paintHandler;
};

} // namespace controls
} // namespace cppui
