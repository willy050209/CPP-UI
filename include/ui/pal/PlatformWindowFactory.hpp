#pragma once

#include <memory>
#include "ui/pal/IPlatformWindow.hpp"

namespace cppui::pal {

class PlatformWindowFactory {
public:
    /// <summary>
    /// 依據當前作業系統建立最適的原生視窗實例
    /// </summary>
    static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc);
};

} // namespace cppui::pal
