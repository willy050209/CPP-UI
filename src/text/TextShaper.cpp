#include "ui/text/TextShaper.hpp"
#ifdef _WIN32
#include "DirectWriteTextEngine.hpp"
#endif

#ifdef _WIN32

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-exception-spec"
#endif

namespace {

class ShapingCaptureRenderer : public IDWriteTextRenderer {
public:
    ShapingCaptureRenderer(const cppui::text::FontDescriptor& font, std::vector<cppui::text::ShapedRun>& outRuns)
        : m_font(font), m_runs(outRuns), m_refCount(1) {}

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWritePixelSnapping) || riid == __uuidof(IDWriteTextRenderer)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return ++m_refCount;
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }

    // IDWritePixelSnapping
    HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void*, BOOL* isDisabled) override {
        *isDisabled = FALSE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetCurrentTransform(void*, DWRITE_MATRIX* transform) override {
        transform->m11 = 1.0f; transform->m12 = 0.0f;
        transform->m21 = 0.0f; transform->m22 = 1.0f;
        transform->dx  = 0.0f; transform->dy  = 0.0f;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void*, FLOAT* pixelsPerDip) override {
        *pixelsPerDip = 1.0f;
        return S_OK;
    }

    // IDWriteTextRenderer
    HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        void*,
        FLOAT,
        FLOAT,
        DWRITE_MEASURING_MODE,
        const DWRITE_GLYPH_RUN* glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDesc,
        IUnknown*
    ) override {
        if (!glyphRun || glyphRun->glyphCount == 0) return S_OK;

        std::vector<cppui::text::ShapedGlyph> glyphs;
        glyphs.reserve(glyphRun->glyphCount);

        float totalWidth = 0.0f;
        bool isRtl = (glyphRun->bidiLevel % 2) != 0;

        for (UINT32 i = 0; i < glyphRun->glyphCount; ++i) {
            cppui::text::ShapedGlyph g;
            g.glyphId = glyphRun->glyphIndices[i];
            g.xAdvance = glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f;
            g.yAdvance = 0.0f;
            if (glyphRun->glyphOffsets) {
                g.xOffset = glyphRun->glyphOffsets[i].advanceOffset;
                g.yOffset = glyphRun->glyphOffsets[i].ascenderOffset;
            } else {
                g.xOffset = 0.0f;
                g.yOffset = 0.0f;
            }
            g.clusterIndex = (glyphRunDesc && glyphRunDesc->clusterMap) ? glyphRunDesc->clusterMap[i] : i;
            totalWidth += g.xAdvance;
            glyphs.push_back(g);
        }

        m_runs.emplace_back(m_font, isRtl, totalWidth, std::move(glyphs));
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DrawUnderline(void*, FLOAT, FLOAT, const DWRITE_UNDERLINE*, IUnknown*) override {
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE DrawStrikethrough(void*, FLOAT, FLOAT, const DWRITE_STRIKETHROUGH*, IUnknown*) override {
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE DrawInlineObject(void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) override {
        return S_OK;
    }

private:
    cppui::text::FontDescriptor            m_font;
    std::vector<cppui::text::ShapedRun>&  m_runs;
    ULONG                                  m_refCount;
};

} // anonymous namespace
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // _WIN32

namespace cppui {
namespace text {

struct TextShaper::Impl {
    std::vector<ShapedRun> Shape(compat::string_view text, const FontDescriptor& font) const {
        std::vector<ShapedRun> runs;
        if (text.empty()) return runs;

#ifdef _WIN32
        IDWriteTextLayout* pLayout = DirectWriteTextEngine::Get().CreateNativeTextLayout(
            text, font, 100000.0f, 100000.0f,
            TextAlignment::Left, TextWrapping::NoWrap, TextTrimming::None
        );

        if (pLayout) {
            auto* renderer = new ShapingCaptureRenderer(font, runs);
            pLayout->Draw(nullptr, renderer, 0.0f, 0.0f);
            renderer->Release();
            pLayout->Release();
        }
#else
        // 備援簡單 Shaping (每個字元分配基本等寬 advance)
        std::vector<ShapedGlyph> glyphs;
        float adv = font.GetSize() * 0.6f;
        for (size_t i = 0; i < text.size(); ++i) {
            ShapedGlyph g;
            g.glyphId = static_cast<uint16_t>(text[i]);
            g.xAdvance = adv;
            g.clusterIndex = static_cast<uint32_t>(i);
            glyphs.push_back(g);
        }
        runs.emplace_back(font, false, adv * text.size(), std::move(glyphs));
#endif
        return runs;
    }
};

TextShaper::TextShaper() : m_impl(new Impl()) {}
TextShaper::~TextShaper() = default;

std::vector<ShapedRun> TextShaper::Shape(compat::string_view text, const FontDescriptor& font) const {
    if (m_impl) {
        return m_impl->Shape(text, font);
    }
    return {};
}

} // namespace text
} // namespace cppui
