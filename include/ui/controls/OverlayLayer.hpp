/*
 * File: OverlayLayer.hpp
 * Purpose: Declares the OverlayLayer panel that hosts floating/popup elements
 *          above normal window content without clipping.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/layout/Panel.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 視窗最頂層懸浮容器 (OverlayLayer)
/// 掛載於表單根節點最頂端，用於承載 Popup、ContextMenu、ToolTip 等不受父容器裁切之浮動控制項。
/// </summary>
class OverlayLayer : public layout::Panel {
public:
    /// <summary>
    /// 建構空白浮層容器實例。
    /// </summary>
    OverlayLayer();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~OverlayLayer() override = default;

    /// <summary>
    /// 新增浮動子元件至浮層中。
    /// </summary>
    /// <param name="element">浮動子元件智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 element 為 nullptr 時擲出。</exception>
    void AddFloatingElement(std::shared_ptr<core::UIElement> element);

    /// <summary>
    /// 從浮層中移除指定浮動子元件。
    /// </summary>
    /// <param name="element">待移除之浮動子元件智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 element 為 nullptr 時擲出。</exception>
    void RemoveFloatingElement(const std::shared_ptr<core::UIElement>& element);

    /// <summary>
    /// 取得當前承載之浮動子元件數量。
    /// </summary>
    /// <returns>浮動子元件數量。</returns>
    COMPAT_NODISCARD size_t GetFloatingElementCount() const noexcept;

    /// <summary>
    /// 命中測試覆寫：僅在命中內部實際可見之浮動子元件時回傳該元件，否則回傳 nullptr 實現點擊穿透。
    /// </summary>
    /// <param name="localPoint">本元件局部座標。</param>
    /// <returns>命中之視覺元件指標，若未命中任何浮動子元件傳回 nullptr。</returns>
    core::Visual* HitTest(core::Point localPoint) override;

    /// <summary>
    /// 沿著視覺樹祖先鏈搜尋所屬表單之 OverlayLayer。
    /// </summary>
    /// <param name="element">起點 UI 元件指標。</param>
    /// <returns>所屬之 OverlayLayer 指標，若未找到回傳 nullptr。</returns>
    static OverlayLayer* GetOverlayLayer(core::UIElement* element);

protected:
    /// <summary>
    /// 測量浮層及其內部浮動子元件。
    /// </summary>
    /// <param name="availableSize">可用尺寸約束。</param>
    /// <returns>期望尺寸大小。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 排版浮層內部浮動子元件。
    /// </summary>
    /// <param name="finalSize">最終配置尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;
};

} // namespace controls
} // namespace cppui
