#pragma once

#include "ui/rendering/backend/IRenderContext.hpp"
#include "SoftwareRenderContext.hpp"

namespace cppui::rendering {

/// <summary>
/// Skia 渲染後端封裝
/// 支援管理 SkSurface / SkCanvas，並在目前階段提供高相容性渲染底層
/// </summary>
class SkiaRenderContext : public IRenderContext {
public:
    SkiaRenderContext() = default;
    ~SkiaRenderContext() override = default;

    bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) override {
        return m_fallbackContext.Init(nativeWindowHandle, width, height);
    }

    void Resize(uint32_t width, uint32_t height) override {
        m_fallbackContext.Resize(width, height);
    }

    IRenderTarget* BeginFrame() override {
        return m_fallbackContext.BeginFrame();
    }

    void EndFrame() override {
        m_fallbackContext.EndFrame();
    }

    void Destroy() override {
        m_fallbackContext.Destroy();
    }

private:
    SoftwareRenderContext m_fallbackContext;
};

} // namespace cppui::rendering
