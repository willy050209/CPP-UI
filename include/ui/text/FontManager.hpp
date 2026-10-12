/*
 * File: FontManager.hpp
 * Purpose: Declares the cross-platform FontManager for system font discovery,
 *          fallback resolution, and font metrics caching.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
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

    /// <summary>
    /// 全域設定預設 UI 字型家族 (例如 "Inter"、"Noto Sans")。
    /// </summary>
    /// <param name="familyName">字型家族名稱視圖，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 familyName 為空時擲出。</exception>
    void SetDefaultFontFamily(compat::string_view familyName);

    /// <summary>
    /// 全域設定預設中文字型家族。
    /// </summary>
    /// <param name="familyName">字型家族名稱視圖，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 familyName 為空時擲出。</exception>
    void SetDefaultChineseFontFamily(compat::string_view familyName);

    /// <summary>
    /// 全域設定預設等寬字型家族。
    /// </summary>
    /// <param name="familyName">字型家族名稱視圖，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 familyName 為空時擲出。</exception>
    void SetDefaultMonospaceFontFamily(compat::string_view familyName);

    /// <summary>
    /// 重設字型家族為作業系統原廠預設狀態。
    /// </summary>
    void ResetDefaultFonts();

    /// <summary>
    /// 自實體檔案註冊並載入字型檔案 (TTF / OTF)，使其可供 DirectWrite、GDI 與跨平台管線使用。
    /// </summary>
    /// <param name="filePath">字型檔案路徑，不可為空。</param>
    /// <param name="customFamilyName">可選之自訂字型家族別名。</param>
    /// <returns>若註冊成功傳回 true，失敗傳回 false。</returns>
    /// <exception cref="std::invalid_argument">當 filePath 為空或檔案不存在時擲出。</exception>
    bool RegisterFontFile(compat::string_view filePath, compat::string_view customFamilyName = "");

    /// <summary>
    /// 自記憶體二進位緩衝區註冊並載入字型資料。
    /// </summary>
    /// <param name="fontData">記憶體字型資料 byte span，不可為空。</param>
    /// <param name="customFamilyName">字型家族識別名稱，不可為空。</param>
    /// <returns>若註冊成功傳回 true，失敗傳回 false。</returns>
    /// <exception cref="std::invalid_argument">當 fontData 為空或 customFamilyName 為空時擲出。</exception>
    bool RegisterFontFromMemory(compat::span<const uint8_t> fontData, compat::string_view customFamilyName);

    /// <summary>
    /// 取得所有已透過手動註冊之自訂字型家族名稱清單。
    /// </summary>
    /// <returns>字型家族名稱字串向量。</returns>
    COMPAT_NODISCARD std::vector<std::string> GetRegisteredFontFamilies() const;

    /// <summary>
    /// 解析指定字型家族名稱所對應之實體字型檔案路徑 (供跨平台光柵化後端使用)。
    /// </summary>
    /// <param name="familyName">字型家族名稱視圖，若為空傳回預設中文字型檔案路徑。</param>
    /// <returns>實體字型檔案路徑字串，若找不到則傳回空字串。</returns>
    COMPAT_NODISCARD std::string ResolveFontFilePath(compat::string_view familyName = "") const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace text
} // namespace cppui
