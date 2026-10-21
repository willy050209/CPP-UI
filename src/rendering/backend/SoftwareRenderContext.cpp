/*
 * File: SoftwareRenderContext.cpp
 * Purpose: Implements the CPU software rasterizer and render target for 2D primitives and text.
 */

#include "SoftwareRenderContext.hpp"
#include "compat/Compat.hpp"
#include "../../text/TextLayoutImpl.hpp"
#include "../../text/FallbackFontMetrics.hpp"
#include <cstdint>
#include <cmath>
#include <algorithm>

#ifndef _WIN32
#include <dlfcn.h>
#include <string>
#include <unordered_map>
#include "ui/text/FontManager.hpp"
#endif

#ifdef _WIN32
#include <dwrite.h>
#ifdef DrawText
#undef DrawText
#endif

namespace cppui {
namespace rendering {

namespace {

typedef HRESULT (WINAPI *DWriteCreateFactoryFn)(
    DWRITE_FACTORY_TYPE factoryType,
    REFIID iid,
    IUnknown** factory
);

struct DWriteRasterResources {
    HMODULE hDWrite = nullptr;
    DWriteCreateFactoryFn fnCreateFactory = nullptr;
    IDWriteFactory* pFactory = nullptr;
    IDWriteGdiInterop* pGdiInterop = nullptr;
    IDWriteRenderingParams* pRenderingParams = nullptr;

    bool EnsureInitialized() {
        if (pGdiInterop && pRenderingParams) return true;

        if (!hDWrite) {
            hDWrite = LoadLibraryW(L"dwrite.dll");
            if (hDWrite) {
                fnCreateFactory = reinterpret_cast<DWriteCreateFactoryFn>(
                    GetProcAddress(hDWrite, "DWriteCreateFactory"));
            }
        }
        if (!fnCreateFactory) return false;

        if (!pFactory) {
            HRESULT hr = fnCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&pFactory)
            );
            if (FAILED(hr) || !pFactory) return false;
        }

        if (!pGdiInterop) {
            HRESULT hr = pFactory->GetGdiInterop(&pGdiInterop);
            if (FAILED(hr) || !pGdiInterop) return false;
        }

        if (!pRenderingParams) {
            HRESULT hr = pFactory->CreateRenderingParams(&pRenderingParams);
            if (FAILED(hr) || !pRenderingParams) return false;
        }

        return true;
    }

    ~DWriteRasterResources() {
        if (pRenderingParams) { pRenderingParams->Release(); pRenderingParams = nullptr; }
        if (pGdiInterop) { pGdiInterop->Release(); pGdiInterop = nullptr; }
        if (pFactory) { pFactory->Release(); pFactory = nullptr; }
        if (hDWrite) { FreeLibrary(hDWrite); hDWrite = nullptr; }
    }
};

static DWriteRasterResources& GetDWriteResources() {
    static DWriteRasterResources res;
    return res;
}

class DWriteLayoutRenderer : public IDWriteTextRenderer {
public:
    DWriteLayoutRenderer(IDWriteBitmapRenderTarget* target, IDWriteRenderingParams* params, COLORREF textColor, float pixelsPerDip = 1.0f)
        : m_target(target), m_params(params), m_textColor(textColor), m_pixelsPerDip(pixelsPerDip > 0.0f ? pixelsPerDip : 1.0f) {}

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(IDWritePixelSnapping) ||
            riid == __uuidof(IDWriteTextRenderer)) {
            *ppvObject = this;
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() override { return 1; }
    STDMETHOD_(ULONG, Release)() override { return 1; }

    STDMETHOD(IsPixelSnappingDisabled)(void* clientDrawingContext, BOOL* isDisabled) override {
        (void)clientDrawingContext;
        *isDisabled = FALSE;
        return S_OK;
    }

    STDMETHOD(GetCurrentTransform)(void* clientDrawingContext, DWRITE_MATRIX* transform) override {
        (void)clientDrawingContext;
        transform->m11 = 1.0f; transform->m12 = 0.0f;
        transform->m21 = 0.0f; transform->m22 = 1.0f;
        transform->dx  = 0.0f; transform->dy  = 0.0f;
        return S_OK;
    }

    STDMETHOD(GetPixelsPerDip)(void* clientDrawingContext, FLOAT* pixelsPerDip) override {
        (void)clientDrawingContext;
        *pixelsPerDip = m_pixelsPerDip;
        return S_OK;
    }

    STDMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        const DWRITE_GLYPH_RUN* glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
        IUnknown* clientDrawingEffect
    ) override {
        (void)clientDrawingContext;
        (void)glyphRunDescription;
        (void)clientDrawingEffect;

        return m_target->DrawGlyphRun(
            baselineOriginX,
            baselineOriginY,
            measuringMode,
            glyphRun,
            m_params,
            m_textColor,
            nullptr
        );
    }

    STDMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        const DWRITE_UNDERLINE* underline,
        IUnknown* clientDrawingEffect
    ) override {
        (void)clientDrawingContext;
        (void)baselineOriginX;
        (void)baselineOriginY;
        (void)underline;
        (void)clientDrawingEffect;
        return S_OK;
    }

    STDMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        const DWRITE_STRIKETHROUGH* strikethrough,
        IUnknown* clientDrawingEffect
    ) override {
        (void)clientDrawingContext;
        (void)baselineOriginX;
        (void)baselineOriginY;
        (void)strikethrough;
        (void)clientDrawingEffect;
        return S_OK;
    }

    STDMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
    ) override {
        if (!inlineObject) return E_POINTER;
        return inlineObject->Draw(clientDrawingContext, this, originX, originY, isSideways, isRightToLeft, clientDrawingEffect);
    }

private:
    IDWriteBitmapRenderTarget* m_target;
    IDWriteRenderingParams*    m_params;
    COLORREF                   m_textColor;
    float                      m_pixelsPerDip = 1.0f;
};

} // anonymous namespace

} // namespace rendering
} // namespace cppui
#endif

namespace cppui {
namespace rendering {

namespace {

// 8x8 ASCII Bitmap font table for fallback text rasterization (zero external dependencies)
static const uint8_t s_font8x8[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 33 '!'
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, // 35 '#'
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // 36 '$'
    {0x00,0x63,0x66,0x0C,0x18,0x33,0x63,0x00}, // 37 '%'
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, // 38 '&'
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, // 39 '''
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // 40 '('
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // 41 ')'
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 42 '*'
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // 43 '+'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // 44 ','
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // 45 '-'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // 46 '.'
    {0x02,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, // 47 '/'
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // 48 '0'
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 49 '1'
    {0x3C,0x66,0x06,0x1C,0x30,0x60,0x7E,0x00}, // 50 '2'
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 51 '3'
    {0x0C,0x1C,0x34,0x64,0x7E,0x04,0x04,0x00}, // 52 '4'
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 53 '5'
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 54 '6'
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00}, // 55 '7'
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 56 '8'
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, // 57 '9'
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00}, // 58 ':'
    {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00}, // 59 ';'
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, // 60 '<'
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // 61 '='
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, // 62 '>'
    {0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00}, // 63 '?'
    {0x3C,0x66,0x6E,0x6A,0x6E,0x60,0x3C,0x00}, // 64 '@'
    {0x3C,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // 65 'A'
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // 66 'B'
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // 67 'C'
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // 68 'D'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, // 69 'E'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00}, // 70 'F'
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, // 71 'G'
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // 72 'H'
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 73 'I'
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00}, // 74 'J'
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // 75 'K'
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // 76 'L'
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // 77 'M'
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // 78 'N'
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 79 'O'
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // 80 'P'
    {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00}, // 81 'Q'
    {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00}, // 82 'R'
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // 83 'S'
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 84 'T'
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 85 'U'
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, // 86 'V'
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // 87 'W'
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // 88 'X'
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // 89 'Y'
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // 90 'Z'
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // 91 '['
    {0x40,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, // 92 '\'
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // 93 ']'
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, // 94 '^'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // 95 '_'
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // 96 '`'
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // 97 'a'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // 98 'b'
    {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00}, // 99 'c'
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // 100 'd'
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // 101 'e'
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00}, // 102 'f'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C}, // 103 'g'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // 104 'h'
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // 105 'i'
    {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x6C,0x38}, // 106 'j'
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // 107 'k'
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 108 'l'
    {0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00}, // 109 'm'
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // 110 'n'
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // 111 'o'
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // 112 'p'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x07}, // 113 'q'
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // 114 'r'
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // 115 's'
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0C,0x00}, // 116 't'
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // 117 'u'
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // 118 'v'
    {0x00,0x00,0x63,0x6B,0x7F,0x3E,0x36,0x00}, // 119 'w'
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // 120 'x'
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C}, // 121 'y'
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // 122 'z'
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // 123 '{'
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, // 124 '|'
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // 125 '}'
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}  // 126 '~'
};

#ifndef _WIN32
typedef int (*FT_Init_FreeType_Fn)(void**);
typedef int (*FT_Done_FreeType_Fn)(void*);
typedef int (*FT_New_Face_Fn)(void*, const char*, long, void**);
typedef int (*FT_Done_Face_Fn)(void*);
typedef int (*FT_Set_Pixel_Sizes_Fn)(void*, unsigned int, unsigned int);
typedef unsigned int (*FT_Get_Char_Index_Fn)(void*, unsigned long);
typedef int (*FT_Load_Glyph_Fn)(void*, unsigned int, int);

struct FT_Bitmap_Raw {
    unsigned int rows;
    unsigned int width;
    int pitch;
    unsigned char* buffer;
    unsigned short num_grays;
    unsigned char pixel_mode;
    unsigned char palette_mode;
    void* palette;
};

struct FT_GlyphSlot_Raw {
    void* library;
    void* face;
    void* next;
    unsigned int glyph_index;
    void* generic[2];
    long metrics[8];
    long linearHoriAdvance;
    long linearVertAdvance;
    long advance_x, advance_y;
    unsigned int format;
    FT_Bitmap_Raw bitmap;
    int bitmap_left;
    int bitmap_top;
};

struct FT_Face_Raw {
    long num_faces;
    long face_index;
    long face_flags;
    long style_flags;
    long num_glyphs;
    char* family_name;
    char* style_name;
    int num_fixed_sizes;
    void* available_sizes;
    int num_charmaps;
    void* charmaps;
    void* generic[2];
    long bbox[4];
    unsigned short units_per_EM;
    short ascender;
    short descender;
    short height;
    short max_advance_width;
    short max_advance_height;
    short underline_position;
    short underline_thickness;
    FT_GlyphSlot_Raw* glyph;
};

struct FreeTypeRasterResources {
    void* hFtLib = nullptr;
    FT_Init_FreeType_Fn fnInit = nullptr;
    FT_Done_FreeType_Fn fnDone = nullptr;
    FT_New_Face_Fn fnNewFace = nullptr;
    FT_Done_Face_Fn fnDoneFace = nullptr;
    FT_Set_Pixel_Sizes_Fn fnSetPixelSizes = nullptr;
    FT_Get_Char_Index_Fn fnGetCharIndex = nullptr;
    FT_Load_Glyph_Fn fnLoadGlyph = nullptr;

    void* ftLibrary = nullptr;
    std::unordered_map<std::string, void*> loadedFaces;
    void* defaultChineseFace = nullptr;
    std::string defaultChinesePath;

    bool EnsureInitialized() {
        if (ftLibrary) return true;
        if (!hFtLib) {
            hFtLib = dlopen("libfreetype.so.6", RTLD_LAZY | RTLD_LOCAL);
            if (!hFtLib) {
                hFtLib = dlopen("libfreetype.so", RTLD_LAZY | RTLD_LOCAL);
            }
            if (!hFtLib) return false;

            fnInit = reinterpret_cast<FT_Init_FreeType_Fn>(dlsym(hFtLib, "FT_Init_FreeType"));
            fnDone = reinterpret_cast<FT_Done_FreeType_Fn>(dlsym(hFtLib, "FT_Done_FreeType"));
            fnNewFace = reinterpret_cast<FT_New_Face_Fn>(dlsym(hFtLib, "FT_New_Face"));
            fnDoneFace = reinterpret_cast<FT_Done_Face_Fn>(dlsym(hFtLib, "FT_Done_Face"));
            fnSetPixelSizes = reinterpret_cast<FT_Set_Pixel_Sizes_Fn>(dlsym(hFtLib, "FT_Set_Pixel_Sizes"));
            fnGetCharIndex = reinterpret_cast<FT_Get_Char_Index_Fn>(dlsym(hFtLib, "FT_Get_Char_Index"));
            fnLoadGlyph = reinterpret_cast<FT_Load_Glyph_Fn>(dlsym(hFtLib, "FT_Load_Glyph"));

            if (!fnInit || !fnNewFace || !fnSetPixelSizes || !fnGetCharIndex || !fnLoadGlyph) {
                dlclose(hFtLib);
                hFtLib = nullptr;
                return false;
            }
        }

        if (fnInit(&ftLibrary) != 0 || !ftLibrary) {
            return false;
        }

        defaultChinesePath = text::FontManager::Get().ResolveFontFilePath();
        if (!defaultChinesePath.empty()) {
            void* face = nullptr;
            if (fnNewFace(ftLibrary, defaultChinesePath.c_str(), 0, &face) == 0 && face) {
                defaultChineseFace = face;
                loadedFaces[defaultChinesePath] = face;
            }
        }

        return true;
    }

    void* GetOrLoadFace(const std::string& fontPath) {
        if (!EnsureInitialized() || fontPath.empty()) return defaultChineseFace;
        auto it = loadedFaces.find(fontPath);
        if (it != loadedFaces.end()) return it->second;

        void* face = nullptr;
        if (fnNewFace(ftLibrary, fontPath.c_str(), 0, &face) == 0 && face) {
            loadedFaces[fontPath] = face;
            return face;
        }
        return defaultChineseFace;
    }

    ~FreeTypeRasterResources() {
        if (fnDoneFace) {
            for (auto& pair : loadedFaces) {
                if (pair.second) fnDoneFace(pair.second);
            }
            loadedFaces.clear();
        }
        if (fnDone && ftLibrary) {
            fnDone(ftLibrary);
            ftLibrary = nullptr;
        }
        if (hFtLib) {
            dlclose(hFtLib);
            hFtLib = nullptr;
        }
    }
};

static FreeTypeRasterResources& GetFreeTypeResources() {
    static FreeTypeRasterResources instance;
    return instance;
}
#endif

} // anonymous namespace

void SoftwareRenderTarget::DrawText(compat::string_view text, core::Point origin, compat::string_view family, float size, uint16_t weight, uint8_t style, core::Color color, const core::Rect& clipRect) {
    if (text.empty() || m_width == 0 || m_height == 0 || color.a == 0) return;

    int32_t clipLeft   = 0;
    int32_t clipTop    = 0;
    int32_t clipRight  = static_cast<int32_t>(m_width);
    int32_t clipBottom = static_cast<int32_t>(m_height);

    if (!clipRect.IsEmpty()) {
        clipLeft   = (std::max)(clipLeft, static_cast<int32_t>(std::floor(clipRect.Left())));
        clipTop    = (std::max)(clipTop, static_cast<int32_t>(std::floor(clipRect.Top())));
        clipRight  = (std::min)(clipRight, static_cast<int32_t>(std::ceil(clipRect.Right())));
        clipBottom = (std::min)(clipBottom, static_cast<int32_t>(std::ceil(clipRect.Bottom())));
    }

    if (clipLeft >= clipRight || clipTop >= clipBottom) return;

#ifdef _WIN32
    int32_t wideLen = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), nullptr, 0);
    if (wideLen > 0) {
        std::vector<wchar_t> wideText(static_cast<size_t>(wideLen));
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), wideText.data(), wideLen);

        std::vector<wchar_t> wideFamily(family.size() + 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, family.data(), static_cast<int32_t>(family.size()), wideFamily.data(), static_cast<int32_t>(family.size()));

        HDC memDC = CreateCompatibleDC(nullptr);
        if (memDC) {
            float dpi = (m_dpiScale > 0.0f) ? m_dpiScale : 1.0f;
            int32_t fontH = static_cast<int32_t>(std::round(size * dpi));
            HFONT hFont = CreateFontW(
                fontH, 0, 0, 0,
                weight,
                (style == 2) ? TRUE : FALSE,
                FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                wideFamily.data()
            );
            HGDIOBJ oldFont = SelectObject(memDC, hFont);

            SIZE textSize = {};
            GetTextExtentPoint32W(memDC, wideText.data(), wideLen, &textSize);

            int32_t textW = textSize.cx;
            int32_t textH = textSize.cy;
            if (textW > 0 && textH > 0) {
                int32_t dstX0 = static_cast<int32_t>(origin.x * dpi);
                int32_t dstY0 = static_cast<int32_t>(origin.y * dpi);

                // 快速外包圍邊界剔除 (Fast culling)
                if (dstX0 >= clipRight || dstX0 + textW <= clipLeft ||
                    dstY0 >= clipBottom || dstY0 + textH <= clipTop) {
                    SelectObject(memDC, oldFont);
                    DeleteObject(hFont);
                    DeleteDC(memDC);
                    return;
                }

                BITMAPINFO bmi = {};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = textW;
                bmi.bmiHeader.biHeight = -textH;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                void* bits = nullptr;
                HBITMAP hbm = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
                if (hbm) {
                    if (bits) {
                        HGDIOBJ oldBm = SelectObject(memDC, hbm);

                        RECT rc = { 0, 0, textW, textH };
                        ::FillRect(memDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                        SetBkMode(memDC, TRANSPARENT);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        TextOutW(memDC, 0, 0, wideText.data(), wideLen);

                        const uint32_t* mask = static_cast<const uint32_t*>(bits);
                        float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

                        int32_t yStart = (std::max)(0, clipTop - dstY0);
                        int32_t yEnd   = (std::min)(textH, clipBottom - dstY0);
                        int32_t xStart = (std::max)(0, clipLeft - dstX0);
                        int32_t xEnd   = (std::min)(textW, clipRight - dstX0);

                        for (int32_t y = yStart; y < yEnd; ++y) {
                            int32_t dstY = dstY0 + y;
                            for (int32_t x = xStart; x < xEnd; ++x) {
                                int32_t dstX = dstX0 + x;

                                uint32_t maskVal = mask[y * textW + x] & 0xFF;
                                if (maskVal == 0) continue;

                                float alpha = (static_cast<float>(maskVal) / 255.0f) * srcAlphaNorm;
                                if (alpha <= 0.0f) continue;

                                size_t dstIdx = static_cast<size_t>(dstY) * m_width + dstX;
                                uint32_t dstPixel = m_pixels[dstIdx];

                                uint8_t dstB = static_cast<uint8_t>(dstPixel & 0xFF);
                                uint8_t dstG = static_cast<uint8_t>((dstPixel >> 8) & 0xFF);
                                uint8_t dstR = static_cast<uint8_t>((dstPixel >> 16) & 0xFF);

                                uint8_t outR = static_cast<uint8_t>(color.r * alpha + dstR * (1.0f - alpha));
                                uint8_t outG = static_cast<uint8_t>(color.g * alpha + dstG * (1.0f - alpha));
                                uint8_t outB = static_cast<uint8_t>(color.b * alpha + dstB * (1.0f - alpha));

                                m_pixels[dstIdx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
                            }
                        }

                        SelectObject(memDC, oldBm);
                    }

                    DeleteObject(hbm);
                    SelectObject(memDC, oldFont);
                    DeleteObject(hFont);
                    DeleteDC(memDC);
                    return;
                }
            }
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            DeleteDC(memDC);
        }
    }
#endif

    // 純 C++ 跨平台點陣字型光柵化備援 (零作業系統依賴)
    const auto metrics = text::FallbackFontHelper::ComputeMetrics(size);
    int32_t scale = metrics.scale;
    int32_t cellW = static_cast<int32_t>(metrics.charWidth);
    int32_t cellH = static_cast<int32_t>(metrics.charHeight);
    int32_t curX = static_cast<int32_t>(origin.x);
    int32_t curY = static_cast<int32_t>(origin.y);
    float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

#ifndef _WIN32
    (void)weight;
    (void)style;
    auto& ftRes = GetFreeTypeResources();
    bool hasFt = ftRes.EnsureInitialized();
    void* activeFace = nullptr;
    if (hasFt) {
        std::string fontPath = text::FontManager::Get().ResolveFontFilePath(family);
        activeFace = ftRes.GetOrLoadFace(fontPath);
        if (!activeFace) {
            activeFace = ftRes.defaultChineseFace;
        }
    }
#endif

    const char* ptr = text.data();
    size_t remaining = text.size();

    while (remaining > 0) {
        if (*ptr == '\n') {
            curX = static_cast<int32_t>(origin.x);
            curY += static_cast<int32_t>(metrics.lineHeight);
            ptr++;
            remaining--;
            continue;
        }

        size_t consumed = 0;
        uint32_t cp = text::FallbackFontHelper::DecodeUtf8(ptr, remaining, consumed);
        bool fullwidth = text::FallbackFontHelper::IsFullwidth(cp);
        int32_t glyphAdvance = fullwidth ? (cellW * 2) : cellW;

#ifndef _WIN32
        bool glyphRendered = false;
        if (hasFt && (activeFace || ftRes.defaultChineseFace)) {
            void* targetFace = activeFace;
            uint32_t glyphIdx = 0;
            if (targetFace && ftRes.fnGetCharIndex) {
                glyphIdx = ftRes.fnGetCharIndex(targetFace, cp);
            }
            if (glyphIdx == 0 && ftRes.defaultChineseFace && ftRes.defaultChineseFace != targetFace) {
                targetFace = ftRes.defaultChineseFace;
                glyphIdx = ftRes.fnGetCharIndex(targetFace, cp);
            }

            if (glyphIdx > 0) {
                uint32_t pxSize = static_cast<uint32_t>(std::round(size));
                if (pxSize == 0) pxSize = 14;
                ftRes.fnSetPixelSizes(targetFace, 0, pxSize);

                if (ftRes.fnLoadGlyph(targetFace, glyphIdx, 4) == 0) { // FT_LOAD_RENDER = 4
                    auto* f = static_cast<FT_Face_Raw*>(targetFace);
                    FT_GlyphSlot_Raw* slot = f->glyph;
                    if (slot) {
                        int32_t baselineY = curY + static_cast<int32_t>(metrics.baseline);
                        int32_t startX = curX + slot->bitmap_left;
                        int32_t startY = baselineY - slot->bitmap_top;
                        const uint8_t* buf = slot->bitmap.buffer;
                        uint32_t rows = slot->bitmap.rows;
                        uint32_t cols = slot->bitmap.width;
                        int32_t pitch = slot->bitmap.pitch;
                        uint8_t mode = slot->bitmap.pixel_mode;

                        if (buf && rows > 0 && cols > 0) {
                            for (uint32_t r = 0; r < rows; ++r) {
                                int32_t py = startY + static_cast<int32_t>(r);
                                if (py < clipTop || py >= clipBottom) continue;

                                for (uint32_t c = 0; c < cols; ++c) {
                                    int32_t px = startX + static_cast<int32_t>(c);
                                    if (px < clipLeft || px >= clipRight) continue;

                                    uint8_t maskVal = 0;
                                    if (mode == 2) {
                                        maskVal = buf[r * pitch + c];
                                    } else if (mode == 1) {
                                        uint8_t byteVal = buf[r * pitch + (c / 8)];
                                        maskVal = (byteVal & (0x80 >> (c % 8))) ? 255 : 0;
                                    }

                                    if (maskVal == 0) continue;
                                    float alpha = (static_cast<float>(maskVal) / 255.0f) * srcAlphaNorm;
                                    if (alpha <= 0.0f) continue;

                                    size_t dstIdx = static_cast<size_t>(py) * m_width + px;
                                    uint32_t dstPixel = m_pixels[dstIdx];

                                    uint8_t dstB = static_cast<uint8_t>(dstPixel & 0xFF);
                                    uint8_t dstG = static_cast<uint8_t>((dstPixel >> 8) & 0xFF);
                                    uint8_t dstR = static_cast<uint8_t>((dstPixel >> 16) & 0xFF);

                                    uint8_t outR = static_cast<uint8_t>(color.r * alpha + dstR * (1.0f - alpha));
                                    uint8_t outG = static_cast<uint8_t>(color.g * alpha + dstG * (1.0f - alpha));
                                    uint8_t outB = static_cast<uint8_t>(color.b * alpha + dstB * (1.0f - alpha));

                                    m_pixels[dstIdx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
                                }
                            }
                        }

                        glyphRendered = true;
                        int32_t adv = static_cast<int32_t>(slot->advance_x >> 6);
                        curX += (adv > 0 ? adv : glyphAdvance);
                    }
                }
            }
        }

        if (!glyphRendered)
#endif
        {
            if (cp >= 32 && cp <= 126) {
                uint8_t uc = static_cast<uint8_t>(cp);
                const uint8_t* glyphBits = s_font8x8[uc - 32];

                for (int32_t r = 0; r < 8; ++r) {
                    uint8_t rowBits = glyphBits[r];
                    for (int32_t col = 0; col < 8; ++col) {
                        if (rowBits & (0x80 >> col)) {
                            for (int32_t dy = 0; dy < scale; ++dy) {
                                int32_t py = curY + r * scale + dy;
                                if (py < clipTop || py >= clipBottom) continue;
                                for (int32_t dx = 0; dx < scale; ++dx) {
                                    int32_t px = curX + col * scale + dx;
                                    if (px < clipLeft || px >= clipRight) continue;

                                    size_t dstIdx = static_cast<size_t>(py) * m_width + px;
                                    uint32_t dstPixel = m_pixels[dstIdx];

                                    uint8_t dstB = static_cast<uint8_t>(dstPixel & 0xFF);
                                    uint8_t dstG = static_cast<uint8_t>((dstPixel >> 8) & 0xFF);
                                    uint8_t dstR = static_cast<uint8_t>((dstPixel >> 16) & 0xFF);

                                    uint8_t outR = static_cast<uint8_t>(color.r * srcAlphaNorm + dstR * (1.0f - srcAlphaNorm));
                                    uint8_t outG = static_cast<uint8_t>(color.g * srcAlphaNorm + dstG * (1.0f - srcAlphaNorm));
                                    uint8_t outB = static_cast<uint8_t>(color.b * srcAlphaNorm + dstB * (1.0f - srcAlphaNorm));

                                    m_pixels[dstIdx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
                                }
                            }
                        }
                    }
                }
            } else if (fullwidth) {
                // CJK 或全形字元備援框 (glyphAdvance x cellH)
                int32_t boxW = glyphAdvance - 2;
                int32_t boxH = cellH - 2;
                for (int32_t by = 0; by < boxH; ++by) {
                    int32_t py = curY + 1 + by;
                    if (py < clipTop || py >= clipBottom) continue;
                    for (int32_t bx = 0; bx < boxW; ++bx) {
                        if (by == 0 || by == boxH - 1 || bx == 0 || bx == boxW - 1) {
                            int32_t px = curX + 1 + bx;
                            if (px < clipLeft || px >= clipRight) continue;

                            size_t dstIdx = static_cast<size_t>(py) * m_width + px;
                            uint32_t dstPixel = m_pixels[dstIdx];

                            uint8_t dstB = static_cast<uint8_t>(dstPixel & 0xFF);
                            uint8_t dstG = static_cast<uint8_t>((dstPixel >> 8) & 0xFF);
                            uint8_t dstR = static_cast<uint8_t>((dstPixel >> 16) & 0xFF);

                            uint8_t outR = static_cast<uint8_t>(color.r * srcAlphaNorm + dstR * (1.0f - srcAlphaNorm));
                            uint8_t outG = static_cast<uint8_t>(color.g * srcAlphaNorm + dstG * (1.0f - srcAlphaNorm));
                            uint8_t outB = static_cast<uint8_t>(color.b * srcAlphaNorm + dstB * (1.0f - srcAlphaNorm));

                            m_pixels[dstIdx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
                        }
                    }
                }
            }
            curX += glyphAdvance;
        }

        ptr += consumed;
        remaining -= consumed;
    }
}

void SoftwareRenderTarget::DrawTextLayout(void* nativeLayoutHandle, core::Point origin, core::Color color, const core::Rect& clipRect) {
    if (!nativeLayoutHandle || m_width == 0 || m_height == 0 || color.a == 0) return;

    int32_t clipLeft   = 0;
    int32_t clipTop    = 0;
    int32_t clipRight  = static_cast<int32_t>(m_width);
    int32_t clipBottom = static_cast<int32_t>(m_height);

    if (!clipRect.IsEmpty()) {
        clipLeft   = (std::max)(clipLeft, static_cast<int32_t>(std::floor(clipRect.Left())));
        clipTop    = (std::max)(clipTop, static_cast<int32_t>(std::floor(clipRect.Top())));
        clipRight  = (std::min)(clipRight, static_cast<int32_t>(std::ceil(clipRect.Right())));
        clipBottom = (std::min)(clipBottom, static_cast<int32_t>(std::ceil(clipRect.Bottom())));
    }

    if (clipLeft >= clipRight || clipTop >= clipBottom) return;

#ifdef _WIN32
    auto& dwriteRes = GetDWriteResources();
    if (dwriteRes.EnsureInitialized()) {
        IDWriteTextLayout* pLayout = static_cast<IDWriteTextLayout*>(nativeLayoutHandle);

        DWRITE_TEXT_METRICS tm = {};
        HRESULT hr = pLayout->GetMetrics(&tm);
        if (SUCCEEDED(hr)) {
            DWRITE_OVERHANG_METRICS om = {};
            pLayout->GetOverhangMetrics(&om);

            // 補償負向 Overhang，確保頂端（如漢字頂部筆畫）、左端（如括號【）與底端（下伸部 g, p, y, q）零像素截斷
            float leftPad   = (std::max)(0.0f, -om.left) + 2.0f;
            float topPad    = (std::max)(0.0f, -om.top) + 2.0f;
            float rightPad  = (std::max)(0.0f, om.right) + 2.0f;
            float bottomPad = (std::max)(0.0f, om.bottom) + 2.0f;

            float layoutW = (std::max)(tm.layoutWidth > 8000.0f ? 0.0f : tm.layoutWidth, tm.width + (std::max)(0.0f, tm.left));
            float layoutH = (std::max)(tm.layoutHeight > 8000.0f ? 0.0f : tm.layoutHeight, tm.height + (std::max)(0.0f, tm.top));

            float dpi = (m_dpiScale > 0.0f) ? m_dpiScale : 1.0f;
            float boundW = layoutW + leftPad + rightPad;
            float boundH = layoutH + topPad + bottomPad;
            int32_t textW = static_cast<int32_t>(std::ceil(boundW * dpi));
            int32_t textH = static_cast<int32_t>(std::ceil(boundH * dpi));
            if (textW > 0 && textH > 0) {
                int32_t dstX0 = static_cast<int32_t>(origin.x * dpi - leftPad * dpi);
                int32_t dstY0 = static_cast<int32_t>(origin.y * dpi - topPad * dpi);

                // 快速外包圍邊界剔除 (Fast culling)
                if (dstX0 >= clipRight || dstX0 + textW <= clipLeft ||
                    dstY0 >= clipBottom || dstY0 + textH <= clipTop) {
                    return;
                }

                // 限制最大點陣尺寸防止記憶體溢出
                textW = std::min(textW, 4096);
                textH = std::min(textH, 4096);

                IDWriteBitmapRenderTarget* pBitmapTarget = nullptr;
                hr = dwriteRes.pGdiInterop->CreateBitmapRenderTarget(nullptr, textW, textH, &pBitmapTarget);
                if (SUCCEEDED(hr) && pBitmapTarget) {
                    pBitmapTarget->SetPixelsPerDip(dpi);

                    HDC memDC = pBitmapTarget->GetMemoryDC();
                    RECT rc = { 0, 0, textW, textH };
                    ::FillRect(memDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

                    DWriteLayoutRenderer renderer(pBitmapTarget, dwriteRes.pRenderingParams, RGB(255, 255, 255), dpi);
                    hr = pLayout->Draw(nullptr, &renderer, leftPad, topPad);

                    if (SUCCEEDED(hr)) {
                        HGDIOBJ hbm = GetCurrentObject(memDC, OBJ_BITMAP);
                        DIBSECTION ds = {};
                        if (GetObject(hbm, sizeof(ds), &ds) == sizeof(ds) && ds.dsBm.bmBits) {
                            const uint32_t* mask = static_cast<const uint32_t*>(ds.dsBm.bmBits);
                            float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

                            int32_t yStart = (std::max)(0, clipTop - dstY0);
                            int32_t yEnd   = (std::min)(textH, clipBottom - dstY0);
                            int32_t xStart = (std::max)(0, clipLeft - dstX0);
                            int32_t xEnd   = (std::min)(textW, clipRight - dstX0);

                            for (int32_t y = yStart; y < yEnd; ++y) {
                                int32_t dstY = dstY0 + y;
                                for (int32_t x = xStart; x < xEnd; ++x) {
                                    int32_t dstX = dstX0 + x;

                                    uint32_t pixel = mask[y * textW + x];
                                    uint8_t r = static_cast<uint8_t>((pixel >> 16) & 0xFF);
                                    uint8_t g = static_cast<uint8_t>((pixel >> 8) & 0xFF);
                                    uint8_t b = static_cast<uint8_t>(pixel & 0xFF);
                                    uint8_t maskVal = static_cast<uint8_t>((static_cast<uint32_t>(r) + static_cast<uint32_t>(g) * 2 + static_cast<uint32_t>(b)) / 4);
                                    if (maskVal == 0) continue;

                                    float alpha = (static_cast<float>(maskVal) / 255.0f) * srcAlphaNorm;
                                    if (alpha <= 0.0f) continue;

                                    size_t dstIdx = static_cast<size_t>(dstY) * m_width + dstX;
                                    uint32_t dstPixel = m_pixels[dstIdx];

                                    uint8_t dstB = static_cast<uint8_t>(dstPixel & 0xFF);
                                    uint8_t dstG = static_cast<uint8_t>((dstPixel >> 8) & 0xFF);
                                    uint8_t dstR = static_cast<uint8_t>((dstPixel >> 16) & 0xFF);

                                    uint8_t outR = static_cast<uint8_t>(color.r * alpha + dstR * (1.0f - alpha));
                                    uint8_t outG = static_cast<uint8_t>(color.g * alpha + dstG * (1.0f - alpha));
                                    uint8_t outB = static_cast<uint8_t>(color.b * alpha + dstB * (1.0f - alpha));

                                    m_pixels[dstIdx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
                                }
                            }
                        }
                    }

                    pBitmapTarget->Release();
                    return;
                }
            }
        }
    }
#endif

    // 跨平台文字排版備援渲染：走純 C++ TextLayout::Impl 行列表
    const auto* layoutImpl = static_cast<const text::TextLayout::Impl*>(nativeLayoutHandle);
    if (layoutImpl) {
        const auto metrics = text::FallbackFontHelper::ComputeMetrics(layoutImpl->font.GetSize());
        for (const auto& line : layoutImpl->lines) {
            if (line.startIndex < layoutImpl->text.size()) {
                size_t len = std::min(line.length, layoutImpl->text.size() - line.startIndex);
                compat::string_view lineStr(layoutImpl->text.data() + line.startIndex, len);
                float yOffset = 0.0f;
                if (line.bounds.height > metrics.charHeight) {
                    yOffset = (line.bounds.height - metrics.charHeight) * 0.5f;
                }
                core::Point linePt(origin.x + line.bounds.x, origin.y + line.bounds.y + yOffset);
                DrawText(
                    lineStr,
                    linePt,
                    layoutImpl->font.GetFamily(),
                    layoutImpl->font.GetSize(),
                    static_cast<uint16_t>(layoutImpl->font.GetWeight()),
                    static_cast<uint8_t>(layoutImpl->font.GetStyle()),
                    color,
                    clipRect
                );
            }
        }
    }
}

bool SoftwareRenderContext::Init(void* nativeWindowHandle, uint32_t width, uint32_t height) {
    m_nativeWindowHandle = nativeWindowHandle;
    m_width = width;
    m_height = height;

#ifdef _WIN32
    m_hwnd = static_cast<HWND>(nativeWindowHandle);
    m_hdc = GetDC(m_hwnd);

    ZeroMemory(&m_bmi, sizeof(BITMAPINFO));
    m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_bmi.bmiHeader.biWidth = static_cast<LONG>(width);
    // biHeight 為負值表示由上往下 (Top-Down DIB)，第一行為螢幕頂部
    m_bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
    m_bmi.bmiHeader.biPlanes = 1;
    m_bmi.bmiHeader.biBitCount = 32;
    m_bmi.bmiHeader.biCompression = BI_RGB;
#endif

    m_target.Resize(width, height);
    return true;
}

void SoftwareRenderContext::Resize(uint32_t width, uint32_t height) {
    if (m_width == width && m_height == height) return;
    m_width = width;
    m_height = height;

#ifdef _WIN32
    m_bmi.bmiHeader.biWidth = static_cast<LONG>(width);
    m_bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
#endif

    m_target.Resize(width, height);
}

IRenderTarget* SoftwareRenderContext::BeginFrame() {
    return &m_target;
}

void SoftwareRenderContext::EndFrame() {
#ifdef _WIN32
    if (!m_hwnd || !m_hdc || m_width == 0 || m_height == 0) return;

    // 將軟體記憶體像素一鍵無縫傳送至 Windows DC
    SetDIBitsToDevice(
        m_hdc,
        0, 0,
        m_width, m_height,
        0, 0,
        0, m_height,
        m_target.GetPixelBuffer(),
        &m_bmi,
        DIB_RGB_COLORS
    );
#endif
}

void SoftwareRenderContext::Destroy() {
#ifdef _WIN32
    if (m_hwnd && m_hdc) {
        ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
    m_hwnd = nullptr;
#endif
    m_width = 0;
    m_height = 0;
}

} // namespace rendering
} // namespace cppui
