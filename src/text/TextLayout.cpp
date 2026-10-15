#include "ui/text/TextLayout.hpp"
#include "TextLayoutImpl.hpp"

namespace cppui {
namespace text {

TextLayout::TextLayout() : m_impl(std::make_shared<Impl>()) {}
TextLayout::~TextLayout() = default;

TextLayout::TextLayout(std::shared_ptr<Impl> impl) : m_impl(std::move(impl)) {}

TextLayout::TextLayout(const TextLayout& other) = default;
TextLayout& TextLayout::operator=(const TextLayout& other) = default;

TextLayout::TextLayout(TextLayout&& other) noexcept = default;
TextLayout& TextLayout::operator=(TextLayout&& other) noexcept = default;

core::Size TextLayout::GetSize() const noexcept {
    return m_impl ? m_impl->size : core::Size();
}

size_t TextLayout::GetLineCount() const noexcept {
    return m_impl ? m_impl->lines.size() : 0;
}

compat::span<const TextLine> TextLayout::GetLines() const noexcept {
    if (!m_impl || m_impl->lines.empty()) {
        return compat::span<const TextLine>();
    }
    return compat::span<const TextLine>(m_impl->lines.data(), m_impl->lines.size());
}

TextHitTestResult TextLayout::HitTestPoint(core::Point point) const {
    if (m_impl) {
        return m_impl->HitTestPoint(point);
    }
    return TextHitTestResult();
}

core::Rect TextLayout::HitTestTextPosition(size_t textPosition) const {
    if (m_impl) {
        return m_impl->HitTestTextPosition(textPosition);
    }
    return core::Rect();
}

compat::string_view TextLayout::GetText() const noexcept {
    if (m_impl) {
        return compat::string_view(m_impl->text.data(), m_impl->text.size());
    }
    return compat::string_view();
}

const FontDescriptor& TextLayout::GetFont() const noexcept {
    static FontDescriptor s_empty;
    return m_impl ? m_impl->font : s_empty;
}

void* TextLayout::GetNativeHandle() const noexcept {
#ifdef _WIN32
    if (m_impl && m_impl->nativeLayout) {
        return static_cast<void*>(m_impl->nativeLayout);
    }
#endif
    return nullptr;
}

} // namespace text
} // namespace cppui
