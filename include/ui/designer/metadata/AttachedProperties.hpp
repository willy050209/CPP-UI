/**
 * @file AttachedProperties.hpp
 * @brief 附加屬性 (Attached Properties) 描述符與標準定義常數 (Grid/Canvas/ToolTip)
 */

#pragma once

#include <string>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 標準附加屬性常數名稱與靜態輔助驗證器
/// </summary>
class CPPUI_API AttachedProperties {
public:
    COMPAT_INLINE_VAR static constexpr const char* GridRow = "Grid.Row";
    COMPAT_INLINE_VAR static constexpr const char* GridColumn = "Grid.Column";
    COMPAT_INLINE_VAR static constexpr const char* GridRowSpan = "Grid.RowSpan";
    COMPAT_INLINE_VAR static constexpr const char* GridColumnSpan = "Grid.ColumnSpan";
    COMPAT_INLINE_VAR static constexpr const char* CanvasLeft = "Canvas.Left";
    COMPAT_INLINE_VAR static constexpr const char* CanvasTop = "Canvas.Top";
    COMPAT_INLINE_VAR static constexpr const char* ToolTipText = "ToolTip.Text";

    /// <summary>
    /// 驗證附加屬性數值合法性。
    /// </summary>
    /// <param name="propName">附加屬性名稱。</param>
    /// <param name="val">傳入數值。</param>
    /// <returns>若合法回傳 void，否則回傳錯誤代碼。</returns>
    COMPAT_NODISCARD static compat::expected<void, DesignerError> Validate(compat::string_view propName, const PropertyValue& val) noexcept {
        if (propName == GridRow || propName == GridColumn) {
            auto res = val.Get<int32_t>();
            if (!res.has_value() || res.value() < 0) {
                DesignerError err;
                err.code = DesignerErrorCode::TypeMismatch;
                err.message = std::string(propName) + " must be a non-negative int32_t";
                return compat::unexpected(std::move(err));
            }
            return {};
        }
        if (propName == GridRowSpan || propName == GridColumnSpan) {
            auto res = val.Get<int32_t>();
            if (!res.has_value() || res.value() < 1) {
                DesignerError err;
                err.code = DesignerErrorCode::TypeMismatch;
                err.message = std::string(propName) + " must be a positive int32_t (>= 1)";
                return compat::unexpected(std::move(err));
            }
            return {};
        }
        if (propName == CanvasLeft || propName == CanvasTop) {
            auto res = val.Get<double>();
            if (!res.has_value()) {
                DesignerError err;
                err.code = DesignerErrorCode::TypeMismatch;
                err.message = std::string(propName) + " must be a double";
                return compat::unexpected(std::move(err));
            }
            return {};
        }
        if (propName == ToolTipText) {
            auto res = val.Get<std::string>();
            if (!res.has_value()) {
                DesignerError err;
                err.code = DesignerErrorCode::TypeMismatch;
                err.message = std::string(propName) + " must be a string";
                return compat::unexpected(std::move(err));
            }
            return {};
        }
        return {};
    }
};

} // namespace designer
} // namespace cppui
