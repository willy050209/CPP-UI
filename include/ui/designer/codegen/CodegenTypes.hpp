/**
 * @file CodegenTypes.hpp
 * @brief CPPUI Designer 程式碼生成引擎配置選項、符號資訊與 SourceMap 映射結構
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {
namespace codegen {

/// <summary>
/// 變數與成員命名風格列舉
/// </summary>
enum class NamingStyle : uint8_t {
    CamelCase,    // 例如: btnSubmit
    PascalCase,   // 例如: BtnSubmit
    SnakeCase,    // 例如: btn_submit
    MemberMStyle  // 例如: m_btnSubmit
};

/// <summary>
/// 程式碼生成引擎組態選項 (CodegenOptions)
/// </summary>
struct CodegenOptions {
    size_t indentSpaces = 4;
    bool useTabs = false;
    bool emitComments = true;
    bool elideDefaults = true;
    NamingStyle memberNaming = NamingStyle::MemberMStyle;
    std::string rootNamespace = "my_app";
    std::string formClassName = "MainForm";
    std::string customBaseClass = ""; // 若非空，自訂 Form 基底類別；否則預設為 "cppui::app::Form"
    bool emitCMake = true;

    /// <summary>取得指定縮排層級的空白或 Tab 字串。</summary>
    /// <param name="level">縮排層級 (從 0 起算)。</param>
    /// <returns>縮排字串。</returns>
    COMPAT_NODISCARD std::string GetIndent(size_t level) const {
        if (useTabs) {
            return std::string(level, '\t');
        }
        return std::string(level * indentSpaces, ' ');
    }
};

/// <summary>
/// 程式碼中的原始碼行號與列號範圍 (SourceSpan)
/// </summary>
struct SourceSpan {
    size_t startLine = 0;
    size_t endLine = 0;
    size_t startCol = 0;
    size_t endCol = 0;

    COMPAT_NODISCARD bool operator==(const SourceSpan& other) const noexcept {
        return startLine == other.startLine && endLine == other.endLine &&
               startCol == other.startCol && endCol == other.endCol;
    }
};

/// <summary>
/// 控制項節點符號描述項 (SymbolInfo)
/// 關聯 AST Node UUID 與生成的 C++ 成員變數名及型別
/// </summary>
struct SymbolInfo {
    std::string nodeUuid;
    NodeType nodeType = NodeType::Button;
    std::string variableName; // 區域變數名稱 (例如 btnSubmit)
    std::string memberName;   // 成員變數名稱 (例如 m_btnSubmit)
    std::string typeName;     // 完整 C++ 類別名稱 (例如 cppui::controls::Button)
    SourceSpan spanInDesignerHpp;

    COMPAT_NODISCARD bool operator==(const SymbolInfo& other) const noexcept {
        return nodeUuid == other.nodeUuid &&
               nodeType == other.nodeType &&
               variableName == other.variableName &&
               memberName == other.memberName &&
               typeName == other.typeName &&
               spanInDesignerHpp == other.spanInDesignerHpp;
    }
};

/// <summary>
/// 雙向符號映射表 (SourceMap)
/// 提供 AST UUID ↔ 生成變數名稱 ↔ 原始碼行號範圍之雙向索引
/// </summary>
class CPPUI_API SourceMap {
public:
    SourceMap() = default;

    /// <summary>註冊符號元資料。</summary>
    /// <param name="info">符號資訊。</param>
    void RegisterSymbol(const SymbolInfo& info);

    /// <summary>依據 AST 節點 UUID 查詢符號資訊。</summary>
    /// <param name="uuid">節點 UUID 字串。</param>
    /// <returns>符號資訊，若不存在為 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<SymbolInfo> FindByNodeUuid(compat::string_view uuid) const;

    /// <summary>依據變數名稱 (區域或成員) 查詢符號資訊。</summary>
    /// <param name="name">變數名稱字串。</param>
    /// <returns>符號資訊，若不存在為 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<SymbolInfo> FindByVariableName(compat::string_view name) const;

    /// <summary>依據行號查詢所在之符號資訊。</summary>
    /// <param name="line">1-based 行號。</param>
    /// <returns>符合行號之符號資訊，若無為 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<SymbolInfo> FindByLine(size_t line) const;

    /// <summary>取得所有已註冊符號清單。</summary>
    /// <returns>符號清單常數引用。</returns>
    COMPAT_NODISCARD const std::vector<SymbolInfo>& GetAllSymbols() const noexcept;

    /// <summary>清空所有符號。</summary>
    void Clear() noexcept;

private:
    std::vector<SymbolInfo> m_symbols;
    std::map<std::string, size_t> m_uuidIndex;
    std::map<std::string, size_t> m_varIndex;
    std::map<std::string, size_t> m_memberIndex;
};

/// <summary>
/// 程式碼生成綜合結果物件 (CodegenResult)
/// </summary>
struct CodegenResult {
    std::string designerHeader;   // MainForm.designer.hpp
    std::string userHeader;       // MainForm.hpp
    std::string userSource;       // MainForm.cpp
    std::string fluentDslSnippet; // 獨立 Fluent DSL 片段
    std::string cmakeSnippet;     // CMakeLists.txt 構建腳本
    SourceMap sourceMap;
    std::vector<std::string> warnings;
};

// --- 命名與型別輔助函式 ---

/// <summary>將識別碼轉換為指定命名風格。</summary>
/// <param name="rawName">原始名稱。</param>
/// <param name="style">目標風格。</param>
/// <returns>格式化後的識別碼。</returns>
CPPUI_API std::string FormatIdentifier(compat::string_view rawName, NamingStyle style);

/// <summary>取得控制項節點對應的完整 C++ 型別名稱。</summary>
/// <param name="type">節點型別。</param>
/// <returns>C++ 型別名稱 (例如 "cppui::controls::Button")。</returns>
CPPUI_API std::string GetWidgetCppType(NodeType type);

/// <summary>取得控制項節點對應的 Fluent 工廠函式名稱。</summary>
/// <param name="type">節點型別。</param>
/// <returns>Fluent DSL 工廠名稱 (例如 "Button")。</returns>
CPPUI_API std::string GetWidgetDslFactory(NodeType type);

} // namespace codegen
} // namespace designer
} // namespace cppui
