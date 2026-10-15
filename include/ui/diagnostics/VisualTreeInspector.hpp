/**
 * @file VisualTreeInspector.hpp
 * @brief 視覺樹診斷器與 F12 DevTools 控制項檢查工具宣告
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/input/InputManager.hpp"

namespace cppui {
namespace diagnostics {

/// <summary>
/// 視覺樹節點除錯結構資訊。
/// </summary>
struct VisualNodeInfo {
    core::UIElement* element = nullptr;
    std::string      typeName;
    core::Rect       bounds;
    uint32_t         depth = 0;
};

/// <summary>
/// 視覺樹診斷器與檢查工具 (VisualTreeInspector / DevTools)
/// 支援 F12 喚出、元素吸管拾取模式 (Element Picker)、幾何與邊界資訊顯示、及選定元素藍色高亮。
/// </summary>
class VisualTreeInspector : public input::IInputInspectorHook {
public:
    /// <summary>
    /// 取得 VisualTreeInspector 全域單例。
    /// </summary>
    /// <returns>單例引用。</returns>
    static VisualTreeInspector& Get() noexcept;

    /// <summary>
    /// 查詢檢查工具是否顯示中。
    /// </summary>
    /// <returns>若顯示傳回 true。</returns>
    COMPAT_NODISCARD bool IsVisible() const noexcept { return m_visible; }

    /// <summary>
    /// 設定檢查工具可見度。
    /// </summary>
    /// <param name="visible">true 顯示，false 隱藏。</param>
    void SetVisible(bool visible);

    /// <summary>
    /// 切換檢查工具之顯示與隱藏狀態。
    /// </summary>
    void Toggle();

    /// <summary>
    /// 查詢是否處於元素吸管拾取模式 (Element Picker Mode)。
    /// </summary>
    /// <returns>若啟用傳回 true。</returns>
    COMPAT_NODISCARD bool IsPickerModeEnabled() const noexcept { return m_pickerMode; }

    /// <summary>
    /// 啟用或關閉元素吸管拾取模式。
    /// </summary>
    /// <param name="enabled">true 啟用，false 關閉。</param>
    void EnablePickerMode(bool enabled);

    /// <summary>
    /// 設定當前滑鼠懸停元素 (吸管模式動態追蹤)。
    /// </summary>
    /// <param name="element">懸停之元素指標。</param>
    void SetHoveredElement(core::UIElement* element);

    /// <summary>
    /// 檢查並鎖定選定之元素節點。
    /// </summary>
    /// <param name="element">目標元素指標。</param>
    void InspectElement(core::UIElement* element);

    /// <summary>
    /// 取得目前選定檢查之元素指標。
    /// </summary>
    /// <returns>元素指標 (若無則為 nullptr)。</returns>
    COMPAT_NODISCARD core::UIElement* GetInspectedElement() const noexcept { return m_inspectedElement; }

    /// <summary>
    /// 遍歷走訪目標元素之視覺樹並生成階層資訊清單。
    /// </summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <returns>視覺樹節點清單。</returns>
    COMPAT_NODISCARD std::vector<VisualNodeInfo> BuildVisualTree(core::UIElement* root) const;

    /// <summary>
    /// 於繪圖上下文渲染檢查工具浮動覆蓋層 (含選中外框、懸停框、元素度量資訊 HUD)。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    /// <param name="windowWidth">視窗寬度。</param>
    /// <param name="windowHeight">視窗高度。</param>
    void RenderOverlay(rendering::DrawingContext& dc, float windowWidth, float windowHeight);

    // --- IInputInspectorHook 實作 ---
    COMPAT_NODISCARD bool IsPickerActive() const noexcept override { return m_pickerMode; }
    void OnMouseMove(core::UIElement* hitUI) override;
    void OnMouseDown(core::UIElement* hitUI) override;
    void OnF12Pressed(core::UIElement& root) override;

private:
    VisualTreeInspector();
    ~VisualTreeInspector() override;

    VisualTreeInspector(const VisualTreeInspector&) = delete;
    VisualTreeInspector& operator=(const VisualTreeInspector&) = delete;

    void TraverseVisualTree(core::UIElement* node, uint32_t depth, std::vector<VisualNodeInfo>& out) const;

    bool             m_visible = false;
    bool             m_pickerMode = false;
    core::UIElement* m_inspectedElement = nullptr;
    core::UIElement* m_hoveredElement = nullptr;
};

} // namespace diagnostics
} // namespace cppui
