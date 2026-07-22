/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2023 0x7ff
 * Copyright (C) 2023 Mineek
 * Copyright (c) 2026 palera1n team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __APPLE__

#include <usb_libusb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libusb-1.0/libusb.h>

#include <utils.h>

void close_usb_handle(usb_handle_t *handle) {
    #if _WIN32
    libusb_release_interface(handle->device, 0);
    #endif
    libusb_close(handle->device);
    libusb_exit(NULL);
}

void reset_usb_handle(const usb_handle_t *handle) {
    libusb_reset_device(handle->device);
}

bool wait_usb_handle(usb_handle_t *handle) {
    if (libusb_init(NULL) != LIBUSB_SUCCESS) {
        return false;
    }

    handle->device = libusb_open_device_with_vid_pid(NULL, handle->vid, handle->pid);
    if (handle->device == NULL) {
        return false;
    }

    #if _WIN32
    libusb_set_auto_detach_kernel_driver(handle->device, 1);

    if (libusb_set_configuration(handle->device, 1) == LIBUSB_SUCCESS ||
        libusb_set_configuration(handle->device, 1) == LIBUSB_ERROR_BUSY) {
        if (libusb_claim_interface(handle->device, 0) == LIBUSB_SUCCESS) {
            return true;
        }
    }
    #else
    if (libusb_set_configuration(handle->device, 1) == LIBUSB_SUCCESS) {
        return true;
    }
    #endif

    libusb_close(handle->device);
    return false;
}

void usb_async_cb(struct libusb_transfer *transfer) {
    *(int *)transfer->user_data = 1;
}

transfer_ret_t send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len) {
    transfer_ret_t result;

    if(!handle || !handle->device) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    int ret = libusb_control_transfer(handle->device, bm_request_type, b_request, w_value, w_index, p_data, (uint16_t)w_len, USB_TIMEOUT);

    if(ret >= 0) {
        result.sz = (uint32_t)ret;
        result.ret = USB_TRANSFER_OK;
    } else if(ret == LIBUSB_ERROR_PIPE) {
        result.ret = USB_TRANSFER_STALL;
        result.sz = 0;
    } else {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
    }

    return result;
}

transfer_ret_t send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout) {
    transfer_ret_t result;

    if(!handle || !handle->device) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    struct libusb_transfer *transfer = libusb_alloc_transfer(0);
    struct timeval tv;
    int completed = 0;
    uint8_t *buf;

    if(transfer != NULL) {
        if((buf = malloc(LIBUSB_CONTROL_SETUP_SIZE + w_len)) != NULL) {
            if((bm_request_type & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
                memcpy(buf + LIBUSB_CONTROL_SETUP_SIZE, p_data, w_len);
            }
            libusb_fill_control_setup(buf, bm_request_type, b_request, w_value, w_index, (uint16_t)w_len);
            libusb_fill_control_transfer(transfer, handle->device, buf, usb_async_cb, &completed, USB_TIMEOUT);
            if(libusb_submit_transfer(transfer) == LIBUSB_SUCCESS) {
                tv.tv_sec = usb_abort_timeout / 1000;
                tv.tv_usec = (usb_abort_timeout % 1000) * 1000;
                while(completed == 0 && libusb_handle_events_timeout_completed(NULL, &tv, &completed) == LIBUSB_SUCCESS) {
                    libusb_cancel_transfer(transfer);
                }
                if(completed != 0) {
                    if((bm_request_type & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
                        memcpy(p_data, libusb_control_transfer_get_data(transfer), transfer->actual_length);
                    }
                    result.sz = (uint32_t)transfer->actual_length;
                    if(transfer->status == LIBUSB_TRANSFER_COMPLETED) {
                        result.ret = USB_TRANSFER_OK;
                    } else if(transfer->status == LIBUSB_TRANSFER_STALL) {
                        result.ret = USB_TRANSFER_STALL;
                    } else {
                        result.ret = USB_TRANSFER_ERROR;
                    }
                }
            }
            free(buf);
        }
        libusb_free_transfer(transfer);
    }

    if(completed == 0) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
    }

    return result;
}

transfer_ret_t send_interface_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len)
{
    transfer_ret_t result;

    if(!handle || !handle->device) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    int ret = libusb_control_transfer(handle->device, bm_request_type, b_request, w_value, w_index, p_data, (uint16_t)w_len, 0);

    if(ret >= 0) {
        result.sz = (uint32_t)ret;
        result.ret = USB_TRANSFER_OK;
    } else if(ret == LIBUSB_ERROR_PIPE) {
        result.ret = USB_TRANSFER_STALL;
        result.sz = 0;
    } else {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
    }

    return result;
}

transfer_ret_t send_interface_bulk_transfer(const usb_handle_t *handle, void *data, int len)
{
    transfer_ret_t result;

    if(!handle || !handle->device) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    static uint32_t maxLen = 0;
    int transferred = 0;
    int ret;

    if(maxLen == 0)
    {
        ret = libusb_bulk_transfer(handle->device, 2, data, len, &transferred, 0);
        if(ret == LIBUSB_SUCCESS)
        {
            result.ret = USB_TRANSFER_OK;
            result.sz = transferred;
            return result;
        }
        else if(ret != LIBUSB_ERROR_NO_MEM)
        {
            result.ret = USB_TRANSFER_ERROR;
            result.sz = transferred;
            return result;
        }
        #if defined(__linux__)
        // We only get here on ENOMEM
        FILE *f = fopen("/sys/module/usbcore/parameters/usbfs_memory_mb", "r");
        if(f)
        {
            char str[32]; // More than enough to hold a uint64 in decimal
            size_t s = fread(str, 1, sizeof(str), f);
            fclose(f);
            if(s == 0 || s >= sizeof(str)) {
                result.ret = USB_TRANSFER_ERROR;
                result.sz = transferred;
                return result;
            }
            str[s] = '\0';
            char *end = NULL;
            unsigned long long max = strtoull(str, &end, 0);
            // Using the limit as-is will lead to ENOMEM, so we multiply
            // by half a MB and impose an appropriate max value.
            if(*end == '\n') ++end;
            if(*end != '\0' || max == 0 || max >= 0x2000) {
                result.ret = USB_TRANSFER_ERROR;
                result.sz = transferred;
                return result;
            }
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
        ret = libusb_bulk_transfer(handle->device, 2, (unsigned char*)data + done, chunk, &transferred, 0);
        done += transferred;
        if(ret == LIBUSB_SUCCESS) continue;
        if(ret != LIBUSB_ERROR_NO_MEM || maxLen <= 0x40) {
            result.ret = USB_TRANSFER_ERROR;
            result.sz = transferred;
            return result;
        }
        maxLen /= 2;
    }

    result.ret = USB_TRANSFER_OK;
    result.sz = transferred;

    return result;
}

void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid) {
    handle->vid = vid;
    handle->pid = pid;
    handle->device = NULL;
}

#endif // __APPLE__
