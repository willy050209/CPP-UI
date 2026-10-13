/*
 * File: PlatformWindowWayland.cpp
 * Purpose: Implements the Linux native Wayland platform window.
 */

#include "PlatformWindowWayland.hpp"

#if defined(CPPUI_HAS_WAYLAND) && CPPUI_HAS_WAYLAND

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace cppui {
namespace pal {
namespace wayland {

namespace {

static void WmBaseHandlePing(void* data, struct xdg_wm_base* wmBase, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(wmBase, serial);
}

static const struct xdg_wm_base_listener s_wmBaseListener = {
    WmBaseHandlePing
};

static void XdgSurfaceHandleConfigure(void* data, struct xdg_surface* xdgSurface, uint32_t serial) {
    auto* self = static_cast<PlatformWindowWayland*>(data);
    xdg_surface_ack_configure(xdgSurface, serial);
    (void)self;
}

static const struct xdg_surface_listener s_xdgSurfaceListener = {
    XdgSurfaceHandleConfigure
};

static void XdgToplevelHandleConfigure(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states) {
    (void)toplevel;
    (void)states;
    auto* self = static_cast<PlatformWindowWayland*>(data);
    if (width > 0 && height > 0) {
        self->SetSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }
}

static void XdgToplevelHandleClose(void* data, struct xdg_toplevel* toplevel) {
    (void)toplevel;
    auto* self = static_cast<PlatformWindowWayland*>(data);
    self->Close();
}

static const struct xdg_toplevel_listener s_xdgToplevelListener = {
    XdgToplevelHandleConfigure,
    XdgToplevelHandleClose,
    nullptr,
    nullptr
};

static void RegistryHandleGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    auto* self = reinterpret_cast<PlatformWindowWayland*>(data);
    (void)version;

    if (std::strcmp(interface, "wl_compositor") == 0) {
        self->m_compositor = static_cast<struct wl_compositor*>(
            wl_registry_bind(registry, name, &wl_surface_interface, 4)
        );
    } else if (std::strcmp(interface, "wl_shm") == 0) {
        self->m_shm = static_cast<struct wl_shm*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)
        );
    } else if (std::strcmp(interface, "xdg_wm_base") == 0) {
        self->m_wmBase = static_cast<struct xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)
        );
        xdg_wm_base_add_listener(self->m_wmBase, &s_wmBaseListener, self);
    } else if (std::strcmp(interface, "wl_seat") == 0) {
        self->m_seat = static_cast<struct wl_seat*>(
            wl_registry_bind(registry, name, &xdg_positioner_interface, 5)
        );
    }
}

static void RegistryHandleGlobalRemove(void* data, struct wl_registry* registry, uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener s_registryListener = {
    RegistryHandleGlobal,
    RegistryHandleGlobalRemove
};

} // anonymous namespace

PlatformWindowWayland::PlatformWindowWayland(const WindowDesc& desc) {
    if (desc.width == 0 || desc.height == 0) {
        throw std::invalid_argument("PlatformWindowWayland: width and height must be > 0.");
    }
    m_width = desc.width;
    m_height = desc.height;
    m_title = std::string(desc.title.data(), desc.title.size());

    InitWayland(desc);
}

PlatformWindowWayland::~PlatformWindowWayland() {
    CleanupWayland();
}

void PlatformWindowWayland::InitWayland(const WindowDesc& desc) {
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        throw std::runtime_error("PlatformWindowWayland: Unable to connect to Wayland display.");
    }

    if (pipe(m_wakePipe) == 0) {
        int flags = fcntl(m_wakePipe[0], F_GETFL, 0);
        fcntl(m_wakePipe[0], F_SETFL, flags | O_NONBLOCK);
    }

    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &s_registryListener, this);
    wl_display_roundtrip(m_display);

    if (!m_compositor || !m_shm || !m_wmBase) {
        CleanupWayland();
        throw std::runtime_error("PlatformWindowWayland: Missing essential Wayland globals.");
    }

    m_surface = wl_compositor_create_surface(m_compositor);
    m_xdgSurface = xdg_wm_base_get_xdg_surface(m_wmBase, m_surface);
    xdg_surface_add_listener(m_xdgSurface, &s_xdgSurfaceListener, this);

    m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
    xdg_toplevel_add_listener(m_xdgToplevel, &s_xdgToplevelListener, this);
    xdg_toplevel_set_title(m_xdgToplevel, m_title.c_str());
    xdg_toplevel_set_app_id(m_xdgToplevel, "cppui_application");

    wl_surface_commit(m_surface);
    wl_display_roundtrip(m_display);

    if (desc.visible) {
        Show();
    }
}

void PlatformWindowWayland::CleanupWayland() {
    for (size_t i = 0; i < 2; ++i) {
        DestroyShmBuffer(m_buffers[i]);
    }

    if (m_xdgToplevel) {
        xdg_toplevel_destroy(m_xdgToplevel);
        m_xdgToplevel = nullptr;
    }
    if (m_xdgSurface) {
        xdg_surface_destroy(m_xdgSurface);
        m_xdgSurface = nullptr;
    }
    if (m_surface) {
        wl_surface_destroy(m_surface);
        m_surface = nullptr;
    }
    if (m_wmBase) {
        xdg_wm_base_destroy(m_wmBase);
        m_wmBase = nullptr;
    }
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }

    if (m_wakePipe[0] != -1) {
        close(m_wakePipe[0]);
        m_wakePipe[0] = -1;
    }
    if (m_wakePipe[1] != -1) {
        close(m_wakePipe[1]);
        m_wakePipe[1] = -1;
    }
}

void PlatformWindowWayland::Show() {
    if (m_isClosed) return;
    m_isVisible = true;
    RequestRedraw();
}

void PlatformWindowWayland::Hide() {
    m_isVisible = false;
}

void PlatformWindowWayland::Close() {
    if (m_isClosed) return;
    m_isClosed = true;
    m_isVisible = false;
    DispatchEvent(RawInputEvent::MakeClose(GetCurrentTimestampMs()));
}

core::Size PlatformWindowWayland::GetSize() const noexcept {
    return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
}

void PlatformWindowWayland::SetSize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        RawInputEvent e;
        e.type = RawEventType::WindowResize;
        e.timestampMs = GetCurrentTimestampMs();
        e.window.width = width;
        e.window.height = height;
        DispatchEvent(e);
        RequestRedraw();
    }
}

core::Size PlatformWindowWayland::GetFramebufferSize() const noexcept {
    return core::Size(static_cast<float>(m_width), static_cast<float>(m_height));
}

void PlatformWindowWayland::SetTitle(compat::string_view title) {
    m_title = std::string(title.data(), title.size());
    if (m_xdgToplevel) {
        xdg_toplevel_set_title(m_xdgToplevel, m_title.c_str());
    }
}

void PlatformWindowWayland::PollEvents() {
    if (!m_display || m_isClosed) return;
    wl_display_dispatch_pending(m_display);
    wl_display_flush(m_display);
}

bool PlatformWindowWayland::WaitEvents(int32_t timeoutMs) {
    if (!m_display || m_isClosed) return false;

    wl_display_flush(m_display);

    struct pollfd pfd[2];
    pfd[0].fd = wl_display_get_fd(m_display);
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    pfd[1].fd = m_wakePipe[0];
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;

    int ret = poll(pfd, 2, timeoutMs);
    if (ret > 0) {
        if (pfd[0].revents & POLLIN) {
            wl_display_dispatch(m_display);
        }
        if (pfd[1].revents & POLLIN) {
            char dummy[64];
            while (read(m_wakePipe[0], dummy, sizeof(dummy)) > 0) {}
        }
        return true;
    }
    return false;
}

void PlatformWindowWayland::WakeUp() {
    if (m_wakePipe[1] != -1) {
        char ch = 1;
        write(m_wakePipe[1], &ch, 1);
    }
}

void PlatformWindowWayland::RequestRedraw() {
    if (m_paintCallback) {
        m_paintCallback();
    }
}

void PlatformWindowWayland::CreateShmBuffer(ShmBuffer& shmBuf, uint32_t width, uint32_t height) {
    DestroyShmBuffer(shmBuf);

    size_t stride = static_cast<size_t>(width) * 4;
    shmBuf.size = stride * height;

    char filename[] = "/tmp/cppui-shm-XXXXXX";
    shmBuf.fd = mkstemp(filename);
    if (shmBuf.fd < 0) return;
    unlink(filename);

    if (ftruncate(shmBuf.fd, static_cast<off_t>(shmBuf.size)) < 0) {
        close(shmBuf.fd);
        shmBuf.fd = -1;
        return;
    }

    shmBuf.data = mmap(nullptr, shmBuf.size, PROT_READ | PROT_WRITE, MAP_SHARED, shmBuf.fd, 0);
    if (shmBuf.data == MAP_FAILED) {
        shmBuf.data = nullptr;
        close(shmBuf.fd);
        shmBuf.fd = -1;
        return;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(m_shm, shmBuf.fd, static_cast<int32_t>(shmBuf.size));
    shmBuf.buffer = wl_shm_pool_create_buffer(pool, 0, width, height, static_cast<int32_t>(stride), WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
}

void PlatformWindowWayland::DestroyShmBuffer(ShmBuffer& shmBuf) {
    if (shmBuf.buffer) {
        wl_buffer_destroy(shmBuf.buffer);
        shmBuf.buffer = nullptr;
    }
    if (shmBuf.data) {
        munmap(shmBuf.data, shmBuf.size);
        shmBuf.data = nullptr;
    }
    if (shmBuf.fd >= 0) {
        close(shmBuf.fd);
        shmBuf.fd = -1;
    }
    shmBuf.isBusy = false;
    shmBuf.size = 0;
}

PlatformWindowWayland::ShmBuffer* PlatformWindowWayland::GetAvailableBuffer(uint32_t width, uint32_t height) {
    m_currentBufferIndex = (m_currentBufferIndex + 1) % 2;
    auto& buf = m_buffers[m_currentBufferIndex];

    size_t neededSize = static_cast<size_t>(width) * height * 4;
    if (buf.size != neededSize || !buf.buffer) {
        CreateShmBuffer(buf, width, height);
    }
    return &buf;
}

void PlatformWindowWayland::PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) {
    if (!m_surface || m_isClosed || pixelBuffer.empty()) return;

    ShmBuffer* shmBuf = GetAvailableBuffer(width, height);
    if (!shmBuf || !shmBuf->data || !shmBuf->buffer) return;

    std::memcpy(shmBuf->data, pixelBuffer.data(), static_cast<size_t>(width) * height * sizeof(uint32_t));

    wl_surface_attach(m_surface, shmBuf->buffer, 0, 0);
    wl_surface_damage_buffer(m_surface, 0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height));
    wl_surface_commit(m_surface);
    wl_display_flush(m_display);
}

void PlatformWindowWayland::DispatchEvent(const RawInputEvent& event) {
    if (m_eventCallback) {
        m_eventCallback(event);
    }
}

uint64_t PlatformWindowWayland::GetCurrentTimestampMs() const noexcept {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
    );
}

} // namespace wayland
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_WAYLAND
