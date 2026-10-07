#include "ui/text/FontManager.hpp"
#ifdef _WIN32
#include "DirectWriteTextEngine.hpp"
#endif

namespace cppui {
namespace text {

struct FontManager::Impl {
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

FontManager::FontManager() : m_impl(new Impl()) {}
FontManager::~FontManager() = default;

FontDescriptor FontManager::GetDefaultFont(float size) const {
    return FontDescriptor("Segoe UI", size);
}

FontDescriptor FontManager::GetDefaultChineseFont(float size) const {
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
    if (m_impl) {
        return m_impl->HasFamily(familyName);
    }
    return false;
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

} // namespace text
} // namespace cppui
