/*
 * File: Form.cpp
 * Purpose: Implements Form desktop window lifecycle, rendering loop, and input handling.
 */

#include "ui/app/Form.hpp"
#include "ui/app/Application.hpp"
#include "ui/controls/OverlayLayer.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include "../rendering/backend/SoftwareRenderContext.hpp"
#include "../rendering/backend/SkiaRenderContext.hpp"
#include "ui/graphics/Bitmap.hpp"
#include <algorithm>
#include <cmath>

namespace cppui {
namespace app {

Form::Form() {
    SetPadding(core::Thickness(0.0f));
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetBackground(theme.Colors().windowBackground);
    m_hasExplicitBackground = false;

    m_overlayLayer = std::make_shared<controls::OverlayLayer>();
    m_overlayLayer->SetWidth(static_cast<float>(m_width));
    m_overlayLayer->SetHeight(static_cast<float>(m_height));
    AddVisualChild(m_overlayLayer);

    m_themeHandlerId = theme::ThemeManager::Get().AddThemeChangedHandler([this](const theme::Theme& t) {
        if (!m_hasExplicitBackground) {
            SetBackground(t.Colors().windowBackground);
            m_hasExplicitBackground = false;
        }
        ApplyTheme(t);
        InvalidateForm();
    });
}

Form::~Form() {
    if (m_themeHandlerId != 0) {
        theme::ThemeManager::Get().RemoveThemeChangedHandler(m_themeHandlerId);
        m_themeHandlerId = 0;
    }
    if (m_renderContext) {
        m_renderContext->Destroy();
        m_renderContext.reset();
    }
    if (!m_isDisposed) {
        Application::UnregisterForm(this);
    }
}

void Form::SetTitle(compat::string_view title) {
    m_title = std::string(title.data(), title.size());
    if (m_window) {
        m_window->SetTitle(m_title);
    }
}

void Form::SetFormSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    if (m_sizeToContent == SizeToContent::Manual) {
        core::UIElement::SetWidth(static_cast<float>(width));
        core::UIElement::SetHeight(static_cast<float>(height));
    }
    if (m_overlayLayer) {
        m_overlayLayer->SetWidth(static_cast<float>(width));
        m_overlayLayer->SetHeight(static_cast<float>(height));
    }
    if (m_window) {
        m_window->SetSize(width, height);
        HandleWindowResize(width, height);
    }
}

void Form::SetWidth(float width) {
    core::UIElement::SetWidth(width);
    uint32_t uWidth = width > 0.0f ? static_cast<uint32_t>(width) : 0;
    if (uWidth > 0 && uWidth != m_width) {
        SetFormSize(uWidth, m_height);
    }
}

void Form::SetHeight(float height) {
    core::UIElement::SetHeight(height);
    uint32_t uHeight = height > 0.0f ? static_cast<uint32_t>(height) : 0;
    if (uHeight > 0 && uHeight != m_height) {
        SetFormSize(m_width, uHeight);
    }
}

bool Form::GetAutoSize() const noexcept {
    return m_sizeToContent == SizeToContent::WidthAndHeight;
}

void Form::SetAutoSize(bool autoSize) {
    SetSizeToContent(autoSize ? SizeToContent::WidthAndHeight : SizeToContent::Manual);
}

void Form::SetSizeToContent(SizeToContent mode) {
    m_sizeToContent = mode;
    if (m_sizeToContent != SizeToContent::Manual) {
        FitContentSize();
    }
}

void Form::SetAutoSizeMode(AutoSizeMode mode) noexcept {
    m_autoSizeMode = mode;
    if (m_sizeToContent != SizeToContent::Manual) {
        FitContentSize();
    }
}

void Form::FitContentSize() {
    if (m_isFittingContent || m_sizeToContent == SizeToContent::Manual) {
        return;
    }

    struct FitGuard {
        bool& flag;
        explicit FitGuard(bool& f) noexcept : flag(f) { flag = true; }
        ~FitGuard() noexcept { flag = false; }
    } guard(m_isFittingContent);

    // 準備可用空間測量邊界 (非自適應軸採用當前尺寸，自適應軸提供未約束大空間)
    float availW = (m_sizeToContent == SizeToContent::Width || m_sizeToContent == SizeToContent::WidthAndHeight)
                       ? 1000000.0f
                       : static_cast<float>(m_width);
    float availH = (m_sizeToContent == SizeToContent::Height || m_sizeToContent == SizeToContent::WidthAndHeight)
                       ? 1000000.0f
                       : static_cast<float>(m_height);

    float savedElemW = core::UIElement::GetWidth();
    float savedElemH = core::UIElement::GetHeight();

    if (m_sizeToContent == SizeToContent::Width || m_sizeToContent == SizeToContent::WidthAndHeight) {
        core::UIElement::SetWidth(-1.0f);
    }
    if (m_sizeToContent == SizeToContent::Height || m_sizeToContent == SizeToContent::WidthAndHeight) {
        core::UIElement::SetHeight(-1.0f);
    }

    InvalidateMeasure();
    Measure(core::Size(availW, availH));

    core::Size desired = GetDesiredSize();

    if (m_sizeToContent != SizeToContent::Width && m_sizeToContent != SizeToContent::WidthAndHeight) {
        core::UIElement::SetWidth(savedElemW);
    }
    if (m_sizeToContent != SizeToContent::Height && m_sizeToContent != SizeToContent::WidthAndHeight) {
        core::UIElement::SetHeight(savedElemH);
    }

    uint32_t targetW = m_width;
    uint32_t targetH = m_height;

    if (m_sizeToContent == SizeToContent::Width || m_sizeToContent == SizeToContent::WidthAndHeight) {
        float calcW = std::ceil(desired.width);
        calcW = compat::clamp(calcW, GetMinWidth(), GetMaxWidth());
        uint32_t newW = static_cast<uint32_t>(calcW);
        if (m_autoSizeMode == AutoSizeMode::GrowOnly) {
            targetW = (std::max)(targetW, newW);
        } else {
            targetW = newW;
        }
    }

    if (m_sizeToContent == SizeToContent::Height || m_sizeToContent == SizeToContent::WidthAndHeight) {
        float calcH = std::ceil(desired.height);
        calcH = compat::clamp(calcH, GetMinHeight(), GetMaxHeight());
        uint32_t newH = static_cast<uint32_t>(calcH);
        if (m_autoSizeMode == AutoSizeMode::GrowOnly) {
            targetH = (std::max)(targetH, newH);
        } else {
            targetH = newH;
        }
    }

    if (targetW == 0) targetW = 100;
    if (targetH == 0) targetH = 100;

    if (targetW != m_width || targetH != m_height) {
        m_width = targetW;
        m_height = targetH;
        if (m_overlayLayer) {
            m_overlayLayer->SetWidth(static_cast<float>(targetW));
            m_overlayLayer->SetHeight(static_cast<float>(targetH));
        }
        if (m_window) {
            m_window->SetSize(targetW, targetH);
            HandleWindowResize(targetW, targetH);
        }
    }
}

void Form::SetContent(std::shared_ptr<core::UIElement> content) {
    controls::ContentControl::SetContent(std::move(content));
    if (m_sizeToContent != SizeToContent::Manual) {
        FitContentSize();
    }
}

void Form::SetContent(compat::string_view text) {
    controls::ContentControl::SetContent(text);
    if (m_sizeToContent != SizeToContent::Manual) {
        FitContentSize();
    }
}

void Form::SetWindowState(WindowState state) {
    m_windowState = state;
}

void Form::Show() {
    if (m_isDisposed) return;
    if (m_sizeToContent != SizeToContent::Manual) {
        FitContentSize();
    }
    EnsurePlatformWindow();

    if (!m_isLoaded) {
        m_isLoaded = true;
        OnLoad();
    }

    if (m_window) {
        m_window->Show();
    }

    if (!m_isShown) {
        m_isShown = true;
        OnShown();
        OnActivated();
    }

    InvalidateForm();

    try {
        Application::RegisterForm(std::static_pointer_cast<Form>(shared_from_this()));
    } catch (...) {
        // 在非 shared_ptr 託管或單元測試環境下容錯
    }
}

void Form::Hide() {
    if (m_window) {
        m_window->Hide();
    }
    OnDeactivate();
}

void Form::Close() {
    if (m_isDisposed) return;

    FormClosingEventArgs closingArgs;
    closingArgs.Cancel = false;
    closingArgs.CloseReasonIsUser = true;

    OnFormClosing(closingArgs);
    for (const auto& handler : m_closingHandlers) {
        if (handler) {
            handler(*this, closingArgs);
        }
    }

    if (closingArgs.Cancel) {
        return; // 使用者程式碼取消關閉操作
    }

    m_isDisposed = true;
    m_inModalLoop = false;

    if (m_renderContext) {
        m_renderContext->Destroy();
        m_renderContext.reset();
    }

    if (m_window) {
        m_window->Close();
    }

    FormClosedEventArgs closedArgs;
    closedArgs.CloseReasonIsUser = true;

    OnFormClosed(closedArgs);
    for (const auto& handler : m_closedHandlers) {
        if (handler) {
            handler(*this, closedArgs);
        }
    }

    Application::UnregisterForm(this);
}

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

DialogResult Form::ShowDialog(std::shared_ptr<Form> owner) {
    if (owner) {
        owner->SetEnabled(false);
#if defined(_WIN32)
        if (owner->GetPlatformWindow() && owner->GetPlatformWindow()->GetNativeHandle()) {
            ::EnableWindow(static_cast<HWND>(owner->GetPlatformWindow()->GetNativeHandle()), FALSE);
        }
#endif
    }

    Show();
    m_inModalLoop = true;

    while (m_inModalLoop && !m_isDisposed && m_window && !m_window->IsClosed()) {
        m_window->WaitEvents(16);
        if (m_dialogResult != DialogResult::None) {
            Close();
            break;
        }
    }

    m_inModalLoop = false;

    if (owner) {
        owner->SetEnabled(true);
#if defined(_WIN32)
        if (owner->GetPlatformWindow() && owner->GetPlatformWindow()->GetNativeHandle()) {
            HWND ownerHwnd = static_cast<HWND>(owner->GetPlatformWindow()->GetNativeHandle());
            ::EnableWindow(ownerHwnd, TRUE);
            ::SetForegroundWindow(ownerHwnd);
        }
#endif
    }

    return m_dialogResult;
}

void Form::OnLoad() {}

void Form::OnShown() {}

void Form::OnActivated() {}

void Form::OnDeactivate() {}

void Form::OnFormClosing(FormClosingEventArgs&) {}

void Form::OnFormClosed(FormClosedEventArgs&) {}

void Form::AddClosingHandler(FormClosingHandler handler) {
    if (handler) {
        m_closingHandlers.push_back(std::move(handler));
    }
}

void Form::AddClosedHandler(FormClosedHandler handler) {
    if (handler) {
        m_closedHandlers.push_back(std::move(handler));
    }
}

void Form::InvalidateForm() {
    if (m_isDisposed || (m_window && m_window->IsClosed())) return;
    InvalidateVisual();
    if (m_window) {
        m_window->RequestRedraw();
    }
}

/// <summary>
/// 將表單當前視覺內容與佈局渲染至離線點陣圖 (Bitmap)。
/// </summary>
std::shared_ptr<graphics::Bitmap> Form::CaptureToBitmap() {
    if (m_sizeToContent != SizeToContent::Manual) {
        FitContentSize();
    }

    uint32_t w = m_width > 0 ? m_width : 400;
    uint32_t h = m_height > 0 ? m_height : 300;

    core::Size avail(static_cast<float>(w), static_cast<float>(h));
    layout::LayoutManager::Get().UpdateLayout(*this, avail);
    if (m_overlayLayer) {
        m_overlayLayer->SetWidth(avail.width);
        m_overlayLayer->SetHeight(avail.height);
        layout::LayoutManager::Get().UpdateLayout(*m_overlayLayer, avail);
        BringVisualChildToFront(m_overlayLayer.get());
    }

    rendering::SoftwareRenderTarget target;
    target.Resize(w, h);
    target.Clear(GetBackground());

    rendering::DisplayList dl;
    rendering::DrawingContext dc(dl);
    Render(dc);

    rendering::DisplayListRenderer renderer;
    renderer.Render(dl, &target);

    return std::make_shared<graphics::Bitmap>(w, h, target.GetPixelSpan());
}

controls::OverlayLayer* Form::GetOverlayLayer() const noexcept {
    return m_overlayLayer.get();
}

void Form::EnsurePlatformWindow() {
    if (m_window) return;

    pal::WindowDesc desc;
    desc.title = m_title;
    desc.width = m_width;
    desc.height = m_height;
    desc.resizable = true;
    desc.visible = false;
    desc.centerScreen = (m_startPosition == FormStartPosition::CenterScreen);

    m_window = pal::PlatformWindowFactory::Create(desc);
    if (Application::GetPlatformBackend() == PlatformBackend::Skia) {
        auto skiaCtx = std::make_unique<rendering::SkiaRenderContext>();
        skiaCtx->Init(m_window->GetNativeHandle(), m_width, m_height);
        m_renderContext = std::move(skiaCtx);
    } else {
        if (!m_renderTarget) {
            m_renderTarget = std::make_unique<rendering::SoftwareRenderTarget>();
        }
        m_renderTarget->Resize(m_width, m_height);
    }
    SetupPlatformEvents();
}

void Form::SetupPlatformEvents() {
    if (!m_window) return;

    m_window->SetEventCallback([this](const pal::RawInputEvent& e) {
        if (e.type == pal::RawEventType::WindowClose) {
            Close();
            return;
        }

        if (e.type == pal::RawEventType::WindowResize) {
            HandleWindowResize(e.window.width, e.window.height);
            return;
        }

        input::InputManager::Get().ProcessRawEvent(*this, e);
        InvalidateForm();
    });

    m_window->SetPaintCallback([this]() {
        RenderFrame();
    });
}

void Form::RenderFrame() {
    if (m_width == 0 || m_height == 0 || !m_window || m_isDisposed || m_window->IsClosed()) return;

    core::Size avail(static_cast<float>(m_width), static_cast<float>(m_height));
    layout::LayoutManager::Get().UpdateLayout(*this, avail);
    if (m_overlayLayer) {
        m_overlayLayer->SetWidth(avail.width);
        m_overlayLayer->SetHeight(avail.height);
        layout::LayoutManager::Get().UpdateLayout(*m_overlayLayer, avail);
        BringVisualChildToFront(m_overlayLayer.get());
    }

    if (Application::GetPlatformBackend() == PlatformBackend::Skia) {
        if (!m_renderContext) {
            auto skiaCtx = std::make_unique<rendering::SkiaRenderContext>();
            skiaCtx->Init(m_window->GetNativeHandle(), m_width, m_height);
            m_renderContext = std::move(skiaCtx);
        }
        auto* target = m_renderContext->BeginFrame();
        if (target) {
            target->Clear(GetBackground());
            rendering::DisplayList dl;
            rendering::DrawingContext dc(dl);
            Render(dc);
            m_renderer.Render(dl, target);
        }
        m_renderContext->EndFrame();
        if (target && target->GetPixelBuffer()) {
            m_window->PresentFramebuffer(compat::span<const uint32_t>(target->GetPixelBuffer(), static_cast<size_t>(m_width) * m_height), m_width, m_height);
        }
    } else {
        if (!m_renderTarget) {
            auto swTarget = std::make_unique<rendering::SoftwareRenderTarget>();
            swTarget->Resize(m_width, m_height);
            m_renderTarget = std::move(swTarget);
        }

        m_renderTarget->Clear(GetBackground());
        rendering::DisplayList dl;
        rendering::DrawingContext dc(dl);
        Render(dc);
        m_renderer.Render(dl, m_renderTarget.get());

        m_window->PresentFramebuffer(m_renderTarget->GetPixelSpan(), m_width, m_height);
    }
}

void Form::HandleWindowResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    if (m_overlayLayer) {
        m_overlayLayer->SetWidth(static_cast<float>(width));
        m_overlayLayer->SetHeight(static_cast<float>(height));
    }
    if (m_renderContext) {
        m_renderContext->Resize(width, height);
    }
    if (m_renderTarget) {
        m_renderTarget->Resize(width, height);
    }
    InvalidateMeasure();
    InvalidateForm();
}

} // namespace app
} // namespace cppui
