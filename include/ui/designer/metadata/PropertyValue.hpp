/**
 * @file PropertyValue.hpp
 * @brief 型別安全屬性值多型變體封裝 (PropertyValue) 與主題 Token 參照
 */

#pragma once

#include <variant>
#include <string>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/text/Font.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 畫刷規格描述項型別
/// </summary>
enum class BrushDescriptorType : uint8_t {
    Solid = 0,
    LinearGradient = 1
};

/// <summary>
/// 畫刷規格描述項
/// </summary>
struct BrushDescriptor {
    BrushDescriptorType type = BrushDescriptorType::Solid;
    core::Color color{ 0, 0, 0, 255 };
    core::Color gradientEndColor{ 0, 0, 0, 255 };
    core::Point startPoint{ 0.0f, 0.0f };
    core::Point endPoint{ 0.0f, 1.0f };

    COMPAT_NODISCARD bool operator==(const BrushDescriptor& other) const noexcept {
        return type == other.type &&
               color == other.color &&
               gradientEndColor == other.gradientEndColor &&
               startPoint == other.startPoint &&
               endPoint == other.endPoint;
    }

    COMPAT_NODISCARD bool operator!=(const BrushDescriptor& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 主題代幣分類列舉
/// </summary>
enum class ThemeTokenType : uint8_t {
    Color = 0,
    Metric = 1,
    Typography = 2
};

/// <summary>
/// 主題代幣鍵值參照項 (ThemeTokenRef)
/// 允許屬性值直接參照主題系統，支援動態深淺色切換。
/// </summary>
struct ThemeTokenRef {
    ThemeTokenType tokenType = ThemeTokenType::Color;
    std::string tokenName; // 例如 "accentPrimary", "controlBackground", "spacingMd"

    COMPAT_NODISCARD bool operator==(const ThemeTokenRef& other) const noexcept {
        return tokenType == other.tokenType && tokenName == other.tokenName;
    }

    COMPAT_NODISCARD bool operator!=(const ThemeTokenRef& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 型別安全屬性值多型變體封裝 (PropertyValue)
/// </summary>
class CPPUI_API PropertyValue {
public:
    using VariantType = std::variant<
        std::monostate,
        bool,
        int32_t,
        int64_t,
        double,
        std::string,
        core::Point,
        core::Size,
        core::Rect,
        core::Thickness,
        layout::HorizontalAlignment,
        layout::VerticalAlignment,
        layout::Orientation,
        layout::Visibility,
        core::Color,
        BrushDescriptor,
        text::FontDescriptor,
        ThemeTokenRef
    >;

    PropertyValue() = default;

    PropertyValue(bool val) : m_value(val) {}
    PropertyValue(int32_t val) : m_value(val) {}
    PropertyValue(int64_t val) : m_value(val) {}
    PropertyValue(double val) : m_value(val) {}
    PropertyValue(float val) : m_value(static_cast<double>(val)) {}
    PropertyValue(const char* val) : m_value(std::string(val)) {}
    PropertyValue(std::string val) : m_value(std::move(val)) {}
    PropertyValue(compat::string_view val) : m_value(std::string(val)) {}
    PropertyValue(core::Point val) : m_value(val) {}
    PropertyValue(core::Size val) : m_value(val) {}
    PropertyValue(core::Rect val) : m_value(val) {}
    PropertyValue(core::Thickness val) : m_value(val) {}
    PropertyValue(layout::HorizontalAlignment val) : m_value(val) {}
    PropertyValue(layout::VerticalAlignment val) : m_value(val) {}
    PropertyValue(layout::Orientation val) : m_value(val) {}
    PropertyValue(layout::Visibility val) : m_value(val) {}
    PropertyValue(core::Color val) : m_value(val) {}
    PropertyValue(BrushDescriptor val) : m_value(val) {}
    PropertyValue(text::FontDescriptor val) : m_value(std::move(val)) {}
    PropertyValue(ThemeTokenRef val) : m_value(std::move(val)) {}

    /// <summary>判斷內部數值是否為空 (monostate)。</summary>
    COMPAT_NODISCARD bool IsEmpty() const noexcept {
        return std::holds_alternative<std::monostate>(m_value);
    }

    /// <summary>檢查目前持有型別是否符合 T。</summary>
    template <typename T>
    COMPAT_NODISCARD bool Is() const noexcept {
        return std::holds_alternative<T>(m_value);
    }

    /// <summary>安全取得內部數值，型別不符時回傳 TypeMismatch 錯誤。</summary>
    template <typename T>
    COMPAT_NODISCARD compat::expected<T, DesignerError> Get() const noexcept {
        if (const T* val = std::get_if<T>(&m_value)) {
            return *val;
        }
        DesignerError err;
        err.code = DesignerErrorCode::TypeMismatch;
        err.message = "PropertyValue type mismatch during Get<T>()";
        return compat::unexpected(std::move(err));
    }

    /// <summary>取得直接變體參考。</summary>
    COMPAT_NODISCARD const VariantType& GetVariant() const noexcept {
        return m_value;
    }

    /// <summary>取得目前持有的型別名稱。</summary>
    COMPAT_NODISCARD compat::string_view GetTypeName() const noexcept;

    /// <summary>轉換為標準字串表示式。</summary>
    COMPAT_NODISCARD std::string ToString() const;

    /// <summary>相等性比較。</summary>
    COMPAT_NODISCARD bool operator==(const PropertyValue& other) const noexcept {
        return m_value == other.m_value;
    }

    /// <summary>不相等性比較。</summary>
    COMPAT_NODISCARD bool operator!=(const PropertyValue& other) const noexcept {
        return !(*this == other);
    }

private:
    VariantType m_value;
};

} // namespace designer
} // namespace cppui
