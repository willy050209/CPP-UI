#include "SoftwareRenderContext.hpp"
#include "compat/compat.hpp"
#include "../../text/TextLayoutImpl.hpp"
#include <cmath>
#include <algorithm>

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
    DWriteLayoutRenderer(IDWriteBitmapRenderTarget* target, IDWriteRenderingParams* params, COLORREF textColor)
        : m_target(target), m_params(params), m_textColor(textColor) {}

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
        *pixelsPerDip = 1.0f;
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

} // anonymous namespace

void SoftwareRenderTarget::DrawText(compat::string_view text, core::Point origin, compat::string_view family, float size, uint16_t weight, uint8_t style, core::Color color) {
    if (text.empty() || m_width == 0 || m_height == 0 || color.a == 0) return;

#ifdef _WIN32
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (wideLen > 0) {
        std::vector<wchar_t> wideText(static_cast<size_t>(wideLen));
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wideText.data(), wideLen);

        std::vector<wchar_t> wideFamily(family.size() + 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, family.data(), static_cast<int>(family.size()), wideFamily.data(), static_cast<int>(family.size()));

        HDC memDC = CreateCompatibleDC(nullptr);
        if (memDC) {
            int fontH = static_cast<int>(size);
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

            int textW = textSize.cx;
            int textH = textSize.cy;
            if (textW > 0 && textH > 0) {
                BITMAPINFO bmi = {};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = textW;
                bmi.bmiHeader.biHeight = -textH;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                void* bits = nullptr;
                HBITMAP hbm = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
                if (hbm && bits) {
                    HGDIOBJ oldBm = SelectObject(memDC, hbm);

                    RECT rc = { 0, 0, textW, textH };
                    ::FillRect(memDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                    SetBkMode(memDC, TRANSPARENT);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    TextOutW(memDC, 0, 0, wideText.data(), wideLen);

                    const uint32_t* mask = static_cast<const uint32_t*>(bits);
                    int32_t dstX0 = static_cast<int32_t>(origin.x);
                    int32_t dstY0 = static_cast<int32_t>(origin.y);
                    float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

                    for (int y = 0; y < textH; ++y) {
                        int dstY = dstY0 + y;
                        if (dstY < 0 || dstY >= static_cast<int>(m_height)) continue;

                        for (int x = 0; x < textW; ++x) {
                            int dstX = dstX0 + x;
                            if (dstX < 0 || dstX >= static_cast<int>(m_width)) continue;

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

                    SelectObject(memDC, oldFont);
                    DeleteObject(hFont);
                    SelectObject(memDC, oldBm);
                    DeleteObject(hbm);
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

    // 純 C++ 跨平台 8x8 點陣字型光柵化備援 (零作業系統依賴)
    int scale = std::max(1, static_cast<int>(std::round(size / 8.0f)));
    int glyphW = 8 * scale;
    int curX = static_cast<int>(origin.x);
    int curY = static_cast<int>(origin.y);
    float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

    for (char c : text) {
        if (c == '\n') {
            curX = static_cast<int>(origin.x);
            curY += 8 * scale + 2;
            continue;
        }
        uint8_t uc = static_cast<uint8_t>(c);
        if (uc < 32 || uc > 126) uc = 32;
        const uint8_t* glyphBits = s_font8x8[uc - 32];

        for (int r = 0; r < 8; ++r) {
            uint8_t rowBits = glyphBits[r];
            for (int col = 0; col < 8; ++col) {
                if (rowBits & (0x80 >> col)) {
                    for (int dy = 0; dy < scale; ++dy) {
                        int py = curY + r * scale + dy;
                        if (py < 0 || py >= static_cast<int>(m_height)) continue;
                        for (int dx = 0; dx < scale; ++dx) {
                            int px = curX + col * scale + dx;
                            if (px < 0 || px >= static_cast<int>(m_width)) continue;

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
        curX += glyphW;
    }
}

void SoftwareRenderTarget::DrawTextLayout(void* nativeLayoutHandle, core::Point origin, core::Color color) {
    if (!nativeLayoutHandle || m_width == 0 || m_height == 0 || color.a == 0) return;

#ifdef _WIN32
    auto& dwriteRes = GetDWriteResources();
    if (!dwriteRes.EnsureInitialized()) return;

    IDWriteTextLayout* pLayout = static_cast<IDWriteTextLayout*>(nativeLayoutHandle);

    DWRITE_TEXT_METRICS tm = {};
    HRESULT hr = pLayout->GetMetrics(&tm);
    if (FAILED(hr)) return;

    DWRITE_OVERHANG_METRICS om = {};
    pLayout->GetOverhangMetrics(&om);

    // 補償負向 Overhang，確保頂端（如漢字頂部筆畫）、左端（如括號【）與底端（下伸部 g, p, y, q）零像素截斷
    float leftPad   = (std::max)(0.0f, -om.left) + 2.0f;
    float topPad    = (std::max)(0.0f, -om.top) + 2.0f;
    float rightPad  = (std::max)(0.0f, om.right) + 2.0f;
    float bottomPad = (std::max)(0.0f, om.bottom) + 2.0f;

    float layoutW = (std::max)(tm.layoutWidth > 8000.0f ? 0.0f : tm.layoutWidth, tm.width + (std::max)(0.0f, tm.left));
    float layoutH = (std::max)(tm.layoutHeight > 8000.0f ? 0.0f : tm.layoutHeight, tm.height + (std::max)(0.0f, tm.top));

    float boundW = layoutW + leftPad + rightPad;
    float boundH = layoutH + topPad + bottomPad;
    int textW = static_cast<int>(std::ceil(boundW));
    int textH = static_cast<int>(std::ceil(boundH));
    if (textW <= 0 || textH <= 0) return;

    // 限制最大點陣尺寸防止記憶體溢出
    textW = std::min(textW, 4096);
    textH = std::min(textH, 4096);

    IDWriteBitmapRenderTarget* pBitmapTarget = nullptr;
    hr = dwriteRes.pGdiInterop->CreateBitmapRenderTarget(nullptr, textW, textH, &pBitmapTarget);
    if (FAILED(hr) || !pBitmapTarget) return;

    // 強制將點陣渲染目標的 PixelsPerDip 鎖定為 1.0f。
    // 若未設定，DirectWrite 在高 DPI（如 125%、150%、200%）螢幕下會自動繼承螢幕 DC 之 DPI 縮放比率，
    // 導致光柵化字形片面膨脹 1.25x~2.0x，不僅右側筆畫超出 textW 被裁切，亦會造成文字嚴重超出 UI 排版容器。
    pBitmapTarget->SetPixelsPerDip(1.0f);

    HDC memDC = pBitmapTarget->GetMemoryDC();
    RECT rc = { 0, 0, textW, textH };
    ::FillRect(memDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

    DWriteLayoutRenderer renderer(pBitmapTarget, dwriteRes.pRenderingParams, RGB(255, 255, 255));
    // 於點陣圖內部以 (leftPad, topPad) 為原點偏移繪製，使負向字形溢出安全保留於點陣圖內
    hr = pLayout->Draw(nullptr, &renderer, leftPad, topPad);

    if (SUCCEEDED(hr)) {
        HGDIOBJ hbm = GetCurrentObject(memDC, OBJ_BITMAP);
        DIBSECTION ds = {};
        if (GetObject(hbm, sizeof(ds), &ds) == sizeof(ds) && ds.dsBm.bmBits) {
            const uint32_t* mask = static_cast<const uint32_t*>(ds.dsBm.bmBits);
            // 貼圖時校正減去 leftPad 與 topPad，使字形基準線（Baseline）精準還原至排版原點
            int32_t dstX0 = static_cast<int32_t>(origin.x - leftPad);
            int32_t dstY0 = static_cast<int32_t>(origin.y - topPad);
            float srcAlphaNorm = static_cast<float>(color.a) / 255.0f;

            for (int y = 0; y < textH; ++y) {
                int dstY = dstY0 + y;
                if (dstY < 0 || dstY >= static_cast<int>(m_height)) continue;

                for (int x = 0; x < textW; ++x) {
                    int dstX = dstX0 + x;
                    if (dstX < 0 || dstX >= static_cast<int>(m_width)) continue;

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
#endif

    // 跨平台文字排版備援渲染：走純 C++ TextLayout::Impl 行列表
    const auto* layoutImpl = static_cast<const text::TextLayout::Impl*>(nativeLayoutHandle);
    if (layoutImpl) {
        for (const auto& line : layoutImpl->lines) {
            if (line.startIndex < layoutImpl->text.size()) {
                size_t len = std::min(line.length, layoutImpl->text.size() - line.startIndex);
                compat::string_view lineStr(layoutImpl->text.data() + line.startIndex, len);
                core::Point linePt(origin.x + line.bounds.x, origin.y + line.bounds.y);
                DrawText(
                    lineStr,
                    linePt,
                    layoutImpl->font.GetFamily(),
                    layoutImpl->font.GetSize(),
                    static_cast<uint16_t>(layoutImpl->font.GetWeight()),
                    static_cast<uint8_t>(layoutImpl->font.GetStyle()),
                    color
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
