/**
 * @file JsonParserInternal.cpp
 * @brief 遞迴下降 JSON 解析器與確定性格式化器實作
 */

#include "designer/serialization/JsonParserInternal.hpp"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cmath>

namespace cppui {
namespace designer {
namespace detail {

namespace {

std::string EscapeJsonString(compat::string_view str) {
    std::string out = "\"";
    for (char ch : str) {
        switch (ch) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    std::ostringstream ss;
                    ss << "\\u" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int32_t>(ch);
                    out += ss.str();
                } else {
                    out += ch;
                }
                break;
        }
    }
    out += "\"";
    return out;
}

class Parser {
public:
    explicit Parser(compat::string_view text) : m_text(text), m_pos(0), m_line(1), m_col(1) {}

    compat::expected<JsonValue, DesignerError> Parse() {
        SkipWhitespace();
        if (m_pos >= m_text.size()) {
            return MakeError("Empty JSON input");
        }
        auto val = ParseValue();
        if (!val.has_value()) {
            return val;
        }
        SkipWhitespace();
        if (m_pos < m_text.size()) {
            return MakeError("Unexpected extra trailing characters after valid JSON root");
        }
        return val;
    }

private:
    compat::expected<JsonValue, DesignerError> ParseValue() {
        SkipWhitespace();
        if (m_pos >= m_text.size()) {
            return MakeError("Unexpected end of JSON input");
        }

        char ch = m_text[m_pos];
        if (ch == '{') return ParseObject();
        if (ch == '[') return ParseArray();
        if (ch == '\"') return ParseString();
        if (ch == 't' || ch == 'f') return ParseBool();
        if (ch == 'n') return ParseNull();
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) return ParseNumber();

        return MakeError(std::string("Unexpected character: '") + ch + "'");
    }

    compat::expected<JsonValue, DesignerError> ParseObject() {
        m_pos++; // skip '{'
        m_col++;
        JsonValue::ObjectType obj;

        while (true) {
            SkipWhitespace();
            if (m_pos >= m_text.size()) {
                return MakeError("Unterminated object: missing '}'");
            }
            if (m_text[m_pos] == '}') {
                m_pos++;
                m_col++;
                return JsonValue(std::move(obj));
            }

            if (m_text[m_pos] != '\"') {
                return MakeError("Expected string key in object");
            }
            auto keyRes = ParseRawString();
            if (!keyRes.has_value()) {
                return compat::unexpected(keyRes.error());
            }

            SkipWhitespace();
            if (m_pos >= m_text.size() || m_text[m_pos] != ':') {
                return MakeError("Expected ':' after object key");
            }
            m_pos++;
            m_col++;

            auto valRes = ParseValue();
            if (!valRes.has_value()) {
                return valRes;
            }

            obj[keyRes.value()] = std::move(valRes.value());

            SkipWhitespace();
            if (m_pos >= m_text.size()) {
                return MakeError("Unterminated object: missing ',' or '}'");
            }
            if (m_text[m_pos] == ',') {
                m_pos++;
                m_col++;
                continue;
            }
            if (m_text[m_pos] == '}') {
                m_pos++;
                m_col++;
                return JsonValue(std::move(obj));
            }
            return MakeError("Expected ',' or '}' in object");
        }
    }

    compat::expected<JsonValue, DesignerError> ParseArray() {
        m_pos++; // skip '['
        m_col++;
        JsonValue::ArrayType arr;

        while (true) {
            SkipWhitespace();
            if (m_pos >= m_text.size()) {
                return MakeError("Unterminated array: missing ']'");
            }
            if (m_text[m_pos] == ']') {
                m_pos++;
                m_col++;
                return JsonValue(std::move(arr));
            }

            auto itemRes = ParseValue();
            if (!itemRes.has_value()) {
                return itemRes;
            }
            arr.push_back(std::move(itemRes.value()));

            SkipWhitespace();
            if (m_pos >= m_text.size()) {
                return MakeError("Unterminated array: missing ',' or ']'");
            }
            if (m_text[m_pos] == ',') {
                m_pos++;
                m_col++;
                continue;
            }
            if (m_text[m_pos] == ']') {
                m_pos++;
                m_col++;
                return JsonValue(std::move(arr));
            }
            return MakeError("Expected ',' or ']' in array");
        }
    }

    compat::expected<std::string, DesignerError> ParseRawString() {
        m_pos++; // skip opening '"'
        m_col++;
        std::string str;

        while (m_pos < m_text.size()) {
            char ch = m_text[m_pos++];
            m_col++;
            if (ch == '\"') {
                return str;
            }
            if (ch == '\\') {
                if (m_pos >= m_text.size()) {
                    return MakeError("Unexpected end of string escape");
                }
                char esc = m_text[m_pos++];
                m_col++;
                switch (esc) {
                    case '\"': str += '\"'; break;
                    case '\\': str += '\\'; break;
                    case '/':  str += '/';  break;
                    case 'b':  str += '\b'; break;
                    case 'f':  str += '\f'; break;
                    case 'n':  str += '\n'; break;
                    case 'r':  str += '\r'; break;
                    case 't':  str += '\t'; break;
                    default:   str += esc;  break;
                }
            } else {
                str += ch;
            }
        }
        return MakeError("Unterminated string");
    }

    compat::expected<JsonValue, DesignerError> ParseString() {
        auto res = ParseRawString();
        if (!res.has_value()) return compat::unexpected(res.error());
        return JsonValue(std::move(res.value()));
    }

    compat::expected<JsonValue, DesignerError> ParseBool() {
        if (m_text.substr(m_pos, 4) == "true") {
            m_pos += 4;
            m_col += 4;
            return JsonValue(true);
        }
        if (m_text.substr(m_pos, 5) == "false") {
            m_pos += 5;
            m_col += 5;
            return JsonValue(false);
        }
        return MakeError("Invalid boolean token");
    }

    compat::expected<JsonValue, DesignerError> ParseNull() {
        if (m_text.substr(m_pos, 4) == "null") {
            m_pos += 4;
            m_col += 4;
            return JsonValue(nullptr);
        }
        return MakeError("Invalid null token");
    }

    compat::expected<JsonValue, DesignerError> ParseNumber() {
        size_t start = m_pos;
        if (m_text[m_pos] == '-') {
            m_pos++;
            m_col++;
        }
        while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
            m_pos++;
            m_col++;
        }
        if (m_pos < m_text.size() && m_text[m_pos] == '.') {
            m_pos++;
            m_col++;
            while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
                m_pos++;
                m_col++;
            }
        }
        if (m_pos < m_text.size() && (m_text[m_pos] == 'e' || m_text[m_pos] == 'E')) {
            m_pos++;
            m_col++;
            if (m_pos < m_text.size() && (m_text[m_pos] == '+' || m_text[m_pos] == '-')) {
                m_pos++;
                m_col++;
            }
            while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
                m_pos++;
                m_col++;
            }
        }

        std::string numStr(m_text.substr(start, m_pos - start));
        try {
            double val = std::stod(numStr);
            return JsonValue(val);
        } catch (...) {
            return MakeError("Invalid number format: " + numStr);
        }
    }

    void SkipWhitespace() {
        while (m_pos < m_text.size()) {
            char ch = m_text[m_pos];
            if (ch == ' ' || ch == '\t' || ch == '\r') {
                m_pos++;
                m_col++;
            } else if (ch == '\n') {
                m_pos++;
                m_line++;
                m_col = 1;
            } else {
                break;
            }
        }
    }

    compat::unexpected<DesignerError> MakeError(std::string msg) const {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidJson;
        err.message = msg + " (at line " + std::to_string(m_line) + ", column " + std::to_string(m_col) + ")";
        return compat::unexpected(std::move(err));
    }

    compat::string_view m_text;
    size_t m_pos;
    size_t m_line;
    size_t m_col;
};

} // anonymous namespace

std::string JsonValue::ToString(int32_t indentLevel) const {
    std::string indent(static_cast<size_t>(indentLevel * 2), ' ');
    std::string childIndent(static_cast<size_t>((indentLevel + 1) * 2), ' ');

    switch (m_type) {
        case JsonType::Null:
            return "null";
        case JsonType::Boolean:
            return m_bool ? "true" : "false";
        case JsonType::Number: {
            if (std::floor(m_number) == m_number && !std::isinf(m_number) && !std::isnan(m_number)) {
                // 若為整數且範圍在 int64 內，避免輸出 .0
                int64_t i = static_cast<int64_t>(m_number);
                return std::to_string(i);
            }
            std::ostringstream ss;
            ss << m_number;
            return ss.str();
        }
        case JsonType::String:
            return EscapeJsonString(m_string);
        case JsonType::Array: {
            if (m_array.empty()) {
                return "[]";
            }
            std::string out = "[\n";
            for (size_t i = 0; i < m_array.size(); ++i) {
                out += childIndent + m_array[i].ToString(indentLevel + 1);
                if (i + 1 < m_array.size()) {
                    out += ",";
                }
                out += "\n";
            }
            out += indent + "]";
            return out;
        }
        case JsonType::Object: {
            if (m_object.empty()) {
                return "{}";
            }
            std::string out = "{\n";
            size_t idx = 0;
            for (const auto& kv : m_object) {
                out += childIndent + EscapeJsonString(kv.first) + ": " + kv.second.ToString(indentLevel + 1);
                if (idx + 1 < m_object.size()) {
                    out += ",";
                }
                out += "\n";
                idx++;
            }
            out += indent + "}";
            return out;
        }
    }
    return "null";
}

compat::expected<JsonValue, DesignerError> JsonValue::Parse(compat::string_view jsonText) {
    Parser p(jsonText);
    return p.Parse();
}

} // namespace detail
} // namespace designer
} // namespace cppui
