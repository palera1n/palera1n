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

#include "pongo_helper.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h> // PRIx64

#if WITH_CIDERRAIN
# include <ciderra1n/usb.h>
# include <ciderra1n/log.h>
# include <ciderra1n/pongo_compress.h>
#else
# include <openra1n/shim.h>
# include <openra1n/utils.h>
#endif

#include "globals.h"
#include "paleinfo.h"

#define CMD_LENGTH_MAX 512

p1_transfer_ret_t issue_pongo_command(const p1_usb_handle_t *handle, const char *command)
{
    p1_transfer_ret_t result;

    uint32_t outpos = 0;
    uint32_t outlen = 0;
    uint8_t in_progress = 1;

    char command_buf[CMD_LENGTH_MAX];
    char stdout_buf[0x2000];

    memset(stdout_buf, 0, sizeof(stdout_buf));

    if (command != NULL) {
        size_t len = strlen(command);

        if (len >= CMD_LENGTH_MAX) {
            LOG_ERROR("Pongo command too long: %s", command);
            result.ret = 1;
            return result;
        }

        LOG_DEBUG("Executing PongoOS command: '%s'", command);

        snprintf(command_buf, sizeof(command_buf), "%s\n", command);
        len = strlen(command_buf);

        #if WITH_CIDERRAIN
        result = usb_ctrl_transfer(handle, 0x21, 4, 1, 0, NULL, 0);
        if (result.ret != kUSBResponseSuccess) goto result;
        result = usb_ctrl_transfer(handle, 0x21, 3, 0, 0, (uint8_t *)command_buf, len);
        if (result.ret != kUSBResponseSuccess) goto result;
        #else
        result = send_interface_control_request(handle, 0x21, 4, 1, 0, NULL, 0);
        if (result.ret != USB_TRANSFER_OK) goto result;
        result = send_interface_control_request(handle, 0x21, 3, 0, 0, command_buf, len);
        if (result.ret != USB_TRANSFER_OK) goto result;
        #endif
    }

    while (in_progress) {
        #if WITH_CIDERRAIN
        result = usb_ctrl_transfer(handle, 0xA1, 2, 0, 0, &in_progress, sizeof(in_progress));
        if (result.ret != kUSBResponseSuccess) goto result;
        #else
        result = send_interface_control_request(handle, 0xA1, 2, 0, 0, &in_progress, sizeof(in_progress));
        if (result.ret != USB_TRANSFER_OK) goto result;
        #endif

        if (!in_progress) break;
        if (outpos > 0x1000) {
            memmove(stdout_buf, stdout_buf + outpos - 0x1000, 0x1000);
            outpos = 0x1000;
        }

        outlen = 0;
        #if WITH_CIDERRAIN
        result = usb_ctrl_transfer(handle, 0xA1, 1, 0, 0, (uint8_t *)(stdout_buf + outpos), 0x1000);
        if (result.ret != kUSBResponseSuccess) goto result;
        outlen = result.wLenDone;
        #else
        result = send_interface_control_request(handle, 0xA1, 1, 0, 0, stdout_buf + outpos, 0x1000);
        if (result.ret != USB_TRANSFER_OK) goto result;
        outlen = result.sz;
        #endif
        outpos += outlen;
    }

result:
    if (result.ret != 0) {
        // boot command, we don't care about results, lets assume success :-)
        if (command != NULL && (!strncmp("boot", command, 4))) {
            result.ret = 0;
            return result;
        }
        LOG_ERROR("PongoOS command output: %.*s", (int)outpos, stdout_buf);
    }

    return result;
}

p1_transfer_ret_t upload_buffer_to_pongo(p1_usb_handle_t *handle, const void *data, size_t length)
{
    p1_transfer_ret_t result;

    if (data == NULL || length == 0) {
        LOG_ERROR("Invalid data buffer or length");
        result.ret = -1;
        goto result;
    }

    LOG_DEBUG("Uploading %zu bytes to PongoOS...", length);

    #if WITH_CIDERRAIN
    result = usb_ctrl_transfer(handle, 0x21, 1, 0, 0, (void *)&length, 4);
    if (result.ret != kUSBResponseSuccess) {
        LOG_ERROR("Failed to initiate PongoOS upload: %d", result.ret);
        goto result;
    }
    result = usb_bulk_upload(handle, (uint8_t *)data, (uint32_t)length);
    if (result.ret != kUSBResponseSuccess) {
        LOG_ERROR("Failed to upload data to PongoOS: %d", result.ret);
        goto result;
    }
    #else
    result = send_interface_control_request(handle, 0x21, 1, 0, 0, (void *)&length, 4);
    if (result.ret != USB_TRANSFER_OK || result.sz != 4) {
        LOG_ERROR("Failed to initiate PongoOS upload: %d", result.ret);
        goto result;
    }
    result = send_interface_bulk_transfer(handle, (void *)data, (uint32_t)length);
    if (result.ret != USB_TRANSFER_OK || result.sz != length) {
        LOG_ERROR("Failed to upload data to PongoOS: %d", result.ret);
        goto result;
    }
    #endif

result:
    return result;
}

p1_checkm8_err_t send_compressed_pongo(p1_usb_handle_t *handle, const uint8_t *pongo_bin, const size_t pongo_bin_length)
{
    uint8_t *pongo_lz4 = NULL;
    size_t pongo_lz4_length = 0;

    if (!pongo_bin) {
        LOG_ERROR("pongoOS is not loaded?");
        return 15;
    }

    // pongo has been moved to sram, due to space constraints
    // we need to make a self-decompressing payload before sending
    #if WITH_CIDERRAIN
    if (lz4_compress_pongo(pongo_bin, pongo_bin_length, &pongo_lz4, &pongo_lz4_length)) {
    #else
    if (!prepare_pongo(&pongo_lz4, &pongo_lz4_length, pongo_bin, pongo_bin_length)) {
    #endif
        // how on earth
        LOG_ERROR("Failed to compress pongo image?");
        free(pongo_lz4);
        return 15;
    }

    p1_checkm8_err_t cr =
    #if WITH_CIDERRAIN
    ra1n_send_pongo(handle, pongo_lz4, pongo_lz4_length);
    #else
    checkm8_boot_pongo(handle, pongo_lz4, pongo_lz4_length);
    #endif

    free(pongo_lz4);

    return cr;
}

// TODO: support early-exit but with all embedded artifacts already loaded

p1_checkm8_err_t send_full_pongo_jailbreak(p1_usb_handle_t *handle)
{
    p1_transfer_ret_t result;

    char paleinfo[64];
    snprintf(paleinfo, sizeof(paleinfo), "palera1n_flags 0x%" PRIx64, palerain_flags);

    char xargs_cmd[0x270];
    snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", boot_args);

    // disables watchdog timer on rootful
    if (palerain_flags & palerain_option_setup_rootful) {
        strncat(xargs_cmd, " wdt=-1", sizeof(xargs_cmd) - strlen(xargs_cmd) - 1);
    }

    result = issue_pongo_command(handle, "fuse lock");
    if (result.ret != 0) goto bad;

    result = issue_pongo_command(handle, "sep auto");
    if (result.ret != 0) goto bad;

    if (g_payload_kpf.data_len > 0) {
        result = upload_buffer_to_pongo(handle, g_payload_kpf.data, g_payload_kpf.data_len);
        if (result.ret != 0) goto bad;

        // TODO: discuss on adding support for compressed artifacts
        // embedded artifacts are lzma compressed, mostly for binary size
        // this tells pongo to decompress after sending the KPF
        // overwritten artifacts should not be compressed, so we always
        // assume not compressed
        if (g_payload_kpf.uncompressed_data_len > 0) {
            char modload_cmd[64];
            snprintf(modload_cmd, sizeof(modload_cmd), "modload %zu", g_payload_kpf.uncompressed_data_len);
            result = issue_pongo_command(handle, modload_cmd);
        } else {
            result = issue_pongo_command(handle, "modload");
        }

        if (result.ret != 0) goto bad;
    }

    // palera1n specific flags
    result = issue_pongo_command(handle, paleinfo);
    if (result.ret != 0) goto bad;

    // this wont run by default if builds dont have a ramdisk (WITH_RAMDISK=0)
    if (g_payload_ramdisk.data_len > 0) {
        result = upload_buffer_to_pongo(handle, g_payload_ramdisk.data, g_payload_ramdisk.data_len);
        if (result.ret != 0) goto bad;

        // TODO: discuss on adding support for compressed artifacts
        // embedded artifacts are lzma compressed, mostly for binary size
        // this tells pongo to decompress after sending the ramdisk
        // overwritten artifacts should not be compressed, so we always
        // assume not compressed
        if (g_payload_ramdisk.uncompressed_data_len > 0) {
            char ramdisk_cmd[64];
            snprintf(ramdisk_cmd, sizeof(ramdisk_cmd), "ramdisk %zu", g_payload_ramdisk.uncompressed_data_len);
            result = issue_pongo_command(handle, ramdisk_cmd);
        } else {
            result = issue_pongo_command(handle, "ramdisk");
        }

        if (result.ret != 0) goto bad;
    }

    // this wont run by default if builds dont have a binpack (WITH_BINPACK=0)
    if (g_payload_overlay.data_len > 0) {
        result = upload_buffer_to_pongo(handle, g_payload_overlay.data, g_payload_overlay.data_len);
        if (result.ret != 0) goto bad;
        result = issue_pongo_command(handle, "overlay");
        if (result.ret != 0) goto bad;
    }

    if (strlen(boot_args) > 0) {
        result = issue_pongo_command(handle, xargs_cmd);
        if (result.ret != 0) goto bad;
    }

    if (palerain_flags & palerain_option_pongo_full)
        goto good;

    // TODO: why do we check results
    result = issue_pongo_command(handle, "bootx");
    if (result.ret != 0) goto bad;

good:
    return 0;
bad:
    return 255;
}
