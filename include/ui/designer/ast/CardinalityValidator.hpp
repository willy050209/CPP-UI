/**
 * @file CardinalityValidator.hpp
 * @brief 容器節點子元素數量約束與階層循環引用靜態檢驗器
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {

class WidgetNode;

/// <summary>
/// 容器基數與樹狀結構驗證閘門 (CardinalityValidator)
/// 實作純函數檢驗邏輯，確保 AST 在編輯期無法形成非法 UI 結構或循環引用。
/// </summary>
class CPPUI_API CardinalityValidator {
public:
    /// <summary>
    /// 檢查目標容器是否允許掛載指定型別之子節點。
    /// </summary>
    /// <param name="parent">父容器節點。</param>
    /// <param name="childType">欲掛載之子節點型別。</param>
    /// <returns>若允許傳回 void，否則傳回具體錯誤碼。</returns>
    COMPAT_NODISCARD static compat::expected<void, DesignerError> CanAddChild(
        const WidgetNode& parent,
        NodeType childType
    ) noexcept;

    /// <summary>
    /// 檢查節點搬移 (Reparent) 是否會觸發祖先循環引用 (Cycle Detection)。
    /// </summary>
    /// <param name="targetParent">目標新父容器。</param>
    /// <param name="nodeToMove">欲搬移之節點。</param>
    /// <returns>若合法無循環傳回 void，否則傳回 CyclicHierarchy 錯誤。</returns>
    COMPAT_NODISCARD static compat::expected<void, DesignerError> CheckCycle(
        const WidgetNode& targetParent,
        const WidgetNode& nodeToMove
    ) noexcept;
};

} // namespace designer
} // namespace cppui
