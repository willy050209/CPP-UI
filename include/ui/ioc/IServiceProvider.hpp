/**
 * @file IServiceProvider.hpp
 * @brief 服務提供者抽象介面，提供執行期型別安全的依賴解析機制
 */

#pragma once

#include "compat/Compat.hpp"
#include <memory>
#include <typeinfo>

namespace cppui {
namespace ioc {

/// <summary>
/// 服務提供者抽象介面 (IServiceProvider)
/// 提供執行期型別安全的依賴解析機制。
/// </summary>
class IServiceProvider {
public:
    virtual ~IServiceProvider() = default;

    /// <summary>
    /// 依據 std::type_info 解析服務指標。
    /// </summary>
    /// <param name="type">服務介面或實作之 type_info 物件。</param>
    /// <returns>指向已解析服務之泛型共享指標，若未註冊則回傳 nullptr。</returns>
    COMPAT_NODISCARD virtual std::shared_ptr<void> GetService(const std::type_info& type) = 0;

    /// <summary>
    /// 依據型別 T 解析服務實例指標。
    /// </summary>
    /// <typeparam name="T">目標服務介面或實作型別。</typeparam>
    /// <returns>指向目標服務型別之 shared_ptr，若未註冊則回傳 nullptr。</returns>
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetService() {
        auto svc = GetService(typeid(T));
        return std::static_pointer_cast<T>(svc);
    }

    /// <summary>
    /// 依據型別 T 解析必要服務，若未註冊則回傳 nullptr。
    /// </summary>
    /// <typeparam name="T">目標服務型別。</typeparam>
    /// <returns>指向目標服務型別之 shared_ptr，若未註冊則回傳 nullptr。</returns>
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetRequiredService() {
        auto svc = GetService<T>();
        return svc;
    }
};

} // namespace ioc
} // namespace cppui
