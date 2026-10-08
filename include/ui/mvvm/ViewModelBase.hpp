#pragma once

#include "compat/compat.hpp"
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
    virtual ~ViewModelBase() = default;

    using PropertyChangedCallback = std::function<void(compat::string_view)>;

    /// <summary>
    /// 註冊屬性變更回呼監聽器
    /// </summary>
    void AddPropertyChangedHandler(PropertyChangedCallback handler);

protected:
    /// <summary>
    /// 廣播屬性變更通知
    /// </summary>
    void NotifyPropertyChanged(compat::string_view propertyName);

    /// <summary>
    /// 設定屬性值並在值改變時自動觸發變更通知
    /// </summary>
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
