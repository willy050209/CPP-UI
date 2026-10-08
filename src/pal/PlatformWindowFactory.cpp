/*
 * File: PlatformWindowFactory.cpp
 * Purpose: Implements the PlatformWindowFactory with fail-fast dimension validation.
 */

#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/PlatformWindowGeneric.hpp"
#include <stdexcept>

#ifdef _WIN32
#include "win32/PlatformWindowWin32.hpp"
#endif

namespace cppui {
namespace pal {

std::shared_ptr<IPlatformWindow> PlatformWindowFactory::Create(const WindowDesc& desc) {
    if (desc.width == 0 || desc.height == 0) {
        throw std::invalid_argument("PlatformWindowFactory::Create: Window dimensions must be greater than zero.");
    }
#ifdef _WIN32
    return std::make_shared<win32::PlatformWindowWin32>(desc);
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

} // namespace pal
} // namespace cppui
