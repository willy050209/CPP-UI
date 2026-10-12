/*
 * File: ThemeManager.cpp
 * Purpose: Implements the ThemeManager singleton for theme dispatch and listeners with modern Pimpl.
 */

#include "ui/theme/ThemeManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cppui {
namespace theme {

struct ThemeManager::Impl {
    Theme m_currentTheme;
    size_t m_nextHandlerId = 1;
    std::vector<std::pair<size_t, ThemeChangedHandler>> m_handlers;

    Impl() : m_currentTheme(Theme::Dark()) {}
};

ThemeManager& ThemeManager::Get() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
    : m_impl(std::make_unique<Impl>()) {
}

ThemeManager::~ThemeManager() = default;

const Theme& ThemeManager::GetCurrentTheme() const noexcept {
    return m_impl->m_currentTheme;
}

void ThemeManager::SetTheme(const Theme& theme) {
    m_impl->m_currentTheme = theme;
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
            m_impl->m_currentTheme.SetMode(ThemeMode::Custom);
            NotifyThemeChanged();
            break;
    }
}

size_t ThemeManager::AddThemeChangedHandler(ThemeChangedHandler handler) {
    if (!handler) {
        throw std::invalid_argument("ThemeManager::AddThemeChangedHandler: handler cannot be null.");
    }
    size_t id = m_impl->m_nextHandlerId++;
    m_impl->m_handlers.emplace_back(id, std::move(handler));
    return id;
}

void ThemeManager::RemoveThemeChangedHandler(size_t handlerId) {
    m_impl->m_handlers.erase(
        std::remove_if(m_impl->m_handlers.begin(), m_impl->m_handlers.end(),
            [handlerId](const std::pair<size_t, ThemeChangedHandler>& item) {
                return item.first == handlerId;
            }),
        m_impl->m_handlers.end()
    );
}

void ThemeManager::NotifyThemeChanged() {
    auto handlersCopy = m_impl->m_handlers;
    for (const auto& item : handlersCopy) {
        if (item.second) {
            item.second(m_impl->m_currentTheme);
        }
    }
}

} // namespace theme
} // namespace cppui
