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
#include <condition_variable>
#include <cstring>
#include <iostream>

#if WITH_CIDERRAIN
# include <ciderra1n/log.h>
#else
# include <openra1n/utils.h>
# include <openra1n/shim.h>
#endif

#include "ra1ncovery/hotplug.hpp"
#include <idevice++/lockdown.hpp>
#include <idevice++/usbmuxd.hpp>
#include <idevice++/provider.hpp>

#if defined(__APPLE__)
  using platform_service_t = io_service_t;
#elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)
  using platform_service_t = libusb_device*;
#elif !defined(__APPLE__) && defined(WITH_CIDERRAIN)
  using platform_service_t = liteusb_handle_t*;
#endif

#ifndef __APPLE__
# define IO_OBJECT_NULL nullptr
#endif

namespace {

struct ManagedDevice {
    DeviceState state;
    bool normalPresent = false;
    bool recoveryPresent = false;
    bool dfuPresent = false;
    hotplug_handle_t handle{};
    uint32_t device_id = 0;

    std::unique_ptr<IdeviceFFI::UsbmuxdDevice> usbmuxd_device;
};

void release_handle(hotplug_handle_t& h) {
#ifdef __APPLE__
    if (h.serv != IO_OBJECT_NULL) {
        IOObjectRelease(h.serv);
        h.serv = IO_OBJECT_NULL;
    }
    if (h.device) {
        (*h.device)->Release(h.device);
        h.device = nullptr;
    }
#elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)
    if (h.serv != nullptr) {
        libusb_unref_device(h.serv);
        h.serv = nullptr;
    }
    h.device = nullptr;
#elif !defined(__APPLE__) && defined(WITH_CIDERRAIN)
    h.handle = nullptr;
#endif
}

void retain_handle(const hotplug_handle_t& h) {
#ifdef __APPLE__
    if (h.serv != IO_OBJECT_NULL) {
        IOObjectRetain(h.serv);
    }
    if (h.device) (*h.device)->AddRef(h.device);
#elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)
    if (h.serv != nullptr) {
        libusb_ref_device(h.serv);
    }
#endif
}

template<auto FreeFn>
struct Releaser {
    template<typename T>
    void operator()(T* ptr) const { if (ptr) FreeFn(ptr); }
};

template<typename T, auto FreeFn>
using unique_c_ptr = std::unique_ptr<T, Releaser<FreeFn>>;

std::mutex g_mutex;
std::vector<DeviceStateCallback> g_callbacks;
std::unordered_map<uint64_t, ManagedDevice> g_devices;
uint64_t g_activeEcid = 0;
std::once_flag g_startOnce;

#ifndef __APPLE__
std::condition_variable g_recoveryCv;
bool g_recoveryRunning = false;
#endif

void dispatch_callbacks(const std::vector<DeviceStateCallback>& callbacks, const DeviceState& state) {
    for (const auto& cb : callbacks) {
        if (cb) cb(state);
    }
}

DeviceState build_public_state_locked() {
    DeviceState out;
    const auto total_devices = static_cast<uint32_t>(g_devices.size());
    out.connectedDeviceCount = total_devices;
    out.multipleDevices = total_devices > 1;

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
    out.connectedDeviceCount = total_devices;
    out.multipleDevices = total_devices > 1;
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

bool lockdown_get_string(IdeviceFFI::Lockdown& client, const char* key, std::string& out) {
    auto res = client.get_value(key, nullptr);
    if (res.is_err()) return false;
    plist_t raw_plist = res.unwrap();
    if (!raw_plist) return false;

    unique_c_ptr<std::remove_pointer_t<plist_t>, plist_free> plist(raw_plist);
    char* raw = nullptr;
    plist_get_string_val(plist.get(), &raw);
    if (raw) {
        out = raw;
        free(raw);
    }
    return !out.empty();
}

bool lockdown_get_uint(IdeviceFFI::Lockdown& client, const char* key, uint64_t& out) {
    auto res = client.get_value(key, nullptr);
    if (res.is_err()) return false;
    plist_t raw_plist = res.unwrap();
    if (!raw_plist) return false;

    unique_c_ptr<std::remove_pointer_t<plist_t>, plist_free> plist(raw_plist);
    plist_get_uint_val(plist.get(), &out);
    return out != 0;
}

bool build_normal_state(const IdeviceFFI::UsbmuxdDevice& device, DeviceState& out) {
    auto id_opt = device.get_id();
    if (id_opt.is_none()) return false;
    uint32_t device_id = id_opt.unwrap();

    auto udid_opt = device.get_udid();
    if (udid_opt.is_none()) return false;
    std::string udid_str = udid_opt.unwrap();

    auto prov_res = IdeviceFFI::Provider::usbmuxd_new(IdeviceFFI::UsbmuxdAddr::default_new(), 0, udid_str, device_id, "ra1ncovery");
    if (prov_res.is_err()) return false;
    auto prov = std::move(prov_res).unwrap();

    auto lockdown_res = IdeviceFFI::Lockdown::connect(prov);
    if (lockdown_res.is_err()) return false;
    auto session = std::move(lockdown_res).unwrap();

    auto pf_res = prov.get_pairing_file();
    if (pf_res.is_ok()) {
        (void)session.start_session(pf_res.unwrap());
    }

    out.connected = true;
    out.mode = DeviceMode::Normal;
    out.udid = udid_str;

    lockdown_get_string(session, "ProductType", out.productType);
    lockdown_get_string(session, "ProductVersion", out.productVersion);
    lockdown_get_uint(session, "UniqueChipID", out.ecid);

    for (int i = 0; irecv_devices[i].product_type; i++) {
        if (irecv_devices[i].product_type == out.productType) {
            out.displayName = irecv_devices[i].name;
            break;
        }
    }

    out.isSupported = SequenceIsSupported(out.productType);
    return true;
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

static void remove_device(platform_service_t service, DeviceMode mode, uint32_t target_device_id = 0) {
    bool changed = false;
    {
        std::lock_guard lock(g_mutex);

        auto it = std::find_if(g_devices.begin(), g_devices.end(), [&](const auto& pair) {
            if (service != IO_OBJECT_NULL) {
                #if !defined(__APPLE__) && defined(WITH_CIDERRAIN)
                return pair.second.handle.handle == service;
                #else
                return pair.second.handle.serv == service;
                #endif
            }
            if (mode == DeviceMode::Normal && target_device_id != 0) {
                return pair.second.normalPresent && pair.second.device_id == target_device_id;
            }
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
                m.usbmuxd_device.reset();
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
    std::unordered_map<uint32_t, std::string> known_normal_devices;

    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto conn_res = IdeviceFFI::UsbmuxdConnection::default_new(0);
        if (conn_res.is_err()) continue;

        auto devices_res = std::move(conn_res).unwrap().get_devices();
        if (devices_res.is_err()) continue;

        std::unordered_map<uint32_t, std::string> current_devices;
        std::vector<IdeviceFFI::UsbmuxdDevice> usb_only_devices;

        auto current_dev_list = std::move(devices_res).unwrap();
        usb_only_devices.reserve(current_dev_list.size());

        for (auto& dev : current_dev_list) {
            auto conn_type_opt = dev.get_connection_type();
            if (conn_type_opt.is_none() || conn_type_opt.unwrap().to_string() != "USB") {
                continue;
            }

            auto id_opt = dev.get_id();
            auto udid_opt = dev.get_udid();
            if (id_opt.is_some() && udid_opt.is_some()) {
                current_devices[id_opt.unwrap()] = udid_opt.unwrap();
                usb_only_devices.push_back(std::move(dev));
            }
        }

        for (auto it = known_normal_devices.begin(); it != known_normal_devices.end();) {
            if (current_devices.find(it->first) == current_devices.end()) {
                uint32_t removed_id = it->first;
                it = known_normal_devices.erase(it);
                remove_device(IO_OBJECT_NULL, DeviceMode::Normal, removed_id);
            } else {
                ++it;
            }
        }

        for (auto& dev : usb_only_devices) {
            uint32_t id = dev.get_id().unwrap();

            if (known_normal_devices.find(id) == known_normal_devices.end()) {
                known_normal_devices[id] = dev.get_udid().unwrap();

                DeviceState discovered;
                if (build_normal_state(dev, discovered)) {
                    if (discovered.ecid == 0) continue;

                    {
                        std::lock_guard lock(g_mutex);
                        ManagedDevice& m = g_devices[discovered.ecid];

                        release_handle(m.handle);

                        m.state = discovered;
                        m.normalPresent = true;
                        m.recoveryPresent = false;
                        m.dfuPresent = false;
                        m.device_id = id;

                        m.usbmuxd_device = std::make_unique<IdeviceFFI::UsbmuxdDevice>(std::move(dev));
                    }
                    publish_state();
                }
            }
        }
    }
}

static void hotplug_callback(hotplug_event_t event, hotplug_handle_t handle) {
    #if !defined(__APPLE__) && defined(WITH_CIDERRAIN)
    platform_service_t srv = handle.handle;
    #else
    platform_service_t srv = handle.serv;
    #endif

    switch (event) {
        case HOTPLUG_EVENT_DFU_ADD:          handle_other_add(handle, DeviceMode::DFU, false); break;
        case HOTPLUG_EVENT_DFU_REMOVE:       remove_device(srv, DeviceMode::DFU); break;
        case HOTPLUG_EVENT_RECOVERY_ADD:     handle_other_add(handle, DeviceMode::Recovery, true); break;
        case HOTPLUG_EVENT_RECOVERY_REMOVE:  remove_device(srv, DeviceMode::Recovery); break;
    }
}

void recovery_listener_worker() {
    if (!start_hotplug_monitoring(hotplug_callback)) return;
    #ifdef __APPLE__
    CFRunLoopRun();
    #elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)
    std::unique_lock<std::mutex> lock(g_mutex);
    g_recoveryRunning = true;
    g_recoveryCv.wait(lock, [] { return !g_recoveryRunning; });
    #endif
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
    uint32_t device_id = 0;
    std::string udid_str = "";

    {
        std::lock_guard lock(g_mutex);
        if (g_activeEcid == 0) return false;

        auto it = g_devices.find(g_activeEcid);
        if (it == g_devices.end() || it->second.state.mode != DeviceMode::Normal) return false;

        device_id = it->second.device_id;
        udid_str = it->second.state.udid;
    }

    auto prov_res = IdeviceFFI::Provider::usbmuxd_new(IdeviceFFI::UsbmuxdAddr::default_new(), 0, udid_str, device_id, "ra1ncovery");
    if (prov_res.is_err()) return false;
    auto prov = std::move(prov_res).unwrap();

    auto lockdown_res = IdeviceFFI::Lockdown::connect(prov);
    if (lockdown_res.is_err()) return false;
    auto session = std::move(lockdown_res).unwrap();

    auto pf_res = prov.get_pairing_file();
    if (pf_res.is_ok()) {
        (void)session.start_session(pf_res.unwrap());
    }

    auto res = session.enter_recovery();
    return res.is_ok();
}

void exit_recovery() {
    hotplug_handle_t handle;
    std::memset(&handle, 0, sizeof(handle));
    {
        std::lock_guard lock(g_mutex);
        if (g_activeEcid == 0) return;

        auto it = g_devices.find(g_activeEcid);
        if (it == g_devices.end() || it->second.state.mode != DeviceMode::Recovery) return;

        handle = it->second.handle;
    }
    exit_recovery(handle);
}

#endif // WITH_GUI || WITH_TUI
