#pragma once

#include <memory>
#include "compat/compat.hpp"
#include "ui/text/Font.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 跨平台系統字型管理與快取中樞
/// </summary>
class FontManager {
public:
    static FontManager& Get();

    FontManager();
    ~FontManager();

    // 禁止複製
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    /// <summary>
    /// 取得系統預設 UI 字型規格 (Windows: Segoe UI, macOS: .SF NS Text, Linux: Ubuntu/DejaVu)
    /// </summary>
    COMPAT_NODISCARD FontDescriptor GetDefaultFont(float size = 14.0f) const;

    /// <summary>
    /// 取得系統預設繁體中文 UI 字型 (Windows: Microsoft JhengHei UI, macOS: PingFang TC)
    /// </summary>
    COMPAT_NODISCARD FontDescriptor GetDefaultChineseFont(float size = 14.0f) const;

    /// <summary>
    /// 取得系統預設等寬字型 (Windows: Consolas / Cascadia Code, macOS: Menlo)
    /// </summary>
    COMPAT_NODISCARD FontDescriptor GetDefaultMonospaceFont(float size = 14.0f) const;

    /// <summary>
    /// 檢驗系統字型庫中是否存在特定字型家族
    /// </summary>
    COMPAT_NODISCARD bool HasFontFamily(compat::string_view familyName) const;

    /// <summary>
    /// 取得特定字型之度量數值
    /// </summary>
    COMPAT_NODISCARD FontMetrics GetFontMetrics(const FontDescriptor& font) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
