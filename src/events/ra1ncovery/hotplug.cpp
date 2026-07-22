/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2026 palera1n team
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#if defined(WITH_GUI) || defined(WITH_TUI)

#include "hotplug.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

constexpr int32_t APPLE_VID       = 0x05AC;
constexpr int32_t PID_DFU         = 0x1227;
constexpr int32_t PID_RECOVERY_1  = 0x1280;
constexpr int32_t PID_RECOVERY_2  = 0x1281;
constexpr int32_t PID_RECOVERY_3  = 0x1282;
constexpr int32_t PID_RECOVERY_4  = 0x1283;

static const std::vector<int32_t> TARGET_PIDS = {
    PID_DFU, PID_RECOVERY_1, PID_RECOVERY_2, PID_RECOVERY_3, PID_RECOVERY_4
};

static hotplug_callback_t gUserCallback = nullptr;

static bool is_target_pid(int32_t pid) {
    return std::find(TARGET_PIDS.begin(), TARGET_PIDS.end(), pid) != TARGET_PIDS.end();
}

static hotplug_event_t get_hotplug_event(int32_t pid, bool arrived) {
    if (pid == PID_DFU) {
        return arrived ? HOTPLUG_EVENT_DFU_ADD : HOTPLUG_EVENT_DFU_REMOVE;
    }
    return arrived ? HOTPLUG_EVENT_RECOVERY_ADD : HOTPLUG_EVENT_RECOVERY_REMOVE;
}

// MARK: IOKit

#if __APPLE__
# include <CoreFoundation/CoreFoundation.h>

static IONotificationPortRef gNotifyPort = nullptr;
static std::vector<io_iterator_t> gIterators;
static CFRunLoopSourceRef gRunLoopSource = nullptr;

static void process_device_iterator(void *refcon, io_iterator_t iterator, bool arrived) {
    int32_t pid = static_cast<int32_t>(reinterpret_cast<uintptr_t>(refcon));
    io_service_t device;

    while ((device = IOIteratorNext(iterator))) {
        if (gUserCallback) {
            hotplug_handle_t handle = { device, nullptr };

            if (arrived) {
                IOCFPlugInInterface **plugIn = nullptr;
                SInt32 score = 0;
                if (IOCreatePlugInInterfaceForService(device, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugIn, &score) == kIOReturnSuccess && plugIn) {
                    (*plugIn)->QueryInterface(plugIn, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID320), (LPVOID *)&handle.device);
                    IODestroyPlugInInterface(plugIn);
                }
            }

            gUserCallback(get_hotplug_event(pid, arrived), handle);

            if (handle.device) {
                (*handle.device)->Release(handle.device);
            }
        }
        IOObjectRelease(device);
    }
}

static void device_added_callback(void *refcon, io_iterator_t iterator) { process_device_iterator(refcon, iterator, true); }
static void device_removed_callback(void *refcon, io_iterator_t iterator) { process_device_iterator(refcon, iterator, false); }

static bool register_pid_node(int32_t pid) {
    void* refcon = reinterpret_cast<void*>(static_cast<uintptr_t>(pid));
    CFNumberRef vid_num = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &APPLE_VID);
    CFNumberRef pid_num = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pid);

    auto setup_notification = [&](const char* notification_type, void (*cb)(void*, io_iterator_t)) -> bool {
        CFMutableDictionaryRef match = IOServiceMatching(kIOUSBDeviceClassName);
        CFDictionarySetValue(match, CFSTR(kUSBVendorID), vid_num);
        CFDictionarySetValue(match, CFSTR(kUSBProductID), pid_num);

        io_iterator_t iter = IO_OBJECT_NULL;
        if (IOServiceAddMatchingNotification(gNotifyPort, notification_type, match, cb, refcon, &iter) == KERN_SUCCESS && iter != IO_OBJECT_NULL) {
            cb(refcon, iter);
            gIterators.push_back(iter);
            return true;
        }
        return false;
    };

    bool ok = setup_notification(kIOFirstMatchNotification, device_added_callback) &&
              setup_notification(kIOTerminatedNotification, device_removed_callback);

    CFRelease(vid_num);
    CFRelease(pid_num);
    return ok;
}

bool start_hotplug_monitoring(hotplug_callback_t callback) {
    if (gNotifyPort != nullptr) return true;
    if (!callback) return false;

    gUserCallback = callback;
    gNotifyPort = IONotificationPortCreate(MACH_PORT_NULL);
    if (!gNotifyPort) return false;

    gRunLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), gRunLoopSource, kCFRunLoopDefaultMode);

    for (int32_t pid : TARGET_PIDS) register_pid_node(pid);
    return true;
}

void stop_hotplug_monitoring(void) {
    if (!gNotifyPort) return;
    if (gRunLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), gRunLoopSource, kCFRunLoopDefaultMode);
        gRunLoopSource = nullptr;
    }
    for (io_iterator_t iter : gIterators) {
        if (iter != IO_OBJECT_NULL) IOObjectRelease(iter);
    }
    gIterators.clear();
    IONotificationPortDestroy(gNotifyPort);
    gNotifyPort = nullptr;
    gUserCallback = nullptr;
}

bool get_usb_device_serial(hotplug_handle_t handle, char* buffer, size_t maxLen) {
    if (handle.serv == IO_OBJECT_NULL || !buffer || maxLen == 0) return false;

    CFTypeRef serialProp = IORegistryEntryCreateCFProperty(handle.serv, CFSTR("USB Serial Number"), kCFAllocatorDefault, 0);
    if (!serialProp) serialProp = IORegistryEntryCreateCFProperty(handle.serv, CFSTR("Serial Number"), kCFAllocatorDefault, 0);
    if (!serialProp) return false;

    bool success = false;
    if (CFGetTypeID(serialProp) == CFStringGetTypeID()) {
        success = CFStringGetCString(static_cast<CFStringRef>(serialProp), buffer, static_cast<CFIndex>(maxLen), kCFStringEncodingUTF8);
    }
    CFRelease(serialProp);
    return success;
}

static bool send_usb_control_request(hotplug_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, void* data, uint16_t wLength) {
    if (!handle.device || (*handle.device)->USBDeviceOpen(handle.device) != kIOReturnSuccess) return false;

    IOUSBDevRequest request = { bmRequestType, bRequest, wValue, wIndex, wLength, data };
    kern_return_t kr = (*handle.device)->DeviceRequest(handle.device, &request);
    (*handle.device)->USBDeviceClose(handle.device);
    return (kr == kIOReturnSuccess);
}

// MARK: Libusb

#elif !defined(__APPLE__) && !defined(WITH_CIDERRAIN)

static libusb_context *gCtx = nullptr;
static std::vector<libusb_hotplug_callback_handle> gCallbackHandles;
static std::thread gEventThread;
static std::atomic<bool> gRunning{false};

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) {
    (void)ctx; (void)user_data;

    struct libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(dev, &desc) != LIBUSB_SUCCESS || desc.idVendor != APPLE_VID || !is_target_pid(desc.idProduct)) return 0;

    if (gUserCallback) {
        libusb_ref_device(dev);
        std::thread([dev, event, desc]() {
            hotplug_handle_t handle = { dev, nullptr };
            bool arrived = (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED);
            gUserCallback(get_hotplug_event(desc.idProduct, arrived), handle);
            libusb_unref_device(dev);
        }).detach();
    }
    return 0;
}

bool start_hotplug_monitoring(hotplug_callback_t callback) {
    if (gCtx != nullptr) return true;
    if (!callback || libusb_init(&gCtx) != LIBUSB_SUCCESS) return false;

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        libusb_exit(gCtx);
        gCtx = nullptr;
        return false;
    }

    gUserCallback = callback;
    gRunning = true;
    gEventThread = std::thread([]() {
        while (gRunning) libusb_handle_events_completed(gCtx, nullptr);
    });

    libusb_hotplug_callback_handle cb_handle;
    int rc = libusb_hotplug_register_callback(
        gCtx, static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
        LIBUSB_HOTPLUG_ENUMERATE, APPLE_VID, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
        hotplug_callback, nullptr, &cb_handle
    );

    if (rc == LIBUSB_SUCCESS) {
        gCallbackHandles.push_back(cb_handle);
        return true;
    }
    stop_hotplug_monitoring();
    return false;
}

void stop_hotplug_monitoring(void) {
    if (!gCtx) return;
    for (auto cb_handle : gCallbackHandles) libusb_hotplug_deregister_callback(gCtx, cb_handle);
    gCallbackHandles.clear();

    gRunning = false;
    libusb_unref_device(nullptr);

    if (gEventThread.joinable()) gEventThread.join();
    libusb_exit(gCtx);
    gCtx = nullptr;
    gUserCallback = nullptr;
}

bool get_usb_device_serial(hotplug_handle_t handle, char* buffer, size_t maxLen) {
    if (!handle.serv || !buffer || maxLen == 0) return false;

    struct libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(handle.serv, &desc) != LIBUSB_SUCCESS || desc.iSerialNumber == 0) return false;

    libusb_device_handle *dev_handle = nullptr;
    if (libusb_open(handle.serv, &dev_handle) != LIBUSB_SUCCESS) return false;

    int rc = libusb_get_string_descriptor_ascii(dev_handle, desc.iSerialNumber, reinterpret_cast<unsigned char*>(buffer), static_cast<int>(maxLen));
    libusb_close(dev_handle);
    return (rc > 0);
}

static bool send_usb_control_request(hotplug_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, void* data, uint16_t wLength) {
    if (!handle.serv) return false;

    libusb_device_handle *dev_handle = handle.device;
    bool must_close = false;

    if (!dev_handle) {
        if (libusb_open(handle.serv, &dev_handle) != LIBUSB_SUCCESS) return false;
        must_close = true;
    }

    int transferred = libusb_control_transfer(
        dev_handle, bmRequestType, bRequest, wValue, wIndex,
        reinterpret_cast<unsigned char*>(data), wLength, 1000
    );

    if (must_close && dev_handle) {
        libusb_close(dev_handle);
    }

    return transferred >= 0;
}

// MARK: Liteusb

#elif !defined(__APPLE__) && defined(WITH_CIDERRAIN)

# include <liteusb.h>
# include <map>
# include <set>

struct PersistentDeviceContext {
    liteusb_device_t device;
    liteusb_handle_t* session = nullptr;
};

static std::thread gHotplugThread;
static std::atomic<bool> gHotplugRunning{false};
static std::map<std::string, std::unique_ptr<PersistentDeviceContext>> gConnectedDevices;

static void hotplug_polling_worker() {
    liteusb_init(LITEUSB_FLAG_NONE);

    while (gHotplugRunning) {
        liteusb_query_t *query = nullptr;
        if (liteusb_query_devices(&query) != LITEUSB_ERR_SUCCESS || !query) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            continue;
        }

        std::set<std::string> active_paths;

        for (uint32_t i = 0; i < query->count; i++) {
            const auto& dev = query->list[i];
            if (dev.vid != APPLE_VID || !is_target_pid(dev.pid) || !dev.sysfs_path) continue;

            std::string path(dev.sysfs_path);
            active_paths.insert(path);

            if (gConnectedDevices.find(path) == gConnectedDevices.end()) {
                auto ctx = std::unique_ptr<PersistentDeviceContext>(new PersistentDeviceContext());
                ctx->device = dev;

                if (liteusb_open_handle(&ctx->device, &ctx->session) == LITEUSB_ERR_SUCCESS) {
                    auto* ctx_ptr = ctx.get();
                    gConnectedDevices[path] = std::move(ctx);

                    if (gUserCallback) {
                        gUserCallback(get_hotplug_event(ctx_ptr->device.pid, true), { reinterpret_cast<liteusb_handle_t*>(ctx_ptr) });
                    }
                }
            }
        }

        for (auto it = gConnectedDevices.begin(); it != gConnectedDevices.end(); ) {
            if (active_paths.find(it->first) == active_paths.end()) {
                if (gUserCallback) {
                    gUserCallback(get_hotplug_event(it->second->device.pid, false), { reinterpret_cast<liteusb_handle_t*>(it->second.get()) });
                }
                if (it->second->session) liteusb_close_handle(it->second->session);
                it = gConnectedDevices.erase(it);
            } else {
                ++it;
            }
        }

        liteusb_release_query(query);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

bool start_hotplug_monitoring(hotplug_callback_t callback) {
    if (gHotplugRunning) return true;
    if (!callback) return false;

    gUserCallback = callback;
    gHotplugRunning = true;
    gHotplugThread = std::thread(hotplug_polling_worker);
    return true;
}

void stop_hotplug_monitoring(void) {
    if (!gHotplugRunning) return;

    gHotplugRunning = false;
    if (gHotplugThread.joinable()) gHotplugThread.join();

    for (const auto& [path, ctx] : gConnectedDevices) {
        if (ctx->session) liteusb_close_handle(ctx->session);
    }
    gConnectedDevices.clear();
    gUserCallback = nullptr;
}

bool get_usb_device_serial(hotplug_handle_t handle, char* buffer, size_t maxLen) {
    auto* ctx = reinterpret_cast<PersistentDeviceContext*>(handle.handle);
    if (!ctx || !ctx->session || !buffer || maxLen == 0) return false;

    char *serial_num = nullptr;
    bool success = false;

    if (liteusb_get_device_serial_num(ctx->session, &serial_num) == LITEUSB_ERR_SUCCESS && serial_num) {
        success = ((size_t)snprintf(buffer, maxLen, "%s", serial_num) < maxLen);
        free(serial_num);
    }
    return success;
}

static bool send_usb_control_request(hotplug_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, void* data, uint16_t wLength) {
    auto* ctx = reinterpret_cast<PersistentDeviceContext*>(handle.handle);
    if (!ctx || !ctx->session) return false;

    uint32_t transferred = 0;
    return liteusb_immediate_ctrl_transfer(
        ctx->session, bmRequestType, bRequest, wValue, wIndex,
        reinterpret_cast<uint8_t*>(data), wLength, 5000, &transferred
    ) == LITEUSB_ERR_SUCCESS;
}

#endif

void exit_recovery(hotplug_handle_t handle) {
    char cmd1[] = "setenv auto-boot true";
    char cmd2[] = "saveenv";
    char cmd3[] = "reboot";

    send_usb_control_request(handle, 0x40, 0, 0, 0, cmd1, sizeof(cmd1));
    send_usb_control_request(handle, 0x40, 0, 0, 0, cmd2, sizeof(cmd2));
    send_usb_control_request(handle, 0x40, 0, 0, 0, cmd3, sizeof(cmd3));
}

#endif // WITH_GUI || WITH_TUI
