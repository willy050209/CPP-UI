#include "ui/app/Form.hpp"
#include "ui/app/Application.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "../rendering/backend/SoftwareRenderContext.hpp"
#include <algorithm>

namespace cppui {
namespace app {

Form::Form() {
    SetPadding(core::Thickness(0.0f));
    SetBackground(core::Color(24, 26, 32, 255));
}

Form::~Form() {
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

    if (m_window) {
        m_window->Close();
    }

    m_isDisposed = true;
    m_inModalLoop = false;

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

DialogResult Form::ShowDialog(std::shared_ptr<Form> owner) {
    (void)owner;
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
    InvalidateVisual();
    if (m_window) {
        m_window->RequestRedraw();
    }
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
    if (!m_renderTarget) {
        m_renderTarget = std::unique_ptr<rendering::SoftwareRenderTarget>(new rendering::SoftwareRenderTarget());
    }
    m_renderTarget->Resize(m_width, m_height);
    SetupPlatformEvents();
}

void Form::SetupPlatformEvents() {
    if (!m_window) return;

    m_window->SetEventCallback([this](const pal::RawInputEvent& e) {
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
    if (m_width == 0 || m_height == 0 || !m_window) return;
    if (!m_renderTarget) {
        m_renderTarget = std::unique_ptr<rendering::SoftwareRenderTarget>(new rendering::SoftwareRenderTarget());
        m_renderTarget->Resize(m_width, m_height);
    }

    core::Size avail(static_cast<float>(m_width), static_cast<float>(m_height));
    layout::LayoutManager::Get().UpdateLayout(*this, avail);

    m_renderTarget->Clear(GetBackground());
    rendering::DisplayList dl;
    rendering::DrawingContext dc(dl);
    Render(dc);
    m_renderer.Render(dl, m_renderTarget.get());

    m_window->PresentFramebuffer(m_renderTarget->GetPixelSpan(), m_width, m_height);
}

void Form::HandleWindowResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    if (m_renderTarget) {
        m_renderTarget->Resize(width, height);
    }
    InvalidateMeasure();
    InvalidateForm();
}

} // namespace app
} // namespace cppui
