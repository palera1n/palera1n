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
#include "common.h"
#define ERR(...) LOG(LOG_VERBOSE, __VA_ARGS__)

// Keep in sync with Pongo
#define PONGO_USB_VENDOR    0x05ac
#define PONGO_USB_PRODUCT   0x4141
#define CMD_LEN_MAX         512
#define UPLOADSZ_MAX        (1024 * 1024 * 128)

typedef struct stuff stuff_t;

static void io_start(stuff_t *stuff);
static void io_stop(stuff_t *stuff);


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

struct stuff
{
    pthread_t th;
    libusb_device *dev;
    usb_device_handle_t handle;
};

static int FoundDevice(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *arg)
{
    LOG(LOG_VERBOSE, "PongoOS USB Device connected\n");
    stuff_t *stuff = arg;
    if(stuff->handle)
    {
        return LIBUSB_SUCCESS;
    }

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
    LOG(LOG_VERBOSE, "PongoOS USB Device disconnected");
    stuff_t *stuff = arg;
    if(stuff->dev != dev)
    {
        return LIBUSB_SUCCESS;
    }

    io_stop(stuff);
    libusb_close(stuff->handle);
    stuff->handle = NULL;
    stuff->dev = NULL;

    return LIBUSB_SUCCESS;
}

int wait_for_pongo(void)
{
    stuff_t stuff = {};
    libusb_hotplug_callback_handle hp[2];

    int r = libusb_init(NULL);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_init: %s", libusb_error_name(r));
        return -1;
    }

    if(!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
    {
        ERR("libusb: no hotplug capability");
        libusb_exit(NULL);
        return -1;
    }

    r = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, PONGO_USB_VENDOR, PONGO_USB_PRODUCT, LIBUSB_HOTPLUG_MATCH_ANY, FoundDevice, &stuff, &hp[0]);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_hotplug: %s", libusb_error_name(r));
        libusb_exit(NULL);
        return -1;
    }

    r = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, PONGO_USB_VENDOR, PONGO_USB_PRODUCT, LIBUSB_HOTPLUG_MATCH_ANY, LostDevice, &stuff, &hp[1]);
    if(r != LIBUSB_SUCCESS)
    {
        ERR("libusb_hotplug: %s", libusb_error_name(r));
        libusb_exit(NULL);
        return -1;
    }

    libusb_device **list;
    ssize_t sz = libusb_get_device_list(NULL, &list);
    if(sz < 0)
    {
        ERR("libusb_get_device_list: %s", libusb_error_name((int)sz));
        libusb_exit(NULL);
        return -1;
    }

    for(ssize_t i = 0; i < sz; ++i)
    {
        struct libusb_device_descriptor desc = {};
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
        r = FoundDevice(NULL, list[i], LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, &stuff);
        for(ssize_t j = i + 1; j < sz; ++j)
        {
            libusb_unref_device(list[j]);
        }
        if(r != LIBUSB_SUCCESS)
        {
            libusb_free_device_list(list, 0);
            libusb_exit(NULL);
            return -1;
        }
        break;
    }
    libusb_free_device_list(list, 0);

    while(spin)
    {
        r = libusb_handle_events(NULL);
        if(r != LIBUSB_SUCCESS)
        {
            ERR("libusb_handle_events: %s", libusb_error_name(r));
            break;
        }
    }

    libusb_exit(NULL);
    return -1;
}


static void io_start(stuff_t *stuff)
{
    int r = pthread_create(&stuff->th, NULL, &pongo_usb_callback, &stuff->handle);
    if(r != 0)
    {
        ERR("pthread_create: %s", strerror(r));
        exit(-1); // TODO: ok with libusb?
    }
	pongo_usb_callback(&stuff->handle);
}

static void io_stop(stuff_t *stuff)
{
    int r = pthread_cancel(stuff->th);
    if(r != 0)
    {
        ERR("pthread_cancel: %s", strerror(r));
        exit(-1); // TODO: ok with libusb?
    }
    r = pthread_join(stuff->th, NULL);
    if(r != 0)
    {
        ERR("pthread_join: %s", strerror(r));
        exit(-1); // TODO: ok with libusb?
    }
}

static void write_stdout(char *buf, uint32_t len)
{
    while(len > 0) {
        if (verbose >= 3) {
                ssize_t s = write(1, buf, len);
            if(s < 0) {
                LOG(LOG_ERROR, "write: %s", strerror(errno));
                pthread_exit(NULL);
            }
            buf += s;
            len -= s;
        } else break;
    }
}

int issue_pongo_command(usb_device_handle_t handle, char *command)
{
    uint32_t outpos = 0;
    uint32_t outlen = 0;
	uint8_t in_progress = 1;
    if (command == NULL) goto fetch_output;
	int ret;
	size_t len = strlen(command);
	char command_buf[512];
	char stdout_buf[0x2000];
	if (len > (CMD_LEN_MAX - 2))
	{
		LOG(LOG_ERROR, "Pongo command %s too long (max %d)", command, CMD_LEN_MAX - 2);
		return EINVAL;
	}
	LOG(LOG_VERBOSE, "Executing PongoOS command: '%s'", command);
	snprintf(command_buf, 512, "%s\n", command);
	len = strlen(command_buf);
	ret = USBControlTransfer(handle, 0x21, 4, 1, 0, 0, NULL, NULL);
	if (ret)
		goto bad;
	ret = USBControlTransfer(handle, 0x21, 3, 0, 0, (uint32_t)len, command_buf, NULL);
fetch_output:
	while (in_progress) {
		ret = USBControlTransfer(handle, 0xa1, 2, 0, 0, (uint32_t)sizeof(in_progress), &in_progress, NULL);
		if (ret == USB_RET_SUCCESS)
		{
			ret = USBControlTransfer(handle, 0xa1, 1, 0, 0, 0x1000, stdout_buf + outpos, &outlen);
			if (ret == USB_RET_SUCCESS)
			{
				write_stdout(stdout_buf + outpos, outlen);
				outpos += outlen;
				if (outpos > 0x1000)
				{
					memmove(stdout_buf, stdout_buf + outpos - 0x1000, 0x1000);
					outpos = 0x1000;
				}
			}
		}
		if (ret != USB_RET_SUCCESS)
		{
			goto bad;
		}
	}
bad:
	if (ret != USB_RET_SUCCESS)
	{
		if (ret == USB_RET_NOT_RESPONDING)
			return 0;
        if (command != NULL && (!strncmp("boot", command, 4) && ret == USB_RET_IO))
            return 0;
		LOG(LOG_ERROR, "USB error: %s", usb_strerror(ret));
		return ret;
	}
	else
		return ret;
}

int upload_pongo_file(usb_device_handle_t handle, unsigned char *buf, unsigned int buf_len)
{
	int ret = 0;
	ret = USBControlTransfer(handle, 0x21, 1, 0, 0, 4, &buf_len, NULL);
	if (ret == USB_RET_SUCCESS)
	{
		ret = USBBulkUpload(handle, buf, buf_len);
		if (ret == USB_RET_SUCCESS)
		{
			LOG(LOG_VERBOSE, "Uploaded %llu bytes to PongoOS", (unsigned long long)buf_len);
		}
	}
	return ret;
}
