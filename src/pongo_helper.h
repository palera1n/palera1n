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

#ifndef USB_PONGO_H
#define USB_PONGO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if WITH_CIDERRAIN
# include <ciderra1n/usb.h>
# include <ciderra1n/ra1n.h>
typedef client_t p1_usb_handle_t;
typedef transfer_t p1_transfer_ret_t;
typedef ra1n_client_err_t p1_checkm8_err_t;
#else
# include <openra1n/shim.h>
# include <openra1n/checkm8.h>
typedef usb_handle_t p1_usb_handle_t;
typedef transfer_ret_t p1_transfer_ret_t;
typedef checkm8_err_t p1_checkm8_err_t;
#endif

p1_checkm8_err_t send_compressed_pongo(p1_usb_handle_t *handle, const uint8_t *pongo_bin, const size_t pongo_bin_length);
p1_checkm8_err_t send_full_pongo_jailbreak(p1_usb_handle_t *handle);

#endif // USB_PONGO_H
