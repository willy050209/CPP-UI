/**
 * @file IRenderContext.hpp
 * @brief 抽象渲染後端目標與上下文介面 (IRenderTarget, IRenderContext)
 */

#pragma once

#include <cstdint>
#include <memory>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 像素格式列舉
/// </summary>
enum class PixelFormat : uint8_t {
    RGBA8888,
    BGRA8888
};

#ifdef DrawText
#undef DrawText
#endif

/// <summary>
/// 渲染後端繪圖畫布介面 (IRenderTarget)
/// 提供基本光柵圖元繪製，供 M1 驗證與 M2 DisplayList 重播器對接。
/// </summary>
class IRenderTarget {
public:
    /// <summary>虛擬解構子。</summary>
    virtual ~IRenderTarget() = default;

    /// <summary>取得畫布寬度 (像素)。</summary>
    /// <returns>寬度像素值。</returns>
    virtual uint32_t GetWidth() const noexcept = 0;

    /// <summary>取得畫布高度 (像素)。</summary>
    /// <returns>高度像素值。</returns>
    virtual uint32_t GetHeight() const noexcept = 0;

    /// <summary>清屏為指定色彩。</summary>
    /// <param name="color">背景色彩。</param>
    virtual void Clear(core::Color color) = 0;

    /// <summary>填滿指定矩形區域。</summary>
    /// <param name="rect">矩形邊界。</param>
    /// <param name="color">填色色彩。</param>
    virtual void FillRect(const core::Rect& rect, core::Color color) = 0;

    /// <summary>取得底層像素記憶體指標 (若為軟體緩衝區)。</summary>
    /// <returns>32-bit 像素陣列指標。</returns>
    virtual uint32_t* GetPixelBuffer() noexcept = 0;

    /// <summary>繪製單行純文字。</summary>
    /// <param name="text">文字字串視圖。</param>
    /// <param name="origin">繪製起點。</param>
    /// <param name="family">字型家族名稱。</param>
    /// <param name="size">字型大小。</param>
    /// <param name="weight">字型粗細。</param>
    /// <param name="style">字型樣式。</param>
    /// <param name="color">文字色彩。</param>
    /// <param name="clipRect">可選之局部剪裁矩形。</param>
    virtual void DrawText(compat::string_view text, core::Point origin, compat::string_view family, float size, uint16_t weight, uint8_t style, core::Color color, const core::Rect& clipRect = core::Rect()) {
        (void)text; (void)origin; (void)family; (void)size; (void)weight; (void)style; (void)color; (void)clipRect;
    }

    /// <summary>繪製排版後的文字佈局。</summary>
    /// <param name="nativeLayoutHandle">原生排版控制代碼。</param>
    /// <param name="origin">繪製起點。</param>
    /// <param name="color">文字色彩。</param>
    /// <param name="clipRect">可選之局部剪裁矩形。</param>
    virtual void DrawTextLayout(void* nativeLayoutHandle, core::Point origin, core::Color color, const core::Rect& clipRect = core::Rect()) {
        (void)nativeLayoutHandle; (void)origin; (void)color; (void)clipRect;
    }

    /// <summary>取得底層 SkCanvas 畫布指標 (若當前 RenderTarget 為 Skia 畫布)。</summary>
    /// <returns>SkCanvas 原生指標，若非 Skia 後端則回傳 nullptr。</returns>
    virtual void* GetSkCanvas() noexcept { return nullptr; }
};

/// <summary>
/// 跨平台渲染後端 Context 介面 (IRenderContext)
/// </summary>
class IRenderContext {
public:
    /// <summary>虛擬解構子。</summary>
    virtual ~IRenderContext() = default;

    /// <summary>
    /// 初始化渲染環境並綁定視窗控制代碼。
    /// </summary>
    /// <param name="nativeWindowHandle">原生視窗控制代碼 (如 HWND)。</param>
    /// <param name="width">初始寬度。</param>
    /// <param name="height">初始高度。</param>
    /// <returns>若初始化成功傳回 true。</returns>
    virtual bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) = 0;

    /// <summary>
    /// 視窗尺寸改變時重設 Framebuffer。
    /// </summary>
    /// <param name="width">新寬度。</param>
    /// <param name="height">新高度。</param>
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    /// <summary>
    /// 開始當前幀繪製，取得 RenderTarget。
    /// </summary>
    /// <returns>當前影格之 IRenderTarget 指標。</returns>
    virtual IRenderTarget* BeginFrame() = 0;

    /// <summary>
    /// 結束當前幀並將畫面提交至螢幕 (SwapBuffers / Present)。
    /// </summary>
    virtual void EndFrame() = 0;

    /// <summary>
    /// 釋放所有繪圖資源。
    /// </summary>
    virtual void Destroy() = 0;
};

} // namespace rendering
} // namespace cppui
