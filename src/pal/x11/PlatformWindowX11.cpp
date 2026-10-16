/*
 * File: PlatformWindowX11.cpp
 * Purpose: Implements the Linux X11 native platform window with event loop and framebuffer blitting.
 */

#include "PlatformWindowX11.hpp"
#include "src/pal/x11/ClipboardX11.hpp"

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11

#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

namespace cppui {
namespace pal {
namespace x11 {

PlatformWindowX11::PlatformWindowX11(const WindowDesc& desc)
    : m_isClosed(false)
    , m_isVisible(false)
    , m_pendingRedraw(false)
    , m_dpiScale(1.0f)
    , m_width(desc.width)
    , m_height(desc.height)
    , m_title(desc.title.data(), desc.title.size()) {
    if (desc.width == 0 || desc.height == 0) {
        throw std::invalid_argument("PlatformWindowX11: Window dimensions must be greater than zero.");
    }

    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        throw std::runtime_error("PlatformWindowX11: Failed to open X11 Display. Please verify DISPLAY environment variable.");
    }

    m_screen = DefaultScreen(m_display);
    Window root = RootWindow(m_display, m_screen);
    m_visual = DefaultVisual(m_display, m_screen);
    m_depth = DefaultDepth(m_display, m_screen);

    XSetWindowAttributes swa = {};
    swa.background_pixel = BlackPixel(m_display, m_screen);
    swa.event_mask = ExposureMask | StructureNotifyMask
                   | KeyPressMask | KeyReleaseMask
                   | ButtonPressMask | ButtonReleaseMask
                   | PointerMotionMask | EnterWindowMask | LeaveWindowMask
                   | FocusChangeMask;

    m_window = XCreateWindow(
        m_display, root,
        0, 0, m_width, m_height,
        0, m_depth, InputOutput, m_visual,
        CWBackPixel | CWEventMask, &swa
    );

    if (!m_window) {
        XCloseDisplay(m_display);
        m_display = nullptr;
        throw std::runtime_error("PlatformWindowX11: Failed to create X11 Window.");
    }

    m_gc = XCreateGC(m_display, m_window, 0, nullptr);

    m_wmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, m_window, &m_wmDeleteWindow, 1);

    m_netWmName = XInternAtom(m_display, "_NET_WM_NAME", False);
    m_utf8String = XInternAtom(m_display, "UTF8_STRING", False);

    // 初始化跨執行緒喚醒 Self-Pipe
    if (pipe(m_wakePipe) == 0) {
        fcntl(m_wakePipe[0], F_SETFL, O_NONBLOCK);
        fcntl(m_wakePipe[1], F_SETFL, O_NONBLOCK);
    }

    QueryDpiScale();
    SetTitle(desc.title);

    // 共享 X11 Display 與 Window 連線至 ClipboardX11
    if (auto* clipboard = dynamic_cast<ClipboardX11*>(GetClipboard())) {
        clipboard->SetConnection(m_display, m_window);
    }

    if (desc.visible) {
        Show();
    }
}

PlatformWindowX11::~PlatformWindowX11() {
    if (auto* clipboard = dynamic_cast<ClipboardX11*>(GetClipboard())) {
        clipboard->ResetConnection(m_display);
    }

    if (m_wakePipe[0] != -1) {
        close(m_wakePipe[0]);
        m_wakePipe[0] = -1;
    }
    if (m_wakePipe[1] != -1) {
        close(m_wakePipe[1]);
        m_wakePipe[1] = -1;
    }

    if (m_display) {
        if (m_gc) {
            XFreeGC(m_display, m_gc);
            m_gc = nullptr;
        }
        if (m_window) {
            XDestroyWindow(m_display, m_window);
            m_window = 0;
        }
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
}

IClipboard* PlatformWindowX11::GetClipboard() const noexcept {
    return Clipboard::GetCurrent();
}

void PlatformWindowX11::Show() {
    if (!m_display || m_isClosed) return;
    m_isVisible = true;
    XMapRaised(m_display, m_window);
    XFlush(m_display);
    RequestRedraw();
}

void PlatformWindowX11::Hide() {
    if (!m_display || m_isClosed) return;
    m_isVisible = false;
    XUnmapWindow(m_display, m_window);
    XFlush(m_display);
}

void PlatformWindowX11::Close() {
    if (m_isClosed) return;
    m_isClosed = true;
    m_isVisible = false;
    if (m_display && m_window) {
        XUnmapWindow(m_display, m_window);
        XFlush(m_display);
    }
    DispatchEvent(RawInputEvent::MakeClose(GetCurrentTimestampMs()));
}

core::Size PlatformWindowX11::GetSize() const noexcept {
    return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
}

void PlatformWindowX11::SetSize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;
    m_width = width;
    m_height = height;
    if (m_display && m_window) {
        XResizeWindow(m_display, m_window, width, height);
        XFlush(m_display);
    }
    DispatchEvent(RawInputEvent::MakeResize(m_width, m_height, m_dpiScale, GetCurrentTimestampMs()));
    RequestRedraw();
}

core::Size PlatformWindowX11::GetFramebufferSize() const noexcept {
    return core::Size(static_cast<float>(m_width) * m_dpiScale, static_cast<float>(m_height) * m_dpiScale);
}

void PlatformWindowX11::SetTitle(compat::string_view title) {
    m_title.assign(title.data(), title.size());
    if (!m_display || !m_window) return;

    XStoreName(m_display, m_window, m_title.c_str());
    if (m_netWmName && m_utf8String) {
        XChangeProperty(
            m_display, m_window, m_netWmName, m_utf8String, 8,
            PropModeReplace, reinterpret_cast<const unsigned char*>(m_title.c_str()),
            static_cast<int32_t>(m_title.size())
        );
    }
    XFlush(m_display);
}

void PlatformWindowX11::PollEvents() {
    if (!m_display || m_isClosed) return;

    // 清空自喚醒 pipe 緩衝區
    if (m_wakePipe[0] != -1) {
        char buf[64];
        while (read(m_wakePipe[0], buf, sizeof(buf)) > 0) {}
    }

    while (XPending(m_display) > 0) {
        XEvent ev;
        XNextEvent(m_display, &ev);
        HandleXEvent(ev);
        if (m_isClosed) break;
    }

    if (m_pendingRedraw && m_paintCallback && !m_isClosed) {
        m_pendingRedraw = false;
        m_paintCallback();
    }
}

bool PlatformWindowX11::WaitEvents(int32_t timeoutMs) {
    if (!m_display || m_isClosed) return false;

    if (XPending(m_display) > 0 || m_pendingRedraw) {
        PollEvents();
        return true;
    }

    struct pollfd fds[2];
    fds[0].fd = ConnectionNumber(m_display);
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    int32_t nfds = 1;
    if (m_wakePipe[0] != -1) {
        fds[1].fd = m_wakePipe[0];
        fds[1].events = POLLIN;
        fds[1].revents = 0;
        nfds = 2;
    }

    int32_t ret = poll(fds, nfds, timeoutMs);
    if (ret > 0) {
        PollEvents();
        return true;
    }
    return false;
}

void PlatformWindowX11::WakeUp() {
    if (m_wakePipe[1] != -1) {
        char sig = 1;
        ssize_t written = write(m_wakePipe[1], &sig, 1);
        (void)written;
    }
}

void PlatformWindowX11::RequestRedraw() {
    m_pendingRedraw = true;
    WakeUp();
}

void PlatformWindowX11::PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) {
    if (!m_display || !m_window || !m_gc || pixelBuffer.empty() || width == 0 || height == 0) return;

    XImage* img = XCreateImage(
        m_display,
        m_visual,
        static_cast<uint32_t>(m_depth),
        ZPixmap,
        0,
        const_cast<char*>(reinterpret_cast<const char*>(pixelBuffer.data())),
        width,
        height,
        32,
        0
    );

    if (img) {
        XPutImage(m_display, m_window, m_gc, img, 0, 0, 0, 0, width, height);
        img->data = nullptr; // 避免 XDestroyImage 釋放非其分配之外部緩衝區
        XDestroyImage(img);
        XFlush(m_display);
    }

    m_pendingRedraw = false;
}

void PlatformWindowX11::HandleXEvent(const XEvent& event) {
    uint64_t ts = GetCurrentTimestampMs();

    switch (event.type) {
        case ConfigureNotify: {
            if (event.xconfigure.width > 0 && event.xconfigure.height > 0) {
                if (event.xconfigure.width != static_cast<int32_t>(m_width) ||
                    event.xconfigure.height != static_cast<int32_t>(m_height)) {
                    m_width = static_cast<uint32_t>(event.xconfigure.width);
                    m_height = static_cast<uint32_t>(event.xconfigure.height);
                    DispatchEvent(RawInputEvent::MakeResize(m_width, m_height, m_dpiScale, ts));
                    RequestRedraw();
                }
            }
            break;
        }

        case Expose: {
            if (event.xexpose.count == 0) {
                RequestRedraw();
            }
            break;
        }

        case MotionNotify: {
            core::Point physPt(static_cast<float>(event.xmotion.x), static_cast<float>(event.xmotion.y));
            core::Point logPt = m_coordTransform.ToLogical(physPt);
            KeyModifiers mods = ConvertModifiers(event.xmotion.state);
            DispatchEvent(RawInputEvent::MakeMouseMove(physPt, physPt, logPt, mods, ts));
            break;
        }

        case ButtonPress: {
            core::Point physPt(static_cast<float>(event.xbutton.x), static_cast<float>(event.xbutton.y));
            core::Point logPt = m_coordTransform.ToLogical(physPt);
            KeyModifiers mods = ConvertModifiers(event.xbutton.state);

            if (event.xbutton.button == 1) {
                DispatchEvent(RawInputEvent::MakeMouseDown(physPt, physPt, logPt, RawMouseButton::Left, 1, mods, ts));
            } else if (event.xbutton.button == 2) {
                DispatchEvent(RawInputEvent::MakeMouseDown(physPt, physPt, logPt, RawMouseButton::Middle, 1, mods, ts));
            } else if (event.xbutton.button == 3) {
                DispatchEvent(RawInputEvent::MakeMouseDown(physPt, physPt, logPt, RawMouseButton::Right, 1, mods, ts));
            } else if (event.xbutton.button == 4) {
                DispatchEvent(RawInputEvent::MakeMouseWheel(physPt, physPt, logPt, 120, 0, mods, ts));
            } else if (event.xbutton.button == 5) {
                DispatchEvent(RawInputEvent::MakeMouseWheel(physPt, physPt, logPt, -120, 0, mods, ts));
            }
            break;
        }

        case ButtonRelease: {
            core::Point physPt(static_cast<float>(event.xbutton.x), static_cast<float>(event.xbutton.y));
            core::Point logPt = m_coordTransform.ToLogical(physPt);
            KeyModifiers mods = ConvertModifiers(event.xbutton.state);

            if (event.xbutton.button == 1) {
                DispatchEvent(RawInputEvent::MakeMouseUp(physPt, physPt, logPt, RawMouseButton::Left, mods, ts));
            } else if (event.xbutton.button == 2) {
                DispatchEvent(RawInputEvent::MakeMouseUp(physPt, physPt, logPt, RawMouseButton::Middle, mods, ts));
            } else if (event.xbutton.button == 3) {
                DispatchEvent(RawInputEvent::MakeMouseUp(physPt, physPt, logPt, RawMouseButton::Right, mods, ts));
            }
            break;
        }

        case KeyPress: {
            char textBuf[32] = {};
            KeySym keysym = 0;
            XComposeStatus compose = {};
            XLookupString(const_cast<XKeyEvent*>(&event.xkey), textBuf, sizeof(textBuf) - 1, &keysym, &compose);
            uint32_t vk = static_cast<uint32_t>(keysym);
            KeyModifiers mods = ConvertModifiers(event.xkey.state);

            RawInputEvent downEvt;
            downEvt.type = RawEventType::KeyDown;
            downEvt.timestampMs = ts;
            downEvt.modifiers = mods;
            downEvt.key.virtualKey = vk;
            downEvt.key.scanCode = static_cast<uint32_t>(event.xkey.keycode);
            downEvt.key.isRepeat = false;
            DispatchEvent(downEvt);

            if (textBuf[0] != '\0') {
                RawInputEvent textEvt;
                textEvt.type = RawEventType::TextInput;
                textEvt.timestampMs = ts;
                textEvt.text.codepoint = static_cast<uint8_t>(textBuf[0]);
                DispatchEvent(textEvt);
            }
            break;
        }

        case KeyRelease: {
            KeySym keysym = XLookupKeysym(const_cast<XKeyEvent*>(&event.xkey), 0);
            uint32_t vk = static_cast<uint32_t>(keysym);
            KeyModifiers mods = ConvertModifiers(event.xkey.state);

            RawInputEvent upEvt;
            upEvt.type = RawEventType::KeyUp;
            upEvt.timestampMs = ts;
            upEvt.modifiers = mods;
            upEvt.key.virtualKey = vk;
            upEvt.key.scanCode = static_cast<uint32_t>(event.xkey.keycode);
            DispatchEvent(upEvt);
            break;
        }

        case ClientMessage: {
            if (static_cast<Atom>(event.xclient.data.l[0]) == m_wmDeleteWindow) {
                Close();
            }
            break;
        }

        case FocusIn: {
            RawInputEvent focusEvt;
            focusEvt.type = RawEventType::WindowFocusGained;
            focusEvt.timestampMs = ts;
            DispatchEvent(focusEvt);
            break;
        }

        case FocusOut: {
            RawInputEvent focusEvt;
            focusEvt.type = RawEventType::WindowFocusLost;
            focusEvt.timestampMs = ts;
            DispatchEvent(focusEvt);
            break;
        }

        case SelectionRequest: {
            if (auto* clipboard = dynamic_cast<ClipboardX11*>(GetClipboard())) {
                clipboard->HandleSelectionRequest(event.xselectionrequest);
            }
            break;
        }

        case SelectionClear: {
            if (auto* clipboard = dynamic_cast<ClipboardX11*>(GetClipboard())) {
                clipboard->HandleSelectionClear(event.xselectionclear);
            }
            break;
        }

        case SelectionNotify: {
            if (auto* clipboard = dynamic_cast<ClipboardX11*>(GetClipboard())) {
                clipboard->HandleSelectionNotify(event.xselection);
            }
            break;
        }

        default:
            break;
    }
}

void PlatformWindowX11::DispatchEvent(const RawInputEvent& event) {
    if (m_eventCallback) {
        m_eventCallback(event);
    }
}

void PlatformWindowX11::QueryDpiScale() {
    if (!m_display) {
        m_dpiScale = 1.0f;
        m_coordTransform.SetDpiScale(1.0f);
        return;
    }

    float dpi = 96.0f;
    char* resourceString = XResourceManagerString(m_display);
    if (resourceString) {
        XrmDatabase db = XrmGetStringDatabase(resourceString);
        if (db) {
            XrmValue value;
            char* type = nullptr;
            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && value.addr) {
                try {
                    dpi = std::stof(value.addr);
                } catch (...) {
                    dpi = 96.0f;
                }
            }
            XrmDestroyDatabase(db);
        }
    }

    m_dpiScale = std::max(1.0f, dpi / 96.0f);
    m_coordTransform.SetDpiScale(m_dpiScale);
}

KeyModifiers PlatformWindowX11::ConvertModifiers(uint32_t state) const noexcept {
    KeyModifiers mods = KeyModifiers::None;
    if (state & ControlMask) mods = mods | KeyModifiers::Control;
    if (state & ShiftMask)   mods = mods | KeyModifiers::Shift;
    if (state & Mod1Mask)    mods = mods | KeyModifiers::Alt;
    if (state & Mod4Mask)    mods = mods | KeyModifiers::Meta;
    if (state & LockMask)    mods = mods | KeyModifiers::CapsLock;
    return mods;
}

uint64_t PlatformWindowX11::GetCurrentTimestampMs() const noexcept {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
    );
}

} // namespace x11
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_X11
