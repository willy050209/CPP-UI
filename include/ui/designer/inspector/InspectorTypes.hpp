/**
 * @file InspectorTypes.hpp
 * @brief CPPUI Designer 屬性與事件檢查器核心型別定義
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"

namespace cppui {
namespace designer {
namespace inspector {

/// <summary>
/// 檢查器分頁模式列舉
/// </summary>
enum class InspectorTab : uint8_t {
    Properties = 0, // 屬性編輯分頁
    Events     = 1  // 事件與 MVVM 綁定分頁
};

/// <summary>
/// 多選屬性數值狀態 (MultiValueState)
/// </summary>
enum class MultiValueState : uint8_t {
    SingleValue = 0, // 單一一致數值
    Mixed       = 1, // 多選時具有不同數值 (顯示 Mixed / 多重數值佔位符)
    Unset       = 2  // 未設定或繼承預設值
};

/// <summary>
/// 屬性輸入驗證結果
/// </summary>
struct ValidationResult {
    bool isValid = true;
    std::string errorMessage;

    COMPAT_NODISCARD static ValidationResult Ok() noexcept {
        return { true, "" };
    }

    COMPAT_NODISCARD static ValidationResult Error(std::string msg) noexcept {
        return { false, std::move(msg) };
    }
};

/// <summary>
/// 檢查器受檢屬性規格描述項 (InspectedProperty)
/// 封裝單一屬性在檢查器介面上的顯示元資料、當前數值與多選狀態。
/// </summary>
struct CPPUI_API InspectedProperty {
    std::string name;                                // 屬性內部識別名稱 (如 "Width", "Canvas.Left")
    std::string displayName;                         // 顯示名稱
    PropertyCategory category = PropertyCategory::Common; // 分組目錄
    std::string typeName;                            // 數值型別名稱 (如 "double", "Thickness", "Color")
    PropertyValue defaultValue;                      // 預設值
    PropertyValue currentValue;                      // 當前生效數值
    MultiValueState valueState = MultiValueState::SingleValue; // 單值或多重值狀態
    bool isAttached = false;                         // 是否為附加屬性
    bool isReadOnly = false;                         // 是否為唯讀屬性
    std::vector<std::string> enumOptions;            // 若為列舉型別時的可選字串清單
    std::vector<std::string> themeTokenSuggestions;  // 主題代幣建議清單 (如 ColorTokens / MetricTokens)

    /// <summary>
    /// 檢查此屬性是否處於多重數值 (Mixed) 狀態。
    /// </summary>
    COMPAT_NODISCARD bool IsMixed() const noexcept {
        return valueState == MultiValueState::Mixed;
    }
};

/// <summary>
/// 事件描述項 (EventDescriptor)
/// 描述控制項所支援之路由事件或 MVVM Command 介面。
/// </summary>
struct CPPUI_API EventDescriptor {
    std::string eventName;          // 事件名稱 (例如 "Click", "TextChanged")
    std::string displayName;        // 顯示名稱 (例如 "Click (點擊事件)")
    std::string eventCategory;      // 分類 (例如 "Mouse", "Keyboard", "Action")
    std::string parameterSignature; // 委派簽章 (例如 "void(RoutedEventArgs&)")
    std::string defaultHandlerPrefix; // 預設處理函式字首 (例如 "OnClick")
    bool isCommand = false;         // 是否支援 MVVM ICommand 綁定
};

/// <summary>
/// 事件與 MVVM 綁定狀態 (EventBindingState)
/// 呈現目前控制項在特定事件上的綁定現況。
/// </summary>
struct CPPUI_API EventBindingState {
    EventDescriptor descriptor;
    std::string boundHandlerOrCommand; // 目前綁定的處理常式或 ViewModel 路徑
    bool isBound = false;              // 是否已建立綁定
    bool isMvvmCommand = false;        // 是否為 Command 綁定 (反之為程式碼後置事件處理常式)
};

} // namespace inspector
} // namespace designer
} // namespace cppui
