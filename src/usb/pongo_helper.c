#include "pongo_helper.h"

#include <stdlib.h>
#include <string.h>

#include <lz4.h>
#include <lz4hc.h>

#include "shim.h"

#include "../utils.h"
#include "../globals.h"
#include "../m8/dfu.h"

#include "../gen/payloads/lz4dec.h"

static bool compress_pongo(uint8_t **out, size_t *out_len)
{
    const uint8_t *src = g_payload_pongo.data;
    int src_size = (int)g_payload_pongo.data_len;

    int max_size = LZ4_compressBound(src_size);

    uint8_t *buf = malloc(max_size);
    if (!buf) return false;

    int size = LZ4_compress_HC(
        (const char *)src,
        (char *)buf,
        src_size,
        max_size,
        LZ4HC_CLEVEL_MAX
    );

    if (size <= 0) {
        free(buf);
        return false;
    }

    *out = buf;
    *out_len = (size_t)size;

    return true;
}

#define SHELLCODE_SZ (512)

bool prepare_pongo(uint8_t **out, size_t *out_len)
{
    size_t pongoSize;
    uint8_t *pongo;

    if (!compress_pongo(&pongo, &pongoSize))
        return false;

    uint8_t *payload = malloc(SHELLCODE_SZ + pongoSize);
    if (!payload) {
        free(pongo);
        return false;
    }

    memcpy(payload, payloads_lz4dec_bin, SHELLCODE_SZ);
    memcpy(payload + SHELLCODE_SZ, pongo, pongoSize);

    free(pongo);

    pongo = payload;
    pongoSize += SHELLCODE_SZ;

    uint32_t *pongoSizeInData = (uint32_t *)(pongo + 0x1fc);
    *pongoSizeInData = (uint32_t)pongoSize - SHELLCODE_SZ;

    *out = pongo;
    *out_len = pongoSize;

    return true;
}

int issue_pongo_command(const usb_handle_t *handle, const char *command) {
    uint32_t outpos = 0;
    uint32_t outlen = 0;
    transfer_ret_t tx_status;
    uint8_t in_progress = 1;

    char command_buf[512];
    char stdout_buf[0x2000];

    memset(stdout_buf, 0, sizeof(stdout_buf));

    if (command != NULL) {
        size_t len = strlen(command);

        if (len > 510) {
            LOG_ERROR("Pongo command too long: %s", command);
            return -1;
        }

        LOG_VERBOSE("Executing PongoOS command: '%s'", command);

        snprintf(command_buf, sizeof(command_buf), "%s\n", command);
        len = strlen(command_buf);

        if (!send_interface_control_request(handle, 0x21, 4, 1, 0, NULL, 0, &tx_status)) return -1;
        if (!send_interface_control_request(handle, 0x21, 3, 0, 0, command_buf, len, &tx_status)) return -1;
    }
    fetch_output:
    while (in_progress) {
        if (!send_interface_control_request(handle, 0xA1, 2, 0, 0, &in_progress, sizeof(in_progress), &tx_status)) goto bad;

        if (in_progress == 0) break;

        if (outpos + 0x1000 >= sizeof(stdout_buf)) {
            memmove(stdout_buf, stdout_buf + 0x1000, sizeof(stdout_buf) - 0x1000);
            outpos -= 0x1000;
        }

        if (!send_interface_control_request(handle, 0xA1, 1, 0, 0, stdout_buf + outpos, 0x1000, &tx_status)) goto bad;

        outlen = 0x1000;
        outpos += outlen;
    }
    bad:
    if (tx_status.ret != USB_TRANSFER_OK) {
        if (command != NULL && strncmp(command,"boot",4) == 0) {
            return 0;
        }

        LOG_ERROR("Pongo USB error: %d", tx_status.ret);
        return -1;
    }

    return 0;
}

bool upload_buffer_to_pongo(usb_handle_t *handle, const void *data, size_t length) {
    if (data == NULL || length == 0) {
        LOG_ERROR("Invalid data buffer or length");
        return false;
    }

    LOG_VERBOSE("Uploading %zu bytes to PongoOS...", length);

    bool ret = false;
    transfer_ret_t tx_status = {0};

    ret = send_interface_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, (void *)&length, 4, &tx_status);
    ret = send_interface_bulk_transfer(handle, (void *)data, (uint32_t)length);

    return ret;
}
