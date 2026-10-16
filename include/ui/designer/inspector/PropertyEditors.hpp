/**
 * @file PropertyEditors.hpp
 * @brief 專屬型別屬性編輯器模型與格式驗證器 (PropertyEditors)
 */

#pragma once

#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"
#include "ui/designer/inspector/InspectorTypes.hpp"

namespace cppui {
namespace designer {
namespace inspector {

/// <summary>
/// 屬性輸入防禦性格式驗證工具集 (PropertyValidators)
/// 提供針對數值、維度約束、Thickness 留白、色碼與識別字的無例外純函數驗證。
/// </summary>
class CPPUI_API PropertyValidators {
public:
    /// <summary>
    /// 驗證浮點數數值格式與範圍 (嚴格防禦 NaN、Inf 及非數字字元)。
    /// </summary>
    /// <param name="text">輸入文字。</param>
    /// <param name="minVal">允許最小值。</param>
    /// <param name="maxVal">允許最大值。</param>
    /// <returns>驗證結果與錯誤訊息。</returns>
    COMPAT_NODISCARD static ValidationResult ValidateDouble(
        compat::string_view text,
        double minVal = -1e9,
        double maxVal = 1e9
    ) noexcept;

    /// <summary>
    /// 驗證整數數值格式與範圍。
    /// </summary>
    /// <param name="text">輸入文字。</param>
    /// <param name="minVal">允許最小值。</param>
    /// <param name="maxVal">允許最大值。</param>
    /// <returns>驗證結果與錯誤訊息。</returns>
    COMPAT_NODISCARD static ValidationResult ValidateInteger(
        compat::string_view text,
        int64_t minVal = -2147483648LL,
        int64_t maxVal = 2147483647LL
    ) noexcept;

    /// <summary>
    /// 驗證尺寸維度 (Width/Height)，強制不得為負數且不得為 NaN/Inf。
    /// </summary>
    /// <param name="text">輸入文字。</param>
    /// <returns>驗證結果。</returns>
    COMPAT_NODISCARD static ValidationResult ValidateDimension(compat::string_view text) noexcept;

    /// <summary>
    /// 驗證 Thickness 外距留白字串 (支援 "5", "5, 10", "5, 10, 5, 10")。
    /// </summary>
    /// <param name="text">輸入文字。</param>
    /// <returns>驗證結果。</returns>
    COMPAT_NODISCARD static ValidationResult ValidateThickness(compat::string_view text) noexcept;

    /// <summary>
    /// 驗證十六進位色彩字串 (如 "#FF0000", "#80FF0000", "#FFF")。
    /// </summary>
    /// <param name="text">輸入文字。</param>
    /// <returns>驗證結果。</returns>
    COMPAT_NODISCARD static ValidationResult ValidateColorHex(compat::string_view text) noexcept;

    /// <summary>
    /// 驗證 C++ 識別字 (用於事件處理常式名稱與節點名稱)。
    /// </summary>
    /// <param name="name">欲驗證之識別字。</param>
    /// <returns>驗證結果。</returns>
    COMPAT_NODISCARD static ValidationResult ValidateIdentifier(compat::string_view name) noexcept;
};

/// <summary>
/// 專屬型別屬性編輯器模型 (PropertyEditorModel)
/// 封裝單選/多選 Mixed 數值呈現、字串雙向序列化解析與防禦性型別轉換。
/// </summary>
class CPPUI_API PropertyEditorModel {
public:
    /// <summary>
    /// 格式化屬性值為 Inspector 顯示字串 (若為 Mixed 則顯示 "<Multiple Values>")。
    /// </summary>
    /// <param name="value">當前屬性值。</param>
    /// <param name="isMixed">是否處於多重數值狀態。</param>
    /// <returns>顯示字串。</returns>
    COMPAT_NODISCARD static std::string FormatDisplayString(
        const PropertyValue& value,
        bool isMixed = false
    );

    /// <summary>
    /// 解析文字字串為布林 PropertyValue。
    /// </summary>
    COMPAT_NODISCARD static compat::expected<PropertyValue, std::string> ParseBool(
        compat::string_view text
    ) noexcept;

    /// <summary>
    /// 解析文字字串為浮點數或整數 PropertyValue。
    /// </summary>
    COMPAT_NODISCARD static compat::expected<PropertyValue, std::string> ParseNumber(
        compat::string_view text,
        bool isInteger = false
    ) noexcept;

    /// <summary>
    /// 解析 Thickness 字串 (支援 "5", "5, 10", "5, 10, 5, 10") 為 PropertyValue。
    /// </summary>
    COMPAT_NODISCARD static compat::expected<PropertyValue, std::string> ParseThickness(
        compat::string_view text
    ) noexcept;

    /// <summary>
    /// 解析十六進位色彩字串為 Color PropertyValue。
    /// </summary>
    COMPAT_NODISCARD static compat::expected<PropertyValue, std::string> ParseColor(
        compat::string_view hex
    ) noexcept;

    /// <summary>
    /// 解析列舉字串為對應型別之 PropertyValue。
    /// </summary>
    COMPAT_NODISCARD static compat::expected<PropertyValue, std::string> ParseEnum(
        compat::string_view propName,
        compat::string_view text
    ) noexcept;

    /// <summary>
    /// 建立參照主題代幣的 PropertyValue。
    /// </summary>
    COMPAT_NODISCARD static PropertyValue CreateThemeTokenValue(
        ThemeTokenType tokenType,
        compat::string_view tokenName
    );
};

} // namespace inspector
} // namespace designer
} // namespace cppui
