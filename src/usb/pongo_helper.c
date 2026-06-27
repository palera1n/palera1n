#include "pongo_helper.h"

#include <stdlib.h>
#include <string.h>

#include <lz4.h>
#include <lz4hc.h>

#include "shim.h"

#include "../utils.h"
#include "../m8/dfu.h"
#include "../gen/payloads/Pongo.h"
#include "../gen/payloads/lz4dec.h"

void compress_pongo(void *out, size_t *out_len) {
    size_t len = payloads_Pongo_bin_len;
    size_t out_len_ = *out_len;
    *out_len = LZ4_compress_HC(payloads_Pongo_bin, out, len, out_len_, LZ4HC_CLEVEL_MAX);
}

void checkm8_boot_pongo(usb_handle_t *handle) {
    transfer_ret_t transfer_ret;
    LOG_VERBOSE("Booting pongoOS");
    LOG_VERBOSE("Compressing pongoOS");
    LOG_VERBOSE("Appending shellcode to the top of pongoOS (512 bytes)");
    void *shellcode = malloc(512);
    memcpy(shellcode, payloads_lz4dec_bin, payloads_lz4dec_bin_len);
    size_t out_len = payloads_Pongo_bin_len;
    void *out = malloc(out_len);
    compress_pongo(out, &out_len);
    LOG_VERBOSE("Compressed pongoOS from %u to %zu bytes", payloads_Pongo_bin_len, out_len);
    void *tmp = malloc(out_len + 512);
    memcpy(tmp, shellcode, 512);
    memcpy(tmp + 512, out, out_len);
    free(out);
    out = tmp;
    out_len += 512;
    free(shellcode);
    LOG_VERBOSE("Setting the compressed size into the shellcode");
    uint32_t* size = (uint32_t*)(out + 0x1fc);
    *size = out_len - 512;
    LOG_VERBOSE("Reconnecting to device");
    init_usb_handle(handle, 0x5AC, 0x1227);
    LOG_VERBOSE("Waiting for device to be ready");
    wait_usb_handle(handle);
    {
        size_t len = 0;
        size_t size;
        while(len < out_len)
        {
        retry:
            size = ((out_len - len) > 0x800) ? 0x800 : (out_len - len);
            send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, (unsigned char*)&out[len], size, &transfer_ret);
            if(transfer_ret.sz != size || transfer_ret.ret != USB_TRANSFER_OK)
            {
                LOG_VERBOSE("retrying at len = %zu", len);
                sleep_ms(100);
                goto retry;
            }
            len += size;
            LOG_VERBOSE("len = %zu", len);
        }
    }
    send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0, NULL);
    LOG_VERBOSE("pongoOS sent, should be booting");
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
        LOG_ERROR("Invalid data buffer or length.");
        return false;
    }

    LOG_VERBOSE("Uploading %zu bytes to PongoOS...", length);

    bool ret = false;
    transfer_ret_t tx_status = {0};

    ret = send_interface_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, (void *)&length, 4, &tx_status);
    ret = send_interface_bulk_transfer(handle, (void *)data, (uint32_t)length);

    return ret;
}
