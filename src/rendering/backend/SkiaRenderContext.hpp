#pragma once

#include "ui/rendering/backend/IRenderContext.hpp"
#include "SoftwareRenderContext.hpp"
#include <memory>

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
#include "SkiaRenderTarget.hpp"
#endif

namespace cppui {
namespace rendering {

/// <summary>
/// Google Skia 向量繪圖核心渲染後端 (SkiaRenderContext)
/// 提供高效能的 CPU/GPU 向量畫布渲染與幀生命週期管理，
/// 當建置系統未啟用 Skia 或處於相容環境時，自動優雅回退至 SoftwareRenderContext。
/// </summary>
class SkiaRenderContext : public IRenderContext {
public:
    SkiaRenderContext();
    ~SkiaRenderContext() override;

    bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) override;
    void Resize(uint32_t width, uint32_t height) override;
    IRenderTarget* BeginFrame() override;
    void EndFrame() override;
    void Destroy() override;

    /// <summary>
    /// 檢查當前是否正在運行原生 Skia 畫布核心
    /// </summary>
    COMPAT_NODISCARD bool IsSkiaActive() const noexcept { return m_useSkia; }

private:
    void* m_nativeWindowHandle = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_useSkia = false;

    SoftwareRenderContext m_fallbackContext;

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    sk_sp<SkSurface> m_surface;
    std::unique_ptr<SkiaRenderTarget> m_skiaTarget;
#endif
};

} // namespace rendering
} // namespace cppui
