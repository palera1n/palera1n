#if defined(WITH_GUI) || defined(WITH_TUI)

#ifndef EVENT_H
#define EVENT_H

#include "state.hpp"

#include <functional>

#ifdef WITH_GUI
# include <wx/wx.h>
#endif

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

using DeviceStateCallback = std::function<void(const DeviceState&)>;
void register_device_state_callback(DeviceStateCallback cb);

void normal_device_event_cb(const idevice_event_t* event, void* user_data);
void recovery_device_event_cb(const irecv_device_event_t* event, void* user_data);

#endif // EVENT_H

#endif // WITH_GUI || WITH_TUI
