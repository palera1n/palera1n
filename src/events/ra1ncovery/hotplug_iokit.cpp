#if defined(WITH_GUI) || defined(WITH_TUI)

#include "hotplug_iokit.hpp"

#include <vector>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>

constexpr int32_t APPLE_VID         = 0x05AC;
constexpr int32_t PID_DFU           = 0x1227;
constexpr int32_t PID_RECOVERY_1    = 0x1280;
constexpr int32_t PID_RECOVERY_2    = 0x1281;
constexpr int32_t PID_RECOVERY_3    = 0x1282;
constexpr int32_t PID_RECOVERY_4    = 0x1283;

static const std::vector<int32_t> TARGET_PIDS = {
    PID_DFU, PID_RECOVERY_1, PID_RECOVERY_2, PID_RECOVERY_3, PID_RECOVERY_4
};

static IONotificationPortRef gNotifyPort = nullptr;
static std::vector<io_iterator_t> gIterators;
static CFRunLoopSourceRef gRunLoopSource = nullptr;
static hotplug_callback_t gUserCallback = nullptr;

static bool is_dfu_pid(int32_t pid) {
    return pid == PID_DFU;
}

static void device_added_callback(void *refcon, io_iterator_t iterator) {
    int32_t pid = static_cast<int32_t>(reinterpret_cast<uintptr_t>(refcon));
    io_service_t device;

    while ((device = IOIteratorNext(iterator))) {
        if (gUserCallback) {
            hotplug_handle_t handle = {};
            handle.serv = device;
            handle.device = nullptr;

            IOCFPlugInInterface **plugInInterface = nullptr;
            SInt32 score = 0;
            kern_return_t kr = IOCreatePlugInInterfaceForService(
                device,
                kIOUSBDeviceUserClientTypeID,
                kIOCFPlugInInterfaceID,
                &plugInInterface,
                &score
            );

            if (kr == kIOReturnSuccess && plugInInterface) {
                (*plugInInterface)->QueryInterface(
                    plugInInterface,
                    CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID320),
                    (LPVOID *)&handle.device
                );
                IODestroyPlugInInterface(plugInInterface);
            }

            hotplug_event_t event = is_dfu_pid(pid) ? HOTPLUG_EVENT_DFU_ADD : HOTPLUG_EVENT_RECOVERY_ADD;
            gUserCallback(event, handle);

            if (handle.device) {
                (*handle.device)->Release(handle.device);
            }
        }
        IOObjectRelease(device);
    }
}

static void device_removed_callback(void *refcon, io_iterator_t iterator) {
    int32_t pid = static_cast<int32_t>(reinterpret_cast<uintptr_t>(refcon));
    io_service_t device;

    while ((device = IOIteratorNext(iterator))) {
        if (gUserCallback) {
            hotplug_handle_t handle = {};
            handle.serv = device;
            handle.device = nullptr;

            hotplug_event_t event = is_dfu_pid(pid) ? HOTPLUG_EVENT_DFU_REMOVE : HOTPLUG_EVENT_RECOVERY_REMOVE;
            gUserCallback(event, handle);
        }
        IOObjectRelease(device);
    }
}

static bool register_pid_node(int32_t pid) {
    void* refcon = reinterpret_cast<void*>(static_cast<uintptr_t>(pid));

    CFNumberRef vid_num = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &APPLE_VID);
    CFNumberRef pid_num = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pid);

    CFMutableDictionaryRef match_add = IOServiceMatching(kIOUSBDeviceClassName);
    CFDictionarySetValue(match_add, CFSTR(kUSBVendorID), vid_num);
    CFDictionarySetValue(match_add, CFSTR(kUSBProductID), pid_num);

    io_iterator_t add_iter = IO_OBJECT_NULL;
    kern_return_t kr = IOServiceAddMatchingNotification(
        gNotifyPort,
        kIOFirstMatchNotification,
        match_add,
        device_added_callback,
        refcon,
        &add_iter
    );

    if (kr == KERN_SUCCESS && add_iter != IO_OBJECT_NULL) {
        device_added_callback(refcon, add_iter);
        gIterators.push_back(add_iter);
    } else {
        CFRelease(vid_num);
        CFRelease(pid_num);
        return false;
    }

    CFMutableDictionaryRef match_remove = IOServiceMatching(kIOUSBDeviceClassName);
    CFDictionarySetValue(match_remove, CFSTR(kUSBVendorID), vid_num);
    CFDictionarySetValue(match_remove, CFSTR(kUSBProductID), pid_num);

    io_iterator_t remove_iter = IO_OBJECT_NULL;
    kr = IOServiceAddMatchingNotification(
        gNotifyPort,
        kIOTerminatedNotification,
        match_remove,
        device_removed_callback,
        refcon,
        &remove_iter
    );

    CFRelease(vid_num);
    CFRelease(pid_num);

    if (kr == KERN_SUCCESS && remove_iter != IO_OBJECT_NULL) {
        device_removed_callback(refcon, remove_iter);
        gIterators.push_back(remove_iter);
        return true;
    }

    return false;
}

bool start_hotplug_monitoring(hotplug_callback_t callback) {
    if (gNotifyPort != nullptr) return true;
    if (!callback) return false;

    gUserCallback = callback;

    gNotifyPort = IONotificationPortCreate(MACH_PORT_NULL);
    if (!gNotifyPort) return false;

    gRunLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), gRunLoopSource, kCFRunLoopDefaultMode);

    for (int32_t pid : TARGET_PIDS) {
        register_pid_node(pid);
    }

    return true;
}

void stop_hotplug_monitoring(void) {
    if (!gNotifyPort) return;

    if (gRunLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), gRunLoopSource, kCFRunLoopDefaultMode);
        gRunLoopSource = nullptr;
    }

    for (io_iterator_t iter : gIterators) {
        if (iter != IO_OBJECT_NULL) {
            IOObjectRelease(iter);
        }
    }
    gIterators.clear();

    IONotificationPortDestroy(gNotifyPort);
    gNotifyPort = nullptr;
    gUserCallback = nullptr;
}

bool get_usb_device_serial(hotplug_handle_t handle, char* buffer, size_t maxLen) {
    if (handle.serv == IO_OBJECT_NULL || !buffer || maxLen == 0) {
        return false;
    }

    CFTypeRef serialProp = IORegistryEntryCreateCFProperty(handle.serv, CFSTR("USB Serial Number"), kCFAllocatorDefault, 0);
    if (!serialProp) serialProp = IORegistryEntryCreateCFProperty(handle.serv, CFSTR("Serial Number"), kCFAllocatorDefault, 0);
    if (!serialProp) return false;

    bool success = false;
    if (CFGetTypeID(serialProp) == CFStringGetTypeID()) {
        CFStringRef cfSerial = static_cast<CFStringRef>(serialProp);

        success = CFStringGetCString(
            cfSerial,
            buffer,
            static_cast<CFIndex>(maxLen),
            kCFStringEncodingUTF8
        );
    }

    CFRelease(serialProp);
    return success;
}

static bool send_usb_control_request(hotplug_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, void* data, uint16_t wLength) {
    if (!handle.device) return false;

    kern_return_t kr = (*handle.device)->USBDeviceOpen(handle.device);
    if (kr != kIOReturnSuccess) {
        return false;
    }

    IOUSBDevRequest request = {
        .bmRequestType = bmRequestType,
        .bRequest      = bRequest,
        .wValue        = wValue,
        .wIndex        = wIndex,
        .wLength       = wLength,
        .pData         = data,
    };

    kr = (*handle.device)->DeviceRequest(handle.device, &request);
    (*handle.device)->USBDeviceClose(handle.device);

    return (kr == kIOReturnSuccess);
}

void exit_recovery(hotplug_handle_t handle) {
    send_usb_control_request(handle, 0x40, 0, 0, 0, (void*)"setenv auto-boot true", sizeof("setenv auto-boot true"));
    send_usb_control_request(handle, 0x40, 0, 0, 0, (void*)"saveenv", sizeof("saveenv"));
    send_usb_control_request(handle, 0x40, 0, 0, 0, (void*)"reboot", sizeof("reboot"));
}

#endif // WITH_GUI || WITH_TUI
