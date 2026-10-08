#pragma once

#include "compat/compat.hpp"
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
    /// 依據 std::type_info 解析服務指標 (純虛擬)
    /// </summary>
    COMPAT_NODISCARD virtual std::shared_ptr<void> GetService(const std::type_info& type) = 0;

    /// <summary>
    /// 樣板輔助函式：依據型別 T 解析服務指標
    /// </summary>
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetService() {
        auto svc = GetService(typeid(T));
        return std::static_pointer_cast<T>(svc);
    }

    /// <summary>
    /// 樣板輔助函式：依據型別 T 解析必要服務，若未註冊則報錯並回傳 nullptr
    /// </summary>
    template<typename T>
    COMPAT_NODISCARD std::shared_ptr<T> GetRequiredService() {
        auto svc = GetService<T>();
        if (!svc) {
            compat::println("[IoC] 錯誤：未找到服務型別 '{}'，請確認已在 ServiceCollection 中註冊！", typeid(T).name());
        }
        return svc;
    }
};

} // namespace ioc
} // namespace cppui
