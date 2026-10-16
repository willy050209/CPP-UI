/**
 * @file CMakeSnippetEmitter.cpp
 * @brief CMake 專案與 Target 建置設定程式碼發射器實作
 */

#include "ui/designer/codegen/CMakeSnippetEmitter.hpp"
#include <sstream>

namespace cppui {
namespace designer {
namespace codegen {

struct CMakeSnippetEmitter::Impl {
    std::string EmitTargetSnippetInternal(
        const FormDocumentNode& document,
        const CodegenOptions& options
    ) const {
        (void)document;
        std::string className = options.formClassName.empty() ? "MainForm" : options.formClassName;

        std::ostringstream ss;
        ss << "# =============================================================================\n"
           << "# Target Definition for " << className << "\n"
           << "# =============================================================================\n"
           << "add_executable(" << className << "\n"
           << "    " << className << ".hpp\n"
           << "    " << className << ".designer.hpp\n"
           << "    " << className << ".cpp\n"
           << "    main.cpp\n"
           << ")\n\n"
           << "target_link_libraries(" << className << " PRIVATE cppui)\n"
           << "target_compile_features(" << className << " PRIVATE cxx_std_20)\n";

        return ss.str();
    }

    std::string EmitStandaloneCMakeListsInternal(
        const FormDocumentNode& document,
        const CodegenOptions& options
    ) const {
        (void)document;
        std::string className = options.formClassName.empty() ? "MainForm" : options.formClassName;

        std::ostringstream ss;
        ss << "cmake_minimum_required(VERSION 3.20)\n"
           << "project(" << className << "App LANGUAGES CXX)\n\n"
           << "set(CMAKE_CXX_STANDARD 20)\n"
           << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
           << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n"
           << "# Find CPPUI Framework Package\n"
           << "find_package(cppui CONFIG REQUIRED)\n\n"
           << "add_executable(" << className << "App\n"
           << "    " << className << ".hpp\n"
           << "    " << className << ".designer.hpp\n"
           << "    " << className << ".cpp\n"
           << "    main.cpp\n"
           << ")\n\n"
           << "target_link_libraries(" << className << "App PRIVATE cppui)\n";

        return ss.str();
    }
};

CMakeSnippetEmitter::CMakeSnippetEmitter() : m_impl(std::make_unique<Impl>()) {}
CMakeSnippetEmitter::~CMakeSnippetEmitter() = default;
CMakeSnippetEmitter::CMakeSnippetEmitter(CMakeSnippetEmitter&& other) noexcept = default;
CMakeSnippetEmitter& CMakeSnippetEmitter::operator=(CMakeSnippetEmitter&& other) noexcept = default;

std::string CMakeSnippetEmitter::EmitTargetSnippet(
    const FormDocumentNode& document,
    const CodegenOptions& options
) const {
    return m_impl->EmitTargetSnippetInternal(document, options);
}

std::string CMakeSnippetEmitter::EmitStandaloneCMakeLists(
    const FormDocumentNode& document,
    const CodegenOptions& options
) const {
    return m_impl->EmitStandaloneCMakeListsInternal(document, options);
}

} // namespace codegen
} // namespace designer
} // namespace cppui
