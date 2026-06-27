#ifndef __APPLE__

#include "usb_libusb.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libusb-1.0/libusb.h>

#include "../../utils.h"

void close_usb_handle(usb_handle_t *handle) {
    libusb_close(handle->device);
    libusb_exit(NULL);
}

void reset_usb_handle(const usb_handle_t *handle) {
    libusb_reset_device(handle->device);
}

bool wait_usb_handle(usb_handle_t *handle) {
    if(libusb_init(NULL) == LIBUSB_SUCCESS) {
        for(;;) {
            if((handle->device = libusb_open_device_with_vid_pid(NULL, handle->vid, handle->pid)) != NULL) {
                if(libusb_set_configuration(handle->device, 1) == LIBUSB_SUCCESS) {
                    return true;
                }
                libusb_close(handle->device);
            }
            sleep_ms(USB_TIMEOUT);
        }
    }
    return false;
}

void usb_async_cb(struct libusb_transfer *transfer) {
    *(int *)transfer->user_data = 1;
}

bool send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, transfer_ret_t *transfer_ret) {
    int ret = libusb_control_transfer(handle->device, bm_request_type, b_request, w_value, w_index, p_data, (uint16_t)w_len, USB_TIMEOUT);

    if(transfer_ret != NULL) {
        if(ret >= 0) {
            transfer_ret->sz = (uint32_t)ret;
            transfer_ret->ret = USB_TRANSFER_OK;
        } else if(ret == LIBUSB_ERROR_PIPE) {
            transfer_ret->ret = USB_TRANSFER_STALL;
        } else {
            transfer_ret->ret = USB_TRANSFER_ERROR;
        }
    }
    return true;
}

bool send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret) {
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
                    if(transfer_ret != NULL) {
                        transfer_ret->sz = (uint32_t)transfer->actual_length;
                        if(transfer->status == LIBUSB_TRANSFER_COMPLETED) {
                            transfer_ret->ret = USB_TRANSFER_OK;
                        } else if(transfer->status == LIBUSB_TRANSFER_STALL) {
                            transfer_ret->ret = USB_TRANSFER_STALL;
                        } else {
                            transfer_ret->ret = USB_TRANSFER_ERROR;
                        }
                    }
                }
            }
            free(buf);
        }
        libusb_free_transfer(transfer);
    }
    return completed != 0;
}

bool send_interface_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, transfer_ret_t *transfer_ret) {
    int ret = libusb_control_transfer(handle->device, bm_request_type, b_request, w_value, w_index, p_data, (uint16_t)w_len, 0);

    if(transfer_ret != NULL) {
        if(ret >= 0) {
            transfer_ret->sz = (uint32_t)ret;
            transfer_ret->ret = USB_TRANSFER_OK;
        } else if(ret == LIBUSB_ERROR_PIPE) {
            transfer_ret->ret = USB_TRANSFER_STALL;
        } else {
            transfer_ret->ret = USB_TRANSFER_ERROR;
        }
    }
    return true;
}

bool send_interface_bulk_transfer(const usb_handle_t *handle, void *data, int len)
{
    static uint32_t maxLen = 0;
    int transferred = 0;
    int32_t r;
    if(maxLen == 0)
    {
        r = libusb_bulk_transfer(handle->device, 2, data, len, &transferred, 0);
        if(r == LIBUSB_SUCCESS)
        {
            return transferred == len ? LIBUSB_SUCCESS : LIBUSB_ERROR_INTERRUPTED;
        }
        else if(r != LIBUSB_ERROR_NO_MEM)
        {
            return r;
        }
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
        r = libusb_bulk_transfer(handle->device, 2, (unsigned char*)data + done, chunk, &transferred, 0);
        done += transferred;
        if(r == LIBUSB_SUCCESS) continue;
        if(r != LIBUSB_ERROR_NO_MEM || maxLen <= 0x40) return r;
        maxLen /= 2;
    }
    return LIBUSB_SUCCESS;
}

void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid) {
    handle->vid = vid;
    handle->pid = pid;
    handle->device = NULL;
}

#endif // __APPLE__
