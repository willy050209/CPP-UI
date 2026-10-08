#pragma once

#include "compat/compat.hpp"
#include "FormEventArgs.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/core/CoordinateTransform.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cppui {
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
    Form();
    ~Form() override;

    // --- 視窗外觀與屬性 ---
    COMPAT_NODISCARD compat::string_view GetTitle() const noexcept { return m_title; }
    void SetTitle(compat::string_view title);

    COMPAT_NODISCARD core::Size GetFormSize() const noexcept {
        return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
    }
    void SetFormSize(uint32_t width, uint32_t height);

    COMPAT_NODISCARD FormStartPosition GetStartPosition() const noexcept { return m_startPosition; }
    void SetStartPosition(FormStartPosition startPos) noexcept { m_startPosition = startPos; }

    COMPAT_NODISCARD WindowState GetWindowState() const noexcept { return m_windowState; }
    void SetWindowState(WindowState state);

    COMPAT_NODISCARD DialogResult GetDialogResult() const noexcept { return m_dialogResult; }
    void SetDialogResult(DialogResult result) noexcept { m_dialogResult = result; }

    COMPAT_NODISCARD bool IsDisposed() const noexcept { return m_isDisposed; }

    // --- 視窗顯示與控制 ---
    virtual void Show();
    virtual void Hide();
    virtual void Close();
    virtual DialogResult ShowDialog(std::shared_ptr<Form> owner = nullptr);

    // --- 虛擬生命週期回呼點 (可供衍生類別覆寫) ---
    virtual void OnLoad();
    virtual void OnShown();
    virtual void OnActivated();
    virtual void OnDeactivate();
    virtual void OnFormClosing(FormClosingEventArgs& e);
    virtual void OnFormClosed(FormClosedEventArgs& e);

    // --- 外部回呼事件監聽 ---
    using FormClosingHandler = std::function<void(Form&, FormClosingEventArgs&)>;
    using FormClosedHandler = std::function<void(Form&, FormClosedEventArgs&)>;
    void AddClosingHandler(FormClosingHandler handler);
    void AddClosedHandler(FormClosedHandler handler);

    // --- 原生視窗與底層存取 ---
    COMPAT_NODISCARD std::shared_ptr<pal::IPlatformWindow> GetPlatformWindow() const noexcept { return m_window; }

    /// <summary>
    /// 觸發內部畫面重新佈局與重繪
    /// </summary>
    void InvalidateForm();

protected:
    void EnsurePlatformWindow();
    void SetupPlatformEvents();
    void RenderFrame();
    void HandleWindowResize(uint32_t width, uint32_t height);

private:
    std::string m_title = "CPPUI Form";
    uint32_t m_width = 800;
    uint32_t m_height = 600;
    FormStartPosition m_startPosition = FormStartPosition::CenterScreen;
    WindowState m_windowState = WindowState::Normal;
    DialogResult m_dialogResult = DialogResult::None;

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
};

} // namespace app
} // namespace cppui
