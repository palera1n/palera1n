#if defined(WITH_GUI) || defined(WITH_TUI)

#include "event.hpp"
#include "../sequence.hpp"
#include "device_info.hpp"

#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>

extern "C" {
#include <idevice/idevice.h>
}

#if WITH_CIDERRAIN
# include <ciderra1n/log.h>
#else
# include <openra1n/utils.h>
# include <openra1n/shim.h>
#endif

#include "ra1ncovery/hotplug_iokit.hpp"

namespace {

struct ManagedDevice {
    DeviceState state;
    bool normalPresent = false;
    bool recoveryPresent = false;
    bool dfuPresent = false;
    hotplug_handle_t handle = { IO_OBJECT_NULL, nullptr };
};

void release_handle(hotplug_handle_t& h) {
    if (h.serv != IO_OBJECT_NULL) {
        IOObjectRelease(h.serv);
        h.serv = IO_OBJECT_NULL;
    }
    if (h.device) {
        (*h.device)->Release(h.device);
        h.device = nullptr;
    }
}

void retain_handle(const hotplug_handle_t& h) {
    if (h.serv != IO_OBJECT_NULL) IOObjectRetain(h.serv);
    if (h.device) (*h.device)->AddRef(h.device);
}

template<auto FreeFn>
struct Releaser {
    template<typename T>
    void operator()(T* ptr) const { if (ptr) FreeFn(ptr); }
};

template<typename T, auto FreeFn>
using unique_c_ptr = std::unique_ptr<T, Releaser<FreeFn>>;

struct ActiveLockdownConnection {
    unique_c_ptr<UsbmuxdConnectionHandle, idevice_usbmuxd_connection_free> conn;
    unique_c_ptr<IdeviceProviderHandle, idevice_provider_free> provider;
    unique_c_ptr<LockdowndClientHandle, lockdownd_client_free> client;
};

std::mutex g_mutex;
std::vector<DeviceStateCallback> g_callbacks;
std::unordered_map<uint64_t, ManagedDevice> g_devices;
uint64_t g_activeEcid = 0;
std::once_flag g_startOnce;
std::unique_ptr<ActiveLockdownConnection> g_activeLockdown = nullptr;

void dispatch_callbacks(const std::vector<DeviceStateCallback>& callbacks, const DeviceState& state) {
    for (const auto& cb : callbacks) {
        if (cb) cb(state);
    }
}

DeviceState build_public_state_locked() {
    DeviceState out;
    out.connectedDeviceCount = static_cast<uint32_t>(g_devices.size());
    out.multipleDevices = out.connectedDeviceCount > 1;

    if (g_devices.empty()) {
        g_activeEcid = 0;
        return out;
    }

    auto it = g_devices.find(g_activeEcid);
    if (it == g_devices.end()) {
        it = g_devices.begin();
    }

    g_activeEcid = it->first;
    out = it->second.state;
    out.connected = true;
    out.connectedDeviceCount = static_cast<uint32_t>(g_devices.size());
    out.multipleDevices = out.connectedDeviceCount > 1;
    return out;
}

void publish_state() {
    std::vector<DeviceStateCallback> callbacks;
    DeviceState state;
    {
        std::lock_guard lock(g_mutex);
        callbacks = g_callbacks;
        state = build_public_state_locked();
    }
    dispatch_callbacks(callbacks, state);
}

bool lockdown_get_string(struct LockdowndClientHandle* client, const char* key, std::string& out) {
    plist_t raw_plist = nullptr;
    unique_c_ptr<IdeviceFfiError, idevice_error_free> err(lockdownd_get_value(client, key, nullptr, &raw_plist));
    if (err || !raw_plist) return false;

    unique_c_ptr<std::remove_pointer_t<plist_t>, plist_free> plist(raw_plist);
    char* raw = nullptr;
    plist_get_string_val(plist.get(), &raw);
    if (raw) {
        out = raw;
        free(raw);
    }
    return !out.empty();
}

bool lockdown_get_uint(struct LockdowndClientHandle* client, const char* key, uint64_t& out) {
    plist_t raw_plist = nullptr;
    unique_c_ptr<IdeviceFfiError, idevice_error_free> err(lockdownd_get_value(client, key, nullptr, &raw_plist));
    if (err || !raw_plist) return false;

    unique_c_ptr<std::remove_pointer_t<plist_t>, plist_free> plist(raw_plist);
    plist_get_uint_val(plist.get(), &out);
    return out != 0;
}

std::unique_ptr<ActiveLockdownConnection> establish_lockdown(struct UsbmuxdDeviceHandle* dev_handle, const std::string& udid_str) {
    auto session = std::make_unique<ActiveLockdownConnection>();

    struct UsbmuxdConnectionHandle* raw_conn = nullptr;
    if (idevice_usbmuxd_new_default_connection(1, &raw_conn) != nullptr || !raw_conn) return nullptr;
    session->conn.reset(raw_conn);

    uint32_t device_id = idevice_usbmuxd_device_get_device_id(dev_handle);
    struct UsbmuxdAddrHandle* addr = nullptr;
    unique_c_ptr<IdeviceFfiError, idevice_error_free> err2(idevice_usbmuxd_default_addr_new(&addr));
    if (err2 || !addr) return nullptr;

    struct IdeviceProviderHandle* raw_provider = nullptr;
    unique_c_ptr<IdeviceFfiError, idevice_error_free> err(usbmuxd_provider_new(
        addr, 1, udid_str.c_str(), device_id, "palera1n", &raw_provider
    ));
    if (err || !raw_provider) return nullptr;
    session->provider.reset(raw_provider);

    struct LockdowndClientHandle* raw_client = nullptr;
    err.reset(lockdownd_connect(session->provider.get(), &raw_client));
    if (err || !raw_client) return nullptr;
    session->client.reset(raw_client);

    return session;
}

bool build_normal_state(struct UsbmuxdDeviceHandle* dev_handle, DeviceState& out, std::unique_ptr<ActiveLockdownConnection>& session) {
    if (!dev_handle) return false;

    unique_c_ptr<char, idevice_string_free> udid_raw(idevice_usbmuxd_device_get_udid(dev_handle));
    std::string udid_str = udid_raw ? udid_raw.get() : "";

    session = establish_lockdown(dev_handle, udid_str);
    if (!session) return false;

    out.connected = true;
    out.mode = DeviceMode::Normal;
    out.udid = udid_str;

    lockdown_get_string(session->client.get(), "ProductType", out.productType);
    lockdown_get_string(session->client.get(), "ProductVersion", out.productVersion);
    lockdown_get_uint(session->client.get(), "UniqueChipID", out.ecid);

    out.isSupported = SequenceIsSupported(out.productType);
    return true;
}

void handle_normal_add(struct UsbmuxdDeviceHandle* dev_handle) {
    DeviceState discovered;
    std::unique_ptr<ActiveLockdownConnection> temp_session;

    if (!build_normal_state(dev_handle, discovered, temp_session))
        return;

    if (discovered.ecid == 0) {
        LOG_DEBUG("Ignoring normal mode device without ECID");
        return;
    }

    {
        std::lock_guard lock(g_mutex);
        ManagedDevice& m = g_devices[discovered.ecid];

        release_handle(m.handle);

        m.state = discovered;
        m.normalPresent = true;
        m.recoveryPresent = false;
        m.dfuPresent = false;

        g_activeLockdown = std::move(temp_session);
    }
    publish_state();
}

static void handle_other_add(hotplug_handle_t handle, DeviceMode mode, bool supported) {
    char serial[256] = {};
    if (!get_usb_device_serial(handle, serial, sizeof(serial))) return;

    const char *ecid_str = nullptr, *product_model = nullptr, *product_name = nullptr;
    get_recovery_info(serial, &ecid_str, &product_model, &product_name);
    uint64_t ecid = ecid_str ? strtoull(ecid_str, nullptr, 16) : 0;

    if (ecid == 0) return;

    {
        std::lock_guard lock(g_mutex);
        ManagedDevice& m = g_devices[ecid];

        release_handle(m.handle);
        m.handle = handle;
        retain_handle(m.handle);

        m.state.connected = true;
        m.state.mode = mode;
        m.state.ecid = ecid;
        m.state.productType = product_model ? product_model : "";
        m.state.productVersion.clear();
        m.state.displayName = product_name ? product_name : "";
        m.state.isSupported = supported;

        m.recoveryPresent = (mode == DeviceMode::Recovery);
        m.dfuPresent = (mode == DeviceMode::DFU);
    }
    publish_state();
}

static void remove_device(io_service_t service, DeviceMode mode) {
    bool changed = false;
    {
        std::lock_guard lock(g_mutex);

        auto it = std::find_if(g_devices.begin(), g_devices.end(), [&](const auto& pair) {
            if (service != IO_OBJECT_NULL) return pair.second.handle.serv == service;
            switch (mode) {
                case DeviceMode::Normal:   return pair.second.normalPresent;
                case DeviceMode::Recovery: return pair.second.recoveryPresent;
                case DeviceMode::DFU:      return pair.second.dfuPresent;
                default:                   return false;
            }
        });

        if (it != g_devices.end()) {
            auto& [ecid, m] = *it;

            if (mode == DeviceMode::Normal) {
                m.normalPresent = false;
                g_activeLockdown.reset();
            } else if (mode == DeviceMode::Recovery) {
                m.recoveryPresent = false;
            } else if (mode == DeviceMode::DFU) {
                m.dfuPresent = false;
            }

            if (!m.normalPresent && !m.recoveryPresent && !m.dfuPresent) {
                release_handle(m.handle);
                g_devices.erase(it);
            }
            changed = true;
        }
    }
    if (changed) publish_state();
}

void usbmuxd_listener_worker() {
    struct UsbmuxdConnectionHandle* conn = nullptr;
    if (idevice_usbmuxd_new_default_connection(1, &conn) != nullptr || !conn) return;
    unique_c_ptr<UsbmuxdConnectionHandle, idevice_usbmuxd_connection_free> conn_guard(conn);

    struct UsbmuxdListenerHandle* listener = nullptr;
    if (idevice_usbmuxd_listen(conn, &listener) != nullptr || !listener) return;
    unique_c_ptr<UsbmuxdListenerHandle, idevice_usbmuxd_listener_handle_free> listener_guard(listener);

    for (;;) {
        bool connect = false;
        struct UsbmuxdDeviceHandle* connection_device = nullptr;
        uint32_t disconnection_id = 0;

        if (idevice_usbmuxd_listener_next(listener, &connect, &connection_device, &disconnection_id) != nullptr) break;

        if (connect && connection_device) {
            handle_normal_add(connection_device);
        } else {
            remove_device(IO_OBJECT_NULL, DeviceMode::Normal);
        }
    }
}

static void hotplug_callback(hotplug_event_t event, hotplug_handle_t handle) {
    switch (event) {
        case HOTPLUG_EVENT_DFU_ADD:          handle_other_add(handle, DeviceMode::DFU, false); break;
        case HOTPLUG_EVENT_DFU_REMOVE:       remove_device(handle.serv, DeviceMode::DFU); break;
        case HOTPLUG_EVENT_RECOVERY_ADD:     handle_other_add(handle, DeviceMode::Recovery, true); break;
        case HOTPLUG_EVENT_RECOVERY_REMOVE:  remove_device(handle.serv, DeviceMode::Recovery); break;
    }
}

void recovery_listener_worker() {
    if (!start_hotplug_monitoring(hotplug_callback)) return;
    CFRunLoopRun();
}

} // namespace

void register_device_state_callback(DeviceStateCallback cb) {
    if (!cb) return;
    DeviceState snapshot;
    {
        std::lock_guard lock(g_mutex);
        g_callbacks.push_back(cb);
        snapshot = build_public_state_locked();
    }
    cb(snapshot);
}

void ensure_device_event_system_started() {
    std::call_once(g_startOnce, []() {
        std::thread(usbmuxd_listener_worker).detach();
        std::thread(recovery_listener_worker).detach();
    });
}

bool enter_recovery() {
    LockdowndClientHandle* client = nullptr;
    {
        std::lock_guard lock(g_mutex);
        if (g_activeEcid == 0) return false;

        auto it = g_devices.find(g_activeEcid);
        if (it == g_devices.end() || it->second.state.mode != DeviceMode::Normal) return false;
        if (!g_activeLockdown || !g_activeLockdown->client) return false;

        client = g_activeLockdown->client.get();
    }

    unique_c_ptr<IdeviceFfiError, idevice_error_free> err(lockdownd_enter_recovery(client));
    return !err;
}

void exit_recovery() {
    hotplug_handle_t handle = { IO_OBJECT_NULL, nullptr };
    {
        std::lock_guard lock(g_mutex);
        if (g_activeEcid == 0) return;

        auto it = g_devices.find(g_activeEcid);
        if (it == g_devices.end() || it->second.state.mode != DeviceMode::Recovery) return;

        handle = it->second.handle;
        if (handle.serv == IO_OBJECT_NULL) return;
    }
    exit_recovery(handle);
}

#endif // WITH_GUI || WITH_TUI
