/**
 * @file SchemaVersion.hpp
 * @brief CPPUI Designer 專案檔格式語意化版本號常數
 */

#pragma once

#include "compat/Compat.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 目前 AST 與 .cppui.json 序列化 Schema 規格版本
/// </summary>
COMPAT_INLINE_VAR static constexpr const char* CurrentSchemaVersion = "1.0.0";

} // namespace designer
} // namespace cppui
