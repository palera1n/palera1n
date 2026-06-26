#include "pongo_helper.h"

#include <stdlib.h>
#include <string.h>

#include <lz4.h>
#include <lz4hc.h>

#include "../usb/shim.h"

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
    LOG("Booting pongoOS");
    LOG("Compressing pongoOS");
    LOG("Appending shellcode to the top of pongoOS (512 bytes)");
    void *shellcode = malloc(512);
    memcpy(shellcode, payloads_lz4dec_bin, payloads_lz4dec_bin_len);
    size_t out_len = payloads_Pongo_bin_len;
    void *out = malloc(out_len);
    compress_pongo(out, &out_len);
    LOG("Compressed pongoOS from %u to %zu bytes", payloads_Pongo_bin_len, out_len);
    void *tmp = malloc(out_len + 512);
    memcpy(tmp, shellcode, 512);
    memcpy(tmp + 512, out, out_len);
    free(out);
    out = tmp;
    out_len += 512;
    free(shellcode);
    LOG("Setting the compressed size into the shellcode");
    uint32_t* size = (uint32_t*)(out + 0x1fc);
    *size = out_len - 512;
    LOG("Reconnecting to device");
    init_usb_handle(handle, 0x5AC, 0x1227);
    LOG("Waiting for device to be ready");
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
                LOG("retrying at len = %zu", len);
                sleep_ms(100);
                goto retry;
            }
            len += size;
            LOG("len = %zu", len);
        }
    }
    send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0, NULL);
    LOG("pongoOS sent, should be booting");
}

int issue_pongo_command(usb_handle_t *handle, char *command, char *outBuffer) {
    bool ret;
    uint8_t inProgress = 1;
    uint32_t outPosition = 0;
    uint32_t outLength = 0;
    transfer_ret_t transferRet;
    char stdoutBuffer[0x2000];
    if (command == NULL) goto fetch_output;
    size_t length = strlen(command);
    char commandBuffer[0x200];
    if (length > (CMD_LENGTH_MAX - 2))
    {
        LOG("Pongo command %s too long (max %d)", command, CMD_LENGTH_MAX - 2);
        return -1;
    }
    LOG("Executing PongoOS command: '%s'", command);
    snprintf(commandBuffer, 512, "%s\n", command);
    length = strlen(commandBuffer);
    ret = send_usb_control_request_no_data(handle, 0x21, 4, 1, 0, 0, NULL);
    if (!ret)
        goto bad;
    ret = send_usb_control_request(handle, 0x21, 3, 0, 0, commandBuffer, (uint32_t)length, NULL);
fetch_output:
    while (inProgress) {
        ret = send_usb_control_request(handle, 0xA1, 2, 0, 0, &inProgress, (uint32_t)sizeof(inProgress), NULL);
        if (ret) {
            ret = send_usb_control_request(handle, 0xA1, 1, 0, 0, stdoutBuffer + outPosition, 0x1000, &transferRet);
            outLength = transferRet.sz;
            if (transferRet.ret == USB_TRANSFER_OK) {
                outPosition += outLength;
                if (outPosition > 0x1000) {
                    memmove(stdoutBuffer, stdoutBuffer + outPosition - 0x1000, 0x1000);
                    outPosition = 0x1000;
                }
            }
        }
        if (transferRet.ret != USB_TRANSFER_OK) {
            goto bad;
        }
    }
bad:
    if (transferRet.ret != USB_TRANSFER_OK)
    {
        if (!strncmp("boot", command, 4)) {
            return 0;
        } else if (command != NULL) {
            LOG("USB transfer error: 0x%x, wLength out 0x%x.", transferRet.ret, transferRet.sz);
            return transferRet.ret;
        } else {
            return -1;
        }
    }
    else {
        if (outBuffer) {
            memcpy(outBuffer, stdoutBuffer, outPosition);
        }
        return ret;
    }
}
