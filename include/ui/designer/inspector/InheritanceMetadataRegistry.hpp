/**
 * @file InheritanceMetadataRegistry.hpp
 * @brief 靜態繼承鏈屬性與事件元資料註冊表 (InheritanceMetadataRegistry)
 */

#pragma once

#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include "ui/designer/inspector/InspectorTypes.hpp"

namespace cppui {
namespace designer {
namespace inspector {

/// <summary>
/// 靜態繼承鏈屬性與事件元資料註冊表 (InheritanceMetadataRegistry)
/// 模擬控制項繼承體系 (Visual -> UIElement -> Control -> ContentControl -> 衍生控制項)，
/// 提供完整的屬性/事件聚合查詢、多選交集計算、動態情境感知附加屬性解析與預設處理常式自動命名。
/// </summary>
class CPPUI_API InheritanceMetadataRegistry {
public:
    /// <summary>
    /// 取得指定節點型別沿繼承鏈聚合的所有公開屬性描述項清單。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <returns>完整屬性描述項清單。</returns>
    COMPAT_NODISCARD static const std::vector<PropertyDescriptor>& GetDescriptorsForType(NodeType type);

    /// <summary>
    /// 取得指定節點型別所支援的所有路由事件與 Command 描述項清單。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <returns>事件描述項清單。</returns>
    COMPAT_NODISCARD static const std::vector<EventDescriptor>& GetEventDescriptorsForType(NodeType type);

    /// <summary>
    /// 計算多個節點型別共同擁有的屬性交集 (Multi-Selection Property Intersection)。
    /// </summary>
    /// <param name="types">選取的節點型別清單。</param>
    /// <returns>交集屬性描述項清單，順序依繼承鏈層級與分類穩定排列。</returns>
    COMPAT_NODISCARD static std::vector<PropertyDescriptor> ComputePropertyIntersection(
        const std::vector<NodeType>& types
    );

    /// <summary>
    /// 計算多個節點型別共同擁有的事件交集 (Multi-Selection Event Intersection)。
    /// </summary>
    /// <param name="types">選取的節點型別清單。</param>
    /// <returns>交集事件描述項清單。</returns>
    COMPAT_NODISCARD static std::vector<EventDescriptor> ComputeEventIntersection(
        const std::vector<NodeType>& types
    );

    /// <summary>
    /// 依據父容器節點型別取得動態掛載的附加屬性描述項清單。
    /// </summary>
    /// <param name="parentType">父容器節點型別 (可為 nullopt 表示無父容器或非特定佈局)。</param>
    /// <returns>應動態呈現之附加屬性描述項清單。</returns>
    COMPAT_NODISCARD static std::vector<PropertyDescriptor> GetAttachedPropertiesForParent(
        compat::optional<NodeType> parentType
    );

    /// <summary>
    /// 根據控制項變數名稱與事件名稱自動生成標準風格之事件處理常式名稱 (例如 "OnSubmitBtnClick")。
    /// </summary>
    /// <param name="nodeName">控制項變數名稱 (例如 "submitBtn")。</param>
    /// <param name="eventName">事件名稱 (例如 "Click")。</param>
    /// <returns>建議之事件處理常式名稱。</returns>
    COMPAT_NODISCARD static std::string GenerateDefaultHandlerName(
        compat::string_view nodeName,
        compat::string_view eventName
    );

    /// <summary>
    /// 取得屬性型別的標準友好名稱 (例如 "double", "string", "Thickness", "Color")。
    /// </summary>
    /// <param name="val">樣品屬性值。</param>
    /// <returns>字串視圖。</returns>
    COMPAT_NODISCARD static std::string GetFriendlyTypeName(const PropertyValue& val);

    /// <summary>
    /// 查詢列舉屬性之所有可用選項字串清單。
    /// </summary>
    /// <param name="propName">屬性名稱。</param>
    /// <returns>選項清單，若非已知列舉屬性則回傳空清單。</returns>
    COMPAT_NODISCARD static std::vector<std::string> GetEnumOptions(compat::string_view propName);

    /// <summary>
    /// 查詢特定分類的主題 Token 建議識別名稱清單。
    /// </summary>
    /// <param name="propName">屬性名稱。</param>
    /// <returns>建議之主題 Token 清單。</returns>
    COMPAT_NODISCARD static std::vector<std::string> GetThemeTokenSuggestions(compat::string_view propName);
};

} // namespace inspector
} // namespace designer
} // namespace cppui
