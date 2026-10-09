/*
 * File: PlatformWindowFactory.cpp
 * Purpose: Implements the PlatformWindowFactory with fail-fast dimension validation.
 */

#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/PlatformWindowGeneric.hpp"
#include <stdexcept>
#include <cstdlib>

#ifdef _WIN32
#include "win32/PlatformWindowWin32.hpp"
#endif

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11
#include "x11/PlatformWindowX11.hpp"
#endif

namespace cppui {
namespace pal {

std::shared_ptr<IPlatformWindow> PlatformWindowFactory::Create(const WindowDesc& desc) {
    if (desc.width == 0 || desc.height == 0) {
        throw std::invalid_argument("PlatformWindowFactory::Create: Window dimensions must be greater than zero.");
    }
#ifdef _WIN32
    return std::make_shared<win32::PlatformWindowWin32>(desc);
#elif defined(CPPUI_HAS_X11) && CPPUI_HAS_X11
    const char* displayEnv = std::getenv("DISPLAY");
    if (displayEnv && displayEnv[0] != '\0') {
        try {
            return std::make_shared<x11::PlatformWindowX11>(desc);
        } catch (...) {
            // X11 連線失敗或無伺服器時優雅後退至純記憶體 Generic 視窗
        }
    }
    return std::make_shared<PlatformWindowGeneric>(desc);
#else
    return std::make_shared<PlatformWindowGeneric>(desc);
#endif
}

std::shared_ptr<IPlatformWindow> PlatformWindowFactory::CreateGeneric(const WindowDesc& desc) {
    if (desc.width == 0 || desc.height == 0) {
        throw std::invalid_argument("PlatformWindowFactory::CreateGeneric: Window dimensions must be greater than zero.");
    }
    return std::make_shared<PlatformWindowGeneric>(desc);
}

std::shared_ptr<IPlatformWindow> PlatformWindowFactory::CreateX11(const WindowDesc& desc) {
    if (desc.width == 0 || desc.height == 0) {
        throw std::invalid_argument("PlatformWindowFactory::CreateX11: Window dimensions must be greater than zero.");
    }
#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11
    return std::make_shared<x11::PlatformWindowX11>(desc);
#else
    (void)desc;
    throw std::runtime_error("PlatformWindowFactory::CreateX11: X11 backend is not compiled in this build.");
#endif
}

} // namespace pal
} // namespace cppui
