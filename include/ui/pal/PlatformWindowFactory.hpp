#pragma once

#include <memory>
#include "ui/pal/IPlatformWindow.hpp"

namespace cppui {
namespace pal {

class PlatformWindowFactory {
public:
    /// <summary>
    /// 依據當前作業系統建立最適的原生視窗實例 (Windows 上為 Win32，其他系統為 Generic)
    /// </summary>
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc = WindowDesc{});

    /// <summary>
    /// 明確建立純 C++ 跨平台通用視窗 (適用於 Headless CI、單元測試、無顯卡環境)
    /// </summary>
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateGeneric(const WindowDesc& desc = WindowDesc{});
};

} // namespace pal
} // namespace cppui
