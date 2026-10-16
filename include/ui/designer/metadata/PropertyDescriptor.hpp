/**
 * @file PropertyDescriptor.hpp
 * @brief 控制項屬性元資料描述項 (PropertyDescriptor) 與全域屬性註冊表
 */

#pragma once

#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 屬性分類目錄列舉 (供 Inspector 屬性編輯面板分組與篩選)
/// </summary>
enum class PropertyCategory : uint8_t {
    Common = 0,     // 常用基礎屬性 (Name, IsEnabled, Visibility)
    Layout = 1,     // 佈局排版屬性 (Width, Height, Margin, HorizontalAlignment)
    Appearance = 2, // 視覺外觀屬性 (Background, BorderBrush, Opacity)
    Text = 3,       // 文字排版屬性 (Text, FontSize, FontFamily, FontWeight)
    Behavior = 4    // 行為屬性 (IsChecked, Minimum, Maximum, Value)
};

/// <summary>
/// 屬性元資料描述項 (PropertyDescriptor)
/// 封裝屬性名稱、預設值、所屬分類與唯讀標記。
/// </summary>
struct PropertyDescriptor {
    std::string name;
    PropertyValue defaultValue;
    PropertyCategory category = PropertyCategory::Common;
    bool isReadOnly = false;

    COMPAT_NODISCARD bool operator==(const PropertyDescriptor& other) const noexcept {
        return name == other.name && defaultValue == other.defaultValue &&
               category == other.category && isReadOnly == other.isReadOnly;
    }
};

/// <summary>
/// 全域控制項屬性元資料註冊表 (PropertyMetadataRegistry)
/// 提供針對各 NodeType 的標準屬性查詢與預設值比對。
/// </summary>
class CPPUI_API PropertyMetadataRegistry {
public:
    /// <summary>
    /// 取得指定控制項型別之所有屬性描述項清單。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <returns>屬性描述項清單。</returns>
    COMPAT_NODISCARD static const std::vector<PropertyDescriptor>& GetDescriptors(NodeType type);

    /// <summary>
    /// 查詢指定控制項屬性之預設值。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <param name="propName">屬性名稱。</param>
    /// <returns>若屬性存在回傳預設值，否則回傳 nullopt。</returns>
    COMPAT_NODISCARD static compat::optional<PropertyValue> GetDefaultValue(NodeType type, compat::string_view propName);

    /// <summary>
    /// 檢查某數值是否等於該屬性之預設值 (用於序列化時之 Default Value Elision)。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <param name="propName">屬性名稱。</param>
    /// <param name="value">欲比對之數值。</param>
    /// <returns>若相等回傳 true。</returns>
    COMPAT_NODISCARD static bool IsDefaultValue(NodeType type, compat::string_view propName, const PropertyValue& value);
};

} // namespace designer
} // namespace cppui
