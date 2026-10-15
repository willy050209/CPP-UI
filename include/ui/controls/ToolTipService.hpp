/*
 * File: ToolTipService.hpp
 * Purpose: Declares ToolTipService for attaching tooltip behavior to UI elements.
 */

#pragma once

#include <memory>
#include "compat/Compat.hpp"
#include "ui/controls/ToolTip.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 懸停提示服務管理器 (ToolTipService)
/// 為任意 UIElement 提供附屬 ToolTip 設定與自動彈出隱藏。
/// </summary>
class ToolTipService {
public:
    /// <summary>
    /// 為指定元素設定純文字提示。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    /// <param name="text">提示字串視圖。</param>
    static void SetToolTip(core::UIElement* element, compat::string_view text);

    /// <summary>
    /// 為指定元素設定自訂 ToolTip 元件。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    /// <param name="toolTip">ToolTip 智慧指標。</param>
    static void SetToolTip(core::UIElement* element, std::shared_ptr<ToolTip> toolTip);

    /// <summary>
    /// 取得指定元素關聯之 ToolTip。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    /// <returns>ToolTip 智慧指標，若無則傳回 nullptr。</returns>
    static std::shared_ptr<ToolTip> GetToolTip(core::UIElement* element);

    /// <summary>
    /// 立即開啟指定元素關聯之提示浮層。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    static void ShowToolTip(core::UIElement* element);

    /// <summary>
    /// 立即隱藏指定元素關聯之提示浮層。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    static void HideToolTip(core::UIElement* element);
};

} // namespace controls
} // namespace cppui
