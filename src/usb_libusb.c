/*
 * pongoOS - https://checkra.in
 *
 * Copyright (C) 2019-2023 checkra1n team
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
#ifdef USE_LIBUSB
#include <errno.h>
#include <fcntl.h>              // open
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>             // exit, strtoull
#include <string.h>             // strlen, strerror, memcpy, memmove
#include <unistd.h>             // close
#include <sys/mman.h>           // mmap, munmap
#include <sys/stat.h>           // fstst

#include <palerain.h>
#define ERR(...) LOG(LOG_ERROR, __VA_ARGS__)

void io_start(stuff_t *stuff);
void io_stop(stuff_t *stuff);

const char *usb_strerror(usb_ret_t err)
{
    return libusb_error_name(err);
}

usb_ret_t USBControlTransfer(usb_device_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint32_t wLength, void *data, uint32_t *wLenDone)
{
    usb_ret_t r = libusb_control_transfer(handle, bmRequestType, bRequest, wValue, wIndex, data, wLength, 0);
    if(r < 0) return r;
    if(wLenDone) *wLenDone = r;
    return USB_RET_SUCCESS;
}

usb_ret_t USBBulkUpload(usb_device_handle_t handle, void *data, int len)
{
    static uint32_t maxLen = 0;
    int transferred = 0;
    usb_ret_t r;
    if(maxLen == 0)
    {
        r = libusb_bulk_transfer(handle, 2, data, len, &transferred, 0);
        if(r == LIBUSB_SUCCESS)
        {
            return transferred == len ? USB_RET_SUCCESS : LIBUSB_ERROR_INTERRUPTED;
        }
        else if(r != LIBUSB_ERROR_NO_MEM)
        {
            return r;
        }
#if defined(__linux__)
        // We only get here on ENOMEM
        FILE *f = fopen("/sys/module/usbcore/parameters/usbfs_memory_mb", "r");
        if(f)
        {
            char str[32]; // More than enough to hold a uint64 in decimal
            size_t s = fread(str, 1, sizeof(str), f);
            fclose(f);
            if(s == 0 || s >= sizeof(str)) return r;
            str[s] = '\0';
            char *end = NULL;
            unsigned long long max = strtoull(str, &end, 0);
            // Using the limit as-is will lead to ENOMEM, so we multiply
            // by half a MB and impose an appropriate max value.
            if(*end == '\n') ++end;
            if(*end != '\0' || max == 0 || max >= 0x2000) return r;
            maxLen = (uint32_t)(max << 19);
        }
        else
#endif
        {
            // Just 1MB by default?
            maxLen = 0x100000;
        }
    }
    // If we get here, we have to chunk our data
    for(int done = transferred; done < len; )
    {
        uint32_t chunk = len - done;
        if(chunk > maxLen) chunk = maxLen;
        transferred = 0;
        r = libusb_bulk_transfer(handle, 2, (unsigned char*)data + done, chunk, &transferred, 0);
        done += transferred;
        if(r == LIBUSB_SUCCESS) continue;
        if(r != LIBUSB_ERROR_NO_MEM || maxLen <= 0x40) return r;
        maxLen /= 2;
    }
    return LIBUSB_SUCCESS;
}

static int FoundDevice(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *arg)
{
    stuff_t *stuff = arg;
    if(stuff->handle)
    {
        return LIBUSB_SUCCESS;
    }
    LOG(LOG_VERBOSE, "PongoOS USB Device connected");

    libusb_device_handle *handle;
    int r = libusb_open(dev, &handle);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_open: %s", libusb_error_name(r));
        return r;
    }

/* nah there would not be a kernel driver for pongoOS in Darwin */
#if !defined(__APPLE__)
    r = libusb_detach_kernel_driver(handle, 0);
    if(r != LIBUSB_SUCCESS && r != LIBUSB_ERROR_NOT_FOUND)
    {
        ERR("libusb_detach_kernel_driver: %s", libusb_error_name(r));
        return r;
    }
#endif

    r = libusb_set_configuration(handle, 1);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_set_configuration: %s", libusb_error_name(r));
        return r;
    }

    r = libusb_claim_interface(handle, 0);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_claim_interface: %s", libusb_error_name(r));
        return r;
    }

    stuff->dev = dev;
    stuff->handle = handle;
    io_start(stuff);

    return LIBUSB_SUCCESS;
}

static int LostDevice(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *arg)
{
    stuff_t *stuff = arg;
    if(stuff->dev != dev)
    {
        return LIBUSB_SUCCESS;
    }
    LOG(LOG_VERBOSE, "PongoOS USB Device disconnected");

    io_stop(stuff);
    libusb_close(stuff->handle);
    libusb_unref_device(dev);
    stuff->handle = NULL;
    stuff->dev = NULL;

    return LIBUSB_SUCCESS;
}

int wait_for_pongo(void)
{
    stuff_t stuff;
    stuff.handle = NULL;
    libusb_hotplug_callback_handle hp[2];
    libusb_context* ctx = NULL;

    int r = libusb_init(&ctx);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_init: %s", libusb_error_name(r));
        return -1;
    }

    if(!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
    {
        ERR("libusb: no hotplug capability");
        libusb_exit(ctx);
        return -1;
    }

    r = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, PONGO_USB_VENDOR, PONGO_USB_PRODUCT, LIBUSB_HOTPLUG_MATCH_ANY, LostDevice, &stuff, &hp[1]);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_hotplug: %s", libusb_error_name(r));
        libusb_exit(ctx);
        return -1;
    }

    while (1) {
        r = libusb_handle_events(ctx);
        if(r != LIBUSB_SUCCESS)
        {
            ERR("libusb_handle_events: %s", libusb_error_name(r));
            break;
        }

        libusb_device **list;
        ssize_t sz = libusb_get_device_list(ctx, &list);
        bool done = false;
        if(sz < 0)
        {
            ERR("libusb_get_device_list: %s", libusb_error_name((int)sz));
            libusb_exit(ctx);
            return -1;
        }

        for(ssize_t i = 0; i < sz; ++i)
        {
            struct libusb_device_descriptor desc;
            r = libusb_get_device_descriptor(list[i], &desc);
            if(r != LIBUSB_SUCCESS)
            {
                ERR("libusb_get_device_descriptor: %s", libusb_error_name(r));
                // continue anyway
            }
            if(desc.idVendor != PONGO_USB_VENDOR || desc.idProduct != PONGO_USB_PRODUCT)
            {
                libusb_unref_device(list[i]);
                continue;
            }
            r = FoundDevice(ctx, list[i], LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, &stuff);
            for (ssize_t j = i; j < sz; ++j) {
                libusb_unref_device(list[j]);
            }

            if(r != LIBUSB_SUCCESS)
            {
                libusb_free_device_list(list, 0);
                libusb_exit(ctx);
                return -1;
            } else {
                done = true;
            }
            break;
        }

        libusb_free_device_list(list, 0);

        if (done) goto end;
    }

    end:
        libusb_exit(ctx);
        return 0;
}
#endif
