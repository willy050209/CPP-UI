/**
 * @file Fluent.hpp
 * @brief 現代 C++ 宣告式 Fluent RAD DSL 建構器與工廠輔助函式
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <initializer_list>
#include <functional>
#include <type_traits>

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Canvas.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/mvvm/ICommand.hpp"

namespace cppui {
namespace dsl {

/// <summary>
/// 宣告式 Fluent Builder 包裝樣板類別 (UIBuilder)。
/// 內部持有 std::shared_ptr&lt;T&gt;，提供型別安全、零成本的現代 C++ 鏈式屬性設定與階層樹建構能力。
/// </summary>
/// <typeparam name="T">所包裝建構之 UI 元件型別。</typeparam>
template <typename T>
class UIBuilder {
public:
    /// <summary>
    /// 預設建構子，自動使用 std::make_shared 建立新元件實例。
    /// </summary>
    UIBuilder() : m_element(std::make_shared<T>()) {}

    /// <summary>
    /// 包裝既有元件實例指標之建構子。
    /// </summary>
    /// <param name="elem">欲包裝之元件 shared_ptr。</param>
    explicit UIBuilder(std::shared_ptr<T> elem) : m_element(std::move(elem)) {}

    /// <summary>
    /// 隱式轉換為 std::shared_ptr&lt;T&gt;。
    /// </summary>
    /// <returns>所持有的元件實例指標。</returns>
    operator std::shared_ptr<T>() const noexcept { return m_element; }

    /// <summary>
    /// 隱式轉換為基底 UIElement 指標。
    /// </summary>
    /// <returns>向上轉型為 UIElement 之實例指標。</returns>
    operator std::shared_ptr<core::UIElement>() const noexcept { return m_element; }

    /// <summary>
    /// 隱式轉換為基底 Visual 指標。
    /// </summary>
    /// <returns>向上轉型為 Visual 之實例指標。</returns>
    operator std::shared_ptr<core::Visual>() const noexcept { return m_element; }

    /// <summary>
    /// 明確取得所持有之元件智慧指標。
    /// </summary>
    /// <returns>內部持有的 std::shared_ptr&lt;T&gt;。</returns>
    COMPAT_NODISCARD std::shared_ptr<T> Get() const noexcept { return m_element; }

    /// <summary>
    /// 箭頭存取運算子，直接存取元件成員。
    /// </summary>
    /// <returns>底層原始指標。</returns>
    COMPAT_NODISCARD T* operator->() const noexcept { return m_element.get(); }

    /// <summary>
    /// 解引用運算子。
    /// </summary>
    /// <returns>底層元件引用。</returns>
    COMPAT_NODISCARD T& operator*() const noexcept { return *m_element; }

    // --- UIElement 基礎幾何與排版屬性 ---

    /// <summary>
    /// 設定元件寬度。
    /// </summary>
    /// <param name="w">指定寬度數值 (像素)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Width(float w) { m_element->SetWidth(w); return *this; }

    /// <summary>
    /// 設定元件高度。
    /// </summary>
    /// <param name="h">指定高度數值 (像素)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Height(float h) { m_element->SetHeight(h); return *this; }

    /// <summary>
    /// 設定元件最小寬度。
    /// </summary>
    /// <param name="mw">最小寬度限制。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& MinWidth(float mw) { m_element->SetMinWidth(mw); return *this; }

    /// <summary>
    /// 設定元件最大寬度。
    /// </summary>
    /// <param name="mw">最大寬度限制。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& MaxWidth(float mw) { m_element->SetMaxWidth(mw); return *this; }

    /// <summary>
    /// 設定元件最小高度。
    /// </summary>
    /// <param name="mh">最小高度限制。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& MinHeight(float mh) { m_element->SetMinHeight(mh); return *this; }

    /// <summary>
    /// 設定元件最大高度。
    /// </summary>
    /// <param name="mh">最大高度限制。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& MaxHeight(float mh) { m_element->SetMaxHeight(mh); return *this; }

    /// <summary>
    /// 設定外距 (Margin)。
    /// </summary>
    /// <param name="m">Thickness 厚度結構。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Margin(const core::Thickness& m) { m_element->SetMargin(m); return *this; }

    /// <summary>
    /// 設定均勻外距 (Margin)。
    /// </summary>
    /// <param name="uniform">四邊相同之外距。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Margin(float uniform) { m_element->SetMargin(core::Thickness(uniform)); return *this; }

    /// <summary>
    /// 設定水平與垂直外距 (Margin)。
    /// </summary>
    /// <param name="h">水平方向 (左右) 外距。</param>
    /// <param name="v">垂直方向 (上下) 外距。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Margin(float h, float v) { m_element->SetMargin(core::Thickness(h, v, h, v)); return *this; }

    /// <summary>
    /// 設定四邊獨立外距 (Margin)。
    /// </summary>
    /// <param name="l">左側外距。</param>
    /// <param name="t">上方外距。</param>
    /// <param name="r">右側外距。</param>
    /// <param name="b">下方外距。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Margin(float l, float t, float r, float b) { m_element->SetMargin(core::Thickness(l, t, r, b)); return *this; }

    /// <summary>
    /// 設定水平對齊方式。
    /// </summary>
    /// <param name="ha">HorizontalAlignment 列舉值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& HorizontalAlignment(layout::HorizontalAlignment ha) { m_element->SetHorizontalAlignment(ha); return *this; }

    /// <summary>
    /// 設定垂直對齊方式。
    /// </summary>
    /// <param name="va">VerticalAlignment 列舉值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& VerticalAlignment(layout::VerticalAlignment va) { m_element->SetVerticalAlignment(va); return *this; }

    /// <summary>
    /// 設定元件可見性。
    /// </summary>
    /// <param name="v">Visibility 列舉值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Visibility(layout::Visibility v) { m_element->SetVisibility(v); return *this; }

    /// <summary>
    /// 設定布林值可見狀態 (Visible 或 Collapsed)。
    /// </summary>
    /// <param name="v">true 表示 Visible，false 表示 Collapsed。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Visible(bool v) {
        m_element->SetVisibility(v ? layout::Visibility::Visible : layout::Visibility::Collapsed);
        return *this;
    }

    /// <summary>
    /// 設定不透明度 (Opacity)。
    /// </summary>
    /// <param name="o">不透明度數值 (0.0f 至 1.0f)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Opacity(float o) { m_element->SetOpacity(o); return *this; }

    /// <summary>
    /// 設定元件是否可取得焦點。
    /// </summary>
    /// <param name="f">true 表示可獲得焦點。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Focusable(bool f) { m_element->SetFocusable(f); return *this; }

    /// <summary>
    /// 設定 Tab 鍵巡訪順序索引。
    /// </summary>
    /// <param name="idx">Tab 索引值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& TabIndex(int32_t idx) { m_element->SetTabIndex(idx); return *this; }

    /// <summary>
    /// 設定是否納入 Tab 鍵巡訪停靠點。
    /// </summary>
    /// <param name="ts">true 表示停靠。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& TabStop(bool ts) { m_element->SetTabStop(ts); return *this; }

    // --- Grid 附加屬性捷徑 ---

    /// <summary>
    /// 設定在 Grid 排版中的列索引 (Row)。
    /// </summary>
    /// <param name="r">從 0 開始之列索引。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Row(size_t r) { layout::Grid::SetRow(*m_element, r); return *this; }

    /// <summary>
    /// 設定在 Grid 排版中的欄索引 (Column)。
    /// </summary>
    /// <param name="c">從 0 開始之欄索引。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Column(size_t c) { layout::Grid::SetColumn(*m_element, c); return *this; }

    /// <summary>
    /// 設定在 Grid 排版中跨越的列數 (RowSpan)。
    /// </summary>
    /// <param name="rs">跨越之列數。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& RowSpan(size_t rs) { layout::Grid::SetRowSpan(*m_element, rs); return *this; }

    /// <summary>
    /// 設定在 Grid 排版中跨越的欄數 (ColumnSpan)。
    /// </summary>
    /// <param name="cs">跨越之欄數。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& ColumnSpan(size_t cs) { layout::Grid::SetColumnSpan(*m_element, cs); return *this; }

    // --- Canvas 附加屬性捷徑 ---

    /// <summary>
    /// 設定在 Canvas 容器中相對於左緣之像素距離。
    /// </summary>
    /// <param name="l">左側像素座標。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Left(float l) { layout::Canvas::SetLeft(*m_element, l); return *this; }

    /// <summary>
    /// 設定在 Canvas 容器中相對於上緣之像素距離。
    /// </summary>
    /// <param name="t">上方像素座標。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Top(float t) { layout::Canvas::SetTop(*m_element, t); return *this; }

    // --- 控制項外觀樣式屬性 (動態相容 Control, Border, TextBlock) ---

    /// <summary>
    /// 設定背景顏色 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="bg">背景色彩 Color 結構。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Background(core::Color bg) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetBackground(bg);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetBackground(bg);
        }
        return *this;
    }

    /// <summary>
    /// 設定前景文字/畫筆色彩 (相容 Control 與 TextBlock)。
    /// </summary>
    /// <param name="fg">前景色彩 Color 結構。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Foreground(core::Color fg) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetForeground(fg);
        } else if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetForeground(fg);
        }
        return *this;
    }

    /// <summary>
    /// 設定邊框顏色 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="brush">邊框筆刷色彩。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& BorderBrush(core::Color brush) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetBorderBrush(brush);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetBorderBrush(brush);
        }
        return *this;
    }

    /// <summary>
    /// 設定邊框粗細結構 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="th">Thickness 邊框粗細。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& BorderThickness(const core::Thickness& th) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetBorderThickness(th);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetBorderThickness(th);
        }
        return *this;
    }

    /// <summary>
    /// 設定均勻邊框粗細 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="uniform">四邊統一之邊框粗細像素。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& BorderThickness(float uniform) { return BorderThickness(core::Thickness(uniform)); }

    /// <summary>
    /// 設定水平與垂直邊框粗細 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="h">水平粗細 (左右)。</param>
    /// <param name="v">垂直粗細 (上下)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& BorderThickness(float h, float v) { return BorderThickness(core::Thickness(h, v)); }

    /// <summary>
    /// 設定四邊獨立邊框粗細 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="l">左邊框粗細。</param>
    /// <param name="t">上邊框粗細。</param>
    /// <param name="r">右邊框粗細。</param>
    /// <param name="b">下邊框粗細。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& BorderThickness(float l, float t, float r, float b) { return BorderThickness(core::Thickness(l, t, r, b)); }

    /// <summary>
    /// 設定圓角半徑 (相容 Control 與 Border)。
    /// </summary>
    /// <param name="r">圓角半徑像素。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& CornerRadius(float r) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetCornerRadius(r);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetCornerRadius(r);
        }
        return *this;
    }

    /// <summary>
    /// 設定內距 (Padding)。
    /// </summary>
    /// <param name="p">Thickness 內距結構。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Padding(const core::Thickness& p) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetPadding(p);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetPadding(p);
        }
        return *this;
    }

    /// <summary>
    /// 設定均勻內距 (Padding)。
    /// </summary>
    /// <param name="uniform">四邊統一之內距像素。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Padding(float uniform) { return Padding(core::Thickness(uniform)); }

    /// <summary>
    /// 設定水平與垂直內距 (Padding)。
    /// </summary>
    /// <param name="h">水平內距 (左右)。</param>
    /// <param name="v">垂直內距 (上下)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Padding(float h, float v) { return Padding(core::Thickness(h, v)); }

    /// <summary>
    /// 設定四邊獨立內距 (Padding)。
    /// </summary>
    /// <param name="l">左側內距。</param>
    /// <param name="t">上方內距。</param>
    /// <param name="r">右側內距。</param>
    /// <param name="b">下方內距。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Padding(float l, float t, float r, float b) { return Padding(core::Thickness(l, t, r, b)); }

    // --- 文字排版樣式屬性 ---

    /// <summary>
    /// 設定字型大小 (相容 TextBlock 與 TextBox)。
    /// </summary>
    /// <param name="size">字級大小 (pt/px)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& FontSize(float size) {
        if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetFontSize(size);
        } else if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetFontSize(size);
        }
        return *this;
    }

    /// <summary>
    /// 設定字型家族名稱 (相容 TextBlock 與 TextBox)。
    /// </summary>
    /// <param name="family">字型家族字串 (如 "Segoe UI")。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& FontFamily(compat::string_view family) {
        if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetFontFamily(family);
        } else if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetFontFamily(family);
        }
        return *this;
    }

    /// <summary>
    /// 設定字型字重 (相容 TextBlock)。
    /// </summary>
    /// <param name="weight">FontWeight 列舉值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& FontWeight(text::FontWeight weight) {
        if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetFontWeight(weight);
        }
        return *this;
    }

    // --- StackPanel 專屬屬性 ---

    /// <summary>
    /// 設定 StackPanel 子項目間距。
    /// </summary>
    /// <param name="s">間距像素值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Spacing(float s) {
        if (auto* sp = dynamic_cast<layout::StackPanel*>(m_element.get())) {
            sp->SetSpacing(s);
        }
        return *this;
    }

    /// <summary>
    /// 設定 StackPanel 排版方向。
    /// </summary>
    /// <param name="o">Orientation 列舉值 (Horizontal 或 Vertical)。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Orientation(layout::Orientation o) {
        if (auto* sp = dynamic_cast<layout::StackPanel*>(m_element.get())) {
            sp->SetOrientation(o);
        }
        return *this;
    }

    // --- Button 專屬事件掛載 ---

    /// <summary>
    /// 掛載按鈕點擊事件回呼 (相容 Button)。
    /// </summary>
    /// <typeparam name="Handler">回呼函數型別。</typeparam>
    /// <param name="handler">點擊事件處理常式。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    template <typename Handler>
    UIBuilder& OnClick(Handler&& handler) {
        if (auto* btn = dynamic_cast<controls::Button*>(m_element.get())) {
            btn->OnClick(std::forward<Handler>(handler));
        }
        return *this;
    }

    // --- CheckBox 專屬屬性與事件 ---

    /// <summary>
    /// 設定核取方塊勾選狀態 (相容 CheckBox)。
    /// </summary>
    /// <param name="isChecked">true 表示已勾選。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Checked(bool isChecked) {
        if (auto* cb = dynamic_cast<controls::CheckBox*>(m_element.get())) {
            cb->SetChecked(isChecked);
        }
        return *this;
    }

    /// <summary>
    /// 掛載核取狀態變更事件回呼 (相容 CheckBox)。
    /// </summary>
    /// <typeparam name="Handler">回呼函數型別。</typeparam>
    /// <param name="handler">狀態變更事件處理常式。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    template <typename Handler>
    UIBuilder& OnCheckedChanged(Handler&& handler) {
        if (auto* cb = dynamic_cast<controls::CheckBox*>(m_element.get())) {
            cb->OnCheckedChanged(std::forward<Handler>(handler));
        }
        return *this;
    }

    // --- TextBox 專屬屬性與事件 ---

    /// <summary>
    /// 設定文字內容 (相容 TextBox 與 TextBlock)。
    /// </summary>
    /// <param name="text">欲設定之字串視圖。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Text(compat::string_view text) {
        if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetText(text);
        } else if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetText(text);
        }
        return *this;
    }

    /// <summary>
    /// 設定輸入提示文字 (相容 TextBox)。
    /// </summary>
    /// <param name="placeholder">佔位提示字串視圖。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Placeholder(compat::string_view placeholder) {
        if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetPlaceholder(placeholder);
        }
        return *this;
    }

    /// <summary>
    /// 掛載文字變更事件回呼 (相容 TextBox)。
    /// </summary>
    /// <typeparam name="Handler">回呼函數型別。</typeparam>
    /// <param name="handler">文字變更事件處理常式。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    template <typename Handler>
    UIBuilder& OnTextChanged(Handler&& handler) {
        if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->OnTextChanged(std::forward<Handler>(handler));
        }
        return *this;
    }

    // --- Signal 響應式文字綁定 ---

    /// <summary>
    /// 將 TextBlock 文字繫結至響應式 Signal，當數值異動時透過 formatter 自動格式化並更新 UI。
    /// </summary>
    /// <typeparam name="ValType">Signal 所持有的數值型別。</typeparam>
    /// <typeparam name="Formatter">接受 ValType 並回傳 std::string 之格式化器型別。</typeparam>
    /// <param name="sig">欲訂閱之 Signal 引用。</param>
    /// <param name="formatter">格式化轉換函式。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    template <typename ValType, typename Formatter>
    UIBuilder& BindText(reactive::Signal<ValType>& sig, Formatter formatter) {
        if (dynamic_cast<controls::TextBlock*>(m_element.get())) {
            auto sharedTb = std::dynamic_pointer_cast<controls::TextBlock>(m_element);
            sig.Bind(sharedTb, [formatter](controls::TextBlock& t, const ValType& val) {
                t.SetText(formatter(val));
            });
        }
        return *this;
    }

    // --- MVVM 命令與 DataContext 綁定 ---

    /// <summary>
    /// 綁定按鈕執行命令 (相容 Button)。
    /// </summary>
    /// <param name="cmd">ICommand 命令實例指標。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Command(std::shared_ptr<mvvm::ICommand> cmd) {
        if (auto* btn = dynamic_cast<controls::Button*>(m_element.get())) {
            btn->SetCommand(cmd);
        }
        return *this;
    }

    /// <summary>
    /// 設定元件之資料上下文 (DataContext)。
    /// </summary>
    /// <param name="dc">ViewModel 或模型實例指標。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& DataContext(std::shared_ptr<void> dc) {
        if (m_element) {
            m_element->SetDataContext(dc);
        }
        return *this;
    }

    // --- PictureBox 專屬屬性與影像綁定 ---

    /// <summary>
    /// 設定 PictureBox 圖片。
    /// </summary>
    /// <param name="bmp">Bitmap 共享指標。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& Image(std::shared_ptr<graphics::Bitmap> bmp) {
        if (auto* pic = dynamic_cast<controls::PictureBox*>(m_element.get())) {
            pic->SetImage(std::move(bmp));
        }
        return *this;
    }

    /// <summary>
    /// 設定 PictureBox 尺寸呈現模式。
    /// </summary>
    /// <param name="mode">PictureBoxSizeMode 列舉值。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& SizeMode(graphics::PictureBoxSizeMode mode) {
        if (auto* pic = dynamic_cast<controls::PictureBox*>(m_element.get())) {
            pic->SetSizeMode(mode);
        }
        return *this;
    }

    /// <summary>
    /// 綁定 Signal 反應式圖片來源。
    /// </summary>
    /// <param name="sig">Bitmap 共享指標信號引用。</param>
    /// <returns>UIBuilder 引用支援鏈式調用。</returns>
    UIBuilder& BindImage(reactive::Signal<std::shared_ptr<graphics::Bitmap>>& sig) {
        if (auto* pic = dynamic_cast<controls::PictureBox*>(m_element.get())) {
            pic->SetImage(sig.Get());
            sig.Subscribe([elem = m_element](const std::shared_ptr<graphics::Bitmap>& val) {
                if (auto* p = dynamic_cast<controls::PictureBox*>(elem.get())) {
                    p->SetImage(val);
                }
            });
        }
        return *this;
    }

private:
    std::shared_ptr<T> m_element;
};

// =============================================================================
// 工廠與宣告式 DSL 輔助函式 (Declarative DSL Factory Functions)
// =============================================================================

/// <summary>
/// 建立垂直流式排版容器 (VStack)。
/// </summary>
/// <param name="children">子元件清單。</param>
/// <returns>StackPanel 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<layout::StackPanel> VStack(std::initializer_list<std::shared_ptr<core::UIElement>> children = {}) {
    auto panel = std::make_shared<layout::StackPanel>();
    panel->SetOrientation(layout::Orientation::Vertical);
    for (const auto& child : children) {
        if (child) panel->AddChild(child);
    }
    return UIBuilder<layout::StackPanel>(panel);
}

/// <summary>
/// 建立水平流式排版容器 (HStack)。
/// </summary>
/// <param name="children">子元件清單。</param>
/// <returns>StackPanel 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<layout::StackPanel> HStack(std::initializer_list<std::shared_ptr<core::UIElement>> children = {}) {
    auto panel = std::make_shared<layout::StackPanel>();
    panel->SetOrientation(layout::Orientation::Horizontal);
    for (const auto& child : children) {
        if (child) panel->AddChild(child);
    }
    return UIBuilder<layout::StackPanel>(panel);
}

/// <summary>
/// 建立邊框容器 (Border)。
/// </summary>
/// <param name="child">可選的單一子元件智慧指標。</param>
/// <returns>Border 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<layout::Border> Border(std::shared_ptr<core::UIElement> child = nullptr) {
    auto border = std::make_shared<layout::Border>();
    if (child) border->SetChild(std::move(child));
    return UIBuilder<layout::Border>(border);
}

/// <summary>
/// 建立網格排版容器 (Grid)。
/// </summary>
/// <param name="children">子元件清單。</param>
/// <returns>Grid 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<layout::Grid> Grid(std::initializer_list<std::shared_ptr<core::UIElement>> children = {}) {
    auto grid = std::make_shared<layout::Grid>();
    for (const auto& child : children) {
        if (child) grid->AddChild(child);
    }
    return UIBuilder<layout::Grid>(grid);
}

/// <summary>
/// 建立文字標籤元件 (Label / TextBlock)。
/// </summary>
/// <param name="text">預設顯示文字視圖。</param>
/// <returns>TextBlock 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<controls::TextBlock> Label(compat::string_view text = "") {
    return UIBuilder<controls::TextBlock>(std::make_shared<controls::TextBlock>(text));
}

/// <summary>
/// 建立按鈕元件 (Button)。
/// </summary>
/// <param name="text">按鈕標題文字視圖。</param>
/// <returns>Button 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<controls::Button> Button(compat::string_view text = "") {
    return UIBuilder<controls::Button>(std::make_shared<controls::Button>(text));
}

/// <summary>
/// 建立核取方塊元件 (CheckBox)。
/// </summary>
/// <param name="label">核取方塊標籤文字視圖。</param>
/// <returns>CheckBox 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<controls::CheckBox> CheckBox(compat::string_view label = "") {
    return UIBuilder<controls::CheckBox>(std::make_shared<controls::CheckBox>(label));
}

/// <summary>
/// 建立單行文字輸入框 (TextBox)。
/// </summary>
/// <param name="placeholder">佔位提示字串視圖。</param>
/// <returns>TextBox 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<controls::TextBox> TextBox(compat::string_view placeholder = "") {
    return UIBuilder<controls::TextBox>(std::make_shared<controls::TextBox>(placeholder));
}

/// <summary>
/// 建立圖片顯示控制項 (PictureBox)。
/// </summary>
/// <param name="image">預設點陣圖影像指標。</param>
/// <returns>PictureBox 的 UIBuilder 包裝實例。</returns>
inline UIBuilder<controls::PictureBox> PictureBox(std::shared_ptr<graphics::Bitmap> image = nullptr) {
    return UIBuilder<controls::PictureBox>(std::make_shared<controls::PictureBox>(std::move(image)));
}

} // namespace dsl
} // namespace cppui
