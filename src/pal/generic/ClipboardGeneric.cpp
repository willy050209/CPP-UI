/*
 * File: ClipboardGeneric.cpp
 * Purpose: Implements generic in-memory clipboard.
 */

#include "ClipboardGeneric.hpp"

namespace cppui {
namespace pal {

bool ClipboardGeneric::ContainsText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_text.empty();
}

std::string ClipboardGeneric::GetText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_text;
}

void ClipboardGeneric::SetText(compat::string_view text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_text.assign(text.data(), text.size());
}

void ClipboardGeneric::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_text.clear();
}

} // namespace pal
} // namespace cppui
