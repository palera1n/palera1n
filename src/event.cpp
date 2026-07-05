#if defined(WITH_GUI) || defined(WITH_TUI)

#include "event.hpp"
#include "sequence.hpp"
#include "utils.h"

#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

namespace {

struct ManagedDevice {
    DeviceState state;
    bool normalPresent = false;
    bool recoveryPresent = false;
};

std::mutex g_mutex;
std::vector<DeviceStateCallback> g_callbacks;
std::unordered_map<uint64_t, ManagedDevice> g_devices; // keyed by ECID
std::unordered_map<std::string, uint64_t> g_udidToEcid;
uint64_t g_activeEcid = 0;
std::once_flag g_startOnce;
irecv_device_event_context_t g_recoveryCtx = nullptr;

void dispatch_callbacks(const std::vector<DeviceStateCallback>& callbacks, const DeviceState& state) {
    for (const auto& cb : callbacks) {
        if (cb)
            cb(state);
    }
}

DeviceState build_public_state_locked() {
    DeviceState out;
    out.connectedDeviceCount = static_cast<uint32_t>(g_devices.size());
    out.multipleDevices = out.connectedDeviceCount > 1;

    if (g_devices.size() != 1) {
        g_activeEcid = 0;
        return out;
    }

    auto it = g_devices.begin();
    if (g_activeEcid != 0) {
        auto activeIt = g_devices.find(g_activeEcid);
        if (activeIt != g_devices.end())
            it = activeIt;
    }

    g_activeEcid = it->first;
    out = it->second.state;
    out.connected = true;
    out.connectedDeviceCount = 1;
    out.multipleDevices = false;
    return out;
}

void publish_state() {
    std::vector<DeviceStateCallback> callbacks;
    DeviceState state;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        callbacks = g_callbacks;
        state = build_public_state_locked();
    }
    dispatch_callbacks(callbacks, state);
}

bool lockdown_get_string(lockdownd_client_t client, const char* key, std::string& out) {
    plist_t value = nullptr;
    if (lockdownd_get_value(client, nullptr, key, &value) != LOCKDOWN_E_SUCCESS || !value)
        return false;

    char* raw = nullptr;
    plist_get_string_val(value, &raw);
    if (raw) {
        out = raw;
        free(raw);
    }

    plist_free(value);
    return !out.empty();
}

bool lockdown_get_uint(lockdownd_client_t client, const char* key, uint64_t& out) {
    plist_t value = nullptr;
    if (lockdownd_get_value(client, nullptr, key, &value) != LOCKDOWN_E_SUCCESS || !value)
        return false;

    plist_get_uint_val(value, &out);
    plist_free(value);
    return out != 0;
}

bool build_normal_state(const idevice_event_t* event, DeviceState& out, std::string& connectionType) {
    idevice_t device = nullptr;
    if (!event || idevice_new(&device, event->udid) != IDEVICE_E_SUCCESS || !device)
        return false;

    lockdownd_client_t client = nullptr;
    if (lockdownd_client_new_with_handshake(device, &client, "palera1n") != LOCKDOWN_E_SUCCESS) {
        idevice_free(device);
        return false;
    }

    out.connected = true;
    out.mode = DeviceMode::Normal;
    out.udid = event->udid ? event->udid : "";

    lockdown_get_string(client, "ProductType", out.productType);
    lockdown_get_string(client, "ProductVersion", out.productVersion);
    lockdown_get_string(client, "ConnectionType", connectionType);
    lockdown_get_uint(client, "UniqueChipID", out.ecid);

    out.isSupported = SequenceIsSupported(out.productType);

    lockdownd_client_free(client);
    idevice_free(device);
    return true;
}

bool build_recovery_state(const irecv_device_event_t* event, DeviceState& out) {
    if (!event || !event->device_info || !event->device_info->ecid)
        return false;

    out.connected = true;
    out.ecid = event->device_info->ecid;
    out.mode = (event->mode == IRECV_K_DFU_MODE) ? DeviceMode::DFU : DeviceMode::Recovery;

    irecv_client_t client = nullptr;
    int attempts = 0;
    const int maxAttempts = 10;
    while (attempts < maxAttempts) {
        if (irecv_open_with_ecid(&client, out.ecid) == IRECV_E_SUCCESS)
            break;
        ++attempts;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    if (client) {
        irecv_device_t device = nullptr;
        if (irecv_devices_get_device_by_client(client, &device) == IRECV_E_SUCCESS && device) {
            out.productType = device->product_type ? device->product_type : "";
            out.displayName = device->display_name ? device->display_name : "";
        }
        irecv_close(client);
    } else {
        LOG_VERBOSE("Failed to open irecv client after retries for ECID=%llu", static_cast<unsigned long long>(out.ecid));
    }

    out.isSupported = SequenceIsSupported(out.productType);
    return true;
}

void handle_normal_add(const idevice_event_t* event) {
    if (!event)
        return;

    if (event->conn_type != CONNECTION_USBMUXD) {
        LOG_VERBOSE("Ignoring non-USB normal mode device event");
        return;
    }

    DeviceState discovered;
    std::string connectionType;
    if (!build_normal_state(event, discovered, connectionType))
        return;

    if (!connectionType.empty() && connectionType != "USB") {
        LOG_VERBOSE("Ignoring over-the-air normal mode device (ConnectionType=%s)", connectionType.c_str());
        return;
    }

    if (discovered.ecid == 0) {
        LOG_VERBOSE("Ignoring normal mode device without ECID");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        ManagedDevice& m = g_devices[discovered.ecid];
        m.state = discovered;
        m.normalPresent = true;
        m.recoveryPresent = false;

        if (!discovered.udid.empty())
            g_udidToEcid[discovered.udid] = discovered.ecid;
    }

    publish_state();
}

void handle_normal_remove(const idevice_event_t* event) {
    if (!event || !event->udid)
        return;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto mapIt = g_udidToEcid.find(event->udid);
        if (mapIt == g_udidToEcid.end())
            return;

        const uint64_t ecid = mapIt->second;
        g_udidToEcid.erase(mapIt);

        auto devIt = g_devices.find(ecid);
        if (devIt == g_devices.end())
            return;

        devIt->second.normalPresent = false;
        if (!devIt->second.normalPresent && !devIt->second.recoveryPresent)
            g_devices.erase(devIt);
    }

    publish_state();
}

void handle_recovery_add(const irecv_device_event_t* event) {
    DeviceState discovered;
    if (!build_recovery_state(event, discovered))
        return;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        ManagedDevice& m = g_devices[discovered.ecid];
        m.state = discovered;
        m.recoveryPresent = true;
        m.normalPresent = false;
    }

    publish_state();
}

void handle_recovery_remove(const irecv_device_event_t* event) {
    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        uint64_t ecid = 0;
        if (event && event->device_info)
            ecid = event->device_info->ecid;

        if (ecid == 0) {
            for (const auto& [key, value] : g_devices) {
                if (!value.recoveryPresent)
                    continue;
                if (ecid != 0) {
                    ecid = 0;
                    break;
                }
                ecid = key;
            }
        }

        if (ecid == 0)
            return;

        auto devIt = g_devices.find(ecid);
        if (devIt == g_devices.end())
            return;

        devIt->second.recoveryPresent = false;
        if (!devIt->second.normalPresent && !devIt->second.recoveryPresent)
            g_devices.erase(devIt);

        for (auto it = g_udidToEcid.begin(); it != g_udidToEcid.end();) {
            if (it->second == ecid && g_devices.find(ecid) == g_devices.end())
                it = g_udidToEcid.erase(it);
            else
                ++it;
        }

        changed = true;
    }

    if (changed)
        publish_state();
}

bool is_supported_recovery_mode(irecv_mode mode) {
    return mode == IRECV_K_DFU_MODE ||
           mode == IRECV_K_RECOVERY_MODE_1 ||
           mode == IRECV_K_RECOVERY_MODE_2 ||
           mode == IRECV_K_RECOVERY_MODE_3 ||
           mode == IRECV_K_RECOVERY_MODE_4;
}

} // namespace

void register_device_state_callback(DeviceStateCallback cb) {
    if (!cb)
        return;

    DeviceState snapshot;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_callbacks.push_back(cb);
        snapshot = build_public_state_locked();
    }

    cb(snapshot);
}

void ensure_device_event_system_started() {
    std::call_once(g_startOnce, []() {
        if (idevice_event_subscribe(normal_device_event_cb, nullptr) != IDEVICE_E_SUCCESS)
            LOG_ERROR("Failed to subscribe to normal mode device events");

        if (irecv_device_event_subscribe(&g_recoveryCtx, recovery_device_event_cb, nullptr) != IRECV_E_SUCCESS)
            LOG_ERROR("Failed to subscribe to recovery/DFU device events");

        char** udids = nullptr;
        int count = 0;
        if (idevice_get_device_list(&udids, &count) == IDEVICE_E_SUCCESS && udids) {
            for (int i = 0; i < count; ++i) {
                idevice_event_t synthetic_event;
                synthetic_event.event = IDEVICE_DEVICE_ADD;
                synthetic_event.udid = udids[i];
                synthetic_event.conn_type = CONNECTION_USBMUXD;
                handle_normal_add(&synthetic_event);
            }
            idevice_device_list_free(udids);
        }
    });
}

void normal_device_event_cb(const idevice_event_t* event, void* user_data) {
    (void)user_data;
    if (!event)
        return;

    if (event->event == IDEVICE_DEVICE_ADD) {
        handle_normal_add(event);
        return;
    }

    if (event->event == IDEVICE_DEVICE_REMOVE)
        handle_normal_remove(event);
}

void recovery_device_event_cb(const irecv_device_event_t* event, void* user_data) {
    (void)user_data;
    if (!event)
        return;

    if (event->type == IRECV_DEVICE_ADD && is_supported_recovery_mode(event->mode)) {
        handle_recovery_add(event);
        return;
    }

    if (event->type == IRECV_DEVICE_REMOVE)
        handle_recovery_remove(event);
}

#endif // WITH_GUI || WITH_TUI
