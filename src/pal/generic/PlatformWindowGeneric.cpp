#include "ui/pal/PlatformWindowGeneric.hpp"
#include <algorithm>
#include <chrono>
#include <thread>

namespace cppui {
namespace pal {

PlatformWindowGeneric::PlatformWindowGeneric(const WindowDesc& desc)
    : m_title(desc.title.data(), desc.title.size())
    , m_width(desc.width > 0 ? desc.width : 800)
    , m_height(desc.height > 0 ? desc.height : 600)
    , m_dpiScale(1.0f)
    , m_isVisible(desc.visible)
    , m_isClosed(false)
    , m_pendingRedraw(false)
    , m_nativeHandle(nullptr) {
    m_framebuffer.resize(static_cast<size_t>(m_width) * m_height, 0xFF000000);
}

void PlatformWindowGeneric::Show() {
    m_isVisible = true;
    m_isClosed = false;
    RequestRedraw();
}

void PlatformWindowGeneric::Hide() {
    m_isVisible = false;
}

void PlatformWindowGeneric::Close() {
    if (!m_isClosed) {
        m_isClosed = true;
        m_isVisible = false;
        if (m_eventCallback) {
            auto now = std::chrono::steady_clock::now();
            uint64_t ts = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
            );
            m_eventCallback(RawInputEvent::MakeClose(ts));
        }
    }
}

bool PlatformWindowGeneric::IsClosed() const noexcept {
    return m_isClosed;
}

core::Size PlatformWindowGeneric::GetSize() const noexcept {
    return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
}

void PlatformWindowGeneric::SetSize(uint32_t width, uint32_t height) {
    if (m_width == width && m_height == height) return;
    m_width = width > 0 ? width : 1;
    m_height = height > 0 ? height : 1;
    m_framebuffer.resize(static_cast<size_t>(m_width) * m_height, 0xFF000000);

    if (m_eventCallback) {
        auto now = std::chrono::steady_clock::now();
        uint64_t ts = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
        );
        m_eventCallback(RawInputEvent::MakeResize(m_width, m_height, m_dpiScale, ts));
    }
    RequestRedraw();
}

core::Size PlatformWindowGeneric::GetFramebufferSize() const noexcept {
    return core::Size(
        static_cast<float>(m_width) * m_dpiScale,
        static_cast<float>(m_height) * m_dpiScale
    );
}

float PlatformWindowGeneric::GetDpiScale() const noexcept {
    return m_dpiScale;
}

void PlatformWindowGeneric::SetDpiScale(float dpiScale) noexcept {
    if (dpiScale <= 0.0f) return;
    if (std::abs(m_dpiScale - dpiScale) < 0.001f) return;
    m_dpiScale = dpiScale;

    if (m_eventCallback) {
        auto now = std::chrono::steady_clock::now();
        uint64_t ts = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
        );
        m_eventCallback(RawInputEvent::MakeDpiChange(m_dpiScale, m_width, m_height, ts));
    }
    RequestRedraw();
}

void PlatformWindowGeneric::SetTitle(compat::string_view title) {
    m_title.assign(title.data(), title.size());
}

compat::string_view PlatformWindowGeneric::GetTitle() const noexcept {
    return compat::string_view(m_title.data(), m_title.size());
}

void* PlatformWindowGeneric::GetNativeHandle() const noexcept {
    return m_nativeHandle;
}

void PlatformWindowGeneric::SetNativeHandle(void* handle) noexcept {
    m_nativeHandle = handle;
}

void PlatformWindowGeneric::SetEventCallback(EventCallback callback) {
    m_eventCallback = std::move(callback);
}

void PlatformWindowGeneric::PollEvents() {
    if (m_isClosed) return;

    if (!m_eventQueue.empty()) {
        std::vector<RawInputEvent> pendingEvents;
        pendingEvents.swap(m_eventQueue);

        if (m_eventCallback) {
            for (const auto& ev : pendingEvents) {
                m_eventCallback(ev);
                if (m_isClosed) break;
            }
        }
    }

    if (m_pendingRedraw && m_paintCallback && !m_isClosed) {
        m_pendingRedraw = false;
        m_paintCallback();
    }
}

bool PlatformWindowGeneric::WaitEvents(int timeoutMs) {
    if (m_isClosed) return false;

    if (!m_eventQueue.empty() || m_pendingRedraw) {
        PollEvents();
        return true;
    }

    if (timeoutMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    } else if (timeoutMs < 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    PollEvents();
    return true;
}

void PlatformWindowGeneric::RequestRedraw() {
    m_pendingRedraw = true;
}

void PlatformWindowGeneric::SetPaintCallback(PaintCallback callback) {
    m_paintCallback = std::move(callback);
}

void PlatformWindowGeneric::PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) {
    if (pixelBuffer.empty() || width == 0 || height == 0) return;

    size_t pixelCount = static_cast<size_t>(width) * height;
    if (m_framebuffer.size() != pixelCount) {
        m_framebuffer.resize(pixelCount);
    }

    size_t copyCount = std::min(pixelCount, pixelBuffer.size());
    std::copy(pixelBuffer.data(), pixelBuffer.data() + copyCount, m_framebuffer.data());

    m_pendingRedraw = false;
}

void PlatformWindowGeneric::InjectEvent(const RawInputEvent& event) {
    if (m_isClosed) return;
    m_eventQueue.push_back(event);
}

compat::span<const uint32_t> PlatformWindowGeneric::GetFramebufferPixels() const noexcept {
    return compat::span<const uint32_t>(m_framebuffer.data(), m_framebuffer.size());
}

bool PlatformWindowGeneric::HasPendingRedraw() const noexcept {
    return m_pendingRedraw;
}

void PlatformWindowGeneric::ClearRedrawRequest() noexcept {
    m_pendingRedraw = false;
}

} // namespace pal
} // namespace cppui
