/**
 * @file DialogCommon.hpp
 * @brief 定義對話框模組共用列舉、資料結構與輔助工具純函數
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/app/FormEventArgs.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace cppui {
namespace dialogs {

/// <summary>
/// 訊息方塊按鈕選項列舉。
/// </summary>
enum class MessageBoxButtons : uint8_t {
    OK,
    OKCancel,
    YesNo,
    YesNoCancel,
    RetryCancel,
    AbortRetryIgnore
};

/// <summary>
/// 訊息方塊圖示外觀選項列舉。
/// </summary>
enum class MessageBoxIcon : uint8_t {
    None,
    Information,
    Warning,
    Error,
    Question
};

/// <summary>
/// 訊息方塊預設焦點按鈕列舉。
/// </summary>
enum class MessageBoxDefaultButton : uint8_t {
    Button1,
    Button2,
    Button3
};

/// <summary>
/// 對話方塊執行後端模式列舉。
/// </summary>
enum class DialogBackendMode : uint8_t {
    Auto,      // 自動判定：Windows GUI 桌面環境使用 Native，否則使用 BuiltIn Form
    Native,    // 強制使用作業系統原生對話框
    BuiltIn    // 強制使用純 CPPUI Form 佈景主題對話框
};

/// <summary>
/// 單一檔案篩選條件結構。
/// </summary>
struct FileFilterItem {
    std::string description;            // 描述文字，例如 "Text Files (*.txt)"
    std::vector<std::string> patterns;  // 樣式清單，例如 {"*.txt"} 或 {"*.jpg", "*.png"}
};

/// <summary>
/// 檔案對話框輔助純函數工具類別 (Pure Static Functions)。
/// </summary>
class DialogUtilities {
public:
    /// <summary>
    /// 解析檔案篩選字串為結構化清單。
    /// 格式範例："Text Files (*.txt)|*.txt|All Files (*.*)|*.*"
    /// </summary>
    /// <param name="filter">標準格式之篩選字串視圖。</param>
    /// <returns>結構化檔案篩選項目清單。</returns>
    COMPAT_NODISCARD static std::vector<FileFilterItem> ParseFilterString(compat::string_view filter);

    /// <summary>
    /// 檢查檔名是否符合通配符樣式 (例如 "*.txt" 或 "*.*")。
    /// </summary>
    /// <param name="fileName">欲檢驗之檔名字串視圖。</param>
    /// <param name="pattern">單一樣式字串視圖。</param>
    /// <returns>若符合樣式傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool MatchesPattern(compat::string_view fileName, compat::string_view pattern);

    /// <summary>
    /// 檢查檔名是否符合篩選項目的任一樣式。
    /// </summary>
    /// <param name="fileName">欲檢驗之檔名字串視圖。</param>
    /// <param name="item">檔案篩選項目。</param>
    /// <returns>若符合任一樣式傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool MatchesFilter(compat::string_view fileName, const FileFilterItem& item);

    /// <summary>
    /// 確保路徑附帶預設副檔名 (若使用者輸入之路徑未包含副檔名)。
    /// </summary>
    /// <param name="filePath">原始檔案路徑字串視圖。</param>
    /// <param name="defaultExt">預設副檔名字串視圖 (可包含或不包含開頭點號)。</param>
    /// <returns>補齊副檔名後之路徑字串。</returns>
    COMPAT_NODISCARD static std::string EnsureExtension(compat::string_view filePath, compat::string_view defaultExt);

    /// <summary>
    /// 格式化檔案位元組大小為易讀字串 (如 "1.2 MB", "450 KB")。
    /// </summary>
    /// <param name="bytes">檔案位元組數。</param>
    /// <returns>易讀之檔案大小格式化字串。</returns>
    COMPAT_NODISCARD static std::string FormatFileSize(uint64_t bytes);
};

/// <summary>
/// 全域對話框服務管理者 (DialogManager)。
/// </summary>
class DialogManager {
public:
    /// <summary>
    /// 設定全域預設對話框後端模式 (預設為 Auto)。
    /// </summary>
    /// <param name="mode">DialogBackendMode 列舉值。</param>
    static void SetDefaultBackendMode(DialogBackendMode mode) noexcept;

    /// <summary>
    /// 取得全域預設對話框後端模式。
    /// </summary>
    /// <returns>DialogBackendMode 列舉值。</returns>
    COMPAT_NODISCARD static DialogBackendMode GetDefaultBackendMode() noexcept;
};

} // namespace dialogs
} // namespace cppui
