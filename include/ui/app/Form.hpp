/**
 * @file Form.hpp
 * @brief 現代化桌面視窗基底類別，封裝原生視窗生命週期、事件迴圈、樣式與子內容呈現
 */

#pragma once

#include "compat/Compat.hpp"
#include "FormEventArgs.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/core/CoordinateTransform.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace cppui {
namespace controls {
class OverlayLayer;
} // namespace controls

namespace graphics {
class Bitmap;
} // namespace graphics

namespace rendering {
class SoftwareRenderTarget;
class IRenderContext;
} // namespace rendering

namespace app {

/// <summary>
/// 現代化桌面視窗基底類別 (Form)
/// 封裝底層平台視窗、訊息泵接駁、雙階段佈局、畫面呈現與生命週期事件。
/// </summary>
class Form : public controls::ContentControl {
public:
    /// <summary>
    /// 建構空白表單實例。
    /// </summary>
    Form();

    /// <summary>
    /// 虛擬解構子，釋放視窗資源並觸發關閉清理。
    /// </summary>
    ~Form() override;

    // --- 視窗外觀與屬性 ---

    /// <summary>
    /// 取得視窗標題字串視圖。
    /// </summary>
    /// <returns>標題字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetTitle() const noexcept { return m_title; }

    /// <summary>
    /// 設定視窗標題。
    /// </summary>
    /// <param name="title">標題字串視圖。</param>
    void SetTitle(compat::string_view title);

    /// <summary>
    /// 取得視窗客戶區尺寸。
    /// </summary>
    /// <returns>尺寸結構 (Size)。</returns>
    COMPAT_NODISCARD core::Size GetFormSize() const noexcept {
        return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
    }

    /// <summary>
    /// 設定視窗客戶區寬度與高度。
    /// </summary>
    /// <param name="width">寬度像素。</param>
    /// <param name="height">高度像素。</param>
    void SetFormSize(uint32_t width, uint32_t height);

    /// <summary>
    /// 設定表單約束寬度 (像素值)。同步更新表單與原生視窗寬度。
    /// </summary>
    /// <param name="width">寬度像素值。</param>
    void SetWidth(float width);

    /// <summary>
    /// 設定表單約束高度 (像素值)。同步更新表單與原生視窗高度。
    /// </summary>
    /// <param name="height">高度像素值。</param>
    void SetHeight(float height);

    /// <summary>
    /// 檢查是否啟用內容自適應 (AutoSize)。
    /// 等價於 GetSizeToContent() == SizeToContent::WidthAndHeight。
    /// </summary>
    /// <returns>若啟用自適應回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool GetAutoSize() const noexcept;

    /// <summary>
    /// 設定是否啟用內容自適應 (AutoSize)。
    /// 若為 true，將 SizeToContent 設為 WidthAndHeight；若為 false 設為 Manual。
    /// </summary>
    /// <param name="autoSize">是否啟用。</param>
    void SetAutoSize(bool autoSize);

    /// <summary>
    /// 取得視窗內容自適應模式 (SizeToContent)。
    /// </summary>
    /// <returns>SizeToContent 列舉值。</returns>
    COMPAT_NODISCARD SizeToContent GetSizeToContent() const noexcept { return m_sizeToContent; }

    /// <summary>
    /// 設定視窗內容自適應模式。
    /// </summary>
    /// <param name="mode">SizeToContent 列舉值。</param>
    void SetSizeToContent(SizeToContent mode);

    /// <summary>
    /// 取得自適應縮放約束模式 (AutoSizeMode)。
    /// </summary>
    /// <returns>AutoSizeMode 列舉值。</returns>
    COMPAT_NODISCARD AutoSizeMode GetAutoSizeMode() const noexcept { return m_autoSizeMode; }

    /// <summary>
    /// 設定自適應縮放約束模式。
    /// </summary>
    /// <param name="mode">AutoSizeMode 列舉值。</param>
    void SetAutoSizeMode(AutoSizeMode mode) noexcept;

    /// <summary>
    /// 依據內部元件樹期望尺寸執行自適應度量並重設視窗大小。
    /// </summary>
    void FitContentSize();

    /// <summary>
    /// 設定子內容元件並於啟用 AutoSize 時自動重整視窗尺寸。
    /// </summary>
    /// <param name="content">子元件智慧指標。</param>
    void SetContent(std::shared_ptr<core::UIElement> content);

    /// <summary>
    /// 設定子內容元件 (由 unique_ptr 轉移所有權)。
    /// </summary>
    /// <param name="content">子元件唯一擁有權智慧指標。</param>
    template <typename T, typename = typename std::enable_if<std::is_base_of<core::UIElement, T>::value>::type>
    void SetContent(std::unique_ptr<T> content) {
        SetContent(std::shared_ptr<core::UIElement>(std::move(content)));
    }

    /// <summary>
    /// 設定純文字內容並於啟用 AutoSize 時自動重整視窗尺寸。
    /// </summary>
    /// <param name="text">字串內容視圖。</param>
    void SetContent(compat::string_view text);

    /// <summary>
    /// 取得視窗初始啟動位置模式。
    /// </summary>
    /// <returns>FormStartPosition 列舉值。</returns>
    COMPAT_NODISCARD FormStartPosition GetStartPosition() const noexcept { return m_startPosition; }

    /// <summary>
    /// 設定視窗初始啟動位置模式。
    /// </summary>
    /// <param name="startPos">FormStartPosition 列舉值。</param>
    void SetStartPosition(FormStartPosition startPos) noexcept { m_startPosition = startPos; }

    /// <summary>
    /// 取得視窗顯示狀態 (正常/最小化/最大化)。
    /// </summary>
    /// <returns>WindowState 列舉值。</returns>
    COMPAT_NODISCARD WindowState GetWindowState() const noexcept { return m_windowState; }

    /// <summary>
    /// 設定視窗顯示狀態。
    /// </summary>
    /// <param name="state">WindowState 列舉值。</param>
    void SetWindowState(WindowState state);

    /// <summary>
    /// 取得互動對話框結果 (DialogResult)。
    /// </summary>
    /// <returns>DialogResult 列舉值。</returns>
    COMPAT_NODISCARD DialogResult GetDialogResult() const noexcept { return m_dialogResult; }

    /// <summary>
    /// 設定互動對話框結果。
    /// </summary>
    /// <param name="result">DialogResult 列舉值。</param>
    void SetDialogResult(DialogResult result) noexcept { m_dialogResult = result; }

    /// <summary>
    /// 檢查視窗是否已被銷毀。
    /// </summary>
    /// <returns>若已銷毀傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsDisposed() const noexcept { return m_isDisposed; }

    // --- 視窗顯示與控制 ---

    /// <summary>
    /// 顯示非強制回應視窗。
    /// </summary>
    virtual void Show();

    /// <summary>
    /// 隱藏視窗。
    /// </summary>
    virtual void Hide();

    /// <summary>
    /// 關閉視窗，觸發 Closing 與 Closed 生命週期。
    /// </summary>
    virtual void Close();

    /// <summary>
    /// 以強制回應對話框形式顯示視窗。
    /// </summary>
    /// <param name="owner">可選之主控擁有者表單 shared_ptr。</param>
    /// <returns>對話框操作結果。</returns>
    virtual DialogResult ShowDialog(std::shared_ptr<Form> owner = nullptr);

    // --- 虛擬生命週期回呼點 (可供衍生類別覆寫) ---

    /// <summary>表單載入時虛擬掛鉤。</summary>
    virtual void OnLoad();

    /// <summary>表單初次顯示時虛擬掛鉤。</summary>
    virtual void OnShown();

    /// <summary>表單獲得焦點作用中時虛擬掛鉤。</summary>
    virtual void OnActivated();

    /// <summary>表單失去焦點非作用中時虛擬掛鉤。</summary>
    virtual void OnDeactivate();

    /// <summary>
    /// 表單即將關閉前之虛擬掛鉤，可透過 e.Cancel = true 取消關閉。
    /// </summary>
    /// <param name="e">關閉事件參數。</param>
    virtual void OnFormClosing(FormClosingEventArgs& e);

    /// <summary>
    /// 表單已關閉後之虛擬掛鉤。
    /// </summary>
    /// <param name="e">關閉完成事件參數。</param>
    virtual void OnFormClosed(FormClosedEventArgs& e);

    // --- 外部回呼事件監聽 ---

    /// <summary>
    /// 表單關閉前處理常式委派。
    /// </summary>
    using FormClosingHandler = std::function<void(Form&, FormClosingEventArgs&)>;

    /// <summary>
    /// 表單關閉後處理常式委派。
    /// </summary>
    using FormClosedHandler = std::function<void(Form&, FormClosedEventArgs&)>;

    /// <summary>
    /// 註冊表單即將關閉事件監聽常式。
    /// </summary>
    /// <param name="handler">事件回呼常式。</param>
    void AddClosingHandler(FormClosingHandler handler);

    /// <summary>
    /// 註冊表單已關閉事件監聽常式。
    /// </summary>
    /// <param name="handler">事件回呼常式。</param>
    void AddClosedHandler(FormClosedHandler handler);

    // --- 原生視窗與底層存取 ---

    /// <summary>
    /// 取得關聯之底層抽象平台視窗 shared_ptr。
    /// </summary>
    /// <returns>平台視窗指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<pal::IPlatformWindow> GetPlatformWindow() const noexcept { return m_window; }

    /// <summary>
    /// 取得視窗頂層懸浮圖層 (OverlayLayer)。
    /// </summary>
    /// <returns>OverlayLayer 指標。</returns>
    COMPAT_NODISCARD controls::OverlayLayer* GetOverlayLayer() const noexcept;

    /// <summary>
    /// 觸發內部畫面重新佈局與重繪。
    /// </summary>
    void InvalidateForm();

    /// <summary>
    /// 將表單當前視覺內容與佈局渲染至離線點陣圖 (Bitmap)。
    /// 支援離線視窗截圖與無頭驗證。
    /// </summary>
    /// <returns>點陣圖共享指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<graphics::Bitmap> CaptureToBitmap();

protected:
    /// <summary>
    /// 確保底層平台原生視窗已初始化建立。
    /// </summary>
    void EnsurePlatformWindow();

    /// <summary>
    /// 連接平台視窗事件至 Form 內部事件通道。
    /// </summary>
    void SetupPlatformEvents();

    /// <summary>
    /// 執行畫面單影格渲染與呈現 (Presentation)。
    /// </summary>
    void RenderFrame();

    /// <summary>
    /// 處理視窗大小改變通知並重新佈局內部內容。
    /// </summary>
    /// <param name="width">新寬度。</param>
    /// <param name="height">新高度。</param>
    void HandleWindowResize(uint32_t width, uint32_t height);

private:
    std::string m_title = "CPPUI Form";
    uint32_t m_width = 800;
    uint32_t m_height = 600;
    FormStartPosition m_startPosition = FormStartPosition::CenterScreen;
    WindowState m_windowState = WindowState::Normal;
    DialogResult m_dialogResult = DialogResult::None;
    SizeToContent m_sizeToContent = SizeToContent::Manual;
    AutoSizeMode m_autoSizeMode = AutoSizeMode::GrowAndShrink;
    bool m_isFittingContent = false;

    bool m_isLoaded = false;
    bool m_isShown = false;
    bool m_isDisposed = false;
    bool m_inModalLoop = false;

    std::shared_ptr<pal::IPlatformWindow> m_window;
    std::unique_ptr<rendering::SoftwareRenderTarget> m_renderTarget;
    std::unique_ptr<rendering::IRenderContext> m_renderContext;
    rendering::DisplayListRenderer m_renderer;
    core::CoordinateTransform m_coordTransform;

    std::vector<FormClosingHandler> m_closingHandlers;
    std::vector<FormClosedHandler> m_closedHandlers;
    size_t m_themeHandlerId = 0;
    std::shared_ptr<controls::OverlayLayer> m_overlayLayer;
};

} // namespace app
} // namespace cppui
