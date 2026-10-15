# PlatformWindowWayland 類別

- **命名空間**: `cppui::pal::wayland`
- **標頭檔**: [`src/pal/wayland/PlatformWindowWayland.hpp`](../../../src/pal/wayland/PlatformWindowWayland.hpp)
- **繼承關係**: 繼承自 [`pal::IPlatformWindow`](pal_iplatformwindow.md)

---

## 概述

`PlatformWindowWayland` 是 Linux 現代桌面環境的原生 Wayland 實體視窗後端。透過 `libwayland-client`、`xdg-shell` 協定以及 `xkbcommon` 鍵盤組字對應，支援無撕裂的 `wl_shm` 雙緩衝共享記憶體渲染、`wl_pointer` 滑鼠事件分發、`wl_keyboard` 輸入法/修飾鍵處理與非同步事件輪詢（Event Polling）。

---

## 核心成員方法

### 生命週期與可見度
- `void Show() override`: 向 Wayland 合成器（Compositor）提交 Surface 角色並呈現視窗。
- `void Hide() override`: 隱藏視窗表面。
- `void Close() override`: 銷毀 Wayland Surface、XDG Toplevel 與連線資源。
- `COMPAT_NODISCARD bool IsClosed() const noexcept override`: 查詢視窗是否已關閉。

### 事件輪詢與跨執行緒喚醒
- `void PollEvents() override`: 非阻塞輪詢 Wayland 事件佇列。
- `bool WaitEvents(int32_t timeoutMs = -1) override`: 休眠阻塞等待 Wayland 事件，降低閒置時的 CPU 佔用。
- `void WakeUp() override`: 跨執行緒向事件迴圈管線發送喚醒通知。

### 緩衝區呈現與繪製
- `void RequestRedraw() override`: 註冊幀回呼（`wl_surface_frame`）並請求合成器排程重繪。
- `void PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) override`: 將 32-bit BGRA/RGBA 緩衝區複製至 `wl_shm_pool` 並提交附掛至 Wayland Surface。
