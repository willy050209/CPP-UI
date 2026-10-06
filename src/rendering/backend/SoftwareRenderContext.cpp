#include "SoftwareRenderContext.hpp"

namespace cppui {
namespace rendering {

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
