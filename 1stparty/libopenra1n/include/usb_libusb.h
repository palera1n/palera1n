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

#ifndef LIBOPENRA1N__LIBUSB_BACKEND_H
#define LIBOPENRA1N__LIBUSB_BACKEND_H

#include <stdbool.h>
#include <libusb-1.0/libusb.h>

#define USB_TIMEOUT (5)

enum usb_transfer {
    USB_TRANSFER_OK,
    USB_TRANSFER_ERROR,
    USB_TRANSFER_STALL,
};

typedef struct {
    enum usb_transfer ret;
    uint32_t sz;
} transfer_ret_t;

typedef struct {
    uint16_t vid, pid;
    struct libusb_device_handle *device;
} usb_handle_t;

void close_usb_handle(usb_handle_t *handle);
void reset_usb_handle(const usb_handle_t *handle);
bool wait_usb_handle(usb_handle_t *handle);
void usb_async_cb(struct libusb_transfer *transfer);
transfer_ret_t send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len);
transfer_ret_t send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout);
transfer_ret_t send_interface_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len);
transfer_ret_t send_interface_bulk_transfer(const usb_handle_t *handle, void *data, int len);
void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid);

#endif // LIBOPENRA1N__LIBUSB_BACKEND_H

#endif // __APPLE__
