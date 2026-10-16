/**
 * @file ProjectExporter.cpp
 * @brief CPPUI Designer 完整 CMake 專案匯出引擎實作
 */

#include "ui/designer/exporter/ProjectExporter.hpp"
#include "ui/designer/codegen/FormCodegenPipeline.hpp"
#include "ui/designer/serialization/FormSerializer.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace cppui {
namespace designer {
namespace exporter {

namespace {

bool WriteFileContent(const std::filesystem::path& path, compat::string_view content) {
    std::ofstream file(path.string(), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return file.good();
}

compat::optional<std::string> ReadFileContent(const std::filesystem::path& path) {
    std::ifstream file(path.string(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return compat::nullopt;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // anonymous namespace

std::string ProjectExporter::MergeUserSource(
    compat::string_view existingSource,
    compat::string_view newGeneratedSource,
    compat::string_view className
) {
    std::string existing(existingSource);
    std::string generated(newGeneratedSource);
    std::string classPrefix = std::string(className) + "::";

    // 尋找新生成代碼中的所有函式 Stub
    // 格式例如: void MainForm::OnSubmitClicked(...) { ... }
    size_t searchPos = 0;
    std::vector<std::string> newStubs;

    while (true) {
        size_t funcPos = generated.find(classPrefix, searchPos);
        if (funcPos == std::string::npos) {
            break;
        }

        // 往回找函式回傳型別開頭 (例如 "void " 或換行後)
        size_t lineStart = generated.rfind('\n', funcPos);
        lineStart = (lineStart == std::string::npos) ? 0 : lineStart + 1;

        // 取得函式名稱
        size_t nameStart = funcPos + classPrefix.length();
        size_t parenPos = generated.find('(', nameStart);
        if (parenPos == std::string::npos) {
            searchPos = funcPos + classPrefix.length();
            continue;
        }

        std::string funcName = generated.substr(nameStart, parenPos - nameStart);

        // 建構子特殊處理 (若為建構子直接略過)
        if (funcName == className) {
            searchPos = parenPos + 1;
            continue;
        }

        // 尋找函式結束的大括號 '}'
        size_t braceStart = generated.find('{', parenPos);
        if (braceStart == std::string::npos) {
            searchPos = parenPos + 1;
            continue;
        }

        size_t braceEnd = generated.find('}', braceStart);
        if (braceEnd == std::string::npos) {
            searchPos = braceStart + 1;
            continue;
        }

        size_t funcEnd = braceEnd + 1;
        std::string fullStub = generated.substr(lineStart, funcEnd - lineStart);

        // 檢查既有原始碼中是否已存在此函式名稱
        std::string needle = classPrefix + funcName + "(";
        if (existing.find(needle) == std::string::npos) {
            newStubs.push_back(fullStub);
        }

        searchPos = funcEnd;
    }

    if (newStubs.empty()) {
        return existing;
    }

    // 將新 Stubs 附加至既有檔案最後一個 namespace 結束括弧前，或直接附加於末尾
    std::string merged = existing;
    size_t lastNamespaceEnd = merged.rfind("} // namespace");
    if (lastNamespaceEnd == std::string::npos) {
        lastNamespaceEnd = merged.rfind('}');
    }

    std::ostringstream appendBlock;
    appendBlock << "\n\n// --- Automatically Appended Event Handler Stubs by CPPUI Designer ---\n";
    for (const auto& stub : newStubs) {
        appendBlock << "\n" << stub << "\n";
    }

    if (lastNamespaceEnd != std::string::npos) {
        merged.insert(lastNamespaceEnd, appendBlock.str() + "\n");
    } else {
        merged.append(appendBlock.str());
    }

    return merged;
}

ExportResult ProjectExporter::ExportProject(
    const FormDocumentNode& doc,
    const ProjectExportOptions& options
) {
    ExportResult res;
    std::filesystem::path root(options.outputDirectory);

    std::error_code ec;
    std::filesystem::create_directories(root / "include" / options.targetNamespace, ec);
    std::filesystem::create_directories(root / "src", ec);
    std::filesystem::create_directories(root / "assets", ec);

    if (ec) {
        res.success = false;
        res.errorMessage = "Failed to create project directories: " + ec.message();
        return res;
    }

    // 執行代碼生成
    codegen::FormCodegenPipeline pipeline;
    codegen::CodegenOptions codegenOpts;
    codegenOpts.rootNamespace = options.targetNamespace;
    codegenOpts.formClassName = options.formClassName;

    auto codegenRes = pipeline.Generate(doc, codegenOpts);
    if (!codegenRes.has_value()) {
        res.success = false;
        res.errorMessage = codegenRes.error().message;
        return res;
    }

    const auto& cr = *codegenRes;

    // 1. 生成 CMakeLists.txt
    if (options.generateCMakeLists) {
        auto cmakePath = root / "CMakeLists.txt";
        if (!std::filesystem::exists(cmakePath)) {
            WriteFileContent(cmakePath, cr.cmakeSnippet);
            res.generatedFiles.push_back(cmakePath.string());
        }
    }

    // 2. 生成 .gitignore
    if (options.generateGitignore) {
        auto gitignorePath = root / ".gitignore";
        if (!std::filesystem::exists(gitignorePath)) {
            const char* gitignoreContent =
                "build/\n"
                "out/\n"
                "bin/\n"
                "*.obj\n"
                "*.o\n"
                "*.exe\n"
                "*.pdb\n"
                "*.lib\n"
                "*.a\n"
                ".vs/\n"
                ".vscode/\n";
            WriteFileContent(gitignorePath, gitignoreContent);
            res.generatedFiles.push_back(gitignorePath.string());
        }
    }

    // 3. 全量覆寫更新 <FormClass>.designer.hpp
    auto designerHppPath = root / "include" / options.targetNamespace / (options.formClassName + ".designer.hpp");
    WriteFileContent(designerHppPath, cr.designerHeader);
    res.generatedFiles.push_back(designerHppPath.string());

    // 4. 生成或保留 <FormClass>.hpp
    auto hppPath = root / "include" / options.targetNamespace / (options.formClassName + ".hpp");
    if (!std::filesystem::exists(hppPath)) {
        WriteFileContent(hppPath, cr.userHeader);
        res.generatedFiles.push_back(hppPath.string());
    }

    // 5. 增量合併或生成 <FormClass>.cpp
    auto cppPath = root / "src" / (options.formClassName + ".cpp");
    if (!std::filesystem::exists(cppPath)) {
        WriteFileContent(cppPath, cr.userSource);
        res.generatedFiles.push_back(cppPath.string());
    } else if (options.incrementalUserCode) {
        auto existingContent = ReadFileContent(cppPath);
        if (existingContent.has_value()) {
            std::string mergedContent = MergeUserSource(*existingContent, cr.userSource, options.formClassName);
            if (mergedContent != *existingContent) {
                WriteFileContent(cppPath, mergedContent);
                res.updatedFiles.push_back(cppPath.string());
            }
        }
    }

    // 6. 生成 src/main.cpp
    if (options.generateMainCpp) {
        auto mainCppPath = root / "src" / "main.cpp";
        if (!std::filesystem::exists(mainCppPath)) {
            std::ostringstream oss;
            oss << "/**\n"
                << " * @file main.cpp\n"
                << " * @brief Application Entry Point\n"
                << " */\n\n"
                << "#include \"ui/CppUI.hpp\"\n"
                << "#include \"" << options.targetNamespace << "/" << options.formClassName << ".hpp\"\n\n"
                << "int main() {\n"
                << "    cppui::app::Application app;\n"
                << "    auto form = std::make_shared<" << options.targetNamespace << "::" << options.formClassName << ">();\n"
                << "    return app.Run(form);\n"
                << "}\n";
            WriteFileContent(mainCppPath, oss.str());
            res.generatedFiles.push_back(mainCppPath.string());
        }
    }

    res.success = true;
    return res;
}

bool ProjectExporter::SaveProjectJson(
    const FormDocumentNode& doc,
    compat::string_view filePath
) {
    auto res = FormSerializer::Serialize(doc);
    if (!res.has_value()) {
        return false;
    }
    return WriteFileContent(std::filesystem::path(std::string(filePath)), *res);
}

compat::expected<std::unique_ptr<FormDocumentNode>, DesignerError> ProjectExporter::LoadProjectJson(
    compat::string_view filePath
) {
    auto content = ReadFileContent(std::filesystem::path(std::string(filePath)));
    if (!content.has_value()) {
        DesignerError err;
        err.code = DesignerErrorCode::DeserializationError;
        err.message = "Failed to open project file: " + std::string(filePath);
        return compat::unexpected(std::move(err));
    }

    return FormSerializer::Deserialize(*content);
}

} // namespace exporter
} // namespace designer
} // namespace cppui
