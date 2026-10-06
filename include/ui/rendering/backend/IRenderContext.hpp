#pragma once

#include <cstdint>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 像素像素格式列舉
/// </summary>
enum class PixelFormat : uint8_t {
    RGBA8888,
    BGRA8888
};

/// <summary>
/// 渲染後端繪圖畫布介面
/// 提供基本光柵圖元繪製，供 M1 驗證與 M2 DisplayList 重播器對接
/// </summary>
class IRenderTarget {
public:
    virtual ~IRenderTarget() = default;

    virtual uint32_t GetWidth() const noexcept = 0;
    virtual uint32_t GetHeight() const noexcept = 0;

    // 清屏與基礎填色
    virtual void Clear(core::Color color) = 0;
    virtual void FillRect(const core::Rect& rect, core::Color color) = 0;

    // 取得底層像素記憶體指標 (若為軟體緩衝區)
    virtual uint32_t* GetPixelBuffer() noexcept = 0;
};

/// <summary>
/// 跨平台渲染後端 Context 介面 (IRenderContext)
/// </summary>
class IRenderContext {
public:
    virtual ~IRenderContext() = default;

    /// <summary>
    /// 初始化渲染環境並綁定視窗控制代碼
    /// </summary>
    virtual bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) = 0;

    /// <summary>
    /// 視窗尺寸改變時重設 Framebuffer
    /// </summary>
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    /// <summary>
    /// 開始當前幀繪製，取得 RenderTarget
    /// </summary>
    virtual IRenderTarget* BeginFrame() = 0;

    /// <summary>
    /// 結束當前幀並將畫面提交至螢幕 (SwapBuffers / Present)
    /// </summary>
    virtual void EndFrame() = 0;

    /// <summary>
    /// 釋放所有繪圖資源
    /// </summary>
    virtual void Destroy() = 0;
};

} // namespace rendering
} // namespace cppui
