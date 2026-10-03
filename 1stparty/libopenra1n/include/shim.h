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

#ifndef LIBOPENRA1N__BACKEND_SHIM_H
#define LIBOPENRA1N__BACKEND_SHIM_H

#ifdef __APPLE__
# include "usb_iokit.h"
#else
# include "usb_libusb.h"
#endif

transfer_ret_t send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len);
transfer_ret_t send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout);

#endif // LIBOPENRA1N__BACKEND_SHIM_H
