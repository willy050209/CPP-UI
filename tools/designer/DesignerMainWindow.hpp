/**
 * @file DesignerMainWindow.hpp
 * @brief CPPUI 官方獨立視覺化表單設計器主視窗 (DesignerMainWindow)
 */

#pragma once

#include <memory>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/app/Form.hpp"

namespace cppui {
namespace designer {
class FormDocumentNode;

namespace tool {

/// <summary>
/// CPPUI 官方視覺化表單設計器主視窗 (DesignerMainWindow)
/// 整合頂部工具列、左側控制項工具箱與階層樹、中央視覺化設計畫布與即時代碼預覽、
/// 以及右側型別安全屬性與事件檢查器。採用現代 Pimpl 慣用法與深常數性封裝。
/// </summary>
class DesignerMainWindow : public app::Form {
public:
    /// <summary>
    /// 建構設計器主視窗實例。
    /// </summary>
    DesignerMainWindow();

    /// <summary>
    /// 解構子，於 .cpp 完整型別處實作。
    /// </summary>
    ~DesignerMainWindow() override;

    DesignerMainWindow(const DesignerMainWindow&) = delete;
    DesignerMainWindow& operator=(const DesignerMainWindow&) = delete;
    DesignerMainWindow(DesignerMainWindow&&) = delete;
    DesignerMainWindow& operator=(DesignerMainWindow&&) = delete;

    /// <summary>
    /// 取得設計器當前表單文件實例。
    /// </summary>
    /// <returns>FormDocumentNode 參照。</returns>
    COMPAT_NODISCARD FormDocumentNode& GetDocument() noexcept;

    /// <summary>
    /// 觸發真實試運行模式 (Live Run)。
    /// </summary>
    void TriggerLiveRun();

    /// <summary>
    /// 觸發切換深色/淺色主題。
    /// </summary>
    void ToggleTheme();

    /// <summary>
    /// 觸發專案 CMake 匯出。
    /// </summary>
    void TriggerExportProject();

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace tool
} // namespace designer
} // namespace cppui
