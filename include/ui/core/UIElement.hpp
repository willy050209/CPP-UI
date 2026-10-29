/*
 * File: UIElement.hpp
 * Purpose: Declares the core UIElement base class with layout sizing, hit testing,
 *          focus management, and routed events.
 */

#pragma once

#include <cstdint>
#include <limits>
#include <algorithm>
#include <functional>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/LifetimeToken.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/core/Visual.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/FocusEventArgs.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 具備佈局幾何約束、髒標記追蹤、雙階段排版管線與路由事件分發的核心 UI 元件基底類別 (UIElement)
/// 繼承自 Visual，引入 Margin、Alignment、Measure/Arrange 及 RoutedEvent 事件處理。
/// </summary>
class UIElement : public Visual {
public:
    using RoutedEventHandler    = std::function<void(input::RoutedEventArgs&)>;
    using PointerEventHandler   = std::function<void(input::PointerEventArgs&)>;
    using KeyEventHandler       = std::function<void(input::KeyEventArgs&)>;
    using TextInputEventHandler = std::function<void(input::TextInputEventArgs&)>;
    using ImeCompositionEventHandler = std::function<void(input::ImeCompositionEventArgs&)>;
    using FocusEventHandler     = std::function<void(input::FocusEventArgs&)>;

    /// <summary>
    /// 建構核心 UI 元件實例。
    /// </summary>
    UIElement() = default;

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~UIElement() override;

    // --- 佈局幾何約束屬性 ---

    /// <summary>
    /// 取得外邊距留白厚度 (Margin)。
    /// </summary>
    /// <returns>Thickness 外距厚度。</returns>
    COMPAT_NODISCARD Thickness GetMargin() const noexcept { return m_margin; }

    /// <summary>
    /// 設定外邊距留白厚度。
    /// </summary>
    /// <param name="margin">Thickness 外距厚度。</param>
    void SetMargin(const Thickness& margin);

    /// <summary>
    /// 取得顯式約束寬度 (負值表示 Auto)。
    /// </summary>
    /// <returns>約束寬度值。</returns>
    COMPAT_NODISCARD float GetWidth() const noexcept { return m_width; }

    /// <summary>
    /// 設定顯式約束寬度。
    /// </summary>
    /// <param name="width">寬度像素值 (或負值表示 Auto)。</param>
    void SetWidth(float width);

    /// <summary>
    /// 取得顯式約束高度 (負值表示 Auto)。
    /// </summary>
    /// <returns>約束高度值。</returns>
    COMPAT_NODISCARD float GetHeight() const noexcept { return m_height; }

    /// <summary>
    /// 設定顯式約束高度。
    /// </summary>
    /// <param name="height">高度像素值 (或負值表示 Auto)。</param>
    void SetHeight(float height);

    /// <summary>
    /// 取得元件是否啟用內容自適應 (AutoSize)。
    /// </summary>
    /// <returns>若啟用傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool GetAutoSize() const noexcept { return m_autoSize; }

    /// <summary>
    /// 設定元件是否啟用內容自適應 (AutoSize)。
    /// </summary>
    /// <param name="autoSize">true 啟用自適應尺寸，false 維持手動固定尺寸。</param>
    void SetAutoSize(bool autoSize);

    /// <summary>
    /// 取得最小寬度約束。
    /// </summary>
    /// <returns>最小寬度像素值。</returns>
    COMPAT_NODISCARD float GetMinWidth() const noexcept { return m_minWidth; }

    /// <summary>
    /// 設定最小寬度約束。
    /// </summary>
    /// <param name="minWidth">最小寬度像素值。</param>
    void SetMinWidth(float minWidth);

    /// <summary>
    /// 取得最大寬度約束。
    /// </summary>
    /// <returns>最大寬度像素值。</returns>
    COMPAT_NODISCARD float GetMaxWidth() const noexcept { return m_maxWidth; }

    /// <summary>
    /// 設定最大寬度約束。
    /// </summary>
    /// <param name="maxWidth">最大寬度像素值。</param>
    void SetMaxWidth(float maxWidth);

    /// <summary>
    /// 取得最小高度約束。
    /// </summary>
    /// <returns>最小高度像素值。</returns>
    COMPAT_NODISCARD float GetMinHeight() const noexcept { return m_minHeight; }

    /// <summary>
    /// 設定最小高度約束。
    /// </summary>
    /// <param name="minHeight">最小高度像素值。</param>
    void SetMinHeight(float minHeight);

    /// <summary>
    /// 取得最大高度約束。
    /// </summary>
    /// <returns>最大高度像素值。</returns>
    COMPAT_NODISCARD float GetMaxHeight() const noexcept { return m_maxHeight; }

    /// <summary>
    /// 設定最大高度約束。
    /// </summary>
    /// <param name="maxHeight">最大高度像素值。</param>
    void SetMaxHeight(float maxHeight);

    /// <summary>
    /// 取得水平對齊方式。
    /// </summary>
    /// <returns>HorizontalAlignment 列舉值。</returns>
    COMPAT_NODISCARD layout::HorizontalAlignment GetHorizontalAlignment() const noexcept { return m_horizontalAlignment; }

    /// <summary>
    /// 設定水平對齊方式。
    /// </summary>
    /// <param name="alignment">HorizontalAlignment 列舉值。</param>
    void SetHorizontalAlignment(layout::HorizontalAlignment alignment);

    /// <summary>
    /// 取得垂直對齊方式。
    /// </summary>
    /// <returns>VerticalAlignment 列舉值。</returns>
    COMPAT_NODISCARD layout::VerticalAlignment GetVerticalAlignment() const noexcept { return m_verticalAlignment; }

    /// <summary>
    /// 設定垂直對齊方式。
    /// </summary>
    /// <param name="alignment">VerticalAlignment 列舉值。</param>
    void SetVerticalAlignment(layout::VerticalAlignment alignment);

    /// <summary>
    /// 取得可見性模式 (Visible / Hidden / Collapsed)。
    /// </summary>
    /// <returns>Visibility 列舉值。</returns>
    COMPAT_NODISCARD layout::Visibility GetVisibility() const noexcept { return m_visibility; }

    /// <summary>
    /// 設定可見性模式。
    /// </summary>
    /// <param name="visibility">Visibility 列舉值。</param>
    void SetVisibility(layout::Visibility visibility);

    // --- 排版結果與邊界快取 ---

    /// <summary>
    /// 取得排版度量期望尺寸 (DesiredSize)。
    /// </summary>
    /// <returns>期望尺寸。</returns>
    COMPAT_NODISCARD Size GetDesiredSize() const noexcept { return m_desiredSize; }

    /// <summary>
    /// 取得排版排列配置矩形邊界 (RenderBounds)。
    /// </summary>
    /// <returns>渲染邊界矩形。</returns>
    COMPAT_NODISCARD Rect GetRenderBounds() const noexcept { return m_renderBounds; }

    /// <summary>
    /// 取得實際佈局尺寸 (ActualSize)。
    /// </summary>
    /// <returns>實際尺寸。</returns>
    COMPAT_NODISCARD Size GetActualSize() const noexcept { return m_renderBounds.GetSize(); }

    // --- 髒標記狀態與控制 ---

    /// <summary>
    /// 檢查度量階段快取是否有效。
    /// </summary>
    /// <returns>若有效回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsMeasureValid() const noexcept { return m_isMeasureValid; }

    /// <summary>
    /// 檢查排列階段快取是否有效。
    /// </summary>
    /// <returns>若有效回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsArrangeValid() const noexcept { return m_isArrangeValid; }

    /// <summary>
    /// 檢查視覺渲染快取是否有效。
    /// </summary>
    /// <returns>若有效回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsVisualValid() const noexcept { return m_isVisualValid; }

    /// <summary>
    /// 標記尺寸為髒區，並向上冒泡通知父容器重新排版。
    /// </summary>
    void InvalidateMeasure();

    /// <summary>
    /// 標記排列為髒區，通知重新配置子節點位置。
    /// </summary>
    void InvalidateArrange();

    /// <summary>
    /// 標記視覺為髒區，僅通知重繪，完全不重跑 Measure 與 Arrange。
    /// </summary>
    void InvalidateVisual();

    // --- 雙階段排版管線入口 ---

    /// <summary>
    /// 執行第一階段尺寸度量 (Measure Pipeline)。
    /// </summary>
    /// <param name="availableSize">父容器提供之可用尺寸約束。</param>
    void Measure(Size availableSize);

    /// <summary>
    /// 執行第二階段排列定位 (Arrange Pipeline)。
    /// </summary>
    /// <param name="finalRect">父容器分配給本元件之最終邊界矩形。</param>
    void Arrange(Rect finalRect);

    // --- 子類別重寫之排版實作點 ---

    /// <summary>
    /// 自訂尺寸度量實作點，供衍生元件覆寫以計算內容所需期望尺寸。
    /// </summary>
    /// <param name="availableSize">可用尺寸。</param>
    /// <returns>元件內容期望尺寸。</returns>
    virtual Size MeasureOverride(Size availableSize);

    /// <summary>
    /// 自訂排列定位實作點，供衍生元件覆寫以排列子元件。
    /// </summary>
    /// <param name="finalSize">實際分配之尺寸。</param>
    virtual void ArrangeOverride(Size finalSize);

    /// <summary>
    /// 取得視覺樹之父 UIElement 指標。
    /// </summary>
    /// <returns>父 UIElement 指標，若無則為 nullptr。</returns>
    COMPAT_NODISCARD UIElement* GetParentUIElement() const noexcept;

    // --- MVVM DataContext 繼承體系 ---

    /// <summary>
    /// 取得資料上下文 (DataContext)；若本節點未設定則沿視覺樹向上繼承尋找。
    /// </summary>
    /// <returns>資料上下文之泛型 shared_ptr。</returns>
    COMPAT_NODISCARD std::shared_ptr<void> GetDataContext() const;

    /// <summary>
    /// 設定本節點之資料上下文 (DataContext)。
    /// </summary>
    /// <param name="dataContext">資料上下文 shared_ptr。</param>
    void SetDataContext(std::shared_ptr<void> dataContext);

    /// <summary>
    /// 依型別 T 取得並轉型資料上下文。
    /// </summary>
    /// <typeparam name="T">目標 ViewModel 或資料型別。</typeparam>
    /// <returns>型別化的 shared_ptr<T>。</returns>
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetDataContentType() const {
        return std::static_pointer_cast<T>(GetDataContext());
    }

    // --- 路由事件註冊與派發 ---

    /// <summary>
    /// 註冊路由事件處理常式。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項。</param>
    /// <param name="handler">事件處理回呼函式。</param>
    /// <param name="handledEventsToo">是否接收已標記為 Handled 之事件，預設為 false。</param>
    void AddHandler(const input::RoutedEvent& routedEvent, RoutedEventHandler handler, bool handledEventsToo = false);

    /// <summary>
    /// 移除指定路由事件之所有處理常式。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項。</param>
    void RemoveHandler(const input::RoutedEvent& routedEvent);

    /// <summary>
    /// 沿視覺樹路由通道觸發引發路由事件。
    /// </summary>
    /// <param name="e">路由事件參數引用。</param>
    void RaiseEvent(input::RoutedEventArgs& e);

    /// <summary>
    /// 內部路由事件派發實作函式。
    /// </summary>
    /// <param name="routedEvent">路由事件描述項。</param>
    /// <param name="e">事件參數引用。</param>
    void RaiseEventInternal(const input::RoutedEvent& routedEvent, input::RoutedEventArgs& e);

    // 便捷事件掛載語法糖

    /// <summary>
    /// 註冊指標下壓 (PointerPressed) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnPointerPressed(PointerEventHandler handler);

    /// <summary>
    /// 註冊指標釋放 (PointerReleased) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnPointerReleased(PointerEventHandler handler);

    /// <summary>
    /// 註冊指標移動 (PointerMoved) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnPointerMoved(PointerEventHandler handler);

    /// <summary>
    /// 註冊指標滾輪 (PointerWheel) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnPointerWheel(PointerEventHandler handler);

    /// <summary>
    /// 註冊指標進入 (PointerEntered) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnPointerEntered(PointerEventHandler handler);

    /// <summary>
    /// 註冊指標離開 (PointerExited) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnPointerExited(PointerEventHandler handler);

    /// <summary>
    /// 註冊按鍵下壓 (KeyDown) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnKeyDown(KeyEventHandler handler);

    /// <summary>
    /// 註冊按鍵釋放 (KeyUp) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnKeyUp(KeyEventHandler handler);

    /// <summary>
    /// 註冊文字輸入 (TextInput) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnTextInput(TextInputEventHandler handler);

    /// <summary>
    /// 註冊輸入法組字 (ImeComposition) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnImeComposition(ImeCompositionEventHandler handler);

    /// <summary>
    /// 註冊獲得焦點 (GotFocus) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnGotFocus(FocusEventHandler handler);

    /// <summary>
    /// 註冊失去焦點 (LostFocus) 事件處理常式。
    /// </summary>
    /// <param name="handler">事件處理常式。</param>
    void OnLostFocus(FocusEventHandler handler);

    // 子類別事件處理虛擬回呼點

    /// <summary>指標下壓虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnPointerPressedCore(input::PointerEventArgs& e) { (void)e; }

    /// <summary>指標釋放虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnPointerReleasedCore(input::PointerEventArgs& e) { (void)e; }

    /// <summary>指標移動虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnPointerMovedCore(input::PointerEventArgs& e) { (void)e; }

    /// <summary>指標滾輪虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnPointerWheelCore(input::PointerEventArgs& e) { (void)e; }

    /// <summary>指標進入虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnPointerEnteredCore(input::PointerEventArgs& e) { (void)e; }

    /// <summary>指標離開虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnPointerExitedCore(input::PointerEventArgs& e) { (void)e; }

    /// <summary>按鍵下壓虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnKeyDownCore(input::KeyEventArgs& e) { (void)e; }

    /// <summary>按鍵釋放虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnKeyUpCore(input::KeyEventArgs& e) { (void)e; }

    /// <summary>文字輸入虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnTextInputCore(input::TextInputEventArgs& e) { (void)e; }

    /// <summary>輸入法組字虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnImeCompositionCore(input::ImeCompositionEventArgs& e) { (void)e; }

    /// <summary>獲得焦點虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnGotFocusCore(input::FocusEventArgs& e) { (void)e; }

    /// <summary>失去焦點虛擬處理點。</summary>
    /// <param name="e">事件參數。</param>
    virtual void OnLostFocusCore(input::FocusEventArgs& e) { (void)e; }

    // --- 命中測試與焦點屬性 ---

    /// <summary>
    /// 執行局部座標命中測試。
    /// </summary>
    /// <param name="localPoint">局部座標點。</param>
    /// <returns>命中的 Visual 指標。</returns>
    COMPAT_NODISCARD Visual* HitTest(Point localPoint) override;

    /// <summary>
    /// 檢查是否參與命中測試。
    /// </summary>
    /// <returns>若參與命中測試傳回 true。</returns>
    COMPAT_NODISCARD bool IsHitTestVisible() const noexcept { return m_isHitTestVisible; }

    /// <summary>
    /// 設定是否參與命中測試。
    /// </summary>
    /// <param name="visible">是否參與命中測試。</param>
    void SetHitTestVisible(bool visible) noexcept { m_isHitTestVisible = visible; }

    /// <summary>
    /// 檢查是否可獲得焦點。
    /// </summary>
    /// <returns>若可獲焦傳回 true。</returns>
    COMPAT_NODISCARD bool IsFocusable() const noexcept { return m_isFocusable; }

    /// <summary>
    /// 設定是否可獲得焦點。
    /// </summary>
    /// <param name="focusable">是否可獲焦。</param>
    void SetFocusable(bool focusable) noexcept { m_isFocusable = focusable; }

    /// <summary>
    /// 檢查目前是否擁有鍵盤焦點。
    /// </summary>
    /// <returns>若正擁有焦點傳回 true。</returns>
    COMPAT_NODISCARD bool IsFocused() const noexcept { return m_isFocused; }

    /// <summary>
    /// 內部設定焦點狀態。
    /// </summary>
    /// <param name="focused">焦點狀態。</param>
    void SetFocusedInternal(bool focused) noexcept;

    /// <summary>
    /// 取得 Tab 鍵跳轉序號。
    /// </summary>
    /// <returns>Tab 鍵序號。</returns>
    COMPAT_NODISCARD int32_t GetTabIndex() const noexcept { return m_tabIndex; }

    /// <summary>
    /// 設定 Tab 鍵跳轉序號。
    /// </summary>
    /// <param name="index">Tab 鍵序號。</param>
    void SetTabIndex(int32_t index) noexcept { m_tabIndex = index; }

    /// <summary>
    /// 檢查是否為 Tab 停靠點。
    /// </summary>
    /// <returns>若為停靠點傳回 true。</returns>
    COMPAT_NODISCARD bool IsTabStop() const noexcept { return m_isTabStop; }

    /// <summary>
    /// 設定是否為 Tab 停靠點。
    /// </summary>
    /// <param name="tabStop">是否為停靠點。</param>
    void SetTabStop(bool tabStop) noexcept { m_isTabStop = tabStop; }

    // --- 生命週期權杖與連線管理 ---

    /// <summary>
    /// 取得與本 UIElement 生命週期繫結之輕量級權杖 (LifetimeToken)。
    /// 當本元件解構時該權杖自動失效，可用於防範非同步回呼、Dispatcher 與定時器之 Use-After-Free。
    /// </summary>
    /// <returns>所屬 LifetimeToken 實例。</returns>
    COMPAT_NODISCARD LifetimeToken GetLifetimeToken() const noexcept { return m_lifetimeToken; }

    /// <summary>
    /// 將 RAII 訂閱連線納入本元件生命週期管理，元件解構時自動中斷訂閱。
    /// </summary>
    /// <param name="connection">欲託管之 ScopedConnection。</param>
    void ManageConnection(reactive::ScopedConnection connection) {
        m_managedConnections.push_back(std::move(connection));
    }

protected:
    core::LifetimeToken                        m_lifetimeToken{ core::LifetimeToken::Create() };
    std::vector<reactive::ScopedConnection>    m_managedConnections;

    struct HandlerEntry {
        uint32_t           eventId = 0;
        RoutedEventHandler handler;
        bool               handledEventsToo = false;

        HandlerEntry() = default;
        HandlerEntry(uint32_t id, RoutedEventHandler h, bool too = false)
            : eventId(id), handler(std::move(h)), handledEventsToo(too) {}
    };
    std::vector<HandlerEntry> m_eventHandlers;
    std::shared_ptr<void>     m_dataContext;

    bool                        m_isHitTestVisible = true;
    bool                        m_isFocusable = false;
    bool                        m_isFocused = false;
    int32_t                     m_tabIndex = 0;
    bool                        m_isTabStop = true;

    Thickness                   m_margin{ 0.0f, 0.0f, 0.0f, 0.0f };
    float                       m_width = -1.0f;  // < 0 表示 Auto (未約束)
    float                       m_height = -1.0f; // < 0 表示 Auto
    float                       m_minWidth = 0.0f;
    float                       m_maxWidth = 1000000.0f;
    float                       m_minHeight = 0.0f;
    float                       m_maxHeight = 1000000.0f;
    layout::HorizontalAlignment m_horizontalAlignment = layout::HorizontalAlignment::Stretch;
    layout::VerticalAlignment   m_verticalAlignment = layout::VerticalAlignment::Stretch;
    layout::Visibility          m_visibility = layout::Visibility::Visible;
    bool                        m_autoSize = false;

    // 排版狀態快取
    Size                        m_desiredSize{ 0.0f, 0.0f };
    Rect                        m_renderBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    Size                        m_previousAvailableSize{ -1.0f, -1.0f };
    Rect                        m_previousFinalRect{ -1.0f, -1.0f, -1.0f, -1.0f };

    bool                        m_isMeasureValid = false;
    bool                        m_isArrangeValid = false;
    bool                        m_isVisualValid = false;
};

} // namespace core
} // namespace cppui
