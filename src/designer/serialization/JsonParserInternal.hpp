/**
 * @file JsonParserInternal.hpp
 * @brief 輕量級內部自包含 JSON 樹狀表示、遞迴下降解析與確定性格式化 (ABI 防火牆內)
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "compat/Compat.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {
namespace detail {

enum class JsonType : uint8_t {
    Null = 0,
    Boolean = 1,
    Number = 2,
    String = 3,
    Array = 4,
    Object = 5
};

class JsonValue {
public:
    using ArrayType = std::vector<JsonValue>;
    using ObjectType = std::map<std::string, JsonValue>; // std::map 保證鍵值依字母嚴格排序 (Deterministic)

    JsonValue() : m_type(JsonType::Null) {}
    JsonValue(std::nullptr_t) : m_type(JsonType::Null) {}
    JsonValue(bool b) : m_type(JsonType::Boolean), m_bool(b) {}
    JsonValue(int32_t num) : m_type(JsonType::Number), m_number(static_cast<double>(num)) {}
    JsonValue(int64_t num) : m_type(JsonType::Number), m_number(static_cast<double>(num)) {}
    JsonValue(double num) : m_type(JsonType::Number), m_number(num) {}
    JsonValue(const char* s) : m_type(JsonType::String), m_string(s) {}
    JsonValue(std::string s) : m_type(JsonType::String), m_string(std::move(s)) {}
    JsonValue(ArrayType arr) : m_type(JsonType::Array), m_array(std::move(arr)) {}
    JsonValue(ObjectType obj) : m_type(JsonType::Object), m_object(std::move(obj)) {}

    COMPAT_NODISCARD JsonType GetType() const noexcept { return m_type; }
    COMPAT_NODISCARD bool IsNull() const noexcept { return m_type == JsonType::Null; }
    COMPAT_NODISCARD bool IsBoolean() const noexcept { return m_type == JsonType::Boolean; }
    COMPAT_NODISCARD bool IsNumber() const noexcept { return m_type == JsonType::Number; }
    COMPAT_NODISCARD bool IsString() const noexcept { return m_type == JsonType::String; }
    COMPAT_NODISCARD bool IsArray() const noexcept { return m_type == JsonType::Array; }
    COMPAT_NODISCARD bool IsObject() const noexcept { return m_type == JsonType::Object; }

    COMPAT_NODISCARD bool AsBool() const noexcept { return m_bool; }
    COMPAT_NODISCARD double AsDouble() const noexcept { return m_number; }
    COMPAT_NODISCARD int64_t AsInt64() const noexcept { return static_cast<int64_t>(m_number); }
    COMPAT_NODISCARD int32_t AsInt32() const noexcept { return static_cast<int32_t>(m_number); }
    COMPAT_NODISCARD const std::string& AsString() const noexcept { return m_string; }
    COMPAT_NODISCARD const ArrayType& AsArray() const noexcept { return m_array; }
    COMPAT_NODISCARD ArrayType& AsArray() noexcept { return m_array; }
    COMPAT_NODISCARD const ObjectType& AsObject() const noexcept { return m_object; }
    COMPAT_NODISCARD ObjectType& AsObject() noexcept { return m_object; }

    COMPAT_NODISCARD const JsonValue* Find(const std::string& key) const noexcept {
        if (m_type != JsonType::Object) return nullptr;
        auto it = m_object.find(key);
        return (it != m_object.end()) ? &it->second : nullptr;
    }

    /// <summary>格式化為確定性 JSON 字串 (依鍵值字母排序，縮排 2 格)。</summary>
    COMPAT_NODISCARD std::string ToString(int32_t indentLevel = 0) const;

    /// <summary>解析 JSON 文字字串，遇錯安全傳回結構化錯誤與行號路徑。</summary>
    COMPAT_NODISCARD static compat::expected<JsonValue, DesignerError> Parse(compat::string_view jsonText);

private:
    JsonType m_type = JsonType::Null;
    bool m_bool = false;
    double m_number = 0.0;
    std::string m_string;
    ArrayType m_array;
    ObjectType m_object;
};

} // namespace detail
} // namespace designer
} // namespace cppui
