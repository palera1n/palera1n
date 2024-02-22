/*
 * pongoOS - https://checkra.in
 *
 * Copyright (C) 2019-2022 checkra1n team
 *
 * This file is part of pongoOS.
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
#ifndef USE_LIBUSB
#include <errno.h>
#include <fcntl.h>              // open
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>             // exit, strtoull
#include <string.h>             // strlen, strerror, memcpy, memmove
#include <unistd.h>             // close
#include <wordexp.h>
#include <sys/mman.h>           // mmap, munmap
#include <sys/stat.h>           // fstst
#include <mach/mach.h>

#include <palerain.h>
#include <CoreFoundation/CoreFoundation.h>

#define ERR(...) LOG(LOG_VERBOSE, __VA_ARGS__)

const char *usb_strerror(usb_ret_t err)
{
    return mach_error_string(err);
}

usb_ret_t USBControlTransfer(usb_device_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint32_t wLength, void *data, uint32_t *wLenDone)
{
    IOUSBDevRequest request =
    {
        .bmRequestType = bmRequestType,
        .bRequest = bRequest,
        .wValue = wValue,
        .wIndex = wIndex,
        .wLength = wLength,
        .pData = data,
    };
    usb_ret_t ret = (*handle)->ControlRequest(handle, 0, &request);
    if(wLenDone) *wLenDone = request.wLenDone;
    return ret;
}

usb_ret_t USBBulkUpload(usb_device_handle_t handle, void *data, uint32_t len)
{
    return (*handle)->WritePipe(handle, 2, data, len);
}

static void FoundDevice(void *refCon, io_iterator_t it)
{
    stuff_t *stuff = refCon;
    if(stuff->regID)
    {
        return;
    }
    io_service_t usbDev = MACH_PORT_NULL;
    while((usbDev = IOIteratorNext(it)))
    {
        uint64_t regID;
        kern_return_t ret = IORegistryEntryGetRegistryEntryID(usbDev, &regID);
        if(ret != KERN_SUCCESS)
        {
            ERR("IORegistryEntryGetRegistryEntryID: %s", mach_error_string(ret));
            goto next;
        }
        SInt32 score = 0;
        IOCFPlugInInterface **plugin = NULL;
        ret = IOCreatePlugInInterfaceForService(usbDev, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
        if(ret != KERN_SUCCESS)
        {
            ERR("IOCreatePlugInInterfaceForService(usbDev): %s", mach_error_string(ret));
            goto next;
        }
        HRESULT result = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*)&stuff->dev);
        (*plugin)->Release(plugin);
        if(result != 0)
        {
            ERR("QueryInterface(dev): 0x%x", result);
            goto next;
        }
        ret = (*stuff->dev)->USBDeviceOpenSeize(stuff->dev);
        if(ret != KERN_SUCCESS)
        {
            ERR("USBDeviceOpenSeize: %s", mach_error_string(ret));
        }
        else
        {
            ret = (*stuff->dev)->SetConfiguration(stuff->dev, 1);
            if(ret != KERN_SUCCESS)
            {
                ERR("SetConfiguration: %s", mach_error_string(ret));
            }
            else
            {
                IOUSBFindInterfaceRequest request =
                {
                    .bInterfaceClass = kIOUSBFindInterfaceDontCare,
                    .bInterfaceSubClass = kIOUSBFindInterfaceDontCare,
                    .bInterfaceProtocol = kIOUSBFindInterfaceDontCare,
                    .bAlternateSetting = kIOUSBFindInterfaceDontCare,
                };
                io_iterator_t iter = MACH_PORT_NULL;
                ret = (*stuff->dev)->CreateInterfaceIterator(stuff->dev, &request, &iter);
                if(ret != KERN_SUCCESS)
                {
                    ERR("CreateInterfaceIterator: %s", mach_error_string(ret));
                }
                else
                {
                    io_service_t usbIntf = MACH_PORT_NULL;
                    while((usbIntf = IOIteratorNext(iter)))
                    {
                        ret = IOCreatePlugInInterfaceForService(usbIntf, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
                        IOObjectRelease(usbIntf);
                        if(ret != KERN_SUCCESS)
                        {
                            ERR("IOCreatePlugInInterfaceForService(usbIntf): %s", mach_error_string(ret));
                            continue;
                        }
                        result = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID*)&stuff->handle);
                        (*plugin)->Release(plugin);
                        if(result != 0)
                        {
                            ERR("QueryInterface(intf): 0x%x", result);
                            continue;
                        }
                        ret = (*stuff->handle)->USBInterfaceOpen(stuff->handle);
                        if(ret != KERN_SUCCESS)
                        {
                            ERR("USBInterfaceOpen: %s", mach_error_string(ret));
                        }
                        else
                        {
                            io_start(stuff);
                            stuff->regID = regID;
                            while((usbIntf = IOIteratorNext(iter))) IOObjectRelease(usbIntf);
                            IOObjectRelease(iter);
                            while((usbDev = IOIteratorNext(it))) IOObjectRelease(usbDev);
                            IOObjectRelease(usbDev);
                            return;
                        }
                        (*stuff->handle)->Release(stuff->handle);
                        stuff->handle = NULL;
                    }
                    IOObjectRelease(iter);
                }
            }
        }

    next:;
        if(stuff->dev)
        {
            (*stuff->dev)->USBDeviceClose(stuff->dev);
            (*stuff->dev)->Release(stuff->dev);
            stuff->dev = NULL;
        }
        IOObjectRelease(usbDev);
    }
}

static void LostDevice(void *refCon, io_iterator_t it)
{
    stuff_t *stuff = refCon;
    io_service_t usbDev = MACH_PORT_NULL;
    while((usbDev = IOIteratorNext(it)))
    {
        uint64_t regID;
        kern_return_t ret = IORegistryEntryGetRegistryEntryID(usbDev, &regID);
        IOObjectRelease(usbDev);
        if(ret == KERN_SUCCESS && stuff->regID == regID)
        {
            io_stop(stuff);
            stuff->regID = 0;
            (*stuff->handle)->USBInterfaceClose(stuff->handle);
            (*stuff->handle)->Release(stuff->handle);
            (*stuff->dev)->USBDeviceClose(stuff->dev);
            (*stuff->dev)->Release(stuff->dev);
        }
    }
}

static const int pongo_usb_vendor = PONGO_USB_VENDOR;
static const int pongo_usb_product = PONGO_USB_PRODUCT;

int wait_for_pongo(void) {
    kern_return_t ret;
    stuff_t stuff;
    io_iterator_t found, lost;

    void* cfdict_keys[3];
    void* cfdict_values[3];

    cfdict_keys[0] = (void*)CFSTR("IOProviderClass");
    cfdict_keys[1] = (void*)CFSTR("idVendor");
    cfdict_keys[2] = (void*)CFSTR("idProduct");

    cfdict_values[0] = (void*)CFSTR("IOUSBDevice");
    cfdict_values[1] = (void*)CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pongo_usb_vendor);
    cfdict_values[2] = (void*)CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pongo_usb_product);

    CFDictionaryRef cfdict = CFDictionaryCreate(kCFAllocatorDefault, (const void**)cfdict_keys, (const void**)cfdict_values, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    IONotificationPortRef notifyPort = IONotificationPortCreate(kIOMasterPortDefault);

    CFRetain(cfdict);
    // CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(notifyPort), kCFRunLoopDefaultMode);

    ret = IOServiceAddMatchingNotification(notifyPort, kIOFirstMatchNotification, cfdict, &FoundDevice, &stuff, &found);
    if(ret != KERN_SUCCESS)
    {
        ERR("IOServiceAddMatchingNotification: %s", mach_error_string(ret));
        return -1;
    }
    FoundDevice(&stuff, found);

    CFRetain(cfdict);
    ret = IOServiceAddMatchingNotification(notifyPort, kIOTerminatedNotification, cfdict, &LostDevice, &stuff, &lost);
    if(ret != KERN_SUCCESS)
    {
        ERR("IOServiceAddMatchingNotification: %s", mach_error_string(ret));
        return -1;
    }
    LostDevice(&stuff, lost);
    // CFRunLoopRun();
    while (get_spin()) {
        sleep(1);
    }
    CFRelease(cfdict);
    return 0;
}
#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif
