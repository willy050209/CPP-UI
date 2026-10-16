/**
 * @file ProjectExporter.hpp
 * @brief CPPUI Designer 完整 CMake 專案匯出引擎與增量使用者代碼保護器
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {
namespace exporter {

/// <summary>
/// 專案匯出設定選項 (ProjectExportOptions)
/// </summary>
struct CPPUI_API ProjectExportOptions {
    std::string outputDirectory = "./ExportedProject";
    std::string projectName = "MyCppUIApp";
    std::string formClassName = "MainForm";
    std::string targetNamespace = "MyApp";
    bool overwriteDesignerHpp = true;
    bool incrementalUserCode = true;
    bool generateMainCpp = true;
    bool generateCMakeLists = true;
    bool generateGitignore = true;
};

/// <summary>
/// 專案匯出執行結果 (ExportResult)
/// </summary>
struct CPPUI_API ExportResult {
    bool success = true;
    std::vector<std::string> generatedFiles;
    std::vector<std::string> updatedFiles;
    std::string errorMessage;
};

/// <summary>
/// 專案匯出引擎 (ProjectExporter)
/// 負責建構完整 CMake 專案目錄樹 (CMakeLists.txt、src/main.cpp、include/、src/)，
/// 針對 .designer.hpp 進行全量更新，並對使用者實作檔 (.cpp) 採取增量追加 Stub，
/// 絕不覆蓋或破壞既有之業務邏輯程式碼。
/// </summary>
class CPPUI_API ProjectExporter {
public:
    /// <summary>
    /// 將 AST 表單文件匯出為完整之獨立 CMake 專案目錄。
    /// </summary>
    /// <param name="doc">表單文件 AST。</param>
    /// <param name="options">匯出設定選項。</param>
    /// <returns>匯出結果結構。</returns>
    COMPAT_NODISCARD static ExportResult ExportProject(
        const FormDocumentNode& doc,
        const ProjectExportOptions& options
    );

    /// <summary>
    /// 增量合併使用者原始碼 (User Code Merger)。
    /// 比對現有檔案內容與新生成之實作檔，保留既有代碼並安全追加尚未實作之事件處理函式 Stub。
    /// </summary>
    /// <param name="existingSource">既有 .cpp 原始碼字串。</param>
    /// <param name="newGeneratedSource">新生成之 .cpp 程式碼片段。</param>
    /// <param name="className">表單類別名稱 (如 "MainForm")。</param>
    /// <returns>合併後之完整原始碼字串。</returns>
    COMPAT_NODISCARD static std::string MergeUserSource(
        compat::string_view existingSource,
        compat::string_view newGeneratedSource,
        compat::string_view className
    );

    /// <summary>
    /// 將 AST 表單儲存為確定性 .cppui.json 專案檔。
    /// </summary>
    /// <param name="doc">表單文件 AST。</param>
    /// <param name="filePath">目標檔案路徑。</param>
    /// <returns>若成功寫入傳回 true。</returns>
    static bool SaveProjectJson(
        const FormDocumentNode& doc,
        compat::string_view filePath
    );

    /// <summary>
    /// 從磁碟載入 .cppui.json 專案檔並重建成 AST 表單文件。
    /// </summary>
    /// <param name="filePath">檔案路徑。</param>
    /// <returns>還原成功之表單文件根節點，失敗傳回結構化錯誤。</returns>
    COMPAT_NODISCARD static compat::expected<std::unique_ptr<FormDocumentNode>, DesignerError> LoadProjectJson(
        compat::string_view filePath
    );
};

} // namespace exporter
} // namespace designer
} // namespace cppui
