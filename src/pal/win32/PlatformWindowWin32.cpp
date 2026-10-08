#ifdef _WIN32

#include "PlatformWindowWin32.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include <windowsx.h>
#include <shellscalingapi.h>

namespace cppui {
namespace pal {
namespace win32 {

namespace {
    const wchar_t* WINDOW_CLASS_NAME = L"CPPUI_PlatformWindowClass";
    const UINT WM_APP_DISPATCH = WM_APP + 101;
    bool g_classRegistered = false;

    void EnableHighDpiAwareness() {
        static bool s_dpiConfigured = false;
        if (!s_dpiConfigured) {
            s_dpiConfigured = true;
            // 優先嘗試 Per-Monitor V2
            SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
    }

    std::wstring Utf8ToUtf16(compat::string_view utf8Str) {
        if (utf8Str.empty()) return L"";
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.length()), nullptr, 0);
        if (len <= 0) return L"";
        std::wstring result(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.length()), &result[0], len);
        return result;
    }
} // namespace

PlatformWindowWin32::PlatformWindowWin32(const WindowDesc& desc)
    : m_width(desc.width), m_height(desc.height) {
    EnableHighDpiAwareness();

    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    if (!g_classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = PlatformWindowWin32::WndProcThunk;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
        wc.hbrBackground = nullptr; // 避免 GDI 閃爍白屏，由後端完全覆蓋繪製
        wc.lpszClassName = WINDOW_CLASS_NAME;

        RegisterClassExW(&wc);
        g_classRegistered = true;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!desc.resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    if (!desc.decorated) {
        style = WS_POPUP;
    }

    // 計算符合客戶區寬高的視窗幾何邊界
    RECT rc = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    AdjustWindowRectEx(&rc, style, FALSE, 0);

    int windowWidth = rc.right - rc.left;
    int windowHeight = rc.bottom - rc.top;

    std::wstring wideTitle = Utf8ToUtf16(desc.title);

    m_hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        wideTitle.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (m_hwnd) {
        m_hdc = GetDC(m_hwnd);
        SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        UpdateDpiScale();

        if (desc.visible) {
            Show();
        }
    }
}

PlatformWindowWin32::~PlatformWindowWin32() {
    Close();
}

void PlatformWindowWin32::Show() {
    if (m_hwnd && !m_isClosed) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
    }
}

void PlatformWindowWin32::Hide() {
    if (m_hwnd && !m_isClosed) {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

void PlatformWindowWin32::Close() {
    if (m_hwnd) {
        m_isClosed = true;
        if (m_hdc) {
            ReleaseDC(m_hwnd, m_hdc);
            m_hdc = nullptr;
        }
        HWND hwnd = m_hwnd;
        m_hwnd = nullptr;
        DestroyWindow(hwnd);
    } else {
        m_isClosed = true;
    }
}

core::Size PlatformWindowWin32::GetSize() const noexcept {
    // 傳回以邏輯像素表示的尺寸 (考量 DPI 縮放)
    return m_coordTransform.ToLogical(core::Size(static_cast<float>(m_width),
                                                 static_cast<float>(m_height)));
}

core::Size PlatformWindowWin32::GetFramebufferSize() const noexcept {
    // 傳回物理像素尺寸
    return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
}

void PlatformWindowWin32::SetSize(uint32_t width, uint32_t height) {
    if (!m_hwnd || m_isClosed) return;
    m_width = width;
    m_height = height;

    RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    DWORD style = static_cast<DWORD>(GetWindowLongPtrW(m_hwnd, GWL_STYLE));
    AdjustWindowRectEx(&rc, style, FALSE, 0);

    SetWindowPos(m_hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void PlatformWindowWin32::SetTitle(compat::string_view title) {
    if (!m_hwnd || m_isClosed) return;
    std::wstring wideTitle = Utf8ToUtf16(title);
    SetWindowTextW(m_hwnd, wideTitle.c_str());
}

void PlatformWindowWin32::PollEvents() {
    if (!m_hwnd || m_isClosed) return;

    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

bool PlatformWindowWin32::WaitEvents(int timeoutMs) {
    if (!m_hwnd || m_isClosed) return false;

    DWORD timeout = (timeoutMs < 0) ? INFINITE : static_cast<DWORD>(timeoutMs);
    DWORD result = MsgWaitForMultipleObjectsEx(0, nullptr, timeout, QS_ALLINPUT, MWMO_ALERTABLE);

    if (result == WAIT_OBJECT_0) {
        PollEvents();
        return true;
    }
    return false;
}

void PlatformWindowWin32::WakeUp() {
    if (m_hwnd && !m_isClosed) {
        PostMessageW(m_hwnd, WM_APP_DISPATCH, 0, 0);
    }
}

void PlatformWindowWin32::RequestRedraw() {
    if (m_hwnd && !m_isClosed) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void PlatformWindowWin32::PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) {
    if (!m_hwnd || pixelBuffer.empty() || width == 0 || height == 0) return;
    HDC hdc = ::GetDC(m_hwnd);
    if (!hdc) return;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(width);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    ::StretchDIBits(
        hdc,
        0, 0, static_cast<int>(width), static_cast<int>(height),
        0, 0, static_cast<int>(width), static_cast<int>(height),
        pixelBuffer.data(),
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );
    ::ReleaseDC(m_hwnd, hdc);
}

void PlatformWindowWin32::UpdateDpiScale() {
    if (m_hwnd) {
        UINT dpi = GetDpiForWindow(m_hwnd);
        m_dpiScale = (dpi > 0) ? (static_cast<float>(dpi) / 96.0f) : 1.0f;
        m_coordTransform.SetDpiScale(m_dpiScale);
    }
}

KeyModifiers PlatformWindowWin32::QueryCurrentModifiers() const noexcept {
    KeyModifiers mods = KeyModifiers::None;
    if (GetKeyState(VK_CONTROL) & 0x8000) mods = mods | KeyModifiers::Control;
    if (GetKeyState(VK_SHIFT) & 0x8000)   mods = mods | KeyModifiers::Shift;
    if (GetKeyState(VK_MENU) & 0x8000)    mods = mods | KeyModifiers::Alt;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods = mods | KeyModifiers::Meta;
    return mods;
}

uint64_t PlatformWindowWin32::GetCurrentTimestampMs() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count()
    );
}

void PlatformWindowWin32::DispatchEvent(const RawInputEvent& event) {
    if (m_eventCallback) {
        m_eventCallback(event);
    }
}

LRESULT CALLBACK PlatformWindowWin32::WndProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* self = reinterpret_cast<PlatformWindowWin32*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self) {
        return self->HandleMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT PlatformWindowWin32::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    uint64_t ts = GetCurrentTimestampMs();
    KeyModifiers mods = QueryCurrentModifiers();

    switch (msg) {
        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (!m_isTrackingMouse) {
                TRACKMOUSEEVENT tme = {};
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = m_hwnd;
                TrackMouseEvent(&tme);
                m_isTrackingMouse = true;

                RawInputEvent enterEvt;
                enterEvt.type = RawEventType::MouseEnter;
                enterEvt.timestampMs = ts;
                DispatchEvent(enterEvt);
            }

            core::Point clientPos(static_cast<float>(x), static_cast<float>(y));
            core::Point physicalPos(static_cast<float>(x), static_cast<float>(y));
            core::Point logicalPos = m_coordTransform.ToLogical(physicalPos);
            DispatchEvent(RawInputEvent::MakeMouseMove(clientPos, physicalPos, logicalPos, mods, ts));
            return 0;
        }

        case WM_MOUSELEAVE: {
            m_isTrackingMouse = false;
            RawInputEvent leaveEvt;
            leaveEvt.type = RawEventType::MouseLeave;
            leaveEvt.timestampMs = ts;
            DispatchEvent(leaveEvt);
            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            SetCapture(m_hwnd);
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            core::Point clientPos(static_cast<float>(x), static_cast<float>(y));
            core::Point physicalPos(static_cast<float>(x), static_cast<float>(y));
            core::Point logicalPos = m_coordTransform.ToLogical(physicalPos);

            RawMouseButton btn = RawMouseButton::Left;
            if (msg == WM_RBUTTONDOWN) btn = RawMouseButton::Right;
            else if (msg == WM_MBUTTONDOWN) btn = RawMouseButton::Middle;

            DispatchEvent(RawInputEvent::MakeMouseDown(clientPos, physicalPos, logicalPos, btn, 1, mods, ts));
            return 0;
        }

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            ReleaseCapture();
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            core::Point clientPos(static_cast<float>(x), static_cast<float>(y));
            core::Point physicalPos(static_cast<float>(x), static_cast<float>(y));
            core::Point logicalPos = m_coordTransform.ToLogical(physicalPos);

            RawMouseButton btn = RawMouseButton::Left;
            if (msg == WM_RBUTTONUP) btn = RawMouseButton::Right;
            else if (msg == WM_MBUTTONUP) btn = RawMouseButton::Middle;

            DispatchEvent(RawInputEvent::MakeMouseUp(clientPos, physicalPos, logicalPos, btn, mods, ts));
            return 0;
        }

        case WM_MOUSEWHEEL: {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(m_hwnd, &pt);
            core::Point clientPos(static_cast<float>(pt.x), static_cast<float>(pt.y));
            core::Point physicalPos(static_cast<float>(pt.x), static_cast<float>(pt.y));
            core::Point logicalPos = m_coordTransform.ToLogical(physicalPos);
            int deltaY = GET_WHEEL_DELTA_WPARAM(wParam);

            DispatchEvent(RawInputEvent::MakeMouseWheel(clientPos, physicalPos, logicalPos, deltaY, 0, mods, ts));
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            RawInputEvent e;
            e.type = RawEventType::KeyDown;
            e.timestampMs = ts;
            e.modifiers = mods;
            e.key.virtualKey = static_cast<uint32_t>(wParam);
            e.key.scanCode = static_cast<uint32_t>((lParam >> 16) & 0xFF);
            e.key.isRepeat = (lParam & (1 << 30)) != 0;
            DispatchEvent(e);
            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            RawInputEvent e;
            e.type = RawEventType::KeyUp;
            e.timestampMs = ts;
            e.modifiers = mods;
            e.key.virtualKey = static_cast<uint32_t>(wParam);
            e.key.scanCode = static_cast<uint32_t>((lParam >> 16) & 0xFF);
            DispatchEvent(e);
            return 0;
        }

        case WM_CHAR: {
            RawInputEvent e;
            e.type = RawEventType::TextInput;
            e.timestampMs = ts;
            e.text.codepoint = static_cast<uint32_t>(wParam);
            DispatchEvent(e);
            return 0;
        }

        case WM_SIZE: {
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            DispatchEvent(RawInputEvent::MakeResize(m_width, m_height, m_dpiScale, ts));
            if (m_paintCallback && m_width > 0 && m_height > 0) {
                m_paintCallback();
            }
            return 0;
        }

        case WM_DPICHANGED: {
            UpdateDpiScale();
            auto* prc = reinterpret_cast<RECT*>(lParam);
            SetWindowPos(m_hwnd, nullptr, prc->left, prc->top,
                         prc->right - prc->left, prc->bottom - prc->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            DispatchEvent(RawInputEvent::MakeResize(m_width, m_height, m_dpiScale, ts));
            if (m_paintCallback && m_width > 0 && m_height > 0) {
                m_paintCallback();
            }
            return 0;
        }

        case WM_SETFOCUS: {
            RawInputEvent e;
            e.type = RawEventType::WindowFocusGained;
            e.timestampMs = ts;
            DispatchEvent(e);
            return 0;
        }

        case WM_KILLFOCUS: {
            RawInputEvent e;
            e.type = RawEventType::WindowFocusLost;
            e.timestampMs = ts;
            DispatchEvent(e);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            if (m_paintCallback && m_width > 0 && m_height > 0) {
                m_paintCallback();
            }
            EndPaint(m_hwnd, &ps);
            (void)hdc;
            return 0;
        }

        case WM_CLOSE: {
            if (m_eventCallback) {
                DispatchEvent(RawInputEvent::MakeClose(ts));
            } else {
                Close();
            }
            return 0;
        }

        case WM_DESTROY: {
            m_isClosed = true;
            return 0;
        }

        case WM_APP_DISPATCH: {
            return 0;
        }

        default:
            break;
    }

    return DefWindowProcW(m_hwnd, msg, wParam, lParam);
}

} // namespace win32
} // namespace pal
} // namespace cppui

#endif // _WIN32
