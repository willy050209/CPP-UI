/*
 * File: ClipboardWayland.cpp
 * Purpose: Implements Linux native Wayland clipboard using wl_data_device_manager protocol.
 */

#if defined(CPPUI_HAS_WAYLAND) && CPPUI_HAS_WAYLAND

#include "src/pal/wayland/ClipboardWayland.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <vector>
#include <chrono>

namespace cppui {
namespace pal {
namespace wayland {

namespace {

static void DataSourceTarget(void* data, struct wl_data_source* source, const char* mime_type) {
    (void)data;
    (void)source;
    (void)mime_type;
}

static void DataSourceSend(void* data, struct wl_data_source* source, const char* mime_type, int32_t fd) {
    (void)source;
    auto* clipboard = static_cast<ClipboardWayland*>(data);
    if (clipboard && mime_type) {
        if (std::strcmp(mime_type, "text/plain;charset=utf-8") == 0 ||
            std::strcmp(mime_type, "text/plain") == 0 ||
            std::strcmp(mime_type, "UTF8_STRING") == 0) {
            const auto& text = clipboard->GetCachedText();
            size_t total = text.size();
            const char* ptr = text.data();
            size_t written = 0;
            while (written < total) {
                ssize_t ret = write(fd, ptr + written, total - written);
                if (ret <= 0) break;
                written += static_cast<size_t>(ret);
            }
        }
    }
    close(fd);
}

static void DataSourceCancelled(void* data, struct wl_data_source* source) {
    (void)source;
    auto* clipboard = static_cast<ClipboardWayland*>(data);
    if (clipboard) {
        clipboard->OnDataSourceCancelled();
    }
}

static const struct wl_data_source_listener s_dataSourceListener = {
    DataSourceTarget,
    DataSourceSend,
    DataSourceCancelled
};

static void DataOfferOffer(void* data, struct wl_data_offer* offer, const char* mime_type) {
    (void)offer;
    auto* clipboard = static_cast<ClipboardWayland*>(data);
    if (clipboard && mime_type) {
        clipboard->OnOfferMimeType(mime_type);
    }
}

static void DataOfferSourceActions(void* data, struct wl_data_offer* offer, uint32_t source_actions) {
    (void)data;
    (void)offer;
    (void)source_actions;
}

static void DataOfferAction(void* data, struct wl_data_offer* offer, uint32_t dnd_action) {
    (void)data;
    (void)offer;
    (void)dnd_action;
}

static const struct wl_data_offer_listener s_dataOfferListener = {
    DataOfferOffer,
    DataOfferSourceActions,
    DataOfferAction
};

static void DataDeviceDataOffer(void* data, struct wl_data_device* device, struct wl_data_offer* id) {
    (void)device;
    auto* clipboard = static_cast<ClipboardWayland*>(data);
    if (clipboard && id) {
        clipboard->OnDataOffer(id);
    }
}

static void DataDeviceEnter(void* data, struct wl_data_device* device, uint32_t serial,
                            struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y,
                            struct wl_data_offer* id) {
    (void)data;
    (void)device;
    (void)serial;
    (void)surface;
    (void)x;
    (void)y;
    (void)id;
}

static void DataDeviceLeave(void* data, struct wl_data_device* device) {
    (void)data;
    (void)device;
}

static void DataDeviceMotion(void* data, struct wl_data_device* device, uint32_t time,
                             wl_fixed_t x, wl_fixed_t y) {
    (void)data;
    (void)device;
    (void)time;
    (void)x;
    (void)y;
}

static void DataDeviceDrop(void* data, struct wl_data_device* device) {
    (void)data;
    (void)device;
}

static void DataDeviceSelection(void* data, struct wl_data_device* device, struct wl_data_offer* id) {
    (void)device;
    auto* clipboard = static_cast<ClipboardWayland*>(data);
    if (clipboard) {
        clipboard->OnSelection(id);
    }
}

static const struct wl_data_device_listener s_dataDeviceListener = {
    DataDeviceDataOffer,
    DataDeviceEnter,
    DataDeviceLeave,
    DataDeviceMotion,
    DataDeviceDrop,
    DataDeviceSelection
};

static void RegistryHandleGlobal(void* data, struct wl_registry* registry, uint32_t id,
                                 const char* interface, uint32_t version) {
    (void)version;
    auto* cb = static_cast<ClipboardWayland*>(data);
    if (!cb) return;

    if (std::strcmp(interface, wl_data_device_manager_interface.name) == 0) {
        // 綁定 wl_data_device_manager (version 3)
        auto* mgr = static_cast<struct wl_data_device_manager*>(
            wl_registry_bind(registry, id, &wl_data_device_manager_interface, 3)
        );
        cb->SetWaylandContext(nullptr, nullptr, mgr);
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        auto* seat = static_cast<struct wl_seat*>(
            wl_registry_bind(registry, id, &wl_seat_interface, 1)
        );
        cb->SetWaylandContext(nullptr, seat, nullptr);
    }
}

static void RegistryHandleGlobalRemove(void* data, struct wl_registry* registry, uint32_t id) {
    (void)data;
    (void)registry;
    (void)id;
}

static const struct wl_registry_listener s_registryListener = {
    RegistryHandleGlobal,
    RegistryHandleGlobalRemove
};

} // anonymous namespace

ClipboardWayland::ClipboardWayland() = default;

ClipboardWayland::~ClipboardWayland() {
    CleanupWayland();
}

void ClipboardWayland::CleanupWayland() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_currentOffer) {
        wl_data_offer_destroy(m_currentOffer);
        m_currentOffer = nullptr;
    }
    if (m_dataSource) {
        wl_data_source_destroy(m_dataSource);
        m_dataSource = nullptr;
    }
    if (m_dataDevice) {
        wl_data_device_destroy(m_dataDevice);
        m_dataDevice = nullptr;
    }
    if (m_dataDeviceManager) {
        wl_data_device_manager_destroy(m_dataDeviceManager);
        m_dataDeviceManager = nullptr;
    }
    if (m_seat) {
        wl_seat_destroy(m_seat);
        m_seat = nullptr;
    }
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    if (m_ownsDisplay && m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
        m_ownsDisplay = false;
    }
}

void ClipboardWayland::EnsureInitialized() {
    if (m_isInitialized) return;
    m_isInitialized = true;

    if (!m_display) {
        m_display = wl_display_connect(nullptr);
        if (!m_display) {
            // 無頭 CI 或無 Wayland 伺服器，自動降級為純記憶體快取
            return;
        }
        m_ownsDisplay = true;

        m_registry = wl_display_get_registry(m_display);
        wl_registry_add_listener(m_registry, &s_registryListener, this);
        wl_display_roundtrip(m_display);
    }

    if (m_dataDeviceManager && m_seat && !m_dataDevice) {
        m_dataDevice = wl_data_device_manager_get_data_device(m_dataDeviceManager, m_seat);
        wl_data_device_add_listener(m_dataDevice, &s_dataDeviceListener, this);
        wl_display_roundtrip(m_display);
    }
}

void ClipboardWayland::SetWaylandContext(struct wl_display* display, struct wl_seat* seat, struct wl_data_device_manager* dataDeviceManager) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (display) {
        if (m_ownsDisplay && m_display && m_display != display) {
            wl_display_disconnect(m_display);
            m_ownsDisplay = false;
        }
        m_display = display;
    }
    if (seat) {
        m_seat = seat;
    }
    if (dataDeviceManager) {
        m_dataDeviceManager = dataDeviceManager;
    }

    if (m_display && m_dataDeviceManager && m_seat && !m_dataDevice) {
        m_dataDevice = wl_data_device_manager_get_data_device(m_dataDeviceManager, m_seat);
        wl_data_device_add_listener(m_dataDevice, &s_dataDeviceListener, this);
        wl_display_flush(m_display);
    }
    m_isInitialized = true;
}

void ClipboardWayland::ResetWaylandContext(struct wl_display* display) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_display == display && !m_ownsDisplay) {
        if (m_currentOffer) {
            wl_data_offer_destroy(m_currentOffer);
            m_currentOffer = nullptr;
        }
        if (m_dataSource) {
            wl_data_source_destroy(m_dataSource);
            m_dataSource = nullptr;
        }
        if (m_dataDevice) {
            wl_data_device_destroy(m_dataDevice);
            m_dataDevice = nullptr;
        }
        m_display = nullptr;
        m_seat = nullptr;
        m_dataDeviceManager = nullptr;
        m_isInitialized = false;
    }
}

void ClipboardWayland::OnDataSourceCancelled() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_dataSource) {
        wl_data_source_destroy(m_dataSource);
        m_dataSource = nullptr;
    }
}

void ClipboardWayland::OnDataOffer(struct wl_data_offer* offer) noexcept {
    wl_data_offer_add_listener(offer, &s_dataOfferListener, this);
}

void ClipboardWayland::OnSelection(struct wl_data_offer* offer) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_currentOffer && m_currentOffer != offer) {
        wl_data_offer_destroy(m_currentOffer);
    }
    m_currentOffer = offer;
    m_offerHasText = false;
}

void ClipboardWayland::OnOfferMimeType(compat::string_view mimeType) noexcept {
    if (mimeType == "text/plain;charset=utf-8" ||
        mimeType == "text/plain" ||
        mimeType == "UTF8_STRING") {
        m_offerHasText = true;
    }
}

bool ClipboardWayland::ContainsText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const_cast<ClipboardWayland*>(this)->EnsureInitialized();

    if (!m_display || !m_dataDevice) {
        return !m_localText.empty();
    }

    wl_display_dispatch_pending(m_display);

    if (m_dataSource) {
        return !m_localText.empty();
    }
    if (m_currentOffer) {
        return m_offerHasText;
    }
    return !m_localText.empty();
}

std::string ClipboardWayland::GetText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const_cast<ClipboardWayland*>(this)->EnsureInitialized();

    if (!m_display || !m_dataDevice) {
        return m_localText;
    }

    wl_display_dispatch_pending(m_display);

    if (m_dataSource) {
        return m_localText;
    }

    if (m_currentOffer && m_offerHasText) {
        int fds[2];
        if (pipe(fds) == 0) {
            wl_data_offer_receive(m_currentOffer, "text/plain;charset=utf-8", fds[1]);
            close(fds[1]);
            wl_display_flush(m_display);

            struct pollfd pfd;
            pfd.fd = fds[0];
            pfd.events = POLLIN;

            std::string result;
            char buffer[1024];

            while (poll(&pfd, 1, 200) > 0) {
                ssize_t bytes = read(fds[0], buffer, sizeof(buffer));
                if (bytes <= 0) break;
                result.append(buffer, static_cast<size_t>(bytes));
            }
            close(fds[0]);

            if (!result.empty()) {
                m_localText = result;
                return result;
            }
        }
    }

    return m_localText;
}

void ClipboardWayland::SetText(compat::string_view text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    EnsureInitialized();

    m_localText = std::string(text.data(), text.size());

    if (!m_display || !m_dataDeviceManager || !m_dataDevice) {
        return;
    }

    if (m_dataSource) {
        wl_data_source_destroy(m_dataSource);
        m_dataSource = nullptr;
    }

    m_dataSource = wl_data_device_manager_create_data_source(m_dataDeviceManager);
    if (!m_dataSource) return;

    wl_data_source_add_listener(m_dataSource, &s_dataSourceListener, this);
    wl_data_source_offer(m_dataSource, "text/plain;charset=utf-8");
    wl_data_source_offer(m_dataSource, "text/plain");
    wl_data_source_offer(m_dataSource, "UTF8_STRING");

    wl_data_device_set_selection(m_dataDevice, m_dataSource, 0);
    wl_display_flush(m_display);
}

void ClipboardWayland::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    EnsureInitialized();

    m_localText.clear();

    if (m_display && m_dataDevice) {
        if (m_dataSource) {
            wl_data_source_destroy(m_dataSource);
            m_dataSource = nullptr;
        }
        wl_data_device_set_selection(m_dataDevice, nullptr, 0);
        wl_display_flush(m_display);
    }
}

} // namespace wayland
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_WAYLAND
