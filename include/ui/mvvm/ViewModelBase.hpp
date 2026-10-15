/**
 * @file ViewModelBase.hpp
 * @brief MVVM 視圖模型基底類別，提供屬性變更通知機制 (INotifyPropertyChanged)
 */

#pragma once

#include "compat/Compat.hpp"
#include <functional>
#include <string>
#include <vector>

namespace cppui {
namespace mvvm {

/// <summary>
/// MVVM 視圖模型基底類別 (ViewModelBase)
/// 提供屬性變更通知機制 (對標 .NET INotifyPropertyChanged / ObservableObject)。
/// </summary>
class ViewModelBase {
public:
    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    virtual ~ViewModelBase() = default;

    /// <summary>
    /// 屬性變更通知回呼委派。
    /// </summary>
    using PropertyChangedCallback = std::function<void(compat::string_view)>;

    /// <summary>
    /// 註冊屬性變更回呼監聽器。
    /// </summary>
    /// <param name="handler">屬性變更處理常式。</param>
    void AddPropertyChangedHandler(PropertyChangedCallback handler);

protected:
    /// <summary>
    /// 廣播屬性變更通知。
    /// </summary>
    /// <param name="propertyName">變更之屬性名稱視圖。</param>
    void NotifyPropertyChanged(compat::string_view propertyName);

    /// <summary>
    /// 設定屬性值並在值改變時自動觸發變更通知。
    /// </summary>
    /// <typeparam name="T">屬性型別。</typeparam>
    /// <param name="storage">存放屬性值的變數引用。</param>
    /// <param name="value">新數值。</param>
    /// <param name="propertyName">屬性名稱視圖。</param>
    /// <returns>若值有變更傳回 true，否則傳回 false。</returns>
    template<typename T>
    bool SetProperty(T& storage, const T& value, compat::string_view propertyName) {
        if (storage == value) {
            return false;
        }
        storage = value;
        NotifyPropertyChanged(propertyName);
        return true;
    }

private:
    std::vector<PropertyChangedCallback> m_handlers;
};

} // namespace mvvm
} // namespace cppui
