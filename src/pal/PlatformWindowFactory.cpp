#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/PlatformWindowGeneric.hpp"

#ifdef _WIN32
#include "win32/PlatformWindowWin32.hpp"
#endif

namespace cppui {
namespace pal {

std::shared_ptr<IPlatformWindow> PlatformWindowFactory::Create(const WindowDesc& desc) {
#ifdef _WIN32
    return std::make_shared<win32::PlatformWindowWin32>(desc);
#else
    return std::make_shared<PlatformWindowGeneric>(desc);
#endif
}

std::shared_ptr<IPlatformWindow> PlatformWindowFactory::CreateGeneric(const WindowDesc& desc) {
    return std::make_shared<PlatformWindowGeneric>(desc);
}

} // namespace pal
} // namespace cppui
