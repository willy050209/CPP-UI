/*
 * File: ClipboardX11.cpp
 * Purpose: Implements Linux X11 native clipboard using X11 Selection protocol with memory fallback.
 */

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11

#include "src/pal/x11/ClipboardX11.hpp"
#include <chrono>
#include <thread>
#include <vector>

namespace cppui {
namespace pal {
namespace x11 {

ClipboardX11::ClipboardX11() = default;

ClipboardX11::~ClipboardX11() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_display) {
        if (m_window) {
            XDestroyWindow(m_display, m_window);
            m_window = 0;
        }
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
}

void ClipboardX11::EnsureInitialized() {
    if (m_isInitialized) return;
    m_isInitialized = true;

    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        // 無可用 X11 Display (如純無頭 CI 測試環境)，退回純記憶體快取
        return;
    }

    m_clipboardAtom = XInternAtom(m_display, "CLIPBOARD", False);
    m_utf8StringAtom = XInternAtom(m_display, "UTF8_STRING", False);
    m_targetsAtom = XInternAtom(m_display, "TARGETS", False);
    m_propertyAtom = XInternAtom(m_display, "CPPUI_CLIPBOARD_PROP", False);

    Window root = DefaultRootWindow(m_display);
    m_window = XCreateSimpleWindow(m_display, root, -10, -10, 1, 1, 0, 0, 0);
    XSelectInput(m_display, m_window, PropertyChangeMask);
    XFlush(m_display);
}

void ClipboardX11::ProcessPendingEvents() const {
    if (!m_display || !m_window) return;

    while (XPending(m_display) > 0) {
        XEvent event;
        XNextEvent(m_display, &event);

        if (event.type == SelectionRequest) {
            XSelectionRequestEvent* req = &event.xselectionrequest;
            XSelectionEvent response = {};
            response.type = SelectionNotify;
            response.display = req->display;
            response.requestor = req->requestor;
            response.selection = req->selection;
            response.target = req->target;
            response.time = req->time;
            response.property = 0; // None

            if (req->target == m_targetsAtom) {
                Atom supportedTargets[2] = { m_targetsAtom, m_utf8StringAtom };
                XChangeProperty(
                    m_display,
                    req->requestor,
                    req->property,
                    XA_ATOM,
                    32,
                    PropModeReplace,
                    reinterpret_cast<const unsigned char*>(supportedTargets),
                    2
                );
                response.property = req->property;
            } else if (req->target == m_utf8StringAtom || req->target == XA_STRING) {
                XChangeProperty(
                    m_display,
                    req->requestor,
                    req->property,
                    req->target,
                    8,
                    PropModeReplace,
                    reinterpret_cast<const unsigned char*>(m_localText.data()),
                    static_cast<int>(m_localText.size())
                );
                response.property = req->property;
            }

            XSendEvent(m_display, req->requestor, False, 0, reinterpret_cast<XEvent*>(&response));
            XFlush(m_display);
        } else if (event.type == SelectionClear) {
            m_ownsClipboard = false;
        }
    }
}

bool ClipboardX11::ContainsText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const_cast<ClipboardX11*>(this)->EnsureInitialized();

    if (!m_display || !m_window) {
        return !m_localText.empty();
    }

    ProcessPendingEvents();

    Window owner = XGetSelectionOwner(m_display, m_clipboardAtom);
    if (owner == 0) {
        return false;
    }
    if (owner == m_window) {
        return !m_localText.empty();
    }
    return true;
}

std::string ClipboardX11::GetText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const_cast<ClipboardX11*>(this)->EnsureInitialized();

    if (!m_display || !m_window) {
        return m_localText;
    }

    ProcessPendingEvents();

    Window owner = XGetSelectionOwner(m_display, m_clipboardAtom);
    if (owner == 0) {
        return std::string();
    }
    if (owner == m_window && m_ownsClipboard) {
        return m_localText;
    }

    // 向選取項擁有者請求轉換為 UTF8_STRING
    XConvertSelection(m_display, m_clipboardAtom, m_utf8StringAtom, m_propertyAtom, m_window, CurrentTime);
    XFlush(m_display);

    const auto startTime = std::chrono::steady_clock::now();
    constexpr auto timeout = std::chrono::milliseconds(200);

    while (std::chrono::steady_clock::now() - startTime < timeout) {
        XEvent event;
        if (XCheckTypedWindowEvent(m_display, m_window, SelectionNotify, &event)) {
            if (event.xselection.property == 0) {
                break;
            }

            Atom actualType = 0;
            int actualFormat = 0;
            unsigned long nitems = 0;
            unsigned long bytesAfter = 0;
            unsigned char* propData = nullptr;

            XGetWindowProperty(
                m_display,
                m_window,
                m_propertyAtom,
                0,
                1024 * 1024,
                False,
                AnyPropertyType,
                &actualType,
                &actualFormat,
                &nitems,
                &bytesAfter,
                &propData
            );

            if (propData) {
                std::string result(reinterpret_cast<char*>(propData), nitems);
                XFree(propData);
                XDeleteProperty(m_display, m_window, m_propertyAtom);
                return result;
            }
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return m_localText;
}

void ClipboardX11::SetText(compat::string_view text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    EnsureInitialized();

    m_localText = std::string(text.data(), text.size());

    if (!m_display || !m_window) {
        return;
    }

    XSetSelectionOwner(m_display, m_clipboardAtom, m_window, CurrentTime);
    XFlush(m_display);
    m_ownsClipboard = true;
}

void ClipboardX11::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    EnsureInitialized();

    m_localText.clear();

    if (m_display && m_window && m_ownsClipboard) {
        XSetSelectionOwner(m_display, m_clipboardAtom, 0, CurrentTime);
        XFlush(m_display);
        m_ownsClipboard = false;
    }
}

} // namespace x11
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_X11
