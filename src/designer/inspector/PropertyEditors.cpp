/**
 * @file PropertyEditors.cpp
 * @brief 專屬型別屬性編輯器模型與格式驗證器實作
 */

#include "ui/designer/inspector/PropertyEditors.hpp"
#include "ui/designer/ast/IdentifierValidator.hpp"
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cppui {
namespace designer {
namespace inspector {

namespace {

std::string Trim(compat::string_view sv) {
    size_t first = 0;
    while (first < sv.size() && std::isspace(static_cast<unsigned char>(sv[first]))) {
        first++;
    }
    if (first >= sv.size()) {
        return "";
    }
    size_t last = sv.size() - 1;
    while (last > first && std::isspace(static_cast<unsigned char>(sv[last]))) {
        last--;
    }
    return std::string(sv.substr(first, last - first + 1));
}

std::string ToLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::vector<std::string> SplitByCommaOrSpace(compat::string_view sv) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : sv) {
        if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

} // anonymous namespace

ValidationResult PropertyValidators::ValidateDouble(
    compat::string_view text,
    double minVal,
    double maxVal
) noexcept {
    std::string s = Trim(text);
    if (s.empty()) {
        return ValidationResult::Error("Value cannot be empty");
    }

    std::string lower = ToLower(s);
    if (lower == "nan" || lower == "inf" || lower == "+inf" || lower == "-inf" ||
        lower == "infinity" || lower == "+infinity" || lower == "-infinity") {
        return ValidationResult::Error("NaN or Infinity values are not permitted");
    }

    char* end = nullptr;
    const double val = std::strtod(s.c_str(), &end);
    if (end == s.c_str() || *end != '\0') {
        return ValidationResult::Error("Invalid numeric format: '" + s + "'");
    }

    if (std::isnan(val) || std::isinf(val)) {
        return ValidationResult::Error("Value evaluated to non-finite number");
    }

    if (val < minVal || val > maxVal) {
        std::ostringstream oss;
        oss << "Value " << val << " out of valid range [" << minVal << ", " << maxVal << "]";
        return ValidationResult::Error(oss.str());
    }

    return ValidationResult::Ok();
}

ValidationResult PropertyValidators::ValidateInteger(
    compat::string_view text,
    int64_t minVal,
    int64_t maxVal
) noexcept {
    std::string s = Trim(text);
    if (s.empty()) {
        return ValidationResult::Error("Value cannot be empty");
    }

    char* end = nullptr;
    const long long val = std::strtoll(s.c_str(), &end, 10);
    if (end == s.c_str() || *end != '\0') {
        return ValidationResult::Error("Invalid integer format: '" + s + "'");
    }

    if (val < minVal || val > maxVal) {
        std::ostringstream oss;
        oss << "Integer " << val << " out of valid range [" << minVal << ", " << maxVal << "]";
        return ValidationResult::Error(oss.str());
    }

    return ValidationResult::Ok();
}

ValidationResult PropertyValidators::ValidateDimension(compat::string_view text) noexcept {
    return ValidateDouble(text, 0.0, 100000.0);
}

ValidationResult PropertyValidators::ValidateThickness(compat::string_view text) noexcept {
    auto tokens = SplitByCommaOrSpace(text);
    if (tokens.empty()) {
        return ValidationResult::Error("Thickness string cannot be empty");
    }
    if (tokens.size() != 1 && tokens.size() != 2 && tokens.size() != 4) {
        return ValidationResult::Error("Thickness requires 1, 2, or 4 numeric values");
    }

    for (const auto& token : tokens) {
        auto res = ValidateDouble(token, 0.0, 10000.0);
        if (!res.isValid) {
            return res;
        }
    }
    return ValidationResult::Ok();
}

ValidationResult PropertyValidators::ValidateColorHex(compat::string_view text) noexcept {
    std::string s = Trim(text);
    if (s.empty()) {
        return ValidationResult::Error("Color string cannot be empty");
    }

    if (s[0] == '#') {
        s = s.substr(1);
    }

    if (s.length() != 3 && s.length() != 4 && s.length() != 6 && s.length() != 8) {
        return ValidationResult::Error("Hex color must have 3, 4, 6, or 8 characters (#RGB, #RGBA, #RRGGBB, #AARRGGBB)");
    }

    for (char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return ValidationResult::Error("Invalid hex digit in color: " + std::string(1, c));
        }
    }

    return ValidationResult::Ok();
}

ValidationResult PropertyValidators::ValidateIdentifier(compat::string_view name) noexcept {
    auto res = IdentifierValidator::ValidateIdentifier(name);
    if (!res.has_value()) {
        return ValidationResult::Error(res.error().message);
    }
    return ValidationResult::Ok();
}

std::string PropertyEditorModel::FormatDisplayString(
    const PropertyValue& value,
    bool isMixed
) {
    if (isMixed) {
        return "<Multiple Values>";
    }

    if (value.IsEmpty()) {
        return "";
    }

    if (value.Is<ThemeTokenRef>()) {
        auto token = value.Get<ThemeTokenRef>().value();
        return "@" + token.tokenName;
    }

    if (value.Is<bool>()) {
        return value.Get<bool>().value() ? "True" : "False";
    }

    if (value.Is<int32_t>()) {
        return std::to_string(value.Get<int32_t>().value());
    }

    if (value.Is<int64_t>()) {
        return std::to_string(value.Get<int64_t>().value());
    }

    if (value.Is<double>()) {
        double d = value.Get<double>().value();
        std::ostringstream oss;
        if (d == std::floor(d) && !std::isinf(d)) {
            oss << static_cast<int64_t>(d);
        } else {
            oss << d;
        }
        return oss.str();
    }

    if (value.Is<std::string>()) {
        return value.Get<std::string>().value();
    }

    if (value.Is<core::Thickness>()) {
        auto th = value.Get<core::Thickness>().value();
        if (th.left == th.top && th.top == th.right && th.right == th.bottom) {
            return std::to_string(static_cast<int32_t>(th.left));
        }
        std::ostringstream oss;
        oss << th.left << ", " << th.top << ", " << th.right << ", " << th.bottom;
        return oss.str();
    }

    if (value.Is<core::Color>()) {
        auto c = value.Get<core::Color>().value();
        std::ostringstream oss;
        oss << "#"
            << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << static_cast<int32_t>(c.r)
            << std::setw(2) << static_cast<int32_t>(c.g)
            << std::setw(2) << static_cast<int32_t>(c.b);
        if (c.a != 255) {
            oss << std::setw(2) << static_cast<int32_t>(c.a);
        }
        return oss.str();
    }

    if (value.Is<layout::HorizontalAlignment>()) {
        switch (value.Get<layout::HorizontalAlignment>().value()) {
            case layout::HorizontalAlignment::Left:    return "Left";
            case layout::HorizontalAlignment::Center:  return "Center";
            case layout::HorizontalAlignment::Right:   return "Right";
            case layout::HorizontalAlignment::Stretch: return "Stretch";
        }
    }

    if (value.Is<layout::VerticalAlignment>()) {
        switch (value.Get<layout::VerticalAlignment>().value()) {
            case layout::VerticalAlignment::Top:     return "Top";
            case layout::VerticalAlignment::Center:  return "Center";
            case layout::VerticalAlignment::Bottom:  return "Bottom";
            case layout::VerticalAlignment::Stretch: return "Stretch";
        }
    }

    if (value.Is<layout::Orientation>()) {
        return value.Get<layout::Orientation>().value() == layout::Orientation::Horizontal
            ? "Horizontal" : "Vertical";
    }

    if (value.Is<layout::Visibility>()) {
        switch (value.Get<layout::Visibility>().value()) {
            case layout::Visibility::Visible:   return "Visible";
            case layout::Visibility::Hidden:    return "Hidden";
            case layout::Visibility::Collapsed: return "Collapsed";
        }
    }

    return value.ToString();
}

compat::expected<PropertyValue, std::string> PropertyEditorModel::ParseBool(
    compat::string_view text
) noexcept {
    std::string s = ToLower(Trim(text));
    if (s == "true" || s == "1" || s == "yes") {
        return PropertyValue(true);
    }
    if (s == "false" || s == "0" || s == "no") {
        return PropertyValue(false);
    }
    return compat::unexpected(std::string("Cannot parse boolean from '") + std::string(text) + "'");
}

compat::expected<PropertyValue, std::string> PropertyEditorModel::ParseNumber(
    compat::string_view text,
    bool isInteger
) noexcept {
    if (isInteger) {
        auto valRes = PropertyValidators::ValidateInteger(text);
        if (!valRes.isValid) {
            return compat::unexpected(valRes.errorMessage);
        }
        long long val = std::strtoll(Trim(text).c_str(), nullptr, 10);
        return PropertyValue(static_cast<int32_t>(val));
    }

    auto valRes = PropertyValidators::ValidateDouble(text);
    if (!valRes.isValid) {
        return compat::unexpected(valRes.errorMessage);
    }
    double val = std::strtod(Trim(text).c_str(), nullptr);
    return PropertyValue(val);
}

compat::expected<PropertyValue, std::string> PropertyEditorModel::ParseThickness(
    compat::string_view text
) noexcept {
    auto valRes = PropertyValidators::ValidateThickness(text);
    if (!valRes.isValid) {
        return compat::unexpected(valRes.errorMessage);
    }

    auto tokens = SplitByCommaOrSpace(text);
    if (tokens.size() == 1) {
        float v = static_cast<float>(std::strtod(tokens[0].c_str(), nullptr));
        return PropertyValue(core::Thickness(v, v, v, v));
    }
    if (tokens.size() == 2) {
        float h = static_cast<float>(std::strtod(tokens[0].c_str(), nullptr));
        float v = static_cast<float>(std::strtod(tokens[1].c_str(), nullptr));
        return PropertyValue(core::Thickness(h, v, h, v));
    }
    if (tokens.size() == 4) {
        float l = static_cast<float>(std::strtod(tokens[0].c_str(), nullptr));
        float t = static_cast<float>(std::strtod(tokens[1].c_str(), nullptr));
        float r = static_cast<float>(std::strtod(tokens[2].c_str(), nullptr));
        float b = static_cast<float>(std::strtod(tokens[3].c_str(), nullptr));
        return PropertyValue(core::Thickness(l, t, r, b));
    }

    return compat::unexpected(std::string("Invalid thickness component count"));
}

compat::expected<PropertyValue, std::string> PropertyEditorModel::ParseColor(
    compat::string_view hex
) noexcept {
    auto valRes = PropertyValidators::ValidateColorHex(hex);
    if (!valRes.isValid) {
        return compat::unexpected(valRes.errorMessage);
    }

    std::string s = Trim(hex);
    if (s[0] == '#') {
        s = s.substr(1);
    }

    uint8_t r = 0, g = 0, b = 0, a = 255;
    if (s.length() == 3) {
        r = static_cast<uint8_t>(std::stoi(std::string(2, s[0]), nullptr, 16));
        g = static_cast<uint8_t>(std::stoi(std::string(2, s[1]), nullptr, 16));
        b = static_cast<uint8_t>(std::stoi(std::string(2, s[2]), nullptr, 16));
    } else if (s.length() == 4) {
        r = static_cast<uint8_t>(std::stoi(std::string(2, s[0]), nullptr, 16));
        g = static_cast<uint8_t>(std::stoi(std::string(2, s[1]), nullptr, 16));
        b = static_cast<uint8_t>(std::stoi(std::string(2, s[2]), nullptr, 16));
        a = static_cast<uint8_t>(std::stoi(std::string(2, s[3]), nullptr, 16));
    } else if (s.length() == 6) {
        r = static_cast<uint8_t>(std::stoi(s.substr(0, 2), nullptr, 16));
        g = static_cast<uint8_t>(std::stoi(s.substr(2, 2), nullptr, 16));
        b = static_cast<uint8_t>(std::stoi(s.substr(4, 2), nullptr, 16));
    } else if (s.length() == 8) {
        r = static_cast<uint8_t>(std::stoi(s.substr(0, 2), nullptr, 16));
        g = static_cast<uint8_t>(std::stoi(s.substr(2, 2), nullptr, 16));
        b = static_cast<uint8_t>(std::stoi(s.substr(4, 2), nullptr, 16));
        a = static_cast<uint8_t>(std::stoi(s.substr(6, 2), nullptr, 16));
    }

    return PropertyValue(core::Color{ r, g, b, a });
}

compat::expected<PropertyValue, std::string> PropertyEditorModel::ParseEnum(
    compat::string_view propName,
    compat::string_view text
) noexcept {
    std::string s = Trim(text);
    if (propName == "HorizontalAlignment") {
        if (s == "Left") return PropertyValue(layout::HorizontalAlignment::Left);
        if (s == "Center") return PropertyValue(layout::HorizontalAlignment::Center);
        if (s == "Right") return PropertyValue(layout::HorizontalAlignment::Right);
        if (s == "Stretch") return PropertyValue(layout::HorizontalAlignment::Stretch);
    } else if (propName == "VerticalAlignment") {
        if (s == "Top") return PropertyValue(layout::VerticalAlignment::Top);
        if (s == "Center") return PropertyValue(layout::VerticalAlignment::Center);
        if (s == "Bottom") return PropertyValue(layout::VerticalAlignment::Bottom);
        if (s == "Stretch") return PropertyValue(layout::VerticalAlignment::Stretch);
    } else if (propName == "Orientation") {
        if (s == "Horizontal") return PropertyValue(layout::Orientation::Horizontal);
        if (s == "Vertical") return PropertyValue(layout::Orientation::Vertical);
    } else if (propName == "Visibility") {
        if (s == "Visible") return PropertyValue(layout::Visibility::Visible);
        if (s == "Hidden") return PropertyValue(layout::Visibility::Hidden);
        if (s == "Collapsed") return PropertyValue(layout::Visibility::Collapsed);
    }
    return compat::unexpected("Unknown enum value '" + s + "' for property '" + std::string(propName) + "'");
}

PropertyValue PropertyEditorModel::CreateThemeTokenValue(
    ThemeTokenType tokenType,
    compat::string_view tokenName
) {
    ThemeTokenRef ref;
    ref.tokenType = tokenType;
    ref.tokenName = std::string(tokenName);
    return PropertyValue(std::move(ref));
}

} // namespace inspector
} // namespace designer
} // namespace cppui
