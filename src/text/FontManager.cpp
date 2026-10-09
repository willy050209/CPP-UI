/*
 * File: FontManager.cpp
 * Purpose: Implements system font query, fallback detection, unified custom font loading, and default font selection.
 */

#include "ui/text/FontManager.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "DirectWriteTextEngine.hpp"
#endif

namespace cppui {
namespace text {

struct FontManager::Impl {
    std::string customDefaultFont;
    std::string customChineseFont;
    std::string customMonospaceFont;
    std::vector<std::string> registeredFamilies;

#ifdef _WIN32
    bool HasFamily(compat::string_view familyName) const {
        IDWriteFactory* pFactory = DirectWriteTextEngine::Get().GetFactory();
        if (!pFactory) return false;

        IDWriteFontCollection* pCollection = nullptr;
        HRESULT hr = pFactory->GetSystemFontCollection(&pCollection, FALSE);
        if (FAILED(hr) || !pCollection) return false;

        std::wstring wideFamily = DirectWriteTextEngine::Utf8ToUtf16(familyName);
        UINT32 index = 0;
        BOOL exists = FALSE;
        pCollection->FindFamilyName(wideFamily.c_str(), &index, &exists);

        pCollection->Release();
        return exists == TRUE;
    }
#else
    bool HasFamily(compat::string_view) const {
        return true;
    }
#endif
};

FontManager& FontManager::Get() {
    static FontManager instance;
    return instance;
}

FontManager::FontManager() : m_impl(std::make_unique<Impl>()) {}
FontManager::~FontManager() = default;

FontDescriptor FontManager::GetDefaultFont(float size) const {
    if (m_impl && !m_impl->customDefaultFont.empty()) {
        return FontDescriptor(m_impl->customDefaultFont, size);
    }
    return FontDescriptor("Segoe UI", size);
}

FontDescriptor FontManager::GetDefaultChineseFont(float size) const {
    if (m_impl && !m_impl->customChineseFont.empty()) {
        return FontDescriptor(m_impl->customChineseFont, size);
    }
#ifdef _WIN32
    if (HasFontFamily("Microsoft JhengHei UI")) {
        return FontDescriptor("Microsoft JhengHei UI", size);
    }
    if (HasFontFamily("Microsoft JhengHei")) {
        return FontDescriptor("Microsoft JhengHei", size);
    }
#endif
    return FontDescriptor("Segoe UI", size);
}

FontDescriptor FontManager::GetDefaultMonospaceFont(float size) const {
    if (m_impl && !m_impl->customMonospaceFont.empty()) {
        return FontDescriptor(m_impl->customMonospaceFont, size);
    }
#ifdef _WIN32
    if (HasFontFamily("Cascadia Code")) {
        return FontDescriptor("Cascadia Code", size);
    }
    if (HasFontFamily("Consolas")) {
        return FontDescriptor("Consolas", size);
    }
#endif
    return FontDescriptor("Courier New", size);
}

bool FontManager::HasFontFamily(compat::string_view familyName) const {
    if (!m_impl) return false;

    // 先行比對手動註冊之自訂字型家族
    for (const auto& reg : m_impl->registeredFamilies) {
        if (reg == familyName) {
            return true;
        }
    }

    return m_impl->HasFamily(familyName);
}

FontMetrics FontManager::GetFontMetrics(const FontDescriptor& font) const {
#ifdef _WIN32
    return DirectWriteTextEngine::Get().GetFontMetrics(font);
#else
    FontMetrics fm;
    fm.ascent = font.GetSize() * 0.8f;
    fm.descent = font.GetSize() * 0.2f;
    fm.lineGap = font.GetSize() * 0.2f;
    fm.capHeight = font.GetSize() * 0.7f;
    fm.xHeight = font.GetSize() * 0.5f;
    return fm;
#endif
}

void FontManager::SetDefaultFontFamily(compat::string_view familyName) {
    if (familyName.empty()) {
        throw std::invalid_argument("FontManager::SetDefaultFontFamily: familyName cannot be empty.");
    }
    if (m_impl) {
        m_impl->customDefaultFont = std::string(familyName.data(), familyName.size());
    }
}

void FontManager::SetDefaultChineseFontFamily(compat::string_view familyName) {
    if (familyName.empty()) {
        throw std::invalid_argument("FontManager::SetDefaultChineseFontFamily: familyName cannot be empty.");
    }
    if (m_impl) {
        m_impl->customChineseFont = std::string(familyName.data(), familyName.size());
    }
}

void FontManager::SetDefaultMonospaceFontFamily(compat::string_view familyName) {
    if (familyName.empty()) {
        throw std::invalid_argument("FontManager::SetDefaultMonospaceFontFamily: familyName cannot be empty.");
    }
    if (m_impl) {
        m_impl->customMonospaceFont = std::string(familyName.data(), familyName.size());
    }
}

void FontManager::ResetDefaultFonts() {
    if (m_impl) {
        m_impl->customDefaultFont.clear();
        m_impl->customChineseFont.clear();
        m_impl->customMonospaceFont.clear();
    }
}

bool FontManager::RegisterFontFile(compat::string_view filePath, compat::string_view customFamilyName) {
    if (filePath.empty()) {
        throw std::invalid_argument("FontManager::RegisterFontFile: filePath cannot be empty.");
    }

    std::string pathStr(filePath.data(), filePath.size());
    std::ifstream fileStream(pathStr.c_str(), std::ios::binary);
    if (!fileStream.good()) {
        throw std::invalid_argument("FontManager::RegisterFontFile: font file does not exist: " + pathStr);
    }
    fileStream.close();

    bool success = true;

#ifdef _WIN32
    std::wstring widePath = DirectWriteTextEngine::Utf8ToUtf16(filePath);
    int32_t fontsAdded = AddFontResourceExW(widePath.c_str(), FR_PRIVATE, nullptr);
    success = (fontsAdded > 0);
#endif

    if (m_impl) {
        std::string family;
        if (!customFamilyName.empty()) {
            family = std::string(customFamilyName.data(), customFamilyName.size());
        } else {
            // 從路徑提取檔名主體做為家族預設備援名稱
            size_t sep = pathStr.find_last_of("/\\");
            std::string fileName = (sep == std::string::npos) ? pathStr : pathStr.substr(sep + 1);
            size_t dot = fileName.find_last_of('.');
            family = (dot == std::string::npos) ? fileName : fileName.substr(0, dot);
        }

        if (!family.empty() && std::find(m_impl->registeredFamilies.begin(), m_impl->registeredFamilies.end(), family) == m_impl->registeredFamilies.end()) {
            m_impl->registeredFamilies.push_back(std::move(family));
        }
    }

    return success;
}

bool FontManager::RegisterFontFromMemory(compat::span<const uint8_t> fontData, compat::string_view customFamilyName) {
    if (fontData.empty()) {
        throw std::invalid_argument("FontManager::RegisterFontFromMemory: fontData cannot be empty.");
    }
    if (customFamilyName.empty()) {
        throw std::invalid_argument("FontManager::RegisterFontFromMemory: customFamilyName cannot be empty.");
    }

    bool success = true;

#ifdef _WIN32
    DWORD numFonts = 0;
    HANDLE hFont = AddFontMemResourceEx(
        const_cast<void*>(reinterpret_cast<const void*>(fontData.data())),
        static_cast<DWORD>(fontData.size()),
        nullptr,
        &numFonts
    );
    success = (hFont != nullptr && numFonts > 0);
#endif

    if (m_impl) {
        std::string family(customFamilyName.data(), customFamilyName.size());
        if (std::find(m_impl->registeredFamilies.begin(), m_impl->registeredFamilies.end(), family) == m_impl->registeredFamilies.end()) {
            m_impl->registeredFamilies.push_back(std::move(family));
        }
    }

    return success;
}

std::vector<std::string> FontManager::GetRegisteredFontFamilies() const {
    if (m_impl) {
        return m_impl->registeredFamilies;
    }
    return {};
}

} // namespace text
} // namespace cppui
