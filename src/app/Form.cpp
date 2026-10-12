/*
 * File: Form.cpp
 * Purpose: Implements Form desktop window lifecycle, rendering loop, and input handling.
 */

#include "ui/app/Form.hpp"
#include "ui/app/Application.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include "../rendering/backend/SoftwareRenderContext.hpp"
#include "../rendering/backend/SkiaRenderContext.hpp"
#include "ui/graphics/Bitmap.hpp"
#include <algorithm>

namespace cppui {
namespace app {

Form::Form() {
    SetPadding(core::Thickness(0.0f));
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetBackground(theme.Colors().windowBackground);
    m_hasExplicitBackground = false;

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
    if (m_window) {
        m_window->SetSize(width, height);
        HandleWindowResize(width, height);
    }
}

void Form::SetWindowState(WindowState state) {
    m_windowState = state;
}

void Form::Show() {
    if (m_isDisposed) return;
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
    uint32_t w = m_width > 0 ? m_width : 400;
    uint32_t h = m_height > 0 ? m_height : 300;

    core::Size avail(static_cast<float>(w), static_cast<float>(h));
    layout::LayoutManager::Get().UpdateLayout(*this, avail);

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

void Form::EnsurePlatformWindow() {
    if (m_window) return;

    pal::WindowDesc desc;
    desc.title = m_title;
    desc.width = m_width;
    desc.height = m_height;
    desc.resizable = true;
    desc.visible = false;

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
            m_renderTarget = std::make_unique<rendering::SoftwareRenderTarget>();
            m_renderTarget->Resize(m_width, m_height);
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
