/**
 * @file LiveFormRunner.hpp
 * @brief CPPUI Designer AST 表單實例化工廠與真實試運行模式 (LiveFormRunner)
 */

#pragma once

#include <memory>
#include <string>
#include <functional>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/app/Form.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/ast/WidgetNode.hpp"

namespace cppui {
namespace designer {
namespace preview {

/// <summary>
/// 即時運行事件回呼函式型別 (傳遞觸發之節點 UUID 與事件名稱)
/// </summary>
using LiveEventCallback = std::function<void(const std::string& nodeUuid, const std::string& eventName)>;

/// <summary>
/// 即時表單實例化與試運行器 (LiveFormRunner)
/// 提供零設計期干擾的純淨真實 UIElement 控制項樹實例化工廠，
/// 支援使用者直接進行按鈕點擊、FocusManager 焦點導覽、彈出層測試與主題即時切換。
/// </summary>
class CPPUI_API LiveFormRunner {
public:
    /// <summary>
    /// 將單一 WidgetNode (及其子樹) 遞迴轉換為真實的 CppUI UIElement 元件實例。
    /// </summary>
    /// <param name="node">AST 節點參考。</param>
    /// <param name="eventCallback">可選之事件觸發回呼函式。</param>
    /// <returns>實例化之 UIElement 智慧指標。</returns>
    COMPAT_NODISCARD static std::shared_ptr<core::UIElement> CreateWidget(
        const WidgetNode& node,
        LiveEventCallback eventCallback = nullptr
    );

    /// <summary>
    /// 依據表單文件 AST 根節點組裝完整的真實 app::Form 視窗。
    /// 不掛載任何設計期 Adorner 飾條與事件攔截盾。
    /// </summary>
    /// <param name="doc">表單文件 AST。</param>
    /// <param name="eventCallback">可選之事件觸發回呼函式。</param>
    /// <returns>實例化之 Form 智慧指標。</returns>
    COMPAT_NODISCARD static std::shared_ptr<app::Form> CreateForm(
        const FormDocumentNode& doc,
        LiveEventCallback eventCallback = nullptr
    );

    /// <summary>
    /// 以模態方式啟動真實表單試運行視窗 (Live Run Modal)。
    /// </summary>
    /// <param name="doc">表單文件 AST。</param>
    /// <param name="owner">可選之父表單指標。</param>
    /// <param name="eventCallback">可選之事件回呼函式。</param>
    /// <returns>對話框操作結果。</returns>
    static app::DialogResult RunModal(
        const FormDocumentNode& doc,
        std::shared_ptr<app::Form> owner = nullptr,
        LiveEventCallback eventCallback = nullptr
    );
};

} // namespace preview
} // namespace designer
} // namespace cppui
