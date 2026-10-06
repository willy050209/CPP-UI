#include "SoftwareRenderContext.hpp"
#include "compat/compat.hpp"
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#include <dwrite.h>

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
#else
    (void)origin;
    (void)color;
#endif
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
