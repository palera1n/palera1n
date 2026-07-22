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

#ifndef MAIN__HOTPLUG_HPP
#define MAIN__HOTPLUG_HPP

#include <stdbool.h>
#include <stdint.h>

#ifdef __APPLE__
# include <IOKit/IOKitLib.h>
# include <IOKit/IOCFPlugIn.h>
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

#endif // MAIN__HOTPLUG_HPP

#endif // WITH_GUI || WITH_TUI
