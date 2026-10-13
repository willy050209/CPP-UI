/*
 * File: Clipboard.cpp
 * Purpose: Implements global static Clipboard facade with platform delegation.
 */

#include "ui/pal/IClipboard.hpp"
#include "generic/ClipboardGeneric.hpp"
#if defined(_WIN32) || defined(WIN32)
#include "win32/ClipboardWin32.hpp"
#endif

namespace cppui {
namespace pal {

namespace {

static std::shared_ptr<IClipboard>& GetCustomInstance() {
    static std::shared_ptr<IClipboard> s_custom;
    return s_custom;
}

static std::shared_ptr<IClipboard> GetDefaultInstance() {
#if defined(_WIN32) || defined(WIN32)
    static auto s_default = std::make_shared<ClipboardWin32>();
#else
    static auto s_default = std::make_shared<ClipboardGeneric>();
#endif
    return s_default;
}

static IClipboard* GetActiveClipboard() {
    auto& custom = GetCustomInstance();
    if (custom) {
        return custom.get();
    }
    return GetDefaultInstance().get();
}

} // namespace

bool Clipboard::ContainsText() {
    auto* cb = GetActiveClipboard();
    return cb ? cb->ContainsText() : false;
}

std::string Clipboard::GetText() {
    auto* cb = GetActiveClipboard();
    return cb ? cb->GetText() : std::string();
}

void Clipboard::SetText(compat::string_view text) {
    auto* cb = GetActiveClipboard();
    if (cb) {
        cb->SetText(text);
    }
}

void Clipboard::Clear() {
    auto* cb = GetActiveClipboard();
    if (cb) {
        cb->Clear();
    }
}

void Clipboard::SetCustomClipboard(std::shared_ptr<IClipboard> customClipboard) {
    GetCustomInstance() = std::move(customClipboard);
}

} // namespace pal
} // namespace cppui
