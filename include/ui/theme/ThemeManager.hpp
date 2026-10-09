/*
 * File: ThemeManager.hpp
 * Purpose: Declares the central ThemeManager singleton hub for global theme dispatch and listener registration.
 */

#pragma once

#include <vector>
#include <functional>
#include <utility>
#include <cstdint>
#include "compat/compat.hpp"
#include "Theme.hpp"

namespace cppui {
namespace theme {

/// <summary>
/// 全域主題管理器中樞 (ThemeManager)
/// 提供進程唯一的設計主題快取、即時亮暗模式切換與全域主題變更事件分發。
/// </summary>
class ThemeManager {
public:
    using ThemeChangedHandler = std::function<void(const Theme&)>;

    /// <summary>
    /// 取得全域 ThemeManager 單例實例。
    /// </summary>
    /// <returns>ThemeManager 單例引用。</returns>
    static ThemeManager& Get();

    /// <summary>
    /// 取得當前全域啟用的主題實例常數引用。
    /// </summary>
    /// <returns>當前 Theme 引用。</returns>
    COMPAT_NODISCARD const Theme& GetCurrentTheme() const noexcept;

    /// <summary>
    /// 設定並套用全域主題實例，並自動向所有已註冊監聽器廣播變更。
    /// </summary>
    /// <param name="theme">欲設定的主題實例。</param>
    void SetTheme(const Theme& theme);

    /// <summary>
    /// 依據模式快速切換全域主題 (Dark, Light, HighContrast)。
    /// </summary>
    /// <param name="mode">欲切換之主題模式列舉值。</param>
    void SetThemeMode(ThemeMode mode);

    /// <summary>
    /// 註冊主題變更通知回呼函式。
    /// </summary>
    /// <param name="handler">回呼處理常式，不可為空。</param>
    /// <returns>該註冊項目之唯一識別碼 (ID)，可用於取消註冊。</returns>
    /// <exception cref="std::invalid_argument">當 handler 為空時擲出。</exception>
    size_t AddThemeChangedHandler(ThemeChangedHandler handler);

    /// <summary>
    /// 依據識別碼取消已註冊之主題變更回呼。
    /// </summary>
    /// <param name="handlerId">欲移除之識別碼。</param>
    void RemoveThemeChangedHandler(size_t handlerId);

    /// <summary>
    /// 主動向所有註冊之監聽器廣播當前主題變更通知。
    /// </summary>
    void NotifyThemeChanged();

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

private:
    ThemeManager();
    ~ThemeManager() = default;

    Theme m_currentTheme;
    size_t m_nextHandlerId = 1;
    std::vector<std::pair<size_t, ThemeChangedHandler>> m_handlers;
};

} // namespace theme
} // namespace cppui
