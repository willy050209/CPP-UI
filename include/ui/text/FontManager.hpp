/*
 * File: FontManager.hpp
 * Purpose: Declares the cross-platform FontManager for system font discovery,
 *          fallback resolution, and font metrics caching.
 */

#pragma once

#include <memory>
#include "compat/compat.hpp"
#include "ui/text/Font.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 跨平台系統字型管理與快取中樞 (FontManager)
/// 提供作業系統字型枚舉、預設 UI 字型解析與度量資訊快取。
/// </summary>
class FontManager {
public:
    /// <summary>
    /// 取得全域 FontManager 單例實例。
    /// </summary>
    /// <returns>FontManager 實例引用。</returns>
    static FontManager& Get();

    /// <summary>
    /// 建構字型管理器實例。
    /// </summary>
    FontManager();

    /// <summary>
    /// 解構字型管理器實例。
    /// </summary>
    ~FontManager();

    // 禁止複製
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    /// <summary>
    /// 取得系統預設 UI 字型規格 (Windows: Segoe UI, macOS: .SF NS Text, Linux: Ubuntu/DejaVu)。
    /// </summary>
    /// <param name="size">字型大小 (pt/px)，預設為 14.0f。</param>
    /// <returns>系統預設 UI 字型描述符。</returns>
    COMPAT_NODISCARD FontDescriptor GetDefaultFont(float size = 14.0f) const;

    /// <summary>
    /// 取得系統預設繁體中文 UI 字型 (Windows: Microsoft JhengHei UI, macOS: PingFang TC)。
    /// </summary>
    /// <param name="size">字型大小 (pt/px)，預設為 14.0f。</param>
    /// <returns>系統預設繁體中文字型描述符。</returns>
    COMPAT_NODISCARD FontDescriptor GetDefaultChineseFont(float size = 14.0f) const;

    /// <summary>
    /// 取得系統預設等寬字型 (Windows: Consolas / Cascadia Code, macOS: Menlo)。
    /// </summary>
    /// <param name="size">字型大小 (pt/px)，預設為 14.0f。</param>
    /// <returns>系統預設等寬字型描述符。</returns>
    COMPAT_NODISCARD FontDescriptor GetDefaultMonospaceFont(float size = 14.0f) const;

    /// <summary>
    /// 檢驗系統字型庫中是否存在特定字型家族。
    /// </summary>
    /// <param name="familyName">字型家族名稱。</param>
    /// <returns>若字型存在傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool HasFontFamily(compat::string_view familyName) const;

    /// <summary>
    /// 取得特定字型之度量數值。
    /// </summary>
    /// <param name="font">目標字型描述符。</param>
    /// <returns>對應之字型度量資訊結構。</returns>
    COMPAT_NODISCARD FontMetrics GetFontMetrics(const FontDescriptor& font) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
