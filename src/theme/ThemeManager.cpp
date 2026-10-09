/*
 * File: ThemeManager.cpp
 * Purpose: Implements the ThemeManager singleton for theme dispatch and listeners.
 */

#include "ui/theme/ThemeManager.hpp"
#include <algorithm>
#include <stdexcept>

namespace cppui {
namespace theme {

ThemeManager& ThemeManager::Get() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
    : m_currentTheme(Theme::Dark()) {
}

const Theme& ThemeManager::GetCurrentTheme() const noexcept {
    return m_currentTheme;
}

void ThemeManager::SetTheme(const Theme& theme) {
    m_currentTheme = theme;
    NotifyThemeChanged();
}

void ThemeManager::SetThemeMode(ThemeMode mode) {
    switch (mode) {
        case ThemeMode::Dark:
            SetTheme(Theme::Dark());
            break;
        case ThemeMode::Light:
            SetTheme(Theme::Light());
            break;
        case ThemeMode::HighContrast:
            SetTheme(Theme::HighContrast());
            break;
        case ThemeMode::Custom:
            m_currentTheme.SetMode(ThemeMode::Custom);
            NotifyThemeChanged();
            break;
    }
}

size_t ThemeManager::AddThemeChangedHandler(ThemeChangedHandler handler) {
    if (!handler) {
        throw std::invalid_argument("ThemeManager::AddThemeChangedHandler: handler cannot be null.");
    }
    size_t id = m_nextHandlerId++;
    m_handlers.emplace_back(id, std::move(handler));
    return id;
}

void ThemeManager::RemoveThemeChangedHandler(size_t handlerId) {
    m_handlers.erase(
        std::remove_if(m_handlers.begin(), m_handlers.end(),
            [handlerId](const std::pair<size_t, ThemeChangedHandler>& item) {
                return item.first == handlerId;
            }),
        m_handlers.end()
    );
}

void ThemeManager::NotifyThemeChanged() {
    auto handlersCopy = m_handlers;
    for (const auto& item : handlersCopy) {
        if (item.second) {
            item.second(m_currentTheme);
        }
    }
}

} // namespace theme
} // namespace cppui
