#if defined(WITH_GUI) || defined(WITH_TUI)

#ifndef MAIN__HOTPLUG_IOKIT_HPP
#define MAIN__HOTPLUG_IOKIT_HPP

#include <stdbool.h>
#include <stdint.h>

#ifdef __APPLE__
# include <IOKit/IOKitLib.h>
# include <IOKit/usb/IOUSBLib.h>
#elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)
# include <libusb-1.0/libusb.h>
#elif !defined(__APPLE__) && defined(WITH_CIDERRAIN)
# include <liteusb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HOTPLUG_EVENT_DFU_ADD,
    HOTPLUG_EVENT_DFU_REMOVE,
    HOTPLUG_EVENT_RECOVERY_ADD,
    HOTPLUG_EVENT_RECOVERY_REMOVE
} hotplug_event_t;

typedef struct {
#ifdef __APPLE__
    io_service_t serv;
    IOUSBDeviceInterface320 **device;
#elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)
    libusb_device *serv;
    libusb_device_handle *device;
#elif !defined(__APPLE__) && defined(WITH_CIDERRAIN)
    liteusb_handle_t *handle;
#endif
} hotplug_handle_t;

typedef void (*hotplug_callback_t)(hotplug_event_t event, hotplug_handle_t handle);

bool start_hotplug_monitoring(hotplug_callback_t callback);
void stop_hotplug_monitoring(void);
bool get_usb_device_serial(hotplug_handle_t handle, char* buffer, size_t maxLen);
void exit_recovery(hotplug_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // MAIN__HOTPLUG_IOKIT_HPP

#endif // WITH_GUI || WITH_TUI
