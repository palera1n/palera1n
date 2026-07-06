#if defined(WITH_GUI) || defined(WITH_TUI)

#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <functional>
#include <string>

#ifdef WITH_GUI
# include <wx/wx.h>
#endif

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

enum class DeviceMode
{
    None,
    Normal,
    Recovery,
    DFU
};

struct DeviceState
{
    bool connected = false;
    bool isSupported = false;
    bool multipleDevices = false;
    DeviceMode mode = DeviceMode::None;
    std::string productVersion;
    std::string productType;
    std::string displayName;
    std::string udid;
    uint64_t ecid = 0;
    uint32_t connectedDeviceCount = 0;
};

using DeviceStateCallback = std::function<void(const DeviceState&)>;
void register_device_state_callback(DeviceStateCallback cb);
void ensure_device_event_system_started();

void normal_device_event_cb(const idevice_event_t* event, void* user_data);
void recovery_device_event_cb(const irecv_device_event_t* event, void* user_data);

#endif // EVENT_H

#endif // WITH_GUI || WITH_TUI
