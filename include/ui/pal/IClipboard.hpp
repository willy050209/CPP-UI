/*
 * File: IClipboard.hpp
 * Purpose: Declares IClipboard interface and Clipboard static access point for system clipboard interactions.
 */

#pragma once

#include <string>
#include <memory>
#include "compat/Compat.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 平台原生剪貼簿介面 (IClipboard)
/// 提供純文字之查詢、取得、設定與清空操作。
/// </summary>
class IClipboard {
public:
    /// <summary>虛擬解構子。</summary>
    virtual ~IClipboard() = default;

    /// <summary>
    /// 查詢剪貼簿目前是否包含純文字內容。
    /// </summary>
    /// <returns>若包含文字傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD virtual bool ContainsText() const = 0;

    /// <summary>
    /// 自剪貼簿取得純文字內容 (UTF-8 編碼)。
    /// </summary>
    /// <returns>文字字串，若無文字傳回空字串。</returns>
    COMPAT_NODISCARD virtual std::string GetText() const = 0;

    /// <summary>
    /// 設定純文字內容至剪貼簿 (UTF-8 編碼)。
    /// </summary>
    /// <param name="text">欲設定之字串視圖。</param>
    virtual void SetText(compat::string_view text) = 0;

    /// <summary>
    /// 清空剪貼簿內容。
    /// </summary>
    virtual void Clear() = 0;
};

/// <summary>
/// 全域系統剪貼簿靜態門面 (Clipboard)
/// 自動轉發操作至當前運行平台之原生剪貼簿實作。
/// </summary>
class Clipboard {
public:
    /// <summary>
    /// 查詢系統剪貼簿是否包含純文字。
    /// </summary>
    /// <returns>若包含文字傳回 true。</returns>
    COMPAT_NODISCARD static bool ContainsText();

    /// <summary>
    /// 自系統剪貼簿讀取 UTF-8 純文字。
    /// </summary>
    /// <returns>文字字串。</returns>
    COMPAT_NODISCARD static std::string GetText();

    /// <summary>
    /// 寫入 UTF-8 純文字至系統剪貼簿。
    /// </summary>
    /// <param name="text">字串視圖。</param>
    static void SetText(compat::string_view text);

    /// <summary>
    /// 清空系統剪貼簿。
    /// </summary>
    static void Clear();

    /// <summary>
    /// 設定自訂剪貼簿實作 (用於單元測試或特殊環境替換)。
    /// </summary>
    /// <param name="customClipboard">自訂剪貼簿指標，傳入 nullptr 恢復預設。</param>
    static void SetCustomClipboard(std::shared_ptr<IClipboard> customClipboard);
};

} // namespace pal
} // namespace cppui
