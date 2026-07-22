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

#include <shim.h>

#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
# include <usb_iokit.h>
#else
# include <usb_libusb.h>
#endif

transfer_ret_t send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len) {
    transfer_ret_t result;
    void *p_data;

    if(w_len == 0) {
        return send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, NULL, 0);
    } else if((p_data = malloc(w_len)) != NULL) {
        memset(p_data, '\0', w_len);
        result = send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len);
        free(p_data);
    }

    return result;
}

transfer_ret_t send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout) {
    transfer_ret_t result;
    void *p_data;

    if(w_len == 0) {
        return send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, NULL, 0, usb_abort_timeout);
    } else if((p_data = malloc(w_len)) != NULL) {
        memset(p_data, '\0', w_len);
        result = send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len, usb_abort_timeout);
        free(p_data);
    }

    return result;
}
